#
# host_cmd.py - inject HostPC commands into the emulated flight computer.
#
# HOW THE FIRMWARE CONSUMES A COMMAND
#
# rtos/MRT/MRT_freertos.cpp, inside the telemetry task loop:
#
#     if (host.available()) {                          // line 310
#         master_parseCommand((char*)host.rxBuffer, &subcom);
#         memset(host.rxBuffer, 0, 2);
#         host.clearRxBuffer();                        // line 320  <-- wedges
#         host.receive(host.rxBuffer, 2);              // line 323  -> BUSY_RX
#     }
#
# host.available() (HBL_uart.c:81) is nothing but
# __HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE).
#
# Two HAL behaviours decide everything here:
#
#   1. UART_ReceiveToIdle_IT ends with __HAL_UART_DISABLE_IT(huart, UART_IT_RXNE)
#      once RxXferCount reaches 0. So after the armed 2-byte reception is
#      complete, RXNE interrupts are OFF but the UART handle is READY again.
#
#   2. MRT_uart_clear_rx_buffer (HBL_uart.c:93) is
#          do { hal_status = HAL_UART_Receive(h, &tmp, 1, 10); }
#          while (hal_status != HAL_TIMEOUT);
#      HAL_UART_Receive returns HAL_BUSY - never HAL_TIMEOUT - while the handle
#      is BUSY_RX. So if the poll runs while host.receive() is still armed, this
#      loop never terminates: the telemetry task wedges, telemetryCheckIn stops
#      being refreshed at MRT_freertos.cpp:198/346, MRT_ensureThreadsRunning
#      counts it to zero and the firmware resets itself with
#      "Cause of reset: TELEMETRY".
#
# THE FIX, WITHOUT TOUCHING THE FIRMWARE
#
# Send the command bytes, then one extra byte.
#
#     byte 1 -> ISR copies it to rxBuffer[0], RxXferCount 2 -> 1
#     byte 2 -> ISR copies it to rxBuffer[1], RxXferCount -> 0, reception ends,
#               RxState = READY and RXNEIE is now disabled
#     byte 3 -> sets RXNE, but with RXNEIE disabled no ISR runs, so RXNE stays
#               set until somebody polls it
#
# The poll therefore sees available() == true, parses the buffer the ISR filled
# in, and clearRxBuffer() finds the handle READY - so its HAL_UART_Receive
# drains the third byte and exits on its 10 ms timeout instead of spinning.
#
# This is the natural hardware behaviour of this HAL version, not a hack: no
# memory pokes, no NVIC manipulation, no register pokes.
#
# USAGE
#
#     include @C:/Users/aneur/code/mrt/renode/host_cmd.py
#     python "hostcmd_str('ea')"     # arm recovery
#     python "hostcmd_str('nn')"     # no operation
#
# CADENCE
#
# Injection is fire-and-forget: RXNE stays latched until the telemetry task
# actually polls it, so a command is never lost, only deferred.
#
# Measured latency from WriteChar to the firmware consuming the command is
# 34 / 16 / 24 ms of emulated time, i.e. 0.1-0.2 s of wall clock. Nothing blocks
# the telemetry thread. sleep 1 between commands is ample.
#
# The board runs Renode at roughly 30% of real time, and most of that CPU is now
# HAL_Delay plus the HAL_GetTick spin inside it - i.e. the firmware genuinely
# waiting, not spinning on hardware. Rewriting the I2C CCR from the monitor does
# not help: Renode's I2C model uses a fixed bit tick rather than deriving it from
# CCR, so the bus speed is not tunable from the platform side.
#

import os
import tempfile
import time

HOST_OBJ      = 0x2000140C
HOST_RXBUFFER = HOST_OBJ + 0x14      # rxBuffer[100]
HOST_RXFLAG   = HOST_OBJ + 0x78      # rxFlag

USART4_NODE = "sysbus.uart4"

# Emulated-time gap between byte writes, in seconds. The ISR has to run between
# them or the UART raises ORE instead of completing the reception. 20 ms is far
# more than needed and still fast enough to feel interactive.
GAP = 0.02

DUMMY = 0x00


