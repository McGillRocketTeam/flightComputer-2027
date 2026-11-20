#define LOCAL_LIS2MDL
#include "sensors/LIS2MDL/MRT_LIS2MDL.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../modules/util/helpers.h"
// float *MRT_LIS2MDL_getMagneticData(LIS2MDL_t *self);

float *MRT_LIS2MDL_getMagneticData(LIS2MDL_t *self) {
	/* Read output only if new value is available */
	lis2mdl_mag_data_ready_get((stmdev_ctx_t*)self->ctx_handle, &self->reg);
	if (self->reg) {
		memset(self->data_raw_magnetic, 0x00, 3 * sizeof(int16_t));
		lis2mdl_magnetic_raw_get((stmdev_ctx_t*)self->ctx_handle, self->data_raw_magnetic);
		self->magnetic_mgauss[0] = lis2mdl_from_lsb_to_mgauss(self->data_raw_magnetic[0]);
		self->magnetic_mgauss[1] = lis2mdl_from_lsb_to_mgauss(self->data_raw_magnetic[1]);
		self->magnetic_mgauss[2] = lis2mdl_from_lsb_to_mgauss(self->data_raw_magnetic[2]);
	}
	return self->magnetic_mgauss;
}

// printf -> bool verbose 
uint8_t MRT_LIS2MDL_Init(LIS2MDL_t* self, void* i2c_bus, 
        uint8_t address, uint8_t boot_time,
		int32_t (*i2c_write)(void *MRT_i2c, 
        uint8_t reg, const uint8_t *bufp, uint16_t len),
		int32_t (*i2c_read)(void *MRT_i2c,
        uint8_t reg, uint8_t *bufp, uint16_t len),
		void (*delay)(int ms),
        uint8_t verbose)
{
	self->delay = delay;
	self->getMagneticData = MRT_LIS2MDL_getMagneticData;

	self->printf("LIS2MDL Init\n");
	self->ctx_handle = (stmdev_ctx_t *)malloc(sizeof(stmdev_ctx_t));

	/* Initialize mems driver interface */
	((stmdev_ctx_t*)self->ctx_handle)->write_reg = i2c_write;
	((stmdev_ctx_t*)self->ctx_handle)->read_reg = i2c_read;
	((stmdev_ctx_t*)self->ctx_handle)->handle = i2c_bus;

	/* Wait sensor boot time */
	self->delay(boot_time);
	
	/* Check device ID */
	self->whoamI = 0;
	lis2mdl_device_id_get((stmdev_ctx_t*)self->ctx_handle, &self->whoamI);
	
    if (verbose) {
        char buf[16];
        snprintf(buf, sizeof buf, "0x%02X", self->whoamI);
        println("Checking sensor ID...");
        if (self->whoamI != address) {
            println("NOT OK\nThis device is: ");
            println(buf);
            return 0;
        } 
    }
	
	self->printf("OK\n");
	self->printf("Restore default configuration...\n");

	// Configure sensor
	lis2mdl_reset_set((stmdev_ctx_t*)self->ctx_handle, PROPERTY_ENABLE);
	self->delay(500);
	do {
		lis2mdl_reset_get((stmdev_ctx_t*)self->ctx_handle, &self->rst);
	} while (self->rst);

	self->printf("Reset\n");

	/* Enable Block Data Update */
	lis2mdl_block_data_update_set((stmdev_ctx_t*)self->ctx_handle, PROPERTY_ENABLE);

	/* Set Output Data Rate */
	lis2mdl_data_rate_set((stmdev_ctx_t*)self->ctx_handle, LIS2MDL_ODR_20Hz);


	lis2mdl_operating_mode_set((stmdev_ctx_t*)self->ctx_handle, LIS2MDL_CONTINUOUS_MODE);
	/* Low Pass Filter */
	// lis2mdl_low_pass_bandwidth_set

	return 1;
}
