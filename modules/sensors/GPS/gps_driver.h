#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "u_ubx_protocol/ubx_messages.h"

#define REPLY_CLASS_ID(class, id) class << 8 | id

enum UBX_MSG_REPLY_TYPE {
	NAV_PVT = REPLY_CLASS_ID(MSG_CLASS_UBX_NAV, 0x07),
	CFG_PRT = REPLY_CLASS_ID(MSG_CLASS_UBX_CFG, MSG_ID_UBX_CFG_PRT),
	CFG_NAV = REPLY_CLASS_ID(MSG_CLASS_UBX_CFG, MSG_ID_UBX_CFG_NAV5),
	CFG_MSG = REPLY_CLASS_ID(MSG_CLASS_UBX_CFG, MSG_ID_UBX_CFG_MSG),
	ACK = REPLY_CLASS_ID(MSG_CLASS_UBX_ACK, MSG_ID_UBX_ACK_ACK),
	NACK = REPLY_CLASS_ID(MSG_CLASS_UBX_ACK, MSG_ID_UBX_ACK_NAK)
};

typedef struct {
	ubx_message_class_t message_class;
	uint16_t message_id;
	uint16_t message_length;
	uint8_t *message_body;
} ubx_packet_t;

typedef struct GPS_Config_t {
    void *uart_bus;
    int32_t (*uart_write)(void *uart_bus, const uint8_t *buf, uint16_t len);
    int32_t (*uart_read)(void *uart_bus, uint8_t *buf, uint16_t len);
    int (*printf)(const char *s, ...);
    void (*delay)(uint32_t ms);
} GPS_Config_t;

typedef struct GPS_Driver_t {
    float latitude;
    float longitude;
    float altitude;

    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;

    // time since GPS was last updated (in ms)
    uint32_t last_update;

    uint8_t dma_buf[128];

    bool nmea_disabled;
    bool nav_configured;
    bool initialized;

    void *uart_bus;
    int32_t (*uart_write)(void *uart_bus, const uint8_t *buf, uint16_t len);
    int32_t (*uart_read)(void *uart_bus, uint8_t *buf, uint16_t len);
    int (*printf)(const char *s, ...);
    void (*delay)(uint32_t ms);
} GPS_Driver_t;

bool GPS_Driver_init(GPS_Driver_t *self, const GPS_Config_t *config);
void GPS_Driver_startDMA(GPS_Driver_t *self, uint16_t len);

void GPS_Driver_dump(GPS_Driver_t *self, const void *data, size_t size);

void GPS_Driver_poll(GPS_Driver_t *self);
void GPS_Driver_disable_nmea_msgs(GPS_Driver_t *self);
void GPS_Driver_factory_reset_gps(GPS_Driver_t *self);
void GPS_Driver_configure_dynamic_model(GPS_Driver_t *self);
void GPS_Driver_parse_dma_buf(GPS_Driver_t *self);
