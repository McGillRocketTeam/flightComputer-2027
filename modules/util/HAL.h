#pragma once

#include <stdint.h>
#include <stm32f4xx_hal.h>

#define NUM_INTERRUPTS 16

#ifdef __cplusplus

struct InterruptBinding {
    void (*callback)(void*) = nullptr;
    void* context = nullptr;
};

void setCallback(uint32_t interruptNum, void (*cb)(void));

void setCallback(uint32_t interruptNum, void (*cb)(void*), void* context);

static void MRT_HAL_functionAsContext(void* ctx);

#endif

#ifdef __cplusplus
extern "C" {
#endif

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

void fireCallback(uint32_t interruptNum);


#ifdef __cplusplus
}
#endif
