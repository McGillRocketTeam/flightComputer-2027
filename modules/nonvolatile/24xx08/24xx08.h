#pragma once

#include <stdint.h>
#include <stddef.h>
#include <nonvolatile/24xx08/24xx08_address.h>
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PAGE_SIZE 16
#define BLOCK_SIZE (EEPROM_SIZE_BYTES / EEPROM_NUM_BLOCKS)

extern I2C_HandleTypeDef hi2c2;
#define I2C_BUS_24xx08 (&hi2c2)

typedef enum {
    STATUS_24xx08_ERROR = -1,
    STATUS_24xx08_OK = 0,
    STATUS_24xx08_TIMEOUT = 1,
    STATUS_24xx08_ERROR_NO_SPACE = 2, // Occurs when trying to write past block limits
} status_24xx08_t;

typedef struct{
    uint8_t dev_addr; // Includes Control Code & Block Address, First byte on the wire
    uint8_t word_addr; // Word address, second byte on the wire
} I2C_Address;

status_24xx08_t write_byte(uint16_t addr, const uint8_t data);

status_24xx08_t write_page(uint16_t addr, const uint8_t* data, size_t len);

status_24xx08_t read_data(uint16_t addr, uint8_t* out, size_t len);

#ifdef __cplusplus
}
#endif