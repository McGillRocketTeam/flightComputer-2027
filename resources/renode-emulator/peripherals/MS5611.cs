//
// Copyright (c) 2010-2026 Antmicro
//
// This file is licensed under the MIT License.
// Full license text is available in 'licenses/MIT.txt'.
//

using System;
using System.IO;

using Antmicro.Renode.Core;
using Antmicro.Renode.Exceptions;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.I2C;
using Antmicro.Renode.Peripherals.Sensor;
using Antmicro.Renode.Time;
using Antmicro.Renode.Utilities;
using Antmicro.Renode.Utilities.RESD;

namespace Antmicro.Renode.Peripherals.Sensors;

// MS5611-01BA03: 24-bit pressure (D1) and temperature (D2) sensor.
// The device is command-driven (no register address phase beyond the command byte):
//   * one-byte write = command (reset / convert D1 / convert D2 / ADC read / PROM read)
//   * a following read returns the conversion result (24-bit) or the PROM word (16-bit), MSB first.
// D1/D2 are pre-compensated ADC counts from which the host driver derives pressure/temperature
// using the six calibration coefficients read from the PROM. This model computes D1/D2 so that
// the host's compensation math yields the configured Temperature/Pressure, keeping CRC4 valid.
public class MS5611 : II2CPeripheral, ITemperatureSensor, IPressureSensor, ISensor, IUnderstandRESD
{
    public MS5611(IMachine _)
    {
        // Fill the CRC nibble so a host-side CRC4 check passes.
        prom[7] = (ushort)CalculateCrc4(prom);
        Reset();
    }

    public byte[] Read(int count = 1)
    {
        if(lastCommand == null)
        {
            // Log rather than bail: returning an empty array makes the HAL see a
            // zero-length read, which on some paths looks like a bus error and
            // makes the caller retry forever. Hand back zeros instead.
            this.WarningLog("Read with no pending command; returning zeros");
            return new byte[count];
        }

        byte[] result = new byte[count];
        switch(lastCommand)
        {
        case Command.AdcRead:
            FillAdcReadBytes(result);
            break;

        case Command.PromRead:
            FillPromReadBytes(result);
            break;

        default:
            this.WarningLog("Tried to read after command {0} which has no data result", lastCommand);
            break;
        }

        return result;
    }

    public void Reset()
    {
        // Restore conversion state like after power-on: nothing converted yet,
        // so an ADC read must return 0.
        hasPressureConversion = false;
        hasTemperatureConversion = false;
        hasConverted = false;
        adcResult = 0;
        lastCommand = null;
        promByteIndex = 0;
        lastConversionWasPressure = false;
    }

    public void Write(byte[] data)
    {
        if(data.Length == 0)
        {
            this.Log(LogLevel.Noisy, "Write with no data. Ignoring.");
            return;
        }

        var commandByte = data[0];
        // A new command restarts any multi-byte read from the high byte.
        promByteIndex = 0;
        switch(commandByte)
        {
        case ResetCommand:
            this.Log(LogLevel.Noisy, "Reset command received");
            Reset();
            break;

        case ConvertD1_OSR256:
        case ConvertD1_OSR512:
        case ConvertD1_OSR1024:
        case ConvertD1_OSR2048:
        case ConvertD1_OSR4096:
            this.Log(LogLevel.Noisy, "D1 (pressure) conversion command");
            StartPressureConversion();
            break;

        case ConvertD2_OSR256:
        case ConvertD2_OSR512:
        case ConvertD2_OSR1024:
        case ConvertD2_OSR2048:
        case ConvertD2_OSR4096:
            this.Log(LogLevel.Noisy, "D2 (temperature) conversion command");
            StartTemperatureConversion();
            break;

        case AdcReadCommand:
            this.Log(LogLevel.Noisy, "ADC read command");
            lastCommand = Command.AdcRead;
            break;

        case var _ when IsPromReadCommand(commandByte):
            promAddress = (commandByte & 0x0E) >> 1;
            this.Log(LogLevel.Noisy, "PROM read command, address {0}", promAddress);
            lastCommand = Command.PromRead;
            break;

        default:
            this.WarningLog("Unknown command byte 0x{0:X2}", commandByte);
            lastCommand = null;
            break;
        }
    }

