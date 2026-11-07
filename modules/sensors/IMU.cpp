#include "sensors/IMU.h"
#include "stm32f4xx_hal.h"
#include <algorithm>

namespace {
constexpr uint32_t BOOT_DELAY_MS = 100;
// Conservative application budget inherited from the old 500 ms reset wait,
// not a datasheet maximum. Poll less often and include bus time in the deadline.
constexpr uint32_t RESET_TIMEOUT_MS = 500;
constexpr uint32_t RESET_POLL_INTERVAL_MS = 10;
constexpr uint32_t BUS_TIMEOUT_MS = 100;
constexpr uint16_t DEVICE_ADDRESS = LSM6DSR_I2C_ADD_L & 0xFEu;

int32_t readRegisters(void* handle, uint8_t reg, uint8_t* data, uint16_t length) {
    return HAL_I2C_Mem_Read(static_cast<I2C_HandleTypeDef*>(handle), DEVICE_ADDRESS,
                            reg, I2C_MEMADD_SIZE_8BIT, data, length, BUS_TIMEOUT_MS);
}

int32_t writeRegisters(void* handle, uint8_t reg, const uint8_t* data, uint16_t length) {
    return HAL_I2C_Mem_Write(static_cast<I2C_HandleTypeDef*>(handle), DEVICE_ADDRESS,
                             reg, I2C_MEMADD_SIZE_8BIT, const_cast<uint8_t*>(data),
                             length, BUS_TIMEOUT_MS);
}
} // namespace

IMU::IMU(I2C_HandleTypeDef* bus, DelayFunction delay)
    : context_{writeRegisters, readRegisters, bus}, delay_(delay) {}

//Getters
const float* IMU::getAcceleration() const {
    return acceleration_mg;
}

const float* IMU::getAngularRate() const {
    return angular_rate_mdps;
}

float IMU::getTemperature() const {
    return temperature_degC;
}

bool IMU::check(int32_t result, ErrorCode failure) {
    // ST callbacks use zero for success; keep this independent of HAL constants.
    if (result) {
        error_ = failure;
        return false;
    }
    return true;
}

Sensor::ErrorCode IMU::status() {
    return error_;
}

bool IMU::isInitialized() {
    return initialized_;
}



