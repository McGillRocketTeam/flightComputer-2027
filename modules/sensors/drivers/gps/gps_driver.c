#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "gps_driver.h"
#include "u_ubx_protocol.h"

// Initializes the GPS driver.
gps_status_t gps_driver_init(gps_driver_t *self, const gps_config_t *config) {
    if (self == NULL) { return GPS_DRIVER_CONFIG_ERROR; }

    self->initialized = false;
    self->status = GPS_DRIVER_OK;

    if (config == NULL || config->printf == NULL) {
        return GPS_DRIVER_CONFIG_ERROR;
    }
    self->printf = config->printf;

    if (config->delay == NULL || config->uart_bus == NULL ||
        config->uart_write == NULL || config->uart_read == NULL ||
        config->get_tick == NULL) {
        self->printf("[GPS] Invalid configuration (null pointer).\r\n");
        self->status = GPS_DRIVER_CONFIG_ERROR;
        return GPS_DRIVER_CONFIG_ERROR;
    }
    self->delay = config->delay;
    self->uart_bus = config->uart_bus;
    self->uart_write = config->uart_write;
    self->uart_read = config->uart_read;
    self->get_tick = config->get_tick;

    // clear DMA buffer
    memset(self->dma_buf, 0, GPS_DMA_BUF_SIZE);

    // initialize other variables
    self->latitude = 0.0f;
    self->longitude = 0.0f;
    self->altitude = 0.0f;
    self->hours = 0;
    self->minutes = 0;
    self->seconds = 0;
    self->last_update = UINT32_MAX;

    self->nmea_disabled = false;
    self->nav_configured = false;

    self->delay(50);

    gps_status_t err = gps_driver_configure_dynamic_model(self, true);
    if (err != GPS_DRIVER_OK) {
        self->status = err;
        return err;
    }

    self->initialized = true;
    self->status = GPS_DRIVER_OK;
    self->printf("[GPS] Initialized GPS Driver.\r\n");
    return GPS_DRIVER_OK;
}

// Start background GPS DMA read
gps_status_t gps_driver_request_dma(gps_driver_t *self, uint16_t len) {
    if (self == NULL || self->uart_read == NULL || self->uart_bus == NULL) {
        return GPS_DRIVER_CONFIG_ERROR;
    }
    if (len > GPS_DMA_BUF_SIZE) {
        self->status = GPS_DRIVER_BUFFER_OVERFLOW;
        return GPS_DRIVER_BUFFER_OVERFLOW;
    }
    memset(self->dma_buf, 0, GPS_DMA_BUF_SIZE);
    if (self->uart_read(self->uart_bus, self->dma_buf, len) != 0) {
        self->status = GPS_DRIVER_ERROR;
        return GPS_DRIVER_ERROR;
    }
    return GPS_DRIVER_OK;
}

// Polls GPS driver and stores data in DMA buffer
gps_status_t gps_driver_poll(gps_driver_t *self) {
    if (self == NULL || self->printf == NULL || self->uart_write == NULL ||
        self->uart_bus == NULL || self->delay == NULL) {
        return GPS_DRIVER_CONFIG_ERROR;
    }

    // UBX-NAV-PVT Navigation position velocity time message (request frame)
    const uint8_t ubx_nav_pvt[8] = {UBX_SYNC_CHAR1,
                                    UBX_SYNC_CHAR2,
                                    MSG_CLASS_UBX_NAV,
                                    MSG_ID_UBX_NAV_PVT,
                                    0x00,  // length
                                    0x00,  // payload
                                    0x08,  // checksum A
                                    0x19}; // checksum B

    gps_status_t err = gps_driver_request_dma(
        self, MSG_LEN_UBX_NAV_PVT + U_UBX_PROTOCOL_OVERHEAD_LENGTH_BYTES);
    if (err != GPS_DRIVER_OK) {
        self->status = err;
        return err;
    }

    if (self->uart_write(self->uart_bus, ubx_nav_pvt, sizeof(ubx_nav_pvt)) !=
        0) {
        self->status = GPS_DRIVER_ERROR;
        return GPS_DRIVER_ERROR;
    }
    self->delay(30);

    err = gps_driver_parse_dma_buf(self); // FIX: race condition?
    if (err != GPS_DRIVER_OK) {
        self->status = err;
        return err;
    }

    return GPS_DRIVER_OK;
}

gps_status_t gps_driver_disable_nmea_msgs(gps_driver_t *self);
gps_status_t gps_driver_factory_reset_gps(gps_driver_t *self);

