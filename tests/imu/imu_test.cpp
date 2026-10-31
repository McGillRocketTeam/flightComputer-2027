#include "sensors/IMU.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>
struct Bus {
    uint8_t regs[256]{};
    uint8_t embedded[256]{};
    int calls = 0;
    int failAt = -1;
    bool stuck = false;
    Bus() { regs[LSM6DSR_WHO_AM_I] = LSM6DSR_ID; }
};
static unsigned waits;
static void delay(uint32_t ms) { waits += ms; }
static int32_t read(void* handle, uint8_t reg, uint8_t* data, uint16_t n) {
    Bus& bus = *static_cast<Bus*>(handle);
    if (++bus.calls == bus.failAt) return -1;
    if (reg == LSM6DSR_CTRL3_C && (bus.regs[reg] & 1) && !bus.stuck)
        bus.regs[reg] &= ~1;
    auto bank = (reg != LSM6DSR_FUNC_CFG_ACCESS && (bus.regs[LSM6DSR_FUNC_CFG_ACCESS] & 0x80)) ? bus.embedded : bus.regs;
    std::memcpy(data, bank + reg, n);
    return 0;
}
static int32_t write(void* handle, uint8_t reg, const uint8_t* data, uint16_t n) {
    Bus& bus = *static_cast<Bus*>(handle);
    if (++bus.calls == bus.failAt) return -1;
    if (reg == LSM6DSR_CTRL3_C && !(bus.regs[LSM6DSR_FUNC_CFG_ACCESS] & 0x80) && (*data & 1)) {
        std::memset(bus.regs, 0, sizeof bus.regs);
        std::memset(bus.embedded, 0, sizeof bus.embedded);
        bus.regs[LSM6DSR_WHO_AM_I] = LSM6DSR_ID;
    }
    auto bank = (reg != LSM6DSR_FUNC_CFG_ACCESS && (bus.regs[LSM6DSR_FUNC_CFG_ACCESS] & 0x80)) ? bus.embedded : bus.regs;
    std::memcpy(bank + reg, data, n);
    return 0;
}
static stmdev_ctx_t context(Bus& bus) { return {write, read, &bus}; }
static void raw(Bus& bus, uint8_t address, int16_t value) {
    uint16_t bits = static_cast<uint16_t>(value);
    bus.regs[address] = static_cast<uint8_t>(bits);
    bus.regs[address + 1] = static_cast<uint8_t>(bits >> 8);
}
static void measurements(Bus& bus, int16_t value) {
    for (unsigned axis = 0; axis < 3; ++axis) {
        raw(bus, LSM6DSR_OUTX_L_A + axis * 2, axis == 1 ? -value : value);
        raw(bus, LSM6DSR_OUTX_L_G + axis * 2, axis == 1 ? -value : value);
    }
    raw(bus, LSM6DSR_OUT_TEMP_L, value);
}
static bool near(float a, float b) { return std::fabs(a - b) < 0.001f; }
static void pollTests() {
    Bus bus;
    IMU sensor(context(bus), delay);
    sensor.poll();
    assert(bus.calls == 0 && sensor.status() == Sensor::ERROR);
    sensor.init();
    assert(sensor.getValidFields() == 0 && sensor.getUpdatedFields() == 0);
    // All combinations of independently ready fields, including no data.
    for (unsigned mask = 0; mask < 8; ++mask) {
        sensor.init();
        measurements(bus, 256);
        bus.regs[LSM6DSR_STATUS_REG] = mask;
        int calls = bus.calls;
        unsigned delays = waits;
        sensor.poll();
        assert(sensor.status() == Sensor::NOMINAL);
        assert(sensor.getUpdatedFields() == mask && sensor.getValidFields() == mask);
        assert(bus.calls - calls == 1 + bool(mask & 1) + bool(mask & 2) + bool(mask & 4));
        assert(waits == delays); // No delays in polling.
        if (mask & 1) {
            assert(sensor.getRawAcceleration()[0] == 256);
            assert(near(sensor.getAcceleration()[0], 256 * 0.488f));
            assert(near(sensor.getAcceleration()[1], -256 * 0.488f));
        }
        if (mask & 2) {
            assert(sensor.getRawAngularRate()[1] == -256);
            assert(near(sensor.getAngularRate()[0], 256 * 70.0f));
        }
        if (mask & 4) {
            assert(sensor.getRawTemperature() == 256);
            assert(near(sensor.getTemperature(), 26));
        }
    }
    // Failure on status or any of the three data reads preserves every cache.
    for (int failure = 1; failure <= 4; ++failure) {
        bus.failAt = -1;
        measurements(bus, 256);
        bus.regs[LSM6DSR_STATUS_REG] = 7;
        sensor.poll();
        measurements(bus, 512);
        bus.failAt = bus.calls + failure;
        sensor.poll();
        assert(sensor.status() == Sensor::ERROR && sensor.isInitialized());
        assert(sensor.getUpdatedFields() == 0 && sensor.getValidFields() == 7);
        assert(sensor.getRawAcceleration()[0] == 256);
        assert(sensor.getRawAngularRate()[0] == 256);
        assert(sensor.getRawTemperature() == 256);
        assert(near(sensor.getAcceleration()[0], 256 * 0.488f));
        assert(near(sensor.getAngularRate()[0], 256 * 70.0f));
        assert(near(sensor.getTemperature(), 26));
        bus.failAt = -1;
        sensor.poll();
        assert(sensor.status() == Sensor::NOMINAL && sensor.getUpdatedFields() == 7);
    }
    // Temperature-only updates preserve valid acceleration and gyro data.
    measurements(bus, -256);
    bus.regs[LSM6DSR_STATUS_REG] = 4;
    sensor.poll();
    assert(sensor.getUpdatedFields() == 4 && sensor.getValidFields() == 7);
    assert(sensor.getRawAcceleration()[0] == 512);
    assert(near(sensor.getTemperature(), 24));
    bus.regs[LSM6DSR_STATUS_REG] = 0;
    sensor.poll();
    assert(sensor.getUpdatedFields() == 0 && sensor.getValidFields() == 7);
    sensor.init();
    assert(sensor.getValidFields() == 0 && sensor.getUpdatedFields() == 0);
}
int main() {
    pollTests();
    Bus bus;
    IMU imu(context(bus), delay);
    imu.init();
    assert(imu.isInitialized() && imu.status() == Sensor::NOMINAL);
    assert((bus.regs[LSM6DSR_CTRL1_XL] & 0xFE) == 0x16);
    assert((bus.regs[LSM6DSR_CTRL2_G] & 0xFF) == 0x1C);
    assert((bus.regs[LSM6DSR_CTRL3_C] & 0x44) == 0x44);
    assert(bus.regs[LSM6DSR_INT1_CTRL] == 0 && bus.regs[LSM6DSR_INT2_CTRL] == 0);
    int operations = bus.calls;
    for (int failure = 1; failure <= operations; ++failure) {
        Bus failing;
        failing.failAt = failure;
        IMU sensor(context(failing), delay);
        sensor.init();
        assert(!sensor.isInitialized() && sensor.status() == Sensor::ERROR);
        // Retry after every failed register operation, including bank switches.
        failing.failAt = -1;
        sensor.init();
        assert(sensor.isInitialized() && sensor.status() == Sensor::NOMINAL);
    }
    bus.failAt = bus.calls + 1;
    imu.init();
    assert(!imu.isInitialized() && imu.status() == Sensor::ERROR);
    Bus wrong;
    wrong.regs[LSM6DSR_WHO_AM_I] = 0;
    IMU absent(context(wrong), delay);
    absent.init();
    assert(!absent.isInitialized() && absent.status() == Sensor::SENSOR_NOT_PRESENT);
    Bus stuck;
    stuck.stuck = true;
    waits = 0;
    IMU timeout(context(stuck), delay);
    timeout.init();
    assert(!timeout.isInitialized() && timeout.status() == Sensor::ERROR && waits == 600);
    stmdev_ctx_t missing{};
    IMU invalid(missing, delay);
    invalid.init();
    assert(!invalid.isInitialized() && invalid.status() == Sensor::ERROR);
    IMU missingDelay(context(wrong), nullptr);
    missingDelay.init();
    assert(!missingDelay.isInitialized() && missingDelay.status() == Sensor::ERROR);
    std::cout << "PASS: polling, conversions, freshness, read failures, configuration, " << operations << " injected bus failures, reset timeout, identity, missing callbacks, reinitialization\n";
}
