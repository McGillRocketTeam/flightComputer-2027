#include "MRT_setup.h"

static stmdev_ctx_t ctx = {lsm6dsr_i2c_write_ctx, lsm6dsr_i2c_read_ctx, &hi2c2};
IMU imu{ctx, HAL_Delay};

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

extern "C" void setup_IMU(){
    HAL_GPIO_WritePin(SENSOR_PWR_EN_GPIO_Port, SENSOR_PWR_EN_Pin, GPIO_PIN_SET);
    if (HAL_GPIO_ReadPin(SENSOR_PWR_EN_GPIO_Port, SENSOR_PWR_EN_Pin) != GPIO_PIN_SET) {
        MRT_terminalPrint(PERIPHERAL, "GPIO", "FAILED", "GPIO set of SENSOR_PWR_EN failed");
    }

    osDelay(500);

    imu.init();

    Sensor::ErrorCode status = imu.status();

    printf("Status: %d", status);

    if (status == Sensor::ErrorCode::NOMINAL) {
        MRT_terminalPrint(PERIPHERAL, "IMU", "OK", "");
    } else if (status == Sensor::ErrorCode::SENSOR_NOT_PRESENT) {
        MRT_terminalPrint(PERIPHERAL, "IMU", "FAILED", "SENSOR NOT PRESENT");
    } else {
        MRT_terminalPrint(PERIPHERAL, "IMU", "FAILED", "Initialization error");
    }
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