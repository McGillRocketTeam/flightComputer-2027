#pragma once

#include "sensors/Sensor.h"
#include "drivers/lsm6dsr/lsm6dsr_reg.h"
#include <cstdint>

class IMU : public Sensor {
public:
    using DelayFunction = void (*)(uint32_t milliseconds);

    IMU(stmdev_ctx_t context, DelayFunction delay);

    void init() override;
    void poll() override;

    ErrorCode status() override;
    bool isInitialized() override;

    const float* getAcceleration() const;
    const float* getAngularRate() const;
    float getTemperature() const;

private:
	//holds the bus callbacks and contex handle
    stmdev_ctx_t context_{};
	//let initialization wait without tying the class directly to a particular timing API.
    DelayFunction delay_;
	
    float acceleration_mg[3]{};
    float angular_rate_mdps[3]{};
    float temperature_degC = 0.0f;

    int16_t data_raw_acceleration[3]{};
    int16_t data_raw_angular_rate[3]{};
    int16_t data_raw_temperature = 0;

    bool initialized_ = false;
    ErrorCode error_ = ERROR;
};