def _rxbuf_word(sb):
    """The two command bytes, as the ISR left them in rxBuffer."""
    return int(sb.ReadDoubleWord(HOST_RXBUFFER)) & 0xFFFF


def hostcmd(b1, b2):
    """Deliver a 2-byte HostPC command the way a host PC would.

    Returns True once the command bytes are confirmed sitting in rxBuffer.
    """
    sb = self.Machine.SystemBus
    uart = self.Machine[USART4_NODE]
    want = (b1 & 0xFF) | ((b2 & 0xFF) << 8)

    # Both bytes back-to-back with no gap. They must land as ONE 2-byte
    # reception: host.receive() arms HAL_UARTEx_ReceiveToIdle_IT for 2 bytes, and
    # that variant completes early on an IDLE line. Any idle gap on USART4 - even
    # 20 ms - makes the reception finish after byte 1 alone, leaving rxBuffer[1]
    # stale and the handle READY. Symptom: rxBuffer = 0x006E rather than 0x6E6E,
    # rxFlag already set, and the command dispatched as "n\0".
    uart.WriteChar(b1 & 0xFF)
    uart.WriteChar(b2 & 0xFF)

    # Let the ISR copy both bytes and close the reception. Only then is RXNEIE
    # disabled and the handle back in READY, which is what makes the next step
    # safe - and what lets clearRxBuffer() terminate instead of spinning on HAL_BUSY.
    time.sleep(GAP)

    if _rxbuf_word(sb) != want:
        return False

    # Byte 3 latches RXNE. Nothing clears it for us, which is the whole point.
    uart.WriteChar(DUMMY)
    return True


def hostcmd_str(s):
    """Convenience wrapper: hostcmd_str('ea').

    Uses ord() rather than s.encode('ascii') because IronPython returns a str
    (not bytes) from .encode('ascii'), which would then be ANDed with an int.
    """
    if len(s) != 2:
        raise ValueError("HostPC commands are exactly 2 bytes, got %r" % s)
    return hostcmd(ord(s[0]), ord(s[1]))


# ---------------------------------------------------------------------------
# RESD sensor playback
# ---------------------------------------------------------------------------
#
# FEEDING A RESD STREAM, AND WHY IT SILENTLY DOES NOTHING WITHOUT startTime
#
# ARBITRARY_TIMESTAMP blocks (what csv2resd emits by default) carry an absolute
# nanosecond timestamp per sample. Your log's first sample is at t=0 and the last
# is tens of seconds in, so if you feed the file to a machine that has already
# been running for a while, every sample is in the past and Renode throws them all
# away:
#
#     [WARNING] RESD: Skipping sample: [-0.11956, -0.040992, 1.004304] g @ 0
#     [WARNING] RESD: Skipping sample: [-0.11956, -0.040992, 1.004304] g @ 22000000
#
# The fix is the third positional argument, startTime, in nanoseconds: it tells
# the feeder when playback begins. Tested against data/data.resd:
#
#     ... @data/data.resd 0 0 CurrentVirtualTime      -> still skipped
#     ... @data/data.resd 0 0 Specified 30000000000   -> still skipped
#     ... @data/data.resd 0 30000000000              -> accepted
#
# So only argument 3 works; the sampleOffsetType/sampleOffsetTime pair does not.
#
# startTime is absolute, not relative, which makes it go stale: a value that was
# "comfortably in the future" becomes "in the past" once the machine runs past it,
# and the samples start being skipped again. feed() below computes it from the
# clock instead.
#
# EMISSION RATE
#
# Playback is paced by the model, not by the file's timestamps:
#
#   LSM6DSR  DataRateToFrequency(ODR)  - the rate the firmware configured, so it
#                                       plays at a sensible speed. Note the ODR
#                                       resets to Disabled (frequency 0), so
#                                       feeding before the board has initialised
#                                       fails with "Frequency must be higher than
#                                       zero".
#   MS5611   OutputFrequency, hardcoded to 1 Hz in peripherals/MS5611.cs. Baro
#                                       and temperature therefore advance one
#                                       sample per second of emulated time, so a
#                                       1925-sample log takes ~32 minutes.
#
# Feed only after the board has finished MRT_Init.

