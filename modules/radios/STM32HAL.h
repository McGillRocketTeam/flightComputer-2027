// STM32Hal.h
#pragma once
#include <RadioLib.h>

#include "stm32f4xx_hal.h"
#include <util/HAL.h>

// --- pin encoding: port index (upper 16 bits) | pin mask (lower 16 bits) ---
static GPIO_TypeDef* const _stm32PortTable[] = {
  GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG, GPIOH
};

inline uint32_t stm32PortIndex(GPIO_TypeDef *port) {
  for (uint32_t i = 0; i < (sizeof(_stm32PortTable) / sizeof(_stm32PortTable[0])); i++) {
    if (_stm32PortTable[i] == port) return i;
  }
  return 0;
}

#define STM32_PIN(port, pin) ((stm32PortIndex(port) << 16) | (pin))

class STM32HAL_Radios : public RadioLibHal {
   public:
    STM32HAL_Radios(SPI_HandleTypeDef* spi)
        : RadioLibHal(GPIO_MODE_INPUT,       // GpioModeInput
                      GPIO_MODE_OUTPUT_PP,   // GpioModeOutput
                      GPIO_PIN_RESET,        // GpioLevelLow
                      GPIO_PIN_SET,          // GpioLevelHigh
                      GPIO_MODE_IT_RISING,   // GpioInterruptRising
                      GPIO_MODE_IT_FALLING), // GpioInterruptFalling
          _spi(spi) {}

    void init() override {}
    void term() override {}

    // ---- GPIO ----
    void pinMode(uint32_t pin, uint32_t mode) override {
        GPIO_InitTypeDef cfg = {0};
        cfg.Pin = pinMask(pin);
        cfg.Mode = mode;
        cfg.Pull = GPIO_NOPULL;
        cfg.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(pinPort(pin), &cfg);
    }

    void digitalWrite(uint32_t pin, uint32_t value) override {
        HAL_GPIO_WritePin(pinPort(pin), pinMask(pin), (GPIO_PinState)value);
    }

    uint32_t digitalRead(uint32_t pin) override {
        return HAL_GPIO_ReadPin(pinPort(pin), pinMask(pin));
    }

    // ---- interrupts ----
    void attachInterrupt(uint32_t interruptNum, void (*interruptCb)(void),
                         uint32_t mode) override {
        GPIO_InitTypeDef cfg = {0};
        cfg.Pin = pinMask(interruptNum);
        cfg.Mode = mode;  // GPIO_MODE_IT_RISING or GPIO_MODE_IT_FALLING
        cfg.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(pinPort(interruptNum), &cfg);

        setCallback(interruptNum, interruptCb);

        // NVIC still has to be enabled for the corresponding EXTI line/IRQn —
        // CubeMX does this once if you set the pin to "GPIO_EXTI" mode in the
        // .ioc, or you can call HAL_NVIC_EnableIRQ()/SetPriority() here
        // directly.
    }

    void detachInterrupt(uint32_t interruptNum) override {
        HAL_GPIO_DeInit(pinPort(interruptNum), pinMask(interruptNum));
        setCallback(interruptNum, nullptr);
    }

    // ---- timing ----
    void delay(RadioLibTime_t ms) override { HAL_Delay(ms); }

    void delayMicroseconds(RadioLibTime_t us) override {
        uint32_t start = DWT->CYCCNT;
        uint32_t cycles = us * (HAL_RCC_GetHCLKFreq() / 1000000UL);
        while ((DWT->CYCCNT - start) < cycles) {
        }
    }

    RadioLibTime_t millis() override { return HAL_GetTick(); }

    RadioLibTime_t micros() override {
        // requires DWT cycle counter enabled at startup (see below)
        return DWT->CYCCNT / (HAL_RCC_GetHCLKFreq() / 1000000UL);
    }

    long pulseIn(uint32_t pin, uint32_t state,
                 RadioLibTime_t timeout) override {
        pinMode(pin, GpioModeInput);
        uint32_t curtick = micros();
        while (digitalRead(pin) != state) {
            if ((micros() - curtick) > timeout) return 0;
        }
        uint32_t pulseStart = micros();
        while (digitalRead(pin) == state) {
            if ((micros() - curtick) > timeout) return 0;
        }
        return micros() - pulseStart;
    }

    // ---- SPI ----
    void spiBegin() override {}
    void spiBeginTransaction() override {}
    void spiTransfer(uint8_t* out, size_t len, uint8_t* in) override {
        HAL_SPI_TransmitReceive(_spi, out, in, len, HAL_MAX_DELAY);
    }
    void spiEndTransaction() override {}
    void spiEnd() override {}

   private:
    SPI_HandleTypeDef* _spi;

    GPIO_TypeDef* pinPort(uint32_t pin) { return _stm32PortTable[pin >> 16]; }
    uint16_t pinMask(uint32_t pin) { return (uint16_t)(pin & 0xFFFF); }
};