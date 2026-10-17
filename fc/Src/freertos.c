/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "MRT_main.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for Telemetry */
osThreadId_t TelemetryHandle;
const osThreadAttr_t Telemetry_attributes = {
  .name = "Telemetry",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Radio */
osThreadId_t RadioHandle;
const osThreadAttr_t Radio_attributes = {
  .name = "Radio",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for Watchdog */
osThreadId_t WatchdogHandle;
const osThreadAttr_t Watchdog_attributes = {
  .name = "Watchdog",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityRealtime,
};
/* Definitions for CAN */
osThreadId_t CANHandle;
const osThreadAttr_t CAN_attributes = {
  .name = "CAN",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartTelemetry(void *argument);
void StartRadio(void *argument);
void StartWatchdog(void *argument);
void StartCAN(void *argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationDaemonTaskStartupHook(void);

/* USER CODE BEGIN DAEMON_TASK_STARTUP_HOOK */
void vApplicationDaemonTaskStartupHook(void)
{
}
/* USER CODE END DAEMON_TASK_STARTUP_HOOK */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
    /* init code for USB_DEVICE */
    MX_USB_DEVICE_Init();
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of Telemetry */
  TelemetryHandle = osThreadNew(StartTelemetry, NULL, &Telemetry_attributes);

  /* creation of Radio */
  RadioHandle = osThreadNew(StartRadio, NULL, &Radio_attributes);

  /* creation of Watchdog */
  WatchdogHandle = osThreadNew(StartWatchdog, NULL, &Watchdog_attributes);

  /* creation of CAN */
  CANHandle = osThreadNew(StartCAN, NULL, &CAN_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartTelemetry */
/**
  * @brief  Function implementing the Telemetry thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartTelemetry */
void StartTelemetry(void *argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartTelemetry */
    MRT_main();
  /* USER CODE END StartTelemetry */
}

/* USER CODE BEGIN Header_StartRadio */
/**
* @brief Function implementing the Radio thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartRadio */
void StartRadio(void *argument)
{
  /* USER CODE BEGIN StartRadio */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartRadio */
}

/* USER CODE BEGIN Header_StartWatchdog */
/**
* @brief Function implementing the Watchdog thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartWatchdog */
void StartWatchdog(void *argument)
{
  /* USER CODE BEGIN StartWatchdog */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartWatchdog */
}

/* USER CODE BEGIN Header_StartCAN */
/**
* @brief Function implementing the CAN thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCAN */
void StartCAN(void *argument)
{
  /* USER CODE BEGIN StartCAN */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartCAN */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

