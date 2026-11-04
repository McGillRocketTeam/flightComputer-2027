#include "MRT_main.h"

#include "cmsis_os.h"
#include "gpio.h"
#include "i2c.h"
#include "sensors/IMU.h"
#include "stm32f427xx.h"
#include <cstring>

// Called on Telemetry thread
void MRT_main() {
    I2C_busy_bit_fix();

    stmdev_ctx_t ctx = {lsm6dsr_i2c_write_ctx, lsm6dsr_i2c_read_ctx, &hi2c2};
    IMU imu{ctx, HAL_Delay};

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

    uint8_t valid_fields = 0;

    HAL_Delay(2000);

    while(status == Sensor::ErrorCode::NOMINAL){
        imu.poll();

        const float* accel = imu.getAcceleration();
        const float* rates = imu.getAngularRate();

        valid_fields = imu.getValidFields();

        printf("IMU Status: %d | ", valid_fields);
        printf("IMU Readings: Accel: %f, %f, %f, Rate: %f, %f, %f\n", accel[0], accel[1], accel[2], rates[0], rates[1], rates[2]);
    }
};