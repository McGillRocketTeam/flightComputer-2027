#pragma once
#include "util/helpers.h"
#include "cmsis_os.h"
#include "spi.h"
#include "main.h"

#ifdef __cplusplus

// #define SD_SPI_HANDLE hspi4

extern "C" {
#endif

void MRT_setup();

// I2C fix for busy bit, STM Erratum
void I2C_busy_bit_fix(void);

#ifdef __cplusplus
}
#endif