UWTICK = 0x20000C58      # HAL's millisecond counter, incremented by SysTick

# (node, method) for every sensor stream data.resd can drive.
RESD_STREAMS = [
    ("sysbus.i2c2.lsm6dsr", "FeedAccelerationSamplesFromRESD"),
    ("sysbus.i2c2.lsm6dsr", "FeedAngularRateSamplesFromRESD"),
    ("sysbus.i2c2.ms5611", "FeedPressureSamplesFromRESD"),
    ("sysbus.i2c2.ms5611", "FeedTemperatureSamplesFromRESD"),
]

# Directory searched when feed()/feedall() is given a bare filename.
# Override without editing this file:  set MRT_DATA_DIR
DATA_DIR = os.environ.get("MRT_DATA_DIR", "data")


def _prefixes():
    """The directories the Monitor itself resolves '@' paths against, in order.

    This is the same list that makes '@data/data.resd' work in a .resc. On this
    machine it is ['C:\\Users\\aneur\\code\\mrt\\renode',
    'C:\\Program Files\\Renode\\'], with the project directory first.

    Do NOT use os.getcwd() here: the Monitor's working directory is the Renode
    install directory, so a bare filename would resolve to
    C:\\Program Files\\Renode\\data\\data.resd and fail.
    """
    try:
        return [str(p) for p in self.CurrentPathPrefixes]
    except Exception:
        return [os.getcwd()]


def resd(name, data_dir=None):
    """Resolve a RESD filename to an absolute path, using the Monitor's own rules.

      'data/data.resd'  resolved against each '@' prefix, first match wins
      'data.resd'       inside DATA_DIR (default 'data', override with MRT_DATA_DIR)
      'C:/abs/x.resd'   used as-is
      '/abs/x.resd'     used as-is (leading '/' accepted on Windows too)
    """
    if name.startswith("/") or (os.altsep and name.startswith(os.altsep)):
        return name

    # A bare filename means "look in the data directory".
    if os.sep not in name and (os.altsep is None or os.altsep not in name):
        name = os.path.join(data_dir or DATA_DIR, name)

    # Absolute Windows path such as C:\foo or C:/foo.
    if len(name) > 2 and name[1] == ":":
        return name

    for prefix in _prefixes():
        candidate = os.path.join(prefix, name)
        if os.path.exists(candidate):
            return os.path.abspath(candidate)

    # Nothing matched: return the first-prefix answer so the error names the path
    # that was tried first, rather than something under the Renode install dir.
    return os.path.abspath(os.path.join(_prefixes()[0], name))


def now_ns():
    """Current emulated time in nanoseconds.

    Read from the firmware's uwTick rather than Renode's ElapsedVirtualTime: a
    .NET TimeSpan's Ticks property counts 100 ns units, not nanoseconds, so using
    it directly would put startTime 10x too small and the samples would be
    discarded again.
    """
    return int(self.Machine.SystemBus.ReadDoubleWord(UWTICK)) * 1000000


def feed(node, method, path, margin_ns=500000000, channel=0, offset=True):
    """Feed one RESD stream, starting playback margin_ns from now.

    'path' may be absolute, relative to the Monitor's '@' prefixes, or a bare
    filename inside DATA_DIR - see resd().

    offset=True passes RESDStreamSampleOffset.CurrentVirtualTime as the fourth
    argument. This is what makes playback work at all for a recorded log: every
    sample in an ARBITRARY_TIMESTAMP file carries an absolute timestamp starting
    near zero, so a machine already running at virtual time ~25 s finds the whole
    file in the past. Without the offset Renode discards it:

        [WARNING] RESD: Skipping sample: 90000 Pa @ 0

    With it, the stream is rebased onto the current virtual time and samples are
    released at their recorded spacing.

    The models use StartExactSampleFeedThread, which takes no frequency at all -
    pacing comes from the timestamps in the file, so nothing is hardcoded.

    The call is handed to the Monitor via Parse() rather than invoked directly with
    Machine[node].Execute(). That matters: the LSM6DSR methods take a string, but
    MS5611's take a ReadFilePath, and ReadFilePath has no Python-callable
    constructor, so Execute() fails on the baro and temperature feeds. Letting the
    Monitor do the call means its own type conversion applies to both.
    """
    resolved = resd(path)
    if not os.path.exists(resolved):
        raise IOError("no such RESD file: %s" % resolved)

    if offset:
        cmd = "%s %s @%s %d 0 CurrentVirtualTime" % (node, method, resolved, channel)
    else:
        cmd = "%s %s @%s %d %d" % (node, method, resolved, channel, now_ns() + margin_ns)

    if not self.Parse(cmd, self.Interaction):
        raise RuntimeError("feed failed: %s" % cmd)
    return resolved


