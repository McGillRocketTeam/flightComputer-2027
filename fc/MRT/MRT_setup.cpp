#include "MRT_setup.h"
#include "i2c.h"
#include "util/MRT_terminal.h"
#include "telemetry/Telemetry.h"
#include "sd_card/SDCard.h"

Telemetry telemetry;
SDCard sd_card;

void MRT_terminalPrint(terminal_label label, const char* peripheral, const char* state, const char* information) {

    if (label == INFO) {
        print("[INFO]");
    } else if (label == CAN) {
        print("[" CAN_P_COLOR "CAN" ANSI_COLOR_RESET "]");
    } else if (label == COMMAND) {
        print("[" COMMAND_P_COLOR "COMMAND" ANSI_COLOR_RESET "]");
    } else if (label == PERIPHERAL) {
        printf("[" PERIPHERAL_P_COLOR "PERIPHERAL:%s" ANSI_COLOR_RESET "]", peripheral);
    } else if (label == STATE_R) {
        print("[" STATE_R_P_COLOR "STATE_R" ANSI_COLOR_RESET "]");
    }

    if (!strcmp(state, "OK") || !strcmp(state, "GOOD") || !strcmp(state, "PASSED")) {
        printf("[" OK_P_COLOR "%s" ANSI_COLOR_RESET "] ", state);
    } else if (!strcmp(state, "WARNING")) {
        print("[" WARNING_P_COLOR "WARNING" ANSI_COLOR_RESET "] ");
    } else if (!strcmp(state, "ERROR") || !strcmp(state, "FAILED")) {
        printf("[" ERROR_P_COLOR "%s" ANSI_COLOR_RESET "] ", state);
    } else {
        print("[] ");
    }

    println(information);
}

extern "C" void MRT_setup(){
    HAL_GPIO_WritePin(SENSOR_PWR_EN_GPIO_Port, SENSOR_PWR_EN_Pin, GPIO_PIN_SET);
    if (HAL_GPIO_ReadPin(SENSOR_PWR_EN_GPIO_Port, SENSOR_PWR_EN_Pin) != GPIO_PIN_SET) {
        MRT_terminalPrint(PERIPHERAL, "GPIO", "FAILED", "GPIO set of SENSOR_PWR_EN failed");
    }

    delay(1000);
    FRESULT res = sd_card.mount();
    if(res != FR_OK){
        MRT_terminalPrint(PERIPHERAL, "SD", "FAILED", "MOUNT FAILED");
        printf("Error Code: %d\n", res);
    } else {
        MRT_terminalPrint(PERIPHERAL, "SD", "OK", "MOUNTED");
    }

    if(!sd_card.openLogFile()){
        MRT_terminalPrint(PERIPHERAL, "SD", "FAILED", "OPEN FAILED");
    } else {
        MRT_terminalPrint(PERIPHERAL, "SD", "OK", "OPENED");
    }

    flight_ptr_SD_t ptrs = {
        .flight_stage = nullptr,
        .alt_agl = &telemetry.rocket_telem.altitude_from_ground_ft,
        .alt_asl = nullptr,
        .vertical_speed = &telemetry.rocket_telem.altitude_rate_of_change_ft_s,
        .apogee = &telemetry.rocket_telem.apogee_from_ground_ft,
        .main_alt = &telemetry.rocket_telem.main_deployment_from_ground_ft,
        .booster_burnout_alt = nullptr,
    };

    // Set to hardcoded constants for r/w testing
    telemetry.rocket_telem.altitude_from_ground_ft = 1;
    telemetry.rocket_telem.altitude_rate_of_change_ft_s = 2;
    telemetry.rocket_telem.apogee_from_ground_ft = 3;
    telemetry.rocket_telem.main_deployment_from_ground_ft = 4;

    uint16_t bitmask = (1u << SD_AT_FLIGHT);

    sd_card.bind_flight(ptrs);

    delay(200);

    for(int i = 0; i < 100; i++){
        printf("%d\n", i);

        if(!sd_card.packet(bitmask)){
            MRT_terminalPrint(PERIPHERAL, "SD", "ERROR", "Write Packet Error");
        }

        // delay(200);
    }

    sd_card.closeLogFile();

    MRT_terminalPrint(PERIPHERAL, "SD", "OK", "CLOSED");

}

void I2C_busy_bit_fix(void) {
    // Write pin for all 3 sensors on the line
    HAL_GPIO_WritePin(SENSOR_PWR_EN_GPIO_Port, SENSOR_PWR_EN_Pin, GPIO_PIN_SET);

    HAL_Delay(200);
//    delay_RTOS(200); // delay to give each power switch time to pull device
                     // power supply high
    // No Idea why RTOS delay worked here, this is before kernel initialize, changed to HAL for Renode testing -AH 2026-10-09

    // software reset I2C peripheral to reset BUSY bit to 0
    int i = 0;
    while ((I2C2->SR2 & I2C_SR2_BUSY) != 0) {
        I2C2->CR1 |= (I2C_CR1_SWRST); // set SWRST bit to 1 to software reset I2C
        I2C2->CR1 |= (I2C_CR1_PE);    // set PE bit to 1 to enable I2C
        MX_I2C2_Init();               // re-init I2C2
        i++;

        if (i >= 1000) {
            printf("I2C peripheral can't be reset");
            return;
        }
    }

    //
    //delay_RTOS(100); // delay to allow I2C sensors time to initialize
    HAL_Delay(100);
    // No Idea why RTOS delay worked here, this is before kernel initialize, changed to HAL for Renode testing -AH 2026-10-09


    printf("I2C fix applied");
}