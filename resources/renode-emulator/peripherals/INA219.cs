//
// INA219 - bidirectional current and voltage monitor (Texas Instruments).
//
// The FC6.1 board hangs this on i2c3 at 0x40 (MRT_setup.h: INA219_I2C hi2c3,
// INA219_ADDRESS 0x40). The driver (Drivers/PIL/INA219/MRT_INA219.c) talks to it
// only through HAL_I2C_Mem_Read / HAL_I2C_Mem_Write with a 1-byte register
// address, so this models the standard "write pointer, then data" I2C form rather
// than the MS5611's command-driven one.
//
// Register map (MRT_INA219.h):
//
//   0x00 CONFIG        bit15 = reset, self-clears back to the power-on 0x399F
//   0x01 SHUNTVOLTAGE  signed, LSB 10/10/40/80 uV depending on the PG gain bits
//   0x02 BUSVOLTAGE    unsigned, LSB 4 mV (16 V range) or 8 mV (32 V range)
//   0x03 POWER         unsigned, LSB = 2 * current_LSB_mA * 20 mV
//   0x04 CURRENT       signed, LSB derived from CALIBRATION
//   0x05 CALIBRATION
//
// Note on the driver rather than this model: INA219_ReadBusVoltage returns
// ((raw >> 3) * 4), which does not match the datasheet LSB of 4 or 8 mV, and
// INA219_ReadShuntVolage returns raw * 0.01. The scaling above is the real
// device behaviour; what the flight code prints is its own arithmetic. This
// model is read-only in that respect - it never fudges a value to match.

using System;

using Antmicro.Renode.Core;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.I2C;
using Antmicro.Renode.Peripherals.Sensor;

namespace Antmicro.Renode.Peripherals.Sensors;

public class INA219 : II2CPeripheral, ISensor
{
    public INA219(IMachine _)
    {
        Reset();
    }

    // Reported bus voltage, in volts.
    public double Voltage
    {
        get => voltage;
        set
        {
            if(value < 0 || value > MaximalOperatingVoltage)
            {
                this.WarningLog($"Value {value} V is out of the INA219 range, ignoring.");
                return;
            }

            voltage = value;
        }
    }

    // Reported shunt current, in milliamps. May be negative (charging back up).
    public double Current
    {
        get => current;
        set
        {
            if(value < MinimumOperatingCurrent || value > MaximalOperatingCurrent)
            {
                this.WarningLog($"Value {value} mA is out of the INA219 range, ignoring.");
                return;
            }

            current = value;
        }
    }

    // Shunt voltage, in millivolts. Only exposed because it is the input that
    // the current register is derived from on the real part.
    public double ShuntVoltage
    {
        get => shuntVoltage;
        set => shuntVoltage = value;
    }

    public void Reset()
    {
        registers = new ushort[6];
        registers[ConfigurationRegister] = ConfigurationResetValue;
        pointer = 0;
        readOffset = 0;

        voltage = DefaultVoltage;
        current = DefaultCurrent;
        shuntVoltage = DefaultShuntVoltage;
    }

    public void Write(byte[] data)
    {
        if(data.Length == 0)
        {
            this.Log(LogLevel.Noisy, "Write with no data, ignoring.");
            return;
        }

        // The first byte of every transaction is the register pointer.
        pointer = data[0];
        readOffset = 0;

        if(pointer >= registers.Length)
        {
            this.WarningLog("Write to register 0x{0:X2} is out of range, ignoring", pointer);
            return;
        }

        // Remaining bytes are register data, big endian, two per register. The
        // driver only ever writes exactly two (HAL_I2C_Mem_Write with a 2-byte
        // buffer), but the part auto-increments after each word, so honour that.
        var pending = 0;
        var value = 0;
        for(var i = 1; i < data.Length; i++)
        {
            value = (value << 8) | data[i];
            pending++;

            if(pending != 2)
            {
                continue;
            }

            registers[pointer] = (ushort)value;
            pending = 0;
            value = 0;

            // CONFIG bit15 is write-only and self-clearing; writing it drops
            // the part back to its power-on configuration.
            if(pointer == ConfigurationRegister && (registers[ConfigurationRegister] & ConfigurationResetBit) != 0)
            {
                registers[ConfigurationRegister] = ConfigurationResetValue;
            }

            pointer++;
        }
    }

    public byte[] Read(int count = 1)
    {
        var result = new byte[count];

        // Refresh the measurement registers on every read, so a host polling
        // them sees live values instead of whatever was cached at reset.
        RefreshMeasurements();

        for(var i = 0; i < count; i++)
        {
            if(pointer >= registers.Length)
            {
                result[i] = 0;
                continue;
            }

            var word = registers[pointer];
            // Renode hands over one byte per call, and repeats until the master
            // stops clocking, so alternate high/low off a persistent offset.
            result[i] = readOffset == 0 ? (byte)(word >> 8) : (byte)(word & 0xFF);
            readOffset ^= 1;

            if(readOffset == 0)
            {
                pointer++;
            }
        }

        return result;
    }

