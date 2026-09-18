#include "MRT_main.h"
#include "gpio.h"
#include "cmsis_os.h"

// Called on Telemetry thread
void blinky(){

    volatile int test = 0;
    volatile bool doLED1 = false;
    GPIO_TypeDef* port = OUT_LED2_GPIO_Port;
    int pin = OUT_LED2_Pin;
    while(1){
        if(doLED1){
            port = OUT_LED1_GPIO_Port;
            pin = OUT_LED1_Pin;
        } else {
            port = OUT_LED2_GPIO_Port;
            pin = OUT_LED2_Pin;
        }


        HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);

        osDelay(500);

        test++;

        HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
        
        osDelay(500);
    }
}