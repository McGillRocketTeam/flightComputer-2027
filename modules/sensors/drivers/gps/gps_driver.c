#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "gps_driver.h"
#include "main.h"
#include "sensors/drivers/gps/ubx_messages.h"
#include "stm32f4xx_hal_def.h"
#include "stm32f4xx_hal_uart.h"
#include "u_ubx_protocol.h"
#include "usart.h"
#include "util/helpers.h"

// Initializes the GPS driver.
gps_status_t gps_driver_init(gps_driver_t *self) {
    if (self == NULL) { return GPS_DRIVER_INVALID_ARG; }
    self->initialized = false;
    self->status = GPS_DRIVER_NOT_INITIALIZED;

    // Assign hardware abstration callbcks
    self->printf = printf;
    self->delay = delay;
    self->get_tick = getMillis;
    self->uart_write = gps_uart_write;
    self->uart_read = gps_uart_read;
    self->uart_bus = &huart6;

    // Clear DMA buffer
    memset(self->dma_buf, 0, GPS_DMA_BUF_SIZE);

    // Clear all data
    self->latitude = 0.0f;
    self->longitude = 0.0f;
    self->altitude = 0.0f;
    self->hours = 0;
    self->minutes = 0;
    self->seconds = 0;
    self->last_update = UINT32_MAX;
    self->nmea_disabled = false;
    self->nav_configured = false;

    // Get baud rate
    self->baud_rate = ((UART_HandleTypeDef *)self->uart_bus)->Init.BaudRate;

    // Mark as initialized
    self->initialized = true;
    self->status = GPS_DRIVER_OK;

    // Disable NMEA messages
    // gps_status_t err = gps_driver_disable_nmea_msgs(self, true);
    // if (err != GPS_DRIVER_OK) {
    //     self->status = err;
    //     return err;
    // }

    self->delay(50);

    // Configure Airborne 4G dynamic platform model
    gps_status_t err = gps_driver_configure_dynamic_model(self, true);
    if (err != GPS_DRIVER_OK) {
        self->status = err;
        return err;
    }

    self->printf("[GPS] Initialized GPS Driver.\r\n");
    return GPS_DRIVER_OK;
}

// Start background GPS DMA read
gps_status_t gps_driver_request_dma(gps_driver_t *self, uint16_t len) {
    if (self == NULL) { return GPS_DRIVER_INVALID_ARG; }
    if (!self->initialized || self->status == GPS_DRIVER_NOT_INITIALIZED) {
        self->initialized = false;
        self->status = GPS_DRIVER_NOT_INITIALIZED;
        return GPS_DRIVER_NOT_INITIALIZED;
    }
    if (len > GPS_DMA_BUF_SIZE) {
        self->status = GPS_DRIVER_BUFFER_OVERFLOW;
        return GPS_DRIVER_BUFFER_OVERFLOW;
    }

    // Clear DMA buffer before reading from DMA and overwriting the buffer
    memset(self->dma_buf, 0, GPS_DMA_BUF_SIZE);

    if (self->uart_read(self->uart_bus, self->dma_buf, len) != HAL_OK) {
        self->status = GPS_DRIVER_UART_ERROR;
        return GPS_DRIVER_UART_ERROR;
    }
    return GPS_DRIVER_OK;
}

// Polls GPS driver and stores data in DMA buffer
gps_status_t gps_driver_poll(gps_driver_t *self) {
    if (self == NULL) { return GPS_DRIVER_INVALID_ARG; }
    if (!self->initialized || self->status == GPS_DRIVER_NOT_INITIALIZED) {
        self->initialized = false;
        self->status = GPS_DRIVER_NOT_INITIALIZED;
        return GPS_DRIVER_NOT_INITIALIZED;
    }

    // UBX-NAV-PVT Navigation position velocity time message (request frame)
    char encoded[U_UBX_PROTOCOL_OVERHEAD_LENGTH_BYTES] = {0};
    int32_t bytes_encoded = uUbxProtocolEncode(
        MSG_CLASS_UBX_NAV, MSG_ID_UBX_NAV_PVT, 0, 0, encoded);

    gps_status_t err = gps_driver_request_dma(
        self, MSG_LEN_UBX_NAV_PVT + U_UBX_PROTOCOL_OVERHEAD_LENGTH_BYTES);
    if (err != GPS_DRIVER_OK) {
        self->status = err;
        return err;
    }

    if (self->uart_write(self->uart_bus, (const uint8_t *)encoded,
                         bytes_encoded, 20) != HAL_OK) {
        self->status = GPS_DRIVER_UART_ERROR;
        return GPS_DRIVER_UART_ERROR;
    }

    self->delay(30); // leave time for callback to parse DMA buffer
    return GPS_DRIVER_OK;
}

