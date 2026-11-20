#include "MRT_main.h"
#include "gpio.h"
#include "cmsis_os.h"

#include <radios/AtomicRules.h>
#include <sensors/LIS2MDLapi.h>
#include <util/helpers.h>

// Called on Telemetry thread
void MRT_main(){
    // Safe to construct here: MX_I2C2_Init() runs in main() at startup, well
    // before osKernelStart() hands control to this thread.
    LIS2MDL_API magnetometer(true);

    if (magnetometer.get_constructor_status()) {
        printf("LIS2MDL ready, WHO_AM_I = 0x%02X\n", magnetometer.whoami());
    } else {
        printf("LIS2MDL init FAILED, WHO_AM_I = 0x%02X (expected 0x40)\n",
               magnetometer.whoami());
    }

    // A FreeRTOS thread function must never return.
    for (;;) {
        if (magnetometer.get_constructor_status()) {
            Vec3 m = magnetometer.getMagneticDataVec3();
            // Printed as integers on purpose: this links with --specs=nano.specs,
            // whose printf has no float support, so a %f here would emit nothing
            // useful. mgauss has plenty of range without the decimals.
            printf("mag [mgauss]  x=%d  y=%d  z=%d\n",
                   (int)m.x, (int)m.y, (int)m.z);
        }
        osDelay(500);
    }
}
