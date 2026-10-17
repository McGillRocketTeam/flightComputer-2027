#pragma once
#include "telemetry/Telemetry.h"
#include "recovery/EnergizeDB.h"
#include "recovery/KalmanFilter.h"

class Recovery{
    public:
     Recovery(Telemetry& telemetry);

     void addEnergizeDB(EnergizeDB& energize);

     int updateFlightStage();

     float calculateAltitude(float pressure);
     float getCurrentAltitude(void);

     float getAltitudeASL();
     float getAltitudeAGL();

     float getApogeeAltitude();
     float getMainAltitude();

     Telemetry::flight_stage getFlightStage();
     void setFlightStage();

    private:
     KalmanFilter altitudeFilter;

     float currentAltitude;
     float padAltitude;
     float apogeeAltitude;
     float mainAltitude;
};