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
#include "stm32f4xx_hal.h"

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
#define HIL_GPIO_A_Pin GPIO_PIN_2
#define HIL_GPIO_A_GPIO_Port GPIOE
#define HIL_GPIO_B_Pin GPIO_PIN_3
#define HIL_GPIO_B_GPIO_Port GPIOE
#define HIL_Main_Detect_Pin GPIO_PIN_5
#define HIL_Main_Detect_GPIO_Port GPIOE
#define HIL_Drogue_Detect_Pin GPIO_PIN_6
#define HIL_Drogue_Detect_GPIO_Port GPIOE
#define SENSOR_PWR_FLG_Pin GPIO_PIN_3
#define SENSOR_PWR_FLG_GPIO_Port GPIOF
#define SENSOR_PWR_EN_Pin GPIO_PIN_4
#define SENSOR_PWR_EN_GPIO_Port GPIOF
#define ED_INT_Pin GPIO_PIN_6
#define ED_INT_GPIO_Port GPIOF
#define ED_INT_EXTI_IRQn EXTI9_5_IRQn
#define CS_BD1_Pin GPIO_PIN_10
#define CS_BD1_GPIO_Port GPIOF
#define OUT_FLASH_WP_Pin GPIO_PIN_1
#define OUT_FLASH_WP_GPIO_Port GPIOA
#define OUT_FLASH_CS_Pin GPIO_PIN_2
#define OUT_FLASH_CS_GPIO_Port GPIOA
#define OUT_FLASH_IO3_Pin GPIO_PIN_3
#define OUT_FLASH_IO3_GPIO_Port GPIOA
#define EXT14_LISMDL_INT_Pin GPIO_PIN_4
#define EXT14_LISMDL_INT_GPIO_Port GPIOA
#define EXT14_LISMDL_INT_EXTI_IRQn EXTI4_IRQn
#define VIN_ADC_Pin GPIO_PIN_1
#define VIN_ADC_GPIO_Port GPIOB
#define IN_BUTTON_Pin GPIO_PIN_0
#define IN_BUTTON_GPIO_Port GPIOG
#define EXTI_ISM330DCL_INT2_Pin GPIO_PIN_7
#define EXTI_ISM330DCL_INT2_GPIO_Port GPIOE
#define EXTI_ISM330DCL_INT2_EXTI_IRQn EXTI9_5_IRQn
#define EXTI_ISM330DLC_INT1_Pin GPIO_PIN_8
#define EXTI_ISM330DLC_INT1_GPIO_Port GPIOE
#define EXTI_ISM330DLC_INT1_EXTI_IRQn EXTI9_5_IRQn
#define IN_SD_CARD_DETECT_Pin GPIO_PIN_11
#define IN_SD_CARD_DETECT_GPIO_Port GPIOE
#define SD_CS_Pin GPIO_PIN_15
#define SD_CS_GPIO_Port GPIOE
#define SX_CS_Pin GPIO_PIN_12
#define SX_CS_GPIO_Port GPIOB
#define SX_SCK_Pin GPIO_PIN_13
#define SX_SCK_GPIO_Port GPIOB
#define SX_MISO_Pin GPIO_PIN_14
#define SX_MISO_GPIO_Port GPIOB
#define SX_MOSI_Pin GPIO_PIN_15
#define SX_MOSI_GPIO_Port GPIOB
#define RD_MAIN_FREQ_RES_Pin GPIO_PIN_8
#define RD_MAIN_FREQ_RES_GPIO_Port GPIOD
#define SX_BUSY_Pin GPIO_PIN_9
#define SX_BUSY_GPIO_Port GPIOD
#define SX_DIO_Pin GPIO_PIN_10
#define SX_DIO_GPIO_Port GPIOD
#define SX_DIO_EXTI_IRQn EXTI15_10_IRQn
#define SX_RST_Pin GPIO_PIN_11
#define SX_RST_GPIO_Port GPIOD
#define RD_PWR_EN_Pin GPIO_PIN_12
#define RD_PWR_EN_GPIO_Port GPIOD
#define RD_PWR_FLAG_Pin GPIO_PIN_13
#define RD_PWR_FLAG_GPIO_Port GPIOD
#define OUT_VR_REC_Pin GPIO_PIN_14
#define OUT_VR_REC_GPIO_Port GPIOD
#define OUT_VR_PWR_Pin GPIO_PIN_15
#define OUT_VR_PWR_GPIO_Port GPIOD
#define PROP_AV_EN_Pin GPIO_PIN_2
#define PROP_AV_EN_GPIO_Port GPIOG
#define PROP_AV_CHECK_Pin GPIO_PIN_3
#define PROP_AV_CHECK_GPIO_Port GPIOG
#define PROP_EMERGENCY_Pin GPIO_PIN_4
#define PROP_EMERGENCY_GPIO_Port GPIOG
#define V_EXT_EN_Pin GPIO_PIN_5
#define V_EXT_EN_GPIO_Port GPIOG
#define PWR_UMB_OFF_Pin GPIO_PIN_6
#define PWR_UMB_OFF_GPIO_Port GPIOG
#define V_EXT_SNS_Pin GPIO_PIN_7
#define V_EXT_SNS_GPIO_Port GPIOG
#define USART6_TX_GPS_Pin GPIO_PIN_6
#define USART6_TX_GPS_GPIO_Port GPIOC
#define USART6_RX_GPS_Pin GPIO_PIN_7
#define USART6_RX_GPS_GPIO_Port GPIOC
#define OUT_TRIG_HW_RESET_Pin GPIO_PIN_15
#define OUT_TRIG_HW_RESET_GPIO_Port GPIOA
#define UART4_TX_Debug_Pin GPIO_PIN_10
#define UART4_TX_Debug_GPIO_Port GPIOC
#define UART4_RX_Debug_Pin GPIO_PIN_11
#define UART4_RX_Debug_GPIO_Port GPIOC
#define OUT_LEDF_Pin GPIO_PIN_2
#define OUT_LEDF_GPIO_Port GPIOD
#define OUT_LED1_Pin GPIO_PIN_3
#define OUT_LED1_GPIO_Port GPIOD
#define OUT_LED2_Pin GPIO_PIN_4
#define OUT_LED2_GPIO_Port GPIOD
#define OUT_LED3_Pin GPIO_PIN_5
#define OUT_LED3_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
