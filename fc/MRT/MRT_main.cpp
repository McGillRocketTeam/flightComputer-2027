#include "MRT_main.h"
#include "gpio.h"
#include "cmsis_os.h"

#include <radios/AtomicRules.h>

// Called on Telemetry thread
void MRT_main(){
    NVStore store{};

    bool status = true;
    store.writeParameter(NV_Param::CAMERA_REC, status);
}