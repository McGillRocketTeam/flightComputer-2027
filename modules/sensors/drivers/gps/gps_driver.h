#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "stm32f4xx_hal_def.h"
#include "stm32f4xx_hal_uart.h"
#include "ubx_messages.h"

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

#define GPS_NUM_ATTEMPTS 3
#define GPS_DMA_BUF_SIZE 128

typedef enum gps_status_t {
    GPS_DRIVER_ERROR = -1,
    GPS_DRIVER_OK = 0,
    GPS_DRIVER_CONFIG_ERROR,
    GPS_DRIVER_TIMEOUT,
    GPS_DRIVER_BUFFER_OVERFLOW,
} gps_status_t;

typedef struct gps_driver_t {
    float latitude;
    float longitude;
    float altitude;

    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;

    // time since GPS was last updated (in ms)
    uint32_t last_update;

    uint8_t dma_buf[GPS_DMA_BUF_SIZE];

    bool nmea_disabled;
    bool nav_configured;
    bool initialized;

    gps_status_t status;

    UART_HandleTypeDef *uart_bus;
    HAL_StatusTypeDef (*uart_write)(UART_HandleTypeDef *uart_bus, const uint8_t *buf, uint16_t len, uint32_t timeout);
    HAL_StatusTypeDef (*uart_read)(UART_HandleTypeDef *uart_bus, uint8_t *buf, uint16_t len);
    int (*printf)(const char *s, ...);
    void (*delay)(int ms);
    long unsigned int (*get_tick)(void);
} gps_driver_t;

typedef struct gps_config_t {
    void *uart_bus;
    int32_t (*uart_write)(void *uart_bus, const uint8_t *buf, uint16_t len);
    int32_t (*uart_read)(void *uart_bus, uint8_t *buf, uint16_t len);
    int (*printf)(const char *s, ...);
    void (*delay)(uint32_t ms);
    uint32_t (*get_tick)(void);
} gps_config_t;

gps_status_t gps_driver_init(gps_driver_t *self);
gps_status_t gps_driver_request_dma(gps_driver_t *self, uint16_t len);

gps_status_t gps_driver_dump(gps_driver_t *self);

gps_status_t gps_driver_poll(gps_driver_t *self);
gps_status_t gps_driver_disable_nmea_msgs(gps_driver_t *self);
gps_status_t gps_driver_factory_reset_gps(gps_driver_t *self);
gps_status_t gps_driver_configure_dynamic_model(gps_driver_t *self, bool force);
gps_status_t gps_driver_parse_dma_buf(gps_driver_t *self);