// Hot restart the GPS
gps_status_t gps_driver_factory_reset_gps(gps_driver_t *self) {
    if (self == NULL) { return GPS_DRIVER_INVALID_ARG; }
    if (!self->initialized || self->status == GPS_DRIVER_NOT_INITIALIZED) {
        self->initialized = false;
        self->status = GPS_DRIVER_NOT_INITIALIZED;
        return GPS_DRIVER_NOT_INITIALIZED;
    }

    // UBX-CFG-RST for hot reset
    uint8_t message[MSG_LEN_UBX_CFG_RST] = {0};

    // encode frame
    char encoded[MSG_LEN_UBX_CFG_RST + U_UBX_PROTOCOL_OVERHEAD_LENGTH_BYTES] = {
        0};
    int32_t bytes_encoded =
        uUbxProtocolEncode(MSG_CLASS_UBX_CFG, MSG_ID_UBX_CFG_RST,
                           (const char *)message, MSG_LEN_UBX_CFG_RST, encoded);
    if (bytes_encoded <= 0) {
        self->status = GPS_DRIVER_INVALID_ARG;
        return GPS_DRIVER_INVALID_ARG;
    }

    if (self->uart_write(self->uart_bus, (const uint8_t *)encoded,
                         bytes_encoded, 100) != 0) {
        self->status = GPS_DRIVER_UART_ERROR;
        return GPS_DRIVER_UART_ERROR;
    }

    self->delay(1000);

    self->nav_configured = false;
    self->nmea_disabled = false;
    self->status = GPS_DRIVER_OK;

    self->printf(
        "[GPS] Factory reset complete. Receiver restored to default.\r\n");
    return GPS_DRIVER_OK;
}

// Configures the GPS to have NMEA messages disabled.
gps_status_t gps_driver_disable_nmea_msgs(gps_driver_t *self, bool force) {
    if (self == NULL) { return GPS_DRIVER_INVALID_ARG; }
    if (!self->initialized || self->status == GPS_DRIVER_NOT_INITIALIZED) {
        self->initialized = false;
        self->status = GPS_DRIVER_NOT_INITIALIZED;
        return GPS_DRIVER_NOT_INITIALIZED;
    }
    if (self->nmea_disabled && !force) {
        self->printf("[GPS] NMEA messages already disabled.\r\n");
        return GPS_DRIVER_OK;
    }

    // UBX-CFG-PRT for UART ports
    uint8_t message[MSG_LEN_UBX_CFG_PRT] = {0};
    message[0] = UBX_PORT_UART1;

    uint32_t mode = 0x000008D0U;
    message[4] = (uint8_t)(mode & 0xFFU);
    message[5] = (uint8_t)((mode >> 8) & 0xFFU);
    message[6] = (uint8_t)((mode >> 16) & 0xFFU);
    message[7] = (uint8_t)((mode >> 24) & 0xFFU);

    uint32_t baud_rate = self->baud_rate;
    message[8] = (uint8_t)(baud_rate & 0xFFU);
    message[9] = (uint8_t)((baud_rate >> 8) & 0xFFU);
    message[10] = (uint8_t)((baud_rate >> 16) & 0xFFU);
    message[11] = (uint8_t)((baud_rate >> 24) & 0xFFU);

    uint16_t in_proto_mask =
        UBX_PROTO_MASK_UBX | UBX_PROTO_MASK_NMEA | UBX_PROTO_MASK_RTCM;
    message[12] = (uint8_t)(in_proto_mask & 0xFFU);
    message[13] = (uint8_t)((in_proto_mask >> 8) & 0xFFU);

    uint16_t out_proto_mask = UBX_PROTO_MASK_UBX;
    message[14] = (uint8_t)(out_proto_mask & 0xFFU);
    message[15] = (uint8_t)((out_proto_mask >> 8) & 0xFFU);

    // encode frame
    char encoded[MSG_LEN_UBX_CFG_PRT + U_UBX_PROTOCOL_OVERHEAD_LENGTH_BYTES] = {
        0};
    int32_t bytes_encoded =
        uUbxProtocolEncode(MSG_CLASS_UBX_CFG, MSG_ID_UBX_CFG_PRT,
                           (const char *)message, MSG_LEN_UBX_CFG_PRT, encoded);
    if (bytes_encoded <= 0) {
        self->status = GPS_DRIVER_INVALID_ARG;
        return GPS_DRIVER_INVALID_ARG;
    }

    self->nmea_disabled = false;
    for (uint8_t i = 0; i < GPS_NUM_ATTEMPTS; ++i) {
        self->printf(
            "[GPS] Attempting to disable NMEA messages (attempt %d/%d).\r\n",
            i + 1, GPS_NUM_ATTEMPTS);

        gps_status_t err = gps_driver_request_dma(
            self, MSG_LEN_UBX_ACK_ACK + U_UBX_PROTOCOL_OVERHEAD_LENGTH_BYTES);
        if (err != GPS_DRIVER_OK) {
            self->status = err;
            continue;
        }

        if (self->uart_write(self->uart_bus, (const uint8_t *)encoded,
                             bytes_encoded, 100) != HAL_OK) {
            self->status = GPS_DRIVER_UART_ERROR;
            continue;
        }
        self->delay(50);

        if (self->nmea_disabled) {
            self->printf("[GPS] NMEA messages successfully disabled.\r\n");
            self->status = GPS_DRIVER_OK;
            return GPS_DRIVER_OK;
        }

        self->delay(20);
    }

    self->printf("[GPS] Failed to disable NMEA messages (timeout).\r\n");
    self->status = GPS_DRIVER_TIMEOUT;
    return GPS_DRIVER_TIMEOUT;
}

