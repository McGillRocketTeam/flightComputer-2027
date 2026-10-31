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
    //The data sheet specifies a minimum delay of 35 millisecond.
    //2026 flight computer and prior uses a custom delay of 100.
    //We will use a custom delay of 100 milliseconds to ensure the sensor has time to boot.
    
    error_ = ERROR;
    if (!context_.write_reg || !context_.read_reg || !delay_) {
        error_ = SENSOR_NOT_PRESENT;
        return;
    }//Here it checks if the pointers are null, if the pointers are null, well, there is no sensors.
    // 2. Wait for the sensor to boot.
    delay_(100);
    // 3. Check its device ID.
    uint8_t who_am_i = 0;
    int32_t result = lsm6dsr_device_id_get(&context_, &who_am_i);    // 4. Reset it and wait for completion, with a timeout.
    // 5. Configure measurement ranges, sample rates, and filters.
    if (result != 0) {
    error_ = ERROR;
    return;
    // 6. Record success or an error.
    }
}

    