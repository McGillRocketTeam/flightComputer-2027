#include "sensors/IMU.h"

IMU::IMU(stmdev_ctx_t context, DelayFunction delay)
    : context_(context), delay_(delay) {}

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
    // 2. Wait for the sensor to boot.
    // 3. Check its device ID.
    // 4. Reset it and wait for completion, with a timeout.
    // 5. Configure measurement ranges, sample rates, and filters.
    // 6. Record success or an error.
}