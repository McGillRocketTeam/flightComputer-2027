#pragma once
#include "sensors/Sensor.h"
#include "sensors/GPS/gps_driver.h"
#include <stdint.h>

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
        explicit GPS(const GPS_Config_t& config);
        ~GPS() = default;

        void init() override;
        void startDMA(uint16_t len);

        void dump(const void *data, size_t size);

        void poll() override;
        void disableNMEAMsgs();
        void factoryResetGPS();
        void configureDynamicModel();
        void parseDMABuf();

        ErrorCode status() override;
        bool isInitialized() override;
        GPSPosition getPosition() const;
        GPSTime getTime() const;
        uint32_t getLastUpdate() const;

    private:
        GPS_Driver_t gps;
        GPS_Config_t config;
};
