/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f3xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PT_VOLTAGE_Pin GPIO_PIN_0
#define PT_VOLTAGE_GPIO_Port GPIOA
#define CS_FLASH_Pin GPIO_PIN_0
#define CS_FLASH_GPIO_Port GPIOB
#define POWER_LED_EN_Pin GPIO_PIN_2
#define POWER_LED_EN_GPIO_Port GPIOB
#define CS_TC_Pin GPIO_PIN_12
#define CS_TC_GPIO_Port GPIOB
#define PT_nCS_Pin GPIO_PIN_9
#define PT_nCS_GPIO_Port GPIOA
#define OUT_LED2_Pin GPIO_PIN_15
#define OUT_LED2_GPIO_Port GPIOA
#define OUT_LED1_Pin GPIO_PIN_4
#define OUT_LED1_GPIO_Port GPIOB
#define BUTTON_Pin GPIO_PIN_5
#define BUTTON_GPIO_Port GPIOB
#define BUTTON_EXTI_IRQn EXTI9_5_IRQn
#define BD1_nINT_Pin GPIO_PIN_6
#define BD1_nINT_GPIO_Port GPIOB
#define BD1_nINT_EXTI_IRQn EXTI9_5_IRQn
#define CS_BD1_Pin GPIO_PIN_7
#define CS_BD1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
