#include "MRT_main.h"

#include "cmsis_os.h"

#include <cstring>

// Called on Telemetry thread
void MRT_main() {
    uint8_t valid_fields = 0;

    HAL_Delay(2000);

    while(1){
        imu.poll();

        const float* accel = imu.getAcceleration();
        const float* rates = imu.getAngularRate();

        valid_fields = imu.getValidFields();

        printf("IMU Status: %d | ", valid_fields);
        printf("IMU Readings: Accel: %f, %f, %f, Rate: %f, %f, %f\n", accel[0], accel[1], accel[2], rates[0], rates[1], rates[2]);
    }
};