// Configure GPS navigation engine settings to dynamic platform model
gps_status_t gps_driver_configure_dynamic_model(gps_driver_t *self,
                                                bool force) {
    if (self == NULL || self->printf == NULL || self->uart_write == NULL ||
        self->uart_bus == NULL || self->delay == NULL) {
        return GPS_DRIVER_CONFIG_ERROR;
    }
    if (self->nav_configured && !force) {
        self->printf("[GPS] Dynamic model already configured.\r\n");
        return GPS_DRIVER_OK;
    }

    uint8_t message[MSG_LEN_UBX_CFG_NAV5] = {0};

    // set bitmask
    uint16_t mask = BITMASK_UBX_CFG_NAV5_dyn | BITMASK_UBX_CFG_NAV5_posFixMode;
    message[0] = (uint8_t)(mask & 0xFFU);
    message[1] = (uint8_t)((mask >> 8) & 0xFFU);

    // set dynamic model and fix mode
    message[2] = DYNAMIC_PLATFORM_MODEL_AIRBORNE_4G;
    message[3] = FIXMODE_AUTO_2D_3D;

    // set fixed altitude variance (fixedAltVar)
    // offset 8, store 10 27 as little endian, which is 0.0001 in decimal
    uint32_t fixed_alt_var = 0x2710U;
    message[8] = (uint8_t)(fixed_alt_var & 0xFFU);
    message[9] = (uint8_t)((fixed_alt_var >> 8) & 0xFFU);
    message[10] = (uint8_t)((fixed_alt_var >> 16) & 0xFFU);
    message[11] = (uint8_t)((fixed_alt_var >> 24) & 0xFFU);

    // encode frame
    char encoded[MSG_LEN_UBX_CFG_NAV5 + U_UBX_PROTOCOL_OVERHEAD_LENGTH_BYTES] =
        {0};
    int32_t bytes_encoded = uUbxProtocolEncode(
        MSG_CLASS_UBX_CFG, MSG_ID_UBX_CFG_NAV5, (const char *)message,
        MSG_LEN_UBX_CFG_NAV5, encoded);
    if (bytes_encoded <= 0) {
        self->status = GPS_DRIVER_CONFIG_ERROR;
        return GPS_DRIVER_CONFIG_ERROR;
    }

    self->nav_configured = false;
    for (uint8_t i = 0; i < GPS_NUM_ATTEMPTS; ++i) {
        self->printf(
            "[GPS] Attempting to configure dynamic model (attempt %d/%d).\r\n",
            i + 1, GPS_NUM_ATTEMPTS);

        gps_status_t err = gps_driver_request_dma(
            self, MSG_LEN_UBX_ACK_ACK + U_UBX_PROTOCOL_OVERHEAD_LENGTH_BYTES);
        if (err != GPS_DRIVER_OK) {
            self->status = err;
            continue;
        }

        if (self->uart_write(self->uart_bus, (const uint8_t *)encoded,
                             bytes_encoded) != 0) {
            self->status = GPS_DRIVER_ERROR;
            continue;
        }

        self->delay(25);

        err = gps_driver_parse_dma_buf(self); // FIX: race condition?
        if (err != GPS_DRIVER_OK) {
            self->status = err;
            continue;
        }

        if (self->nav_configured) {
            self->printf("[GPS] Successfully configured dynamic model.\r\n");
            self->status = GPS_DRIVER_OK;
            return GPS_DRIVER_OK;
        }

        self->delay(20);
    }

    self->printf("[GPS] Failed to configure dynamic model (timeout).\r\n");
    self->status = GPS_DRIVER_TIMEOUT;
    return GPS_DRIVER_TIMEOUT;
}

gps_status_t gps_driver_parse_dma_buf(gps_driver_t *self);

// Prints a hex dump of the entire GPS DMA buffer.
gps_status_t gps_driver_dump(gps_driver_t *self) {
    if (self == NULL || self->printf == NULL) {
        return GPS_DRIVER_CONFIG_ERROR;
    }
    const unsigned char *p = (const unsigned char *)self->dma_buf;

    self->printf("[GPS] Dump of DMA buffer (%u bytes)\r\n",
                 (unsigned int)GPS_DMA_BUF_SIZE);

    for (size_t i = 0; i < GPS_DMA_BUF_SIZE; i += 16) {
        // each byte is formatted as 2 hex digits + 1 space (3 chars)
        // 50B = 16B * 3 chars + 1 space char (in middle) + null terminator
        // set to 64 to avoid any overflow/underflow
        char hex[64] = {0};
        uint8_t hex_idx = 0;
        char ascii[17] = {0};

        for (size_t j = 0; j < 16; ++j) {
            if (i + j < GPS_DMA_BUF_SIZE) {
                unsigned char c = p[i + j];
                hex_idx +=
                    snprintf(hex + hex_idx, sizeof(hex) - hex_idx, "%02X ", c);
                ascii[j] = (c >= ' ' && c <= '~') ? c : '.';
            } else {
                // pad spaces for incomplete rows
                hex_idx +=
                    snprintf(hex + hex_idx, sizeof(hex) - hex_idx, "   ");
                ascii[j] = ' ';
            }
            if (j == 7) {
                // add extra space after every 8 bytes
                hex_idx += snprintf(hex + hex_idx, sizeof(hex) - hex_idx, " ");
            }
        }
        ascii[16] = '\0';

        self->printf("0x%04X: %s |  %s\r\n", (unsigned int)i, hex, ascii);
    }
    self->printf("\r\n");
    return GPS_DRIVER_OK;
}
