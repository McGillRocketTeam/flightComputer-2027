//
// Copyright (c) 2010-2026 Antmicro
//
// This file is licensed under the MIT License.
// Full license text is available in 'licenses/MIT.txt'.
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Logging;

namespace Antmicro.Renode.Peripherals.SPI
{
    /// <summary>
    /// Minimal NXP PCAL9714 8-bit I/O expander, sufficient to satisfy the driver's
    /// initialisation sequence.
    ///
    /// Wire format, recovered from the firmware's bus_read/bus_write in rtos.elf:
    ///
    ///   bus_read   : GPIO(chip select) -> Transmit(reg | 0x01)
    ///                             -> Transmit(address)
    ///                             -> Receive(data, count)
    ///   bus_write  : GPIO(chip select) -> Transmit(reg & ~0x01)
    ///                             -> Transmit(address)
    ///                             -> Transmit(data, count)
    ///
    /// so every transaction is [command][address][data...], where bit 0 of the
    /// command byte selects read (1) or write (0). Each transaction is a separate
    /// SPI frame, so FinishTransmission() rewinds to expecting a command byte.
    ///
    /// Why a stub is needed: pcal9714_init() reads register 0x40 and requires the
    /// result to equal 0xFF
    ///
    ///     8003aa0: bl   pcal9714_read_reg   ; reg 0x40
    ///     8003aaa: cmp  r3, #0             ; status must be 0
    ///     8003ab0: cmp  r3, #255           ; value must be 0xFF
    ///
    /// With no device on the bus, HAL_SPI_Transmit/HAL_SPI_Receive block in
    /// SPI_WaitFlagStateUntilTimeout until their 1 s timeout expires. That burns
    /// the 3 s IWDG budget during MRT_Init and the watchdog fires mid-init.
    ///
    /// Caveat: this is not a functional expander. Every register reads back as
    /// `fillValue` (0xFF) until written, so continuity, arm/energize and pin-level
    /// reads all report fabricated but self-consistent values. It exists to get
    /// pcal9714_init() past its chip-ID check and stop the SPI timeouts, not to
    /// emulate the recovery subsystem's real behaviour.
    /// </summary>
    public class PCAL9714 : ISPIPeripheral
    {
        public PCAL9714(IMachine machine)
        {
            // Property initialisers run before the constructor body, so FillValue
            // is already set when Reset() reads it.
            Reset();
        }

        public void Reset()
        {
            registers = new byte[256];
            for(var i = 0; i < registers.Length; i++)
            {
                registers[i] = FillValue;
            }

            phase = Phase.Command;
            isRead = false;
            address = 0;
            transactionCount = 0;
        }

        public void FinishTransmission()
        {
            // bus_read and bus_write each raise chip select around a whole
            // transaction, so every frame starts with a fresh command byte.
            phase = Phase.Command;
        }

        public byte Transmit(byte data)
        {
            if(transactionCount < LogTransactionCount)
            {
                transactionCount++;
                this.Log(LogLevel.Noisy, "PCAL9714 SPI byte #{0}: phase={1} 0x{2:X2}", transactionCount, phase, data);
            }

            switch(phase)
            {
                case Phase.Command:
                    isRead = (data & 0x01) != 0;
                    this.DebugLog("command 0x{0:X2} = {1}", data, isRead ? "read" : "write");
                    address = 0;
                    phase = Phase.Address;
                    return 0;

                case Phase.Address:
                    address = data;
                    phase = Phase.Data;
                    return 0;

                default:
                    if(isRead)
                    {
                        var value = registers[address];
                        address++;
                        return value;
                    }

                    this.DebugLog("reg[0x{0:X2}] = 0x{1:X2}", address, data);
                    registers[address] = data;
                    address++;
                    return 0;
            }
        }

        /// <summary>
        /// Value returned by any register that has not been written. Defaults to
        /// 0xFF so the chip-ID read in pcal9714_init() passes. Override from the
        /// platform description if you want a different pin/port state.
        /// </summary>
        public byte FillValue { get; set; } = 0xFF;

        private byte[] registers;
        private Phase phase;
        private bool isRead;
        private byte address;
        private int transactionCount;

        private const int LogTransactionCount = 12;

        private enum Phase
        {
            Command,
            Address,
            Data
        }
    }
}