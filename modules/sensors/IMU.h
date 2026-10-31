#pragma once

#include "sensors/Sensor.h"
#include "drivers/lsm6dsr/lsm6dsr_reg.h"
#include <cstdint>

class IMU : public Sensor {
public:
    using DelayFunction = void (*)(uint32_t milliseconds);

    IMU(stmdev_ctx_t context, DelayFunction delay);

    void init() override;
    // Synchronous: call directly from an ISR only with ISR-safe bus callbacks.
    // Otherwise notify a task to poll. Serialize poll/getters across contexts.
    void poll() override;

    ErrorCode status() override;
    bool isInitialized() override;

    // Cached X/Y/Z in mg and mdps; temperature in degrees Celsius.
    const float* getAcceleration() const;
    const float* getAngularRate() const;
    float getTemperature() const;
    const int16_t* getRawAcceleration() const { return data_raw_acceleration; }
    const int16_t* getRawAngularRate() const { return data_raw_angular_rate; }
    int16_t getRawTemperature() const { return data_raw_temperature; }

    enum Field : uint8_t { ACCELERATION = 1, ANGULAR_RATE = 2, TEMPERATURE = 4 };
    // Valid fields have been collected since init; updated fields belong to
    // the last poll. Different sensors may become ready independently.
    uint8_t getValidFields() const { return validFields_; }
    uint8_t getUpdatedFields() const { return updatedFields_; }

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

    uint8_t validFields_ = 0;
    uint8_t updatedFields_ = 0;
    bool initialized_ = false;
    ErrorCode error_ = ERROR;
};