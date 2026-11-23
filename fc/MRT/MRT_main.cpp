#include "MRT_main.h"

#include "cmsis_os.h"

// Called on Telemetry thread
void MRT_main() {
    

    HAL_Delay(2000);

    while(1){
        gps.poll();
        bool gps_hasfix = gps.hasFix();
        GPSPosition pos = gps.getPosition();
        if (gps_hasfix) {
            printf("GPS has fix, lat: %f, long: %f\n", pos.latitude, pos.longitude);
        } else {
            printf("GPS does not have fix\n");
        }
    }
};