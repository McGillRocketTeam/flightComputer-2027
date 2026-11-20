#pragma once

#include "LIS2MDL/MRT_LIS2MDL.h"

/* LIS2MDL API frontend
 *
 * API frontend of the electormagnetic sensor LIS2MDL
 * 
 * API constructors:
 *
 *  default constructor:
 *  - pass in all function pointers (can be nullptr in the defualt contructor)
 *  - Abstract contructor : bool verbose (turns on verbosity or not)
 *
 * API details & methods
 * get_status --> return the exit code of the constructor, 0 if successful
 * getMagneticData --> returns Vec3 (x, y, z) 
 * returns of the electromganetic reading of the magnetometer
 *
 *
 */
#ifdef __cplusplus
using I2CWriteFn = int32_t (*)(void* bus, uint8_t reg, const uint8_t* buf, uint16_t len);
using I2CReadFn  = int32_t (*)(void* bus, uint8_t reg, uint8_t* buf, uint16_t len);
using DelayFn    = void (*)(int ms);
using PrintfFn   = int (*)(const char* s, ...);

struct Vec3 {
    float x;
    float y;
    float z;

    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
};

class LIS2MDL_API {


private:

    // Was 0, which meant Init's "wait sensor boot time" called delay(0) and
    // waited for nothing. 100ms is what the 2026 flight computer used
    // (LIS2MDL_BOOT_TIME in MRT_setup.h) and it comfortably covers the
    // datasheet's power-up time.
    static constexpr uint8_t BOOT_TIME = 100;
    static constexpr uint16_t DELAY = 500;

    LIS2MDL_t _dev{};        // <-- here
    bool      _ok = false;
    void*     _bus = nullptr;

public:

    //   i2c address   the LIS2MDL answers on one address, LIS2MDL_I2C_ADD
    //   expected id   WHO_AM_I always reads back LIS2MDL_ID
    //   i2c_write     STM32 HAL_I2C_Mem_Write, wrapped in the .cpp
    //   i2c_read      STM32 HAL_I2C_Mem_Read, wrapped in the .cpp
    //   delay         delay() from util/helpers.h
    //   printf        printf() from util/helpers.h
    //   i2c_bus       hi2c2, the sensor bus — see which_bus() below
    //
    // 'verbose' is the only thing left, because it is a choice rather than a
    // fact about the board.
    explicit LIS2MDL_API(bool verbose = false);

    // Explicit-injection constructor, kept for host-side builds and tests:
    // the C layer takes function pointers precisely so the driver can run
    // against a mock I2C instead of real STM32 HAL. Pass a bus handle and the
    // four platform functions yourself. 'address' is compared against WHO_AM_I
    // by MRT_LIS2MDL_Init, so it wants LIS2MDL_ID, not the I2C address.
    LIS2MDL_API(void* i2c_bus,
                uint8_t address,
                I2CWriteFn i2c_write,
                I2CReadFn  i2c_read,
                DelayFn    delay,
                PrintfFn   printf_fn,
                bool verbose = false);

    ~LIS2MDL_API();



    Vec3 getMagneticDataVec3();

    bool get_constructor_status() const;

    uint8_t whoami() const;

    // Which I2C_HandleTypeDef the sensor was found on, as a void* so this
    // header stays free of HAL types. nullptr if the probe found nothing,
    // which is also when get_constructor_status() is false.
    void* which_bus() const;

};

// idea : constants file to or ganize run and compile time constants.
#endif