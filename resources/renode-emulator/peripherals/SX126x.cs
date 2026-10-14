//
// Copyright (c) 2010-2026 Antmicro
//
// This file is licensed under the MIT License.
// Full license text is available in 'licenses/MIT.txt'.
//

using System;
using System.Collections.Generic;

using Antmicro.Renode.Core;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.SPI
{
    /// <summary>
    /// SX127x/SX126x-family LoRa radio, wired to match fc.ioc.
    ///
    /// Bus: SPI2 (hspi2 = 0x20000668), confirmed from the SX127x opcodes seen on
    /// that bus in rtos.elf: 0x80 0x8A 0x8D 0x8B 0x8C 0x1D 0x9F 0xA0 0x86.
    ///
    /// Pin map from fc.ioc, and which side drives which:
    ///
    ///     PB12  SX_CS    GPIO_Output  PULLUP   MCU -> radio   (not modelled here)
    ///     PB13  SX_SCK   SPI2_SCK     -        MCU -> radio   (the SPI bus)
    ///     PB14  SX_MISO  SPI2_MISO    -        radio -> MCU   (SPI RX)
    ///     PB15  SX_MOSI  SPI2_MOSI    -        MCU -> radio   (SPI TX)
    ///     PD10  SX_DIO   GPXTI10      PULLDOWN radio -> MCU   <- Connections[0]
    ///     PD11  SX_RST   GPIO_Output  NOPULL   MCU -> radio   (not modelled here)
    ///     PD9   SX_BUSY  GPIO_Input   PULLDOWN radio -> MCU   <- Connections[2]
    ///
    /// CS and RST are driven by the MCU, so they are not outputs of this model.
    ///
    /// Why DIO0 must be actively driven: with no model, PD10 was undriven and a
    /// transition on it raised EXTI line 10 during MRT_Init. HAL_GPIO_EXTI_Callback
    /// then called xQueueGiveFromISR(sxDioSemaphore, ...) with sxDioSemaphore
    /// still NULL, which spins forever at 0x8031e34 and starves the 3 s IWDG.
    /// Holding DIO0 low means no edge, so the callback never runs early.
    ///
    /// BUSY is held low because this model completes every command instantly.
    /// DIO0/DIO1 are held low because no packet is ever received, so there is
    /// nothing to signal. TX/RX done and timeout are deliberately not emitted.
    /// </summary>
    public class SX126x : ISPIPeripheral, INumberedGPIOOutput
    {
        public SX126x(IMachine machine)
        {
            dio0 = new GPIO();
            dio1 = new GPIO();
            busy = new GPIO();

            // must be built here, not in a property initialiser: those run before
            // the constructor body, so the GPIOs would not exist yet
            connections = new Dictionary<int, IGPIO>
            {
                { 0, dio0 },   // DIO0 -> PD10 (SX_DIO, EXTI line 10)
                { 1, dio1 },   // DIO1, not routed on this board
                { 2, busy }    // BUSY -> PD9 (SX_BUSY)
            };

            this.machine = machine;

            // The GPIO ports do not exist yet when a peripheral's preinit runs,
            // so TryConnect fails here. Wire lazily on the first SPI transfer
            // instead, which is well after MX_GPIO_Init has run.
            Reset();
        }

        public void Reset()
        {
            registers = new byte[RegisterCount];
            for(var i = 0; i < registers.Length; i++)
            {
                registers[i] = InitialValue;
            }

            phase = Phase.Idle;
            address = 0;
            pendingRead = false;

            // idle radio: no packet pending, not busy
            dio0.Set(false);
            dio1.Set(false);
            busy.Set(false);
        }

        public void FinishTransmission()
        {
            phase = Phase.Idle;
            pendingRead = false;
        }

        public byte Transmit(byte data)
        {
            EnsureWired();

            switch(phase)
            {
                case Phase.Idle:
                    if((data & WriteFlag) != 0)
                    {
                        // register write: bit 7 set, low bits are the address
                        address = (byte)(data & AddressMask);
                        phase = Phase.Command;
                        this.Log(LogLevel.Noisy, "reg write 0x{0:X2}", address);
                        return InitialValue;
                    }

                    // register read: latch the value, auto-increment afterwards
                    address = (byte)(data & AddressMask);
                    phase = Phase.Command;
                    pendingRead = true;
                    this.Log(LogLevel.Noisy, "reg read 0x{0:X2}", address);
                    return registers[address];

                case Phase.Command:
                    // second address byte, used by the FIFO registers
                    address = data;
                    phase = Phase.Data;
                    return pendingRead ? registers[address] : InitialValue;

                default:
                    // Index before advancing. "address - 1" promotes to int and
                    // reads registers[-1] once a 256-byte FIFO burst wraps
                    // address from 255 back to 0, which threw
                    // IndexOutOfRangeException out of HandleDataWrite and took
                    // the whole machine down.
                    var index = address;
                    address = (byte)((address + 1) & 0xFF);

                    if(pendingRead)
                    {
                        return registers[index];
                    }

                    registers[index] = data;
                    return InitialValue;
            }
        }

        public IReadOnlyDictionary<int, IGPIO> Connections => connections;

        /// <summary>
        /// Value returned on every transmit, and held by unwritten registers.
        /// 0xFF is what Renode returns when a SPI master transmits with no slave
        /// attached, so the driver's existing init path is preserved. Set to 0x00
        /// to emulate a cold or unpowered radio.
        /// </summary>
        public byte InitialValue { get; set; } = 0xFF;

        private bool wired;

        /// <summary>
        /// Attach DIO0 and BUSY to their pins on the first SPI transfer. Doing it
        /// in the constructor is too early: a peripheral's preinit runs before
        /// the rest of the platform exists, so gpioPortD cannot be resolved yet.
        /// </summary>
        private void EnsureWired()
        {
            if(wired)
            {
                return;
            }

            wired = true;

            // Renode's repl parser only accepts alternate-function GPIO
            // connections ("N -> gpioPortX#pin@AF", see stm32l071.repl), which is
            // the wrong thing for a radio DIO. STM32_GPIOPort is an
            // IGPIOReceiver, so IGPIO.Connect does the job from here instead.
            if(!TryConnect(machine, dio0, "gpioPortD", 10))
            {
                this.Log(LogLevel.Warning, "DIO0 not wired to gpioPortD#10; SX_DIO will float");
            }
            else
            {
                // re-assert the idle level now that the pin is attached
                dio0.Set(false);
            }

            if(!TryConnect(machine, busy, "gpioPortD", 9))
            {
                this.Log(LogLevel.Warning, "BUSY not wired to gpioPortD#9; SX_BUSY will float");
            }
            else
            {
                busy.Set(false);
            }
        }

        private bool TryConnect(IMachine machine, IGPIO output, string portName, int pin)
        {
            try
            {
                IPeripheral peripheral = null;
                foreach(var candidate in new[] { portName, "sysbus." + portName })
                {
                    if(machine.TryGetByName(candidate, out peripheral) && peripheral != null)
                    {
                        break;
                    }

                    peripheral = null;
                }

                if(peripheral == null)
                {
                    this.Log(LogLevel.Warning, "no peripheral named {0}", portName);
                    return false;
                }

                var receiver = peripheral as IGPIOReceiver;
                if(receiver == null)
                {
                    this.Log(LogLevel.Warning, "{0} is not an IGPIOReceiver", portName);
                    return false;
                }

                output.Connect(receiver, pin);
                this.Log(LogLevel.Noisy, "wired output to {0} pin {1}", portName, pin);
                return true;
            }
            catch(Exception e)
            {
                this.Log(LogLevel.Warning, "connect {0} pin {1} failed: {2}", portName, pin, e.Message);
                return false;
            }
        }

        private readonly IGPIO dio0;
        private readonly IGPIO dio1;
        private readonly IGPIO busy;
        private readonly IReadOnlyDictionary<int, IGPIO> connections;
        private readonly IMachine machine;

        private byte[] registers;
        private Phase phase;
        private byte address;
        private bool pendingRead;

        private const int RegisterCount = 256;
        private const byte WriteFlag = 0x80;
        private const byte AddressMask = 0x7F;

        private enum Phase
        {
            Idle,
            Command,
            Data
        }
    }
}