// Configure GPS navigation engine settings to dynamic platform model
gps_status_t gps_driver_configure_dynamic_model(gps_driver_t *self,
                                                bool force) {
    if (self == NULL) { return GPS_DRIVER_INVALID_ARG; }
    if (!self->initialized || self->status == GPS_DRIVER_NOT_INITIALIZED) {
        self->initialized = false;
        self->status = GPS_DRIVER_NOT_INITIALIZED;
        return GPS_DRIVER_NOT_INITIALIZED;
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
    // offset 8, store 10 27 as little endian, which is 10000 in decimal
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
        self->status = GPS_DRIVER_INVALID_ARG;
        return GPS_DRIVER_INVALID_ARG;
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
                             bytes_encoded, 100) != HAL_OK) {
            self->status = GPS_DRIVER_UART_ERROR;
            continue;
        }
        self->delay(50); // leave time for callback to parse DMA buffer

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

// Parses the GPS DMA buffer and stores the retrieved values in the GPS Driver
// struct.
gps_status_t gps_driver_parse_dma_buf(gps_driver_t *self) {
    if (self == NULL) { return GPS_DRIVER_INVALID_ARG; }
    if (!self->initialized || self->status == GPS_DRIVER_NOT_INITIALIZED) {
        self->initialized = false;
        self->status = GPS_DRIVER_NOT_INITIALIZED;
        return GPS_DRIVER_NOT_INITIALIZED;
    }

    uint16_t len = (uint16_t)(self->dma_buf[4] | (self->dma_buf[5] << 8));
    if (self->dma_buf[0] != UBX_SYNC_CHAR1 ||
        self->dma_buf[1] != UBX_SYNC_CHAR2 || !len) {
        return GPS_DRIVER_NO_DATA;
    }

    uint16_t class_id = (uint16_t)((self->dma_buf[2] << 8) | self->dma_buf[3]);

    // Debug dump
    // gps_driver_dump(self);

    // variable declarations
    const uint8_t zeros[6] = {0};
    bool success;
    gps_status_t err;
    uint8_t acked_class;
    uint8_t acked_id;
    uint16_t src;

    switch (class_id) {
    case NAV_PVT:
        return _gps_driver_parse_pvt(self);

    case CFG_NAV:
        // reply[8] = dynmodel | reply[9] = fixMode
        self->nav_configured =
            (self->dma_buf[8] == DYNAMIC_PLATFORM_MODEL_AIRBORNE_4G &&
             self->dma_buf[9] == FIXMODE_AUTO_2D_3D);
        break;

    case CFG_MSG:
        success = memcmp(self->dma_buf + 8, zeros, 6) == 0;
        self->printf("[GPS] CFG_MSG %s\r\n",
                     (success)
                         ? "succeeded (messages are disabled)."
                         : "failed (messages may still appear). Retrying...");
        if (!success) {
            err = gps_driver_disable_nmea_msgs(self, true);
            if (err != GPS_DRIVER_OK) { return err; }
        }
        break;

    case ACK:
        acked_class = self->dma_buf[U_UBX_PROTOCOL_HEADER_LENGTH_BYTES];
        acked_id = self->dma_buf[U_UBX_PROTOCOL_HEADER_LENGTH_BYTES + 1];
        src = (uint16_t)((acked_class << 8) | acked_id);
        self->printf("[GPS] Received ACK (class: 0x%02X | id: 0x%02X)\r\n",
                     acked_class, acked_id);

        if (src == CFG_NAV) {
            self->nav_configured = true;
            self->printf("[GPS] CFG_NAV configured (ACK).\r\n");
        } else if (src == CFG_PRT) {
            self->nmea_disabled = true;
            self->printf("[GPS] CFG_PRT configured (ACK).\r\n");
        }
        break;

    case NACK:
        self->printf("[GPS] Received NACK (class: 0x%02X | id: 0x%02X)\r\n",
                     self->dma_buf[U_UBX_PROTOCOL_HEADER_LENGTH_BYTES],
                     self->dma_buf[U_UBX_PROTOCOL_HEADER_LENGTH_BYTES + 1]);
        return GPS_DRIVER_NACK;

    default:
        break;
    }

    return GPS_DRIVER_OK;
}

