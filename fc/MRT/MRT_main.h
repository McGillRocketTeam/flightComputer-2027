#pragma once 
#include <util/HAL.h>

#ifdef __cplusplus
extern "C" {
#endif

void MRT_main();

#ifdef __cplusplus
}
#endif


/* Thread Plan:
    Watchdog
    Commands
    Recovery
    Telemetry
    CAN (But better, without crashing program)
*/