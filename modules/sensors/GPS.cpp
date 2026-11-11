#include <cstdbool>
#include <cstdint>
#include <cstring>

#include "GPS.h"
#include "drivers/gps/gps_driver.h"

GPS::GPS() {
    memset(&_gps, 0, sizeof(gps_driver_t));
}

void GPS::init() { gps_driver_init(&_gps); }

Sensor::ErrorCode GPS::requestDMA(uint16_t len) {
    return translateStatus(gps_driver_request_dma(&_gps, len));
}

Sensor::ErrorCode GPS::dump(const void *data) {
    return translateStatus(gps_driver_dump(&_gps));
}

void GPS::poll() { gps_driver_poll(&_gps); }

Sensor::ErrorCode GPS::disableNMEAMsgs() {
    return translateStatus(gps_driver_disable_nmea_msgs(&_gps));
}

Sensor::ErrorCode GPS::factoryResetGPS() {
    return translateStatus(gps_driver_factory_reset_gps(&_gps));
}

Sensor::ErrorCode GPS::configureDynamicModel(bool force) {
    return translateStatus(gps_driver_configure_dynamic_model(&_gps, force));
}

Sensor::ErrorCode GPS::parseDMABuf() {
    return translateStatus(gps_driver_parse_dma_buf(&_gps));
}

Sensor::ErrorCode GPS::status() { return translateStatus(_gps.status); }

bool GPS::isInitialized() { return _gps.initialized; }

GPSPosition GPS::getPosition() const {
    return {_gps.latitude, _gps.longitude, _gps.altitude};
}

GPSTime GPS::getTime() const {
    return {_gps.hours, _gps.minutes, _gps.seconds};
}

uint32_t GPS::getLastUpdate() const { return _gps.last_update; }

Sensor::ErrorCode translateStatus(gps_status_t status) {
    switch (status) {
    case GPS_DRIVER_OK:
        return Sensor::ErrorCode::NOMINAL;
    case GPS_DRIVER_CONFIG_ERROR:
        return Sensor::ErrorCode::GPS_CONFIG_ERROR;
    case GPS_DRIVER_TIMEOUT:
        return Sensor::ErrorCode::GPS_TIMEOUT;
    case GPS_DRIVER_BUFFER_OVERFLOW:
        return Sensor::ErrorCode::GPS_BUFFER_OVERFLOW;
    case GPS_DRIVER_ERROR:
    default:
        return Sensor::ErrorCode::ERROR;
    }
}
