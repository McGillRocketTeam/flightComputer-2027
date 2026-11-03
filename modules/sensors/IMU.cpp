#include "sensors/IMU.h"

IMU::IMU(stmdev_ctx_t context, DelayFunction delay)
    : context_(context), delay_(delay) {}

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

Sensor::ErrorCode IMU::status() {
    return error_;
}

bool IMU::isInitialized() {
    return initialized_;
}



void IMU::init() {
    // 1. Validate the bus callbacks and delay function.
    initialized_ = false;
    validFields_ = 0;
    updatedFields_ = 0;

    //The data sheet specifies a minimum delay of 35 millisecond.
    //2026 flight computer and prior uses a custom delay of 100.
    //We will use a custom delay of 100 milliseconds to ensure the sensor has time to boot.
    // Correction: 35 ms is a typical turn-on time, not a specified minimum.
    // The 100 ms delay follows the 2026 implementation.

    error_ = ERROR;

    if (!context_.write_reg || !context_.read_reg || !delay_) {
        return;
    }//Here it checks if the pointers are null, if the pointers are null, well, there is no sensors.
    // Correction: null callbacks mean missing software dependencies,
    // not necessarily a missing sensor. Keep error_ set to ERROR.

    // 2. Wait for the sensor to boot.
    delay_(100);

    // Restore the user bank in case an earlier configuration call failed
    // while accessing embedded-function registers.
    if (lsm6dsr_mem_bank_set(&context_, LSM6DSR_USER_BANK) != 0) {
        return;
    }

    // Apply the required interface settings before further access.
    // For the reference I2C setup, disable I3C and check the return value.
    if (lsm6dsr_i3c_disable_set(&context_, LSM6DSR_I3C_DISABLE) != 0) {
        return;
    }

    // 3. Check its device ID.
    uint8_t who_am_i = 0;
    int32_t result = lsm6dsr_device_id_get(&context_, &who_am_i);

    if (result != 0) {
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
    if (lsm6dsr_reset_set(&context_, PROPERTY_ENABLE) != 0) {
        return;
    }

    uint8_t resetting = 1;
    // Allow at most 500 one-millisecond waits, plus bus transaction time.
    // The bus callbacks must also enforce their own transaction timeouts.
    for (uint32_t attempt = 0; attempt < 500; ++attempt) {
        delay_(1);
        if (lsm6dsr_reset_get(&context_, &resetting) != 0) {
            return;
        }
        if (resetting == 0) {
            break;
        }
    }
    if (resetting != 0) {
        return;
    }

    if (lsm6dsr_i3c_disable_set(&context_, LSM6DSR_I3C_DISABLE) != 0) {
        return;
    }

    // 5. Configure measurement ranges, sample rates, and filters.
    if (lsm6dsr_xl_full_scale_set(&context_, LSM6DSR_16g) != 0) {
        return;
    }

    if (lsm6dsr_gy_full_scale_set(&context_, LSM6DSR_2000dps) != 0) {
        return;
    }

    if (lsm6dsr_xl_data_rate_set(&context_, LSM6DSR_XL_ODR_12Hz5) != 0) {
        return;
    }

    if (lsm6dsr_gy_data_rate_set(&context_, LSM6DSR_GY_ODR_12Hz5) != 0) {
        return;
    }

    // Enable block data update and register auto-increment.
    // Configure the required power modes, filters, FIFO behavior,
    // and interrupt routing. Check every driver's return value.
    if (lsm6dsr_block_data_update_set(&context_, PROPERTY_ENABLE) != 0 ||
        lsm6dsr_auto_increment_set(&context_, PROPERTY_ENABLE) != 0 ||
        lsm6dsr_xl_power_mode_set(&context_, LSM6DSR_HIGH_PERFORMANCE_MD) != 0 ||
        lsm6dsr_gy_power_mode_set(&context_, LSM6DSR_GY_HIGH_PERFORMANCE) != 0 ||
        lsm6dsr_fifo_mode_set(&context_, LSM6DSR_BYPASS_MODE) != 0 ||
        lsm6dsr_xl_filter_lp2_set(&context_, PROPERTY_ENABLE) != 0 ||
        lsm6dsr_xl_hp_path_on_out_set(&context_, LSM6DSR_LP_ODR_DIV_10) != 0 ||
        lsm6dsr_gy_filter_lp1_set(&context_, PROPERTY_DISABLE) != 0) {
        return;
    }

    // Polling does not require sensor interrupts. Explicitly disable routes;
    // GPIO/EXTI integration can later enable data-ready on the wired pin.
    lsm6dsr_pin_int1_route_t int1Route{};
    lsm6dsr_pin_int2_route_t int2Route{};
    if (lsm6dsr_pin_int1_route_set(&context_, &int1Route) != 0 ||
        lsm6dsr_pin_int2_route_set(&context_, &int2Route) != 0) {
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
    if (lsm6dsr_status_reg_get(&context_, &ready) != 0) {
        error_ = ERROR;
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
    if ((ready.xlda && lsm6dsr_acceleration_raw_get(&context_, acceleration) != 0) ||
        (ready.gda && lsm6dsr_angular_rate_raw_get(&context_, angularRate) != 0) ||
        (ready.tda && lsm6dsr_temperature_raw_get(&context_, &temperature) != 0)) {
        error_ = ERROR;
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
