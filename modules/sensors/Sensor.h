#pragma once

class Sensor {
  public:
    enum ErrorCode {
        NOMINAL = 0,
        ERROR,
        SENSOR_NOT_PRESENT,
        SENSOR_NOT_INITIALIZED,

        // GPS driver error codes
        GPS_INVALID_ARGUMENT,
        GPS_UART_ERROR,
        GPS_TIMEOUT,
        GPS_BUFFER_OVERFLOW,
        GPS_INVALID_PACKET,
        GPS_NACK,
    };

    virtual ~Sensor() = default;

    virtual void init() = 0;
    virtual ErrorCode status() = 0;
    virtual bool isInitialized() = 0;

    virtual void poll() = 0;

  protected:
    Sensor() = default;
    Sensor(const Sensor &) = delete;
    Sensor &operator=(const Sensor &) = delete;
};
