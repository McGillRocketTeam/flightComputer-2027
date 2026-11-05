#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "gps_driver.h"
#include "u_ubx_protocol/u_ubx_protocol.h"

bool GPS_Driver_init(GPS_Driver_t *self, const GPS_Config_t *config) {
    self->latitude = 0.0f;
    self->longitude = 0.0f;
    self->altitude = 0.0f;

    self->hours = 0;
    self->minutes = 0;
    self->seconds = 0;

    self->last_update = UINT32_MAX;

    memset(self->dma_buf, 0, sizeof(self->dma_buf));

    self->uart_bus = config->uart_bus;
    self->uart_write = config->uart_write;
    self->uart_read = config->uart_read;
    self->printf = config->printf;
    self->delay = config->delay;

    self->nmea_disabled = false;
    self->nav_configured = false;

    self->delay(50);
    GPS_Driver_configure_dynamic_model(self);
    GPS_Driver_startDMA(self, 10);

    self->initialized = true;
    return true;
}

void GPS_Driver_startDMA(GPS_Driver_t *self, uint16_t len) {
    self->uart_read(self->uart_bus, self->dma_buf, len);
}

void GPS_Driver_dump(GPS_Driver_t *self, const void *data, size_t size);

void GPS_Driver_poll(GPS_Driver_t *self);
void GPS_Driver_disable_nmea_msgs(GPS_Driver_t *self);
void GPS_Driver_factory_reset_gps(GPS_Driver_t *self);
void GPS_Driver_configure_dynamic_model(GPS_Driver_t *self);
void GPS_Driver_parse_dma_buf(GPS_Driver_t *self);