def feedall(path, margin_ns=500000000, strict=False):
    """Feed acceleration, angular rate, pressure and temperature from one file.

    A RESD only carries the sample types it was built from, so a file containing
    just pressure and temperature will not satisfy the accelerometer feed - the
    model raises "Provided RESD file doesn't contain data for ...AccelerationSample".
    That used to abort the whole call, leaving every sensor unfed. Streams whose
    type is absent are now skipped and reported instead.

    NOTE: the models also append a completion line to a status file, because the
    console is unusable - see feedstatus().

    Returns the list of streams that were fed. With strict=True any failure is
    raised instead.
    """
    # Resolve and check the file once, up front. feed() raises IOError for a missing
    # file, which the loop below would otherwise swallow and report as four
    # "skipped ..." lines - hiding the real cause behind a symptom.
    resolved = resd(path)
    if not os.path.exists(resolved):
        raise IOError("feedall: no such RESD file: %s (looked for %r)" % (resolved, path))

    report("feedall: starting playback from %s" % resolved)

    fed = []
    for node, method in RESD_STREAMS:
        try:
            feed(node, method, resolved, margin_ns)
            fed.append(method)
        except Exception as e:
            if strict:
                raise
            report("feedall: skipped %s (%s)" % (method, e))

    if not fed:
        raise RuntimeError("feedall: %s matched none of the sensor streams" % resolved)

    report("feedall: playback started on %d of %d stream(s): %s"
           % (len(fed), len(RESD_STREAMS), ", ".join(fed)))
    return fed


def feedstatus():
    """Print the RESD completion status file, which the models append to.

    Each model's [AfterRESDSample] handler appends one line here when its stream
    runs out of samples. This exists because the Renode log is unusable for this:
    the i2c/i2c3/rtc 'Unhandled write/read' lines arrive thousands of times per run,
    and there is no global log-level setter reachable from Monitor Python in 1.17
    (Logger.MinimumLogLevel is read-only, and Logger.SetLogLevel takes a backend,
    level and sourceId, and does not suppress those).
    """
    from System.IO import File

    path = _status_path()
    if not File.Exists(path):
        print("feedstatus: no status file yet at %s" % path)
        return []
    lines = [l for l in File.ReadAllLines(path) if l.strip()]
    for line in lines:
        print("  %s" % line)
    print("feedstatus: %d entry/entries in %s" % (len(lines), path))
    return lines


def _status_path():
    import os
    import tempfile
    return os.path.join(tempfile.gettempdir(), "renode_resd_status.txt")


def report(message):
    """Surface a message where it can actually be seen.

    print() goes to stdout, which the Renode GUI discards, and the log is buried
    under thousands of unimplemented-register warnings. So this writes to the same
    temp status file the models append to, which feedstatus() reads.
    """
    from System.IO import File

    try:
        File.AppendAllText(_status_path(), message + "\n")
    except Exception:
        pass

    try:
        from Antmicro.Renode.Logging import Logger, LogLevel
        Logger.Log(LogLevel.Warning, message)
    except Exception:
        pass

    print(message)
    return message


def resdstatus():
    """Which RESD streams have finished playing back.

    Each model's [AfterRESDSample] handler stops and nulls its RESD thread when
    the stream runs out of samples, so 'field is None' means that stream is done.
    """
    from System.Reflection import BindingFlags

    nonpublic = BindingFlags.NonPublic | BindingFlags.Instance

    fields = {
        "sysbus.i2c2.lsm6dsr": ["accelerometerResdThread", "gyroResdThread"],
        "sysbus.i2c2.ms5611": ["pressureFeederThread", "temperatureFeederThread"],
    }

    status = []
    for node, names in fields.items():
        try:
            dev = self.Machine[node]
            ty = dev.GetType()
        except Exception:
            continue
        for name in names:
            try:
                f = ty.GetField(name, nonpublic)
            except Exception:
                continue
            if f is None:
                continue
            status.append((node.split(".")[-1], name, f.GetValue(dev) is None))
    return status


