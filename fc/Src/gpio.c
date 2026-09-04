/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, HIL_GPIO_A_Pin|HIL_GPIO_B_Pin|HIL_Main_Detect_Pin|HIL_Drogue_Detect_Pin
                          |SD_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, SENSOR_PWR_EN_Pin|CS_BD1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, OUT_FLASH_WP_Pin|OUT_FLASH_IO3_Pin|OUT_TRIG_HW_RESET_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SX_CS_GPIO_Port, SX_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SX_RST_GPIO_Port, SX_RST_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, RD_PWR_EN_Pin|OUT_VR_REC_Pin|OUT_VR_PWR_Pin|OUT_LEDF_Pin
                          |OUT_LED1_Pin|OUT_LED2_Pin|OUT_LED3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, PROP_AV_EN_Pin|V_EXT_EN_Pin|PWR_UMB_OFF_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : HIL_GPIO_A_Pin HIL_GPIO_B_Pin HIL_Main_Detect_Pin HIL_Drogue_Detect_Pin
                           SD_CS_Pin */
  GPIO_InitStruct.Pin = HIL_GPIO_A_Pin|HIL_GPIO_B_Pin|HIL_Main_Detect_Pin|HIL_Drogue_Detect_Pin
                          |SD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : SENSOR_PWR_FLG_Pin */
  GPIO_InitStruct.Pin = SENSOR_PWR_FLG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SENSOR_PWR_FLG_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SENSOR_PWR_EN_Pin CS_BD1_Pin */
  GPIO_InitStruct.Pin = SENSOR_PWR_EN_Pin|CS_BD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : ED_INT_Pin */
  GPIO_InitStruct.Pin = ED_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ED_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : OUT_FLASH_WP_Pin OUT_FLASH_IO3_Pin OUT_TRIG_HW_RESET_Pin */
  GPIO_InitStruct.Pin = OUT_FLASH_WP_Pin|OUT_FLASH_IO3_Pin|OUT_TRIG_HW_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : OUT_FLASH_CS_Pin */
  GPIO_InitStruct.Pin = OUT_FLASH_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OUT_FLASH_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : EXT14_LISMDL_INT_Pin */
  GPIO_InitStruct.Pin = EXT14_LISMDL_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(EXT14_LISMDL_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : IN_BUTTON_Pin */
  GPIO_InitStruct.Pin = IN_BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(IN_BUTTON_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : EXTI_ISM330DCL_INT2_Pin EXTI_ISM330DLC_INT1_Pin */
  GPIO_InitStruct.Pin = EXTI_ISM330DCL_INT2_Pin|EXTI_ISM330DLC_INT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : IN_SD_CARD_DETECT_Pin */
  GPIO_InitStruct.Pin = IN_SD_CARD_DETECT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(IN_SD_CARD_DETECT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SX_CS_Pin */
  GPIO_InitStruct.Pin = SX_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(SX_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RD_MAIN_FREQ_RES_Pin RD_PWR_FLAG_Pin */
  GPIO_InitStruct.Pin = RD_MAIN_FREQ_RES_Pin|RD_PWR_FLAG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : SX_BUSY_Pin */
  GPIO_InitStruct.Pin = SX_BUSY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(SX_BUSY_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SX_DIO_Pin */
  GPIO_InitStruct.Pin = SX_DIO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(SX_DIO_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SX_RST_Pin RD_PWR_EN_Pin OUT_VR_REC_Pin OUT_VR_PWR_Pin
                           OUT_LEDF_Pin OUT_LED1_Pin OUT_LED2_Pin OUT_LED3_Pin */
  GPIO_InitStruct.Pin = SX_RST_Pin|RD_PWR_EN_Pin|OUT_VR_REC_Pin|OUT_VR_PWR_Pin
                          |OUT_LEDF_Pin|OUT_LED1_Pin|OUT_LED2_Pin|OUT_LED3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PROP_AV_EN_Pin V_EXT_EN_Pin PWR_UMB_OFF_Pin */
  GPIO_InitStruct.Pin = PROP_AV_EN_Pin|V_EXT_EN_Pin|PWR_UMB_OFF_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : PROP_AV_CHECK_Pin PROP_EMERGENCY_Pin V_EXT_SNS_Pin */
  GPIO_InitStruct.Pin = PROP_AV_CHECK_Pin|PROP_EMERGENCY_Pin|V_EXT_SNS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
