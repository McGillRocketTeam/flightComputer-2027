#pragma once
#include "sensors/Sensor.h"
#include "drivers/lsm6dsr/lsm6dsr_reg.h"
#include <cstdint>

class IMU : public Sensor {
public:
    // Constructor: receive dependencies, no hardware transactions.
   
    // init(): configure hardware.
    void init();
    // poll(): collect available readings.
    void poll();
    // const getters: return cached readings.
    const float* getAcceleration() const;
    const float* getAngularRate() const;
    float getTemperature() const;

private:
    stmdev_ctx_t context_{};
    
	//Variables
	float acceleration_mg[3];
	float angular_rate_mdps[3];
	float temperature_degC;
	uint8_t whoamI;
	uint8_t rst;
	uint8_t reg;
	int16_t data_raw_acceleration[3];
	int16_t data_raw_angular_rate[3];
	int16_t data_raw_temperature;

	//User functions
	float* (*getAcceleration)(struct LSM6DSR_t* self);
	float* (*getAngularRate)(struct LSM6DSR_t* self);
	float (*getTemperature)(struct LSM6DSR_t* self);
	void (*pollAll)(struct LSM6DSR_t* self);

	//Helper functions
	int (*printf)(const char* s, ...);
	void (*delay)(int ms);
	void* ctx_handle;

    // Cached raw and converted measurements.
    // Initialization and data-validity state.
    
};