def waitfeed(timeout=900, interval=0.5, stream="all"):
    """Block until every fed RESD stream has finished, then log a summary.

    NOTE: this blocks the Monitor for the whole playback, which freezes the GUI.
    For interactive use prefer the models' own completion messages - each one's
    [AfterRESDSample] handler logs "RESD playback complete: ..." on its own, so
    you can just watch the Log pane without blocking anything. This helper is
    for scripts that need to act once playback is done.

    Playback is asynchronous: feed()/feedall() start a background thread and
    return immediately, releasing samples as virtual time advances. Expect a log
    of ~60 s of emulated time to take several minutes of wall clock, since this
    board runs Renode at roughly 30% of real time.

    Output goes through Logger so it appears in the Renode log pane; print() only
    reaches stdout, which the GUI discards.

    Returns True if everything finished, False on timeout.
    """
    import time
    from Antmicro.Renode.Logging import Logger, LogLevel

    deadline = time.time() + timeout
    announced = False

    while True:
        status = resdstatus()
        if stream != "all":
            status = [s for s in status if stream in s[1]]

        pending = [s for s in status if not s[2]]
        done = [s for s in status if s[2]]

        if not status:
            Logger.Log(LogLevel.Warning, "waitfeed: no active RESD streams - call feedall() first")
            return False

        if not pending:
            break

        if not announced:
            Logger.Log(LogLevel.Info, "waitfeed: waiting for %d RESD stream(s) to finish..." % len(pending))
            announced = True

        if time.time() > deadline:
            still = ", ".join(["%s.%s" % (n, f) for n, f, _ in pending])
            Logger.Log(LogLevel.Warning, "waitfeed: TIMEOUT after %ds, still playing: %s" % (timeout, still))
            return False

        time.sleep(interval)

    for name, field, _ in done:
        Logger.Log(LogLevel.Info, "  feed finished: %s.%s" % (name, field))
    Logger.Log(LogLevel.Info, "waitfeed: all %d RESD stream(s) complete" % len(done))
    return True
    """Print what each sensor model currently holds, so you can see data arriving.

    This is the only direct view of the values: telemetry carries accel and gyro
    in the binary packet, not as UART text, so a feed that works perfectly still
    looks like nothing happened on the terminal.

    LSM6DSR is reached through its private commonFifo by reflection - it exposes no
    public accessor, and I2C peripherals have no memory-mapped registers to read.
    MS5611 and INA219 have public properties, so those are read directly.
    """
    import time
    from System.Reflection import BindingFlags

    nonpublic = BindingFlags.NonPublic | BindingFlags.Instance

    def fifo_sample(node, prop):
        dev = self.Machine[node]
        fifo = dev.GetType().GetField("commonFifo", nonpublic).GetValue(dev)
        value = fifo.GetType().GetProperty(prop).GetValue(fifo, None)
        return "null" if value is None else str(value)

    def prop(node, name):
        dev = self.Machine[node]
        p = dev.GetType().GetProperty(name)
        return getattr(dev, name) if p else "n/a"

    for i in range(count):
        if i:
            time.sleep(interval)
        from Antmicro.Renode.Logging import Logger, LogLevel
        Logger.Log(LogLevel.Info, "t=%4.1fs  accel=%-40s gyro=%-40s press=%-10s temp=%-10s V=%-8s I=%s" % (
            i * interval,
            fifo_sample("sysbus.i2c2.lsm6dsr", "AccelerationSample"),
            fifo_sample("sysbus.i2c2.lsm6dsr", "AngularRateSample"),
            prop("sysbus.i2c2.ms5611", "Pressure"),
            prop("sysbus.i2c2.ms5611", "Temperature"),
            prop("sysbus.i2c3.ina219", "Voltage"),
            prop("sysbus.i2c3.ina219", "Current")))