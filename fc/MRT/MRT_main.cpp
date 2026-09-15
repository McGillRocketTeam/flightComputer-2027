#include "MRT_main.h"
#include "gpio.h"
#include "cmsis_os.h"

// Called on Telemetry thread
void blinky(){
    while(1){
        HAL_GPIO_WritePin(OUT_LED2_GPIO_Port, OUT_LED2_Pin, GPIO_PIN_SET);

        osDelay(500);

        HAL_GPIO_WritePin(OUT_LED2_GPIO_Port, OUT_LED2_Pin, GPIO_PIN_RESET);
        
        osDelay(500);
    }
}