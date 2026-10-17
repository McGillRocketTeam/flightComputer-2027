/*
 * HBL_helpers.c
 *
 *  Created on: Dec 20, 2022
 *      Author: Jacoby
 */
#ifdef STM32

// DEFAULT INCLUDES
#include <util/helpers.h>

//**************************************************//
// PLATFORM INCLUDES
#include <usart.h>
#include <i2c.h>
#include <can.h>

//**************************************************//
// HELPER DEFINES
#define DEBUGUART huart4

//**************************************************//
// GLOBAL HELPERS

void println(const char* s) {
#ifdef PRINT
    HAL_UART_Transmit(&DEBUGUART, (uint8_t*)s, strlen(s), HAL_MAX_DELAY);
    HAL_UART_Transmit(&DEBUGUART, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);
#endif
}

void print(const char* s) {
#ifdef PRINT
    HAL_UART_Transmit(&DEBUGUART, (uint8_t*)s, strlen(s), HAL_MAX_DELAY);
#endif
}

int printf(const char* s, ...) {
#ifdef PRINT
    char buffer[256];
    va_list args;
    va_start(args, s);
    int length = vsnprintf(buffer, sizeof(buffer), s, args);
    va_end(args);
    print(buffer);
    return length;
#else
    (void)s;
    return 0;
#endif
}

void no_print(const char* s) {
    // Buffer function
}

int no_printf(const char* s, ...) {
    // Buffer function
    return 0;
}

void delay(int ms) {
#ifdef FREERTOS
    osDelay(ms);
#else
    HAL_Delay(ms);
#endif
}

uint32_t getMillis(void) {
    return HAL_GetTick();
}

#endif
