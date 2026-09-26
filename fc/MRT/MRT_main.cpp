#include "MRT_main.h"
#include "gpio.h"
#include "cmsis_os.h"

#include <radios/AtomicRules.h>

// Called on Telemetry thread
void MRT_main(){

    test_atomic_rules();
}