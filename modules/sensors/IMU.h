#pragma once

#include "sensors/Sensor.h"
#include "drivers/lsm6dsr/lsm6dsr_reg.h"
#include <cstdint>

// This module owns fixed STM32 HAL I2C adapters.
#include "stm32f4xx_hal.h"

class IMU : public Sensor {
public:
    using DelayFunction = void (*)(uint32_t milliseconds);
    IMU(I2C_HandleTypeDef* bus, DelayFunction delay);

    void init() override;
    // Blocking HAL I2C: call from task/main context, never directly from an ISR.
    // An ISR should notify the task that polls. Serialize init/poll/getters.
    void poll() override;
    ErrorCode status() override;
    bool isInitialized() override;

    // Copy X/Y/Z in mg or mdps, and temperature in degrees Celsius.
    // Return false without changing output if the latest poll lacks this field.
    bool getAcceleration(float (&output)[3]) const;
    bool getAngularRate(float (&output)[3]) const;
    bool getTemperature(float& output) const;
    // Raw outputs are signed sensor counts (LSB), not physical units.
    bool getRawAcceleration(int16_t (&output)[3]) const;
    bool getRawAngularRate(int16_t (&output)[3]) const;
    bool getRawTemperature(int16_t& output) const;

    enum Field : uint8_t {
        ACCELERATION = (1u << 0),
        ANGULAR_RATE = (1u << 1),
        TEMPERATURE = (1u << 2)
    };
    // Both masks describe the most recent poll, not lifetime availability.
    uint8_t getValidFields() const { return validFields_; }
    uint8_t getUpdatedFields() const { return updatedFields_; }

private:
    bool check(int32_t result, ErrorCode failure);
    // Fixed HAL adapters; only the bus handle varies per instance.
    stmdev_ctx_t context_{};
    // Supplied by the caller so startup can precede kernel initialization.
    DelayFunction delay_;
    float acceleration_mg[3]{};       // mg, X/Y/Z
    float angular_rate_mdps[3]{};     // mdps, X/Y/Z
    float temperature_degC = 0.0f;    // degrees Celsius
    int16_t data_raw_acceleration[3]{}; // signed sensor counts, X/Y/Z
    int16_t data_raw_angular_rate[3]{}; // signed sensor counts, X/Y/Z
    int16_t data_raw_temperature = 0;  // signed sensor counts
    uint8_t validFields_ = 0;
    uint8_t updatedFields_ = 0;
    bool initialized_ = false;
    ErrorCode error_ = SENSOR_NOT_INITIALIZED;
};
