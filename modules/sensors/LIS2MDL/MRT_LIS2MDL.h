#ifndef INC_MRT_LIS2MDL_H_
#define INC_MRT_LIS2MDL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifdef LOCAL_LIS2MDL
#include "sensors/LIS2MDL/lis2mdl_reg.h"
#endif

typedef struct LIS2MDL_t {
	// init/config
	uint8_t reg;
	uint8_t whoamI; // bool
	uint8_t rst;

	// data
	float magnetic_mgauss[3];
	int16_t data_raw_magnetic[3];
	
	// Helper functions
	float *(*getMagneticData)(struct LIS2MDL_t *self);
	int (*printf)(const char* s, ...);
	void (*delay)(int ms);

	//Private (not really but not useful externally).
	void *ctx_handle;
} LIS2MDL_t;



//**************************************************//
/*****PUBLIC FUNCTIONS DECLARATIONS*****/

// constructor function

uint8_t MRT_LIS2MDL_Init(LIS2MDL_t* lis2mdl_handler, 
        void* i2c_bus, 
        uint8_t address, 
        uint8_t boot_time,
		int32_t (*i2c_write)(void *MRT_i2c, uint8_t reg, 
        const uint8_t *bufp, uint16_t len),
		int32_t (*i2c_read)(void *MRT_i2c, 
        uint8_t reg, 
        uint8_t *bufp, 
        uint16_t len),
		void (*delay)(int ms),
        uint8_t verbose);



#ifdef __cplusplus
}
#endif

#endif /* INC_MRT_LIS2MDL_H_ */
