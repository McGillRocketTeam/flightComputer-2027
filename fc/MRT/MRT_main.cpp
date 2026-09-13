#include "MRT_main.h"
#include "gpio.h"
#include "cmsis_os.h"

// Called on Telemetry thread
void blinky(){
    while(1){
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);

        osDelay(500);

        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        
        osDelay(500);
    }
}