#include <cstdint>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "GPS/gps_driver.h"
#include "sensors/GPS.h"

GPS::GPS(const GPS_Config_t &config) {
    memset(&gps, 0, sizeof(GPS_Driver_t));
    this->config = config;
}

void GPS::init() { GPS_Driver_init(&gps, &config); }

void GPS::startDMA(uint16_t len) { GPS_Driver_startDMA(&gps, len); }

void GPS::dump(const void *data, size_t size) {
    GPS_Driver_dump(&gps, data, size);
}

void GPS::poll() { GPS_Driver_poll(&gps); }

void GPS::disableNMEAMsgs() { GPS_Driver_disable_nmea_msgs(&gps); }

void GPS::factoryResetGPS() { GPS_Driver_factory_reset_gps(&gps); }

void GPS::configureDynamicModel() { GPS_Driver_configure_dynamic_model(&gps); }

void GPS::parseDMABuf() { GPS_Driver_parse_dma_buf(&gps); }

/* Getters */

Sensor::ErrorCode GPS::status() { return Sensor::ErrorCode::ERROR; }

bool GPS::isInitialized() { return gps.initialized; }

GPSPosition GPS::getPosition() const {
    return {gps.latitude, gps.longitude, gps.altitude};
}

GPSTime GPS::getTime() const { return {gps.hours, gps.minutes, gps.seconds}; }

uint32_t GPS::getLastUpdate() const { return gps.last_update; }
