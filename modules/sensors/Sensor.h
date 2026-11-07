#pragma once

class Sensor {
   public:
    enum ErrorCode {
        NOMINAL = 0,
        ERROR,
        SENSOR_NOT_PRESENT,
        INVALID_ARGUMENT,
        SENSOR_NOT_INITIALIZED,
        COMMUNICATION_ERROR,
        IMU_INTERFACE_ERROR,
        IMU_RESET_ERROR,
        IMU_RESET_TIMEOUT,
        IMU_ACCEL_CONFIG_ERROR,
        IMU_GYRO_CONFIG_ERROR,
        IMU_FILTER_CONFIG_ERROR,
        IMU_CONFIG_ERROR,
        IMU_INTERRUPT_CONFIG_ERROR,
    };

    virtual ~Sensor() = default;

    virtual void init() = 0;
    virtual ErrorCode status() = 0;
    virtual bool isInitialized() = 0;

    virtual void poll() = 0;

   protected:
    Sensor() = default;
    Sensor(const Sensor&) = delete;
    Sensor& operator=(const Sensor&) = delete;
};