void IMU::init() {
    // 1. Validate the I2C handle and delay function.
    initialized_ = false;
    validFields_ = 0;
    updatedFields_ = 0;

    //The data sheet specifies a minimum delay of 35 millisecond.
    //2026 flight computer and prior uses a custom delay of 100.
    //We will use a custom delay of 100 milliseconds to ensure the sensor has time to boot.
    // Correction: 35 ms is a typical turn-on time, not a specified minimum.
    // The 100 ms delay follows the 2026 implementation.

    error_ = ERROR;

    if (!context_.handle || !delay_) {
        error_ = INVALID_ARGUMENT;
        return;
    }
    // The adapters are fixed in the constructor; null dependencies are a
    // software setup error, not evidence that the sensor is absent.

    // 2. Wait for the sensor to boot.
    delay_(BOOT_DELAY_MS);

    // Restore the user bank in case an earlier configuration call failed
    // while accessing embedded-function registers.
    if (!check(lsm6dsr_mem_bank_set(&context_, LSM6DSR_USER_BANK), IMU_INTERFACE_ERROR)) {
        return;
    }

    // Apply the required interface settings before further access.
    // For the reference I2C setup, disable I3C and check the return value.
    if (!check(lsm6dsr_i3c_disable_set(&context_, LSM6DSR_I3C_DISABLE), IMU_INTERFACE_ERROR)) {
        return;
    }

    // 3. Check its device ID.
    uint8_t who_am_i = 0;
    int32_t result = lsm6dsr_device_id_get(&context_, &who_am_i);

    if (!check(result, COMMUNICATION_ERROR)) {
        return;
    }

    if (who_am_i != LSM6DSR_ID) {
        error_ = SENSOR_NOT_PRESENT;
        return;
    }

    // 4. Reset it and wait for completion, with a timeout.
    // Request reset using lsm6dsr_reset_set().
    // Check its return value.
    // Repeatedly call lsm6dsr_reset_get(), checking each return value.
    // Stop when the reset bit clears, or return ERROR on timeout.
    // Reapply interface settings after reset.
    if (!check(lsm6dsr_reset_set(&context_, PROPERTY_ENABLE), IMU_RESET_ERROR)) {
        return;
    }

    uint8_t resetting = 1;
    // HAL_GetTick measures elapsed milliseconds, including bus transactions.
    // The budget can be exceeded by at most one bounded transaction. HAL tick
    // and the delay callback must remain operational during initialization.
    const uint32_t resetStart = HAL_GetTick();
    while (true) {
        const uint32_t elapsed = HAL_GetTick() - resetStart;
        if (elapsed >= RESET_TIMEOUT_MS) break;
        delay_(std::min(RESET_POLL_INTERVAL_MS, RESET_TIMEOUT_MS - elapsed));
        if (!check(lsm6dsr_reset_get(&context_, &resetting), IMU_RESET_ERROR)) {
            return;
        }
        if (!resetting) break;
    }
    if (resetting || HAL_GetTick() - resetStart >= RESET_TIMEOUT_MS) {
        error_ = IMU_RESET_TIMEOUT;
        return;
    }

    if (!check(lsm6dsr_i3c_disable_set(&context_, LSM6DSR_I3C_DISABLE), IMU_INTERFACE_ERROR)) {
        return;
    }

    // 5. Configure measurement ranges, sample rates, and filters.
    if (!check(lsm6dsr_xl_full_scale_set(&context_, LSM6DSR_16g), IMU_ACCEL_CONFIG_ERROR)) {
        return;
    }

    if (!check(lsm6dsr_gy_full_scale_set(&context_, LSM6DSR_2000dps), IMU_GYRO_CONFIG_ERROR)) {
        return;
    }

    if (!check(lsm6dsr_xl_data_rate_set(&context_, LSM6DSR_XL_ODR_12Hz5), IMU_ACCEL_CONFIG_ERROR)) {
        return;
    }

    if (!check(lsm6dsr_gy_data_rate_set(&context_, LSM6DSR_GY_ODR_12Hz5), IMU_GYRO_CONFIG_ERROR)) {
        return;
    }

    // Enable block data update and register auto-increment.
    // Configure the required power modes, filters, FIFO behavior,
    // and interrupt routing. Check every driver's return value.
    if (!check(lsm6dsr_block_data_update_set(&context_, PROPERTY_ENABLE), IMU_CONFIG_ERROR)) {
        return;
    }
    if (!check(lsm6dsr_auto_increment_set(&context_, PROPERTY_ENABLE), IMU_CONFIG_ERROR)) {
        return;
    }
    if (!check(lsm6dsr_xl_power_mode_set(&context_, LSM6DSR_HIGH_PERFORMANCE_MD), IMU_ACCEL_CONFIG_ERROR)) {
        return;
    }
    if (!check(lsm6dsr_gy_power_mode_set(&context_, LSM6DSR_GY_HIGH_PERFORMANCE), IMU_GYRO_CONFIG_ERROR)) {
        return;
    }
    if (!check(lsm6dsr_fifo_mode_set(&context_, LSM6DSR_BYPASS_MODE), IMU_CONFIG_ERROR)) {
        return;
    }
    if (!check(lsm6dsr_xl_filter_lp2_set(&context_, PROPERTY_ENABLE), IMU_FILTER_CONFIG_ERROR)) {
        return;
    }
    if (!check(lsm6dsr_xl_hp_path_on_out_set(&context_, LSM6DSR_LP_ODR_DIV_10), IMU_FILTER_CONFIG_ERROR)) {
        return;
    }
    if (!check(lsm6dsr_gy_filter_lp1_set(&context_, PROPERTY_DISABLE), IMU_FILTER_CONFIG_ERROR)) {
        return;
    }

    // Polling leaves sensor interrupt routes disabled. No EXTI binding is needed.
    lsm6dsr_pin_int1_route_t int1Route{};
    lsm6dsr_pin_int2_route_t int2Route{};
    if (!check(lsm6dsr_pin_int1_route_set(&context_, &int1Route), IMU_INTERRUPT_CONFIG_ERROR) ||
        !check(lsm6dsr_pin_int2_route_set(&context_, &int2Route), IMU_INTERRUPT_CONFIG_ERROR)) {
        return;
    }

    // 6. Record success or an error.
    initialized_ = true;
    error_ = NOMINAL;
}

void IMU::poll() {
    updatedFields_ = 0;
    if (!initialized_) {
        // Preserve the initialization failure reported by init().
        return;
    }

    lsm6dsr_status_reg_t ready{};
    if (!check(lsm6dsr_status_reg_get(&context_, &ready), COMMUNICATION_ERROR)) {
        return;
    }

    // No fresh data is normal between output periods, not a bus failure.
    if (!ready.xlda && !ready.gda && !ready.tda) {
        error_ = NOMINAL;
        return;
    }

    // 1. Read the raw sensor data.
    // Stage all requested reads so a failure cannot partially publish a sample.
    int16_t acceleration[3]{};
    int16_t angularRate[3]{};
    int16_t temperature = 0;
    if ((ready.xlda && lsm6dsr_acceleration_raw_get(&context_, acceleration)) ||
        (ready.gda && lsm6dsr_angular_rate_raw_get(&context_, angularRate)) ||
        (ready.tda && lsm6dsr_temperature_raw_get(&context_, &temperature))) {
        error_ = COMMUNICATION_ERROR;
        return;
    }

    // 2. Convert the raw data to physical units.
    // These conversions match the fixed ranges selected by init().
    if (ready.xlda) {
        for (unsigned axis = 0; axis < 3; ++axis) {
            data_raw_acceleration[axis] = acceleration[axis];
            acceleration_mg[axis] = lsm6dsr_from_fs16g_to_mg(acceleration[axis]);
        }
        updatedFields_ |= ACCELERATION;
    }
    if (ready.gda) {
        for (unsigned axis = 0; axis < 3; ++axis) {
            data_raw_angular_rate[axis] = angularRate[axis];
            angular_rate_mdps[axis] = lsm6dsr_from_fs2000dps_to_mdps(angularRate[axis]);
        }
        updatedFields_ |= ANGULAR_RATE;
    }
    if (ready.tda) {
        data_raw_temperature = temperature;
        temperature_degC = lsm6dsr_from_lsb_to_celsius(temperature);
        updatedFields_ |= TEMPERATURE;
    }
    validFields_ |= updatedFields_;
    error_ = NOMINAL;
}
