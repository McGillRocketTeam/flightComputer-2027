#include <cstdbool>
#include <cstdint>
#include <cstring>

#include "GPS.h"
#include "drivers/gps/gps_driver.h"

GPS::GPS() {
    memset(&_gps, 0, sizeof(gps_driver_t));
    _gps.status = GPS_DRIVER_NOT_INITIALIZED;
    _gps.initialized = false;
}

void GPS::init() { gps_driver_init(&_gps); }

Sensor::ErrorCode GPS::requestDMA(uint16_t len) {
    if (!isInitialized()) { return Sensor::ErrorCode::SENSOR_NOT_INITIALIZED; }
    return translateStatus(gps_driver_request_dma(&_gps, len));
}

Sensor::ErrorCode GPS::dump(const void *data) {
    if (!isInitialized()) { return Sensor::ErrorCode::SENSOR_NOT_INITIALIZED; }
    return translateStatus(gps_driver_dump(&_gps));
}

void GPS::poll() {
    if (!isInitialized()) { return; }
    gps_driver_poll(&_gps);
}

Sensor::ErrorCode GPS::factoryResetGPS() {
    if (!isInitialized()) { return Sensor::ErrorCode::SENSOR_NOT_INITIALIZED; }
    return translateStatus(gps_driver_factory_reset_gps(&_gps));
}

Sensor::ErrorCode GPS::disableNMEAMsgs(bool force) {
    if (!isInitialized()) { return Sensor::ErrorCode::SENSOR_NOT_INITIALIZED; }
    return translateStatus(gps_driver_disable_nmea_msgs(&_gps, force));
}

Sensor::ErrorCode GPS::configureDynamicModel(bool force) {
    if (!isInitialized()) { return Sensor::ErrorCode::SENSOR_NOT_INITIALIZED; }
    return translateStatus(gps_driver_configure_dynamic_model(&_gps, force));
}

Sensor::ErrorCode GPS::parseDMABuf() {
    if (!isInitialized()) { return Sensor::ErrorCode::SENSOR_NOT_INITIALIZED; }
    return translateStatus(gps_driver_parse_dma_buf(&_gps));
}

Sensor::ErrorCode GPS::status() { return translateStatus(_gps.status); }

bool GPS::isInitialized() { return _gps.initialized; }
bool GPS::isNavConfigured() const { return _gps.nav_configured; }
bool GPS::isNMEADisabled() const { return _gps.nmea_disabled; }
bool GPS::hasFix() const { return _gps.has_fix; }

GPSPosition GPS::getPosition() const {
    return {_gps.latitude, _gps.longitude, _gps.altitude};
}
GPSTime GPS::getTime() const {
    return {_gps.hours, _gps.minutes, _gps.seconds};
}
uint32_t GPS::getLastUpdate() const { return _gps.last_update; }
uint8_t GPS::getFixType() const { return _gps.fix_type; }

Sensor::ErrorCode translateStatus(gps_status_t status) {
    switch (status) {
    case GPS_DRIVER_OK:
        return Sensor::ErrorCode::NOMINAL;
    case GPS_DRIVER_NOT_INITIALIZED:
        return Sensor::ErrorCode::SENSOR_NOT_INITIALIZED;
    case GPS_DRIVER_INVALID_ARG:
        return Sensor::ErrorCode::GPS_INVALID_ARGUMENT;
    case GPS_DRIVER_UART_ERROR:
        return Sensor::ErrorCode::GPS_UART_ERROR;
    case GPS_DRIVER_TIMEOUT:
        return Sensor::ErrorCode::GPS_TIMEOUT;
    case GPS_DRIVER_BUFFER_OVERFLOW:
        return Sensor::ErrorCode::GPS_BUFFER_OVERFLOW;
    case GPS_DRIVER_INVALID_PACKET:
        return Sensor::ErrorCode::GPS_INVALID_PACKET;
    case GPS_DRIVER_NACK:
        return Sensor::ErrorCode::GPS_NACK;
    case GPS_DRIVER_ERROR:
    default:
        return Sensor::ErrorCode::ERROR;
    }
}