    public void FinishTransmission()
    {
        // The pointer persists between the write phase and the following read
        // phase of the same transaction (HAL_I2C_Mem_Read relies on that), so it
        // must survive here. Only the byte phase resets.
        readOffset = 0;
    }

    private void RefreshMeasurements()
    {
        // Bus voltage range selects the LSB; BRNG is bit13 of CONFIG.
        var busRange32V = (registers[ConfigurationRegister] & BusRange32VBit) != 0;
        var busLsbMv = busRange32V ? BusLsbMv32VRange : BusLsbMv16VRange;
        var busRaw = (int)Math.Round((voltage * 1000) / busLsbMv);
        registers[BusVoltageRegister] = (ushort)Clamp(busRaw, 0, 0x7FFF);

        // PG[1:0] in bits 12:11 selects the shunt full scale and its LSB.
        var gain = (registers[ConfigurationRegister] & ShuntGainMask) >> 11;
        var shuntLsbUv = ShuntLsbUv[gain];
        // shuntVoltage is in mV, the LSB is in uV.
        var shuntRaw = (int)Math.Round((shuntVoltage * 1000) / shuntLsbUv);
        registers[ShuntVoltageRegister] = (ushort)Clamp(shuntRaw, short.MinValue, short.MaxValue);

        // Current LSB follows the datasheet's anchoring, calibration 4096 giving the
        // canonical 100 uA/bit (the 32 V / 2 A setting). That is consistent with
        // both calibration routines the driver ships: CAL 4096 pairs with
        // ina219_currentDivider_mA = 10 (100 uA/bit) and CAL 10240 pairs with 25
        // (40 uA/bit). So the LSB depends on CALIBRATION alone, not on the gain
        // bits - gain scales the shunt side, which CALIBRATION is chosen to match.
        var calibration = registers[CalibrationRegister];
        var currentLsbUa = calibration == 0
            ? DefaultCurrentLsbUa
            : Math.Max(CanonicalCurrentLsbNumerator / (double)calibration, MinimumCurrentLsbUa);

        var currentRaw = (int)Math.Round((current * 1000) / currentLsbUa);
        registers[CurrentRegister] = (ushort)Clamp(currentRaw, short.MinValue, short.MaxValue);

        // Power LSB is 2 * current_LSB_mA * 20 mV. mV * mA is uW, so divide by 1000
        // to land in the mW the LSB is expressed in.
        var powerLsbMw = 2 * (currentLsbUa / 1000.0) * 20;
        var powerRaw = (int)Math.Round(((voltage * 1000) * current) / 1000.0 / powerLsbMw);
        registers[PowerRegister] = (ushort)Clamp(powerRaw, 0, 0xFFFF);
    }

    private static int Clamp(int value, int min, int max)
    {
        if(value < min)
        {
            return min;
        }

        return value > max ? max : value;
    }

    private ushort[] registers;
    private byte pointer;
    private int readOffset;

    private double voltage;
    private double current;
    private double shuntVoltage;

    private const int ConfigurationRegister = 0x00;
    private const int ShuntVoltageRegister = 0x01;
    private const int BusVoltageRegister = 0x02;
    private const int PowerRegister = 0x03;
    private const int CurrentRegister = 0x04;
    private const int CalibrationRegister = 0x05;

    // Power-on configuration, datasheet default: 32 V range, gain 8, 12-bit
    // bus and shunt ADC, continuous shunt+bus mode.
    private const ushort ConfigurationResetValue = 0x399F;
    private const ushort ConfigurationResetBit = 0x8000;
    private const ushort BusRange32VBit = 0x2000;
    private const ushort ShuntGainMask = 0x1800;

    private const double BusLsbMv16VRange = 4.0;
    private const double BusLsbMv32VRange = 8.0;

    // Shunt LSB in microvolts for PG = 00, 01, 10, 11 (40, 80, 160, 320 mV FS).
    private static readonly double[] ShuntLsbUv = { 10, 10, 40, 80 };

    // currentLSB_uA = this / CALIBRATION, i.e. 100 uA at the canonical 4096.
    private const double CanonicalCurrentLsbNumerator = 409600;

    private const double DefaultCurrentLsbUa = 100;
    private const double MinimumCurrentLsbUa = 0.001;

    private const double DefaultVoltage = 5.0;
    private const double DefaultCurrent = 120;
    private const double DefaultShuntVoltage = 12;

    private const double MaximalOperatingVoltage = 26;
    private const double MaximalOperatingCurrent = 3200;
    private const double MinimumOperatingCurrent = -3200;
}
