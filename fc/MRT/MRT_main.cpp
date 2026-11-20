#include "MRT_main.h"

#include "cmsis_os.h"

// Called on Telemetry thread
void MRT_main() {
    

    HAL_Delay(2000);

    while(1){
        
        Vec3 magnetic_field = imu.getMagneticDataVec3();

        printf("IMU Status: %d | ", imu.get_constructor_status());
        printf("IMU Readings: \nRate: %f, %f, %f\n", magnetic_field.x, magnetic_field.y, magnetic_field.z);
    }
};