// Parses PVT from GPS DMA buffer
// Modified from:
// https://github.com/u-blox/ubxlib/blob/a2d986e2944141176eedaee6dc4be87be7eea57b/example/gnss/msg_main.c#L234
gps_status_t _gps_driver_parse_pvt(gps_driver_t *self) {
    if (self == NULL) { return GPS_DRIVER_INVALID_ARG; }
    if (!self->initialized || self->status == GPS_DRIVER_NOT_INITIALIZED) {
        self->initialized = false;
        self->status = GPS_DRIVER_NOT_INITIALIZED;
        return GPS_DRIVER_NOT_INITIALIZED;
    }

    uint8_t *buf = self->dma_buf + U_UBX_PROTOCOL_HEADER_LENGTH_BYTES;

    if (buf[21] & 0x01) {
        int32_t longitude7 = uUbxProtocolUint32Decode((const char *)(buf + 24));
        int32_t latitude7 = uUbxProtocolUint32Decode((const char *)(buf + 28));
        int32_t altitude3 = uUbxProtocolUint32Decode((const char *)(buf + 36));

        self->longitude = (float)(longitude7 / 1e7);
        self->latitude = (float)(latitude7 / 1e7);
        self->altitude = (float)(altitude3 / 1e3);

        self->hours = buf[8];
        self->minutes = buf[9];
        self->seconds = buf[10];

        self->last_update = self->get_tick();

        self->printf("[GPS] Polled location -> lat: %f | long: %f | alt: %f @ "
                     "%d:%d:%d\r\n",
                     self->latitude, self->longitude, self->altitude,
                     self->hours, self->minutes, self->seconds);
    } else {
        self->printf("[GPS] NAV_PVT no valid fix (gnssFixOK flag not enabled). "
                     "GPS data not updated.\r\n");
    }

    return GPS_DRIVER_OK;
}

// Prints a hex dump of the entire GPS DMA buffer.
gps_status_t gps_driver_dump(gps_driver_t *self) {
    if (self == NULL) { return GPS_DRIVER_INVALID_ARG; }
    if (!self->initialized || self->status == GPS_DRIVER_NOT_INITIALIZED) {
        self->initialized = false;
        self->status = GPS_DRIVER_NOT_INITIALIZED;
        return GPS_DRIVER_NOT_INITIALIZED;
    }

    self->printf("[GPS] Dump of DMA buffer (%u bytes)\r\n",
                 (unsigned int)GPS_DMA_BUF_SIZE);

    const unsigned char *p = (const unsigned char *)self->dma_buf;
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

static int gps_uart_read(void *uart_bus, uint8_t *buf, uint16_t len) {
    UART_HandleTypeDef *bus = (UART_HandleTypeDef *)uart_bus;

    // Force stop if previous transfer timed out
    if (bus->RxState != HAL_UART_STATE_READY) { HAL_UART_DMAStop(bus); }

    // RxState must be HAL_UART_STATE_READY
    return (int)HAL_UART_Receive_DMA(bus, buf, len);
}

static int gps_uart_write(void *uart_bus, const uint8_t *buf, uint16_t len,
                          uint32_t timeout) {
    UART_HandleTypeDef *bus = (UART_HandleTypeDef *)uart_bus;

    // Force stop if previous transfer timed out
    if (bus->gState != HAL_UART_STATE_READY) { HAL_UART_AbortTransmit(bus); }

    // gState must be HAL_UART_STATE_READY
    return (int)HAL_UART_Transmit(bus, buf, len, timeout);
}