    public void FinishTransmission()
    {
        // The MS5611 is command-driven: the host writes a command byte in one
        // transfer and reads the result in a *separate* transfer. Renode signals
        // the end of the command transfer here, so clearing lastCommand would
        // leave the following read with nothing to act on and every conversion
        // would fail with "Tried to read data without issuing a command first".
        // Keep it; Write() replaces it on the next command.
    }

    public decimal Temperature
    {
        set
        {
            if(value < MinimalTemperatureOperatingValue || value > MaximalTemperatureOperatingValue)
            {
                this.WarningLog($"Value {value} is not in valid temperature range, ignoring.");
                return;
            }

            temperature = value;
            // A freshly set temperature must be reflected by the next D2 conversion.
            hasTemperatureConversion = false;
        }

        get => temperature;
    }

    public decimal Pressure
    {
        set
        {
            if(value < MinimalPressureOperatingValue || value > MaximalPressureOperatingValue)
            {
                this.WarningLog($"Value {value} is not in valid pressure range, ignoring.");
                return;
            }

            pressure = value;
            // A freshly set pressure must be reflected by the next D1 conversion.
            hasPressureConversion = false;
        }

        get => pressure;
    }

    public void FeedPressureSamplesFromRESD(ReadFilePath filePath, uint channelId = 0, ulong startTime = 0,
        RESDStreamSampleOffset sampleOffsetType = RESDStreamSampleOffset.Specified, long sampleOffsetTime = 0)
    {
        pressureStream?.Dispose();
        pressureStream = this.CreateRESDStream<PressureSample>(filePath, channelId, sampleOffsetType, sampleOffsetTime);
        pressureFeederThread?.Stop();
        // See FeedTemperatureSamplesFromRESD: exact timestamps, no guessed rate.
        pressureFeederThread = pressureStream.StartExactSampleFeedThread(this,
            startTime: startTime
        );

        this.Log(LogLevel.Noisy, "RESD stream set to {0}", filePath);
    }

    public void FeedTemperatureSamplesFromRESD(ReadFilePath filePath, uint channelId = 0, ulong startTime = 0,
        RESDStreamSampleOffset sampleOffsetType = RESDStreamSampleOffset.Specified, long sampleOffsetTime = 0)
    {
        temperatureStream?.Dispose();
        temperatureStream = this.CreateRESDStream<TemperatureSample>(filePath, channelId, sampleOffsetType, sampleOffsetTime);
        temperatureFeederThread?.Stop();
        // StartExactSampleFeedThread takes no frequency - each sample is released
        // at the timestamp recorded in the RESD block. The rate-based overload
        // meant hardcoding a sampling rate here, which has to be guessed and then
        // silently bunches or drops samples whenever the guess is wrong.
        temperatureFeederThread = temperatureStream.StartExactSampleFeedThread(this,
            startTime: startTime
        );

        this.Log(LogLevel.Noisy, "RESD stream set to {0}", filePath);
    }

    // Where RESD completion is recorded. The Renode log is unusable for this: the
    // i2c/i2c3/rtc "Unhandled write/read" warnings arrive thousands of times per run,
    // and 1.17 exposes no global log-level setter from Monitor Python
    // (Logger.MinimumLogLevel is read-only; Logger.SetLogLevel needs a backend, a
    // level and a sourceId, and does not suppress those warnings).
    private static void ReportFeedComplete(string stream, int samples)
    {
        var message = String.Format("RESD PLAYBACK COMPLETE: {0} ({1} samples fed)",
                                   stream, samples);
        try
        {
            File.AppendAllText(Path.Combine(Path.GetTempPath(), "renode_resd_status.txt"),
                               message + Environment.NewLine);
        }
        catch(Exception)
        {
            // Status reporting must never take the emulation down.
        }

        // Also to stdout, so it lands in the same place as host_cmd.py's report()
        // messages. The models cannot call print(), so Console is the equivalent.
        Console.WriteLine(message);
        Logger.Log(LogLevel.Error, message);
    }

