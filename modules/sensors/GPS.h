#pragma once
#include <cstdint>

#include "Sensor.h"
#include "drivers/gps/gps_driver.h"

struct GPSPosition {
    float latitude;
    float longitude;
    float altitude;
};

struct GPSTime {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
};

class GPS : Sensor {
  public:
    explicit GPS();
    ~GPS() = default;

    void init() override;
    Sensor::ErrorCode requestDMA(uint16_t len);

    Sensor::ErrorCode dump(const void *data);

    void poll() override;
    Sensor::ErrorCode reset();
    Sensor::ErrorCode disableNMEAMsgs(bool force = false);
    Sensor::ErrorCode configureDynamicModel(bool force = false);
    Sensor::ErrorCode parseDMABuf();

    ErrorCode status() override;
    bool isInitialized() override;
    bool isNavConfigured() const;
    bool isNMEADisabled() const;
    bool hasFix() const;
    GPSPosition getPosition() const;
    GPSTime getTime() const;
    uint32_t getLastUpdate() const;
    uint8_t getFixType() const;

  private:
    Sensor::ErrorCode translateStatus(gps_status_t status);

    gps_driver_t _gps;
};
