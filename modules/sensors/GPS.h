#pragma once
#include "sensors/Sensor.h"

class GPS : Sensor {
    public:
        void init() override;
        ErrorCode status() override;
        bool isInitialized() override;
        void poll() override;
};