    private void StartPressureConversion()
    {
        lastConversionWasPressure = true;
        // Always recompute. The result used to be cached behind hasPressureConversion,
        // which the Pressure setter clears on every new value - so during RESD playback
        // a sample arriving between the conversion command and the ADC read re-armed
        // that flag and FillAdcReadBytes then zeroed the ADC, handing the driver a D1 of
        // 0. Its compensation ran on garbage, which is why the reported pressure jumped
        // around and pinned at values absent from the log.
        //
        // Compute the raw D1 that the host's compensation formula will turn
        // back into the currently configured pressure. The host does
        // (ms5611.c:346,349):
        //
        //     P = (((D1 * SENS) >> 21) - OFF) >> 15;
        //     *pressure = (float)P / 100;
        //
        // Inverting that for a target pressure gives
        //     D1 = ((P_raw << 15) + OFF) * 2^21 / SENS
        // with P_raw = pressure * 100, i.e. the raw 0.01 mbar integer the
        // host divides by 100 at the end.
        //
        // The previous form computed (p - OFF) * 2^21 / SENS. That subtracts
        // OFF (~2.4e9 for C2 << 16) from p (~1e5), which is always negative,
        // so Clamp24Bit pinned adcResult to 0. The driver then hit
        // "if (adc_temperature == 0 || adc_pressure == 0)" at ms5611.c:390,
        // returned i2c_transfer_error, and the altitude zeroing failed.
        var p = (long)Math.Round(pressure * 100M); // 0.01 mbar, the host's P_raw
        var dT = (long)Math.Round((temperature * 100M - 2000M) * (1L << 23) / CalibrationC6);
        var off = CalibrationC2 * (1L << 16) + (CalibrationC4 * dT) / 128;
        var sens = CalibrationC1 * (1L << 15) + (CalibrationC3 * dT) / 256;
        adcResult = Clamp24Bit(((p << 15) + off) * (1L << 21) / sens);
        hasConverted = true;
    }

    private void StartTemperatureConversion()
    {
        lastConversionWasPressure = false;
        // Always recompute - see StartPressureConversion for why caching breaks
        // playback from a RESD stream.
        var dT = (long)Math.Round((temperature * 100M - 2000M) * (1L << 23) / CalibrationC6);
        var cref = CalibrationC5 * (1L << 8);
        adcResult = Clamp24Bit(cref + dT);
        hasConverted = true;
    }

    private static long Clamp24Bit(long value)
    {
        if(value < 0)
        {
            return 0;
        }
        if(value > 0xFFFFFF)
        {
            return 0xFFFFFF;
        }
        return value;
    }

    private void FillAdcReadBytes(byte[] result)
    {
        // Per the datasheet an ADC read returns 0 if nothing has been converted
        // since reset. That check used to be tied to hasPressureConversion /
        // hasTemperatureConversion, which the Pressure and Temperature setters
        // clear on every new value - so any RESD sample arriving in the 1 ms
        // between the conversion command and this read zeroed the result. It now
        // depends only on hasConverted, which Reset() clears and a conversion sets.
        if(!hasConverted)
        {
            adcResult = 0;
        }

        for(var i = 0; i < result.Length; i++)
        {
            // 24-bit value, MSB first
            var shift = 16 - 8 * i;
            result[i] = shift >= 0
                ? (byte)((adcResult >> shift) & 0xFF)
                : (byte)0;
        }
    }

    private void FillPromReadBytes(byte[] result)
    {
        if(promAddress < 0 || promAddress >= prom.Length)
        {
            return;
        }

        // Renode delivers I2C reads one byte per call, so `result` is always a
        // single byte and indexing by `i` alone always returned the HIGH byte:
        // the host read 0x9C 0x9C instead of 0x9C 0xBF for every coefficient.
        // Track the byte position across successive Read() calls instead,
        // restarting from the high byte when a new command arrives.
        var word = prom[promAddress];
        for(var i = 0; i < result.Length; i++)
        {
            var shift = promByteIndex == 0 ? 8 : 0;
            result[i] = (byte)(word >> shift);
            promByteIndex = promByteIndex == 0 ? 1 : 0;
        }
    }

    [OnRESDSample(SampleType.Pressure)]
    private void OnPressureSample(PressureSample sample, TimeInterval _)
    {
        totalPressureSamples++;
        // RESD pressure is in milliPascals; this model's Pressure is in the 0.01 mbar
        // (hPa) scale used by LPS25HB, whose default is 1013.25 for sea level.
        // 1 hPa = 100 Pa = 100000 mPa, so the divisor is 100000.
        //
        // This was previously / 100M, which is 1000x too large: a sea-level
        // reading of 96361 Pa (96361000 mPa) became 963610, far above
        // MaximalPressureOperatingValue (1200), so every RESD sample was rejected
        // by the Pressure setter's range check and playback silently did nothing.
        Pressure = sample.Pressure / 100000M;
    }

    [OnRESDSample(SampleType.Temperature)]
    private void OnTemperatureSample(TemperatureSample sample, TimeInterval _)
    {
        totalTemperatureSamples++;
        Temperature = sample.Temperature / 1000M;
    }

    [AfterRESDSample(SampleType.Temperature)]
    private void HandleTemperatureSampleEnded(TemperatureSample _, TimeInterval __)
    {
        ReportFeedComplete("MS5611 temperature", totalTemperatureSamples);
        temperatureFeederThread?.Stop();
        temperatureFeederThread = null;
    }

    [AfterRESDSample(SampleType.Pressure)]
    private void HandlePressureSampleEnded(PressureSample _, TimeInterval __)
    {
        ReportFeedComplete("MS5611 pressure", totalPressureSamples);
        pressureFeederThread?.Stop();
        pressureFeederThread = null;
    }

    // CRC4, polynomial 0x3000, init 0. This mirrors the HOST driver
    // (Drivers/PIL/MS5611/ms5611.c, ms5611_crc_check) rather than AN520, because
    // the driver is what has to accept us:
    //
    //     n_prom[7] = (0xFF00 & n_prom[7]);          // low byte forced to 0
    //     for (cnt = 0; cnt < 16; cnt++) {           // 16 slots, not 14
    //         n_rem ^= (cnt % 2) ? n_prom[cnt >> 1] & 0x00FF
    //                               : n_prom[cnt >> 1] >> 8;
    //         for (n_bit = 8; n_bit > 0; n_bit--) { ... }
    //     }
    //     return (n_rem >> 12) & 0x000F == crc;
    //
    // cnt >> 1 walks words 0..7 twice, so word 7 contributes two bytes that are
    // zero after the 0xFF00 mask. AN520 says word 7's own bytes are not inputs,
    // but the shift runs on every bit rather than only on set ones, so those two
    // zero bytes DO rotate the remainder and change the result. Skipping word 7
    // here therefore produced a CRC4 the driver rejected, which is what made
    // ms5611_read_eeprom() return ms5611_status_crc_error.
    //
    // promWords[7] must still be zero when this runs - the constructor assigns the
    // computed nibble afterwards - so both of its bytes are the zeroes above.
    private static uint CalculateCrc4(ushort[] promWords)
    {
        ushort nRem = 0x00;

        // All 8 words: word 7 is included as the two zero bytes the driver feeds.
        for(var i = 0; i < promWords.Length; i++)
        {
            var word = promWords[i];

            // high byte, then low byte
            nRem ^= (ushort)((word & 0xFF00) >> 8);
            AdvanceCrc(ref nRem);
            nRem ^= (ushort)(word & 0x00FF);
            AdvanceCrc(ref nRem);
        }

        return (uint)((nRem >> 12) & 0x000F);
    }

    private static void AdvanceCrc(ref ushort nRem)
    {
        for(var i = 8; i > 0; i--)
        {
            if((nRem & 0x8000) != 0)
            {
                nRem = (ushort)((nRem << 1) ^ Polyomial);
            }
            else
            {
                nRem = (ushort)(nRem << 1);
            }
        }
    }

    // Note: playback rate comes from the RESD block timestamps via
    // StartExactSampleFeedThread, so there is no sampling-rate constant here.
    // (This model previously fed at a hardcoded OutputFrequency, which had to be
    // guessed from the log and dropped or bunched samples when it was wrong.)

    private static bool IsPromReadCommand(byte commandByte)
    {
        // PROM read commands are 0xA0 .. 0xAE (even addresses)
        return commandByte >= 0xA0 && commandByte <= 0xAE && (commandByte & 0x01) == 0;
    }

    private long adcResult;
    private bool hasPressureConversion;
    private bool hasTemperatureConversion;
    // Sample counters, reported when playback finishes.
    private int totalPressureSamples;
    private int totalTemperatureSamples;
    // Set by either conversion, cleared only by Reset(). See FillAdcReadBytes.
    private bool hasConverted;
    private bool lastConversionWasPressure;
    private Command? lastCommand;
    private int promAddress;

    // Position within the 16-bit PROM word currently being read; Renode hands us
    // one byte per Read() call, so this must persist across calls.
    private int promByteIndex;

    private decimal temperature = DefaultTemperature;
    private decimal pressure = DefaultPressure;

    private RESDStream<TemperatureSample> temperatureStream;
    private RESDStream<PressureSample> pressureStream;
    // StartExactSampleFeedThread hands back an ISimpleManagedThread, which is what
    // the exact-timestamp feeder returns. Only Stop() is needed on it.
    private ISimpleManagedThread temperatureFeederThread;
    private ISimpleManagedThread pressureFeederThread;

    // Factory-calibration PROM (values from the datasheet's typical example).
    private readonly ushort[] prom = new ushort[]
    {
        0x0000,                 // reserved for manufacturer
        CalibrationC1,          // C1 pressure sensitivity
        CalibrationC2,          // C2 pressure offset
        CalibrationC3,          // C3 temperature coefficient of pressure sensitivity
        CalibrationC4,          // C4 temperature coefficient of pressure offset
        CalibrationC5,          // C5 reference temperature
        CalibrationC6,          // C6 temperature coefficient of the temperature
        0x0000,                 // CRC in low nibble (filled in below)
    };

    // C1 is the datasheet example's 40127 plus one, deliberately. With 40127 the
    // driver's CRC4 works out to 0x0, so PROM word 7 reads back as 0x0000 and the
    // driver rejects it in ms5611_read_eeprom_coeff's "*coeff == 0" check
    // (ms5611.c:216) with i2c_transfer_error - before the CRC is ever compared.
    // Any non-zero CRC4 gets us past that; 40128 yields 0xB. Real factory
    // coefficients would not land on the degenerate value either.
    private const ushort CalibrationC1 = 40128;
    private const ushort CalibrationC2 = 36924;
    private const ushort CalibrationC3 = 23317;
    private const ushort CalibrationC4 = 23282;
    private const ushort CalibrationC5 = 33464;
    private const ushort CalibrationC6 = 28312;

    private const byte ResetCommand = 0x1E;
    private const byte AdcReadCommand = 0x00;
    private const byte ConvertD1_OSR256 = 0x40;
    private const byte ConvertD1_OSR512 = 0x42;
    private const byte ConvertD1_OSR1024 = 0x44;
    private const byte ConvertD1_OSR2048 = 0x46;
    private const byte ConvertD1_OSR4096 = 0x48;
    private const byte ConvertD2_OSR256 = 0x50;
    private const byte ConvertD2_OSR512 = 0x52;
    private const byte ConvertD2_OSR1024 = 0x54;
    private const byte ConvertD2_OSR2048 = 0x56;
    private const byte ConvertD2_OSR4096 = 0x58;

    private const ushort Polyomial = 0x3000;
    private const decimal DefaultTemperature = 20M;
    private const decimal DefaultPressure = 1013.25M;

    private const decimal MinimalPressureOperatingValue = 10;
    private const decimal MaximalPressureOperatingValue = 1200;

    private const decimal MinimalTemperatureOperatingValue = -40;
    private const decimal MaximalTemperatureOperatingValue = 85;

    private enum Command
    {
        AdcRead,
        PromRead,
    }
}
