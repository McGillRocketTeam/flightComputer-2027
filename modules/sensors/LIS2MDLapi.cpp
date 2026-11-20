// LOCAL_LIS2MDL makes MRT_LIS2MDL.h pull in lis2mdl_reg.h, which is where
// LIS2MDL_I2C_ADD / LIS2MDL_ID / LIS2MDL_WHO_AM_I live. Must precede the
// header include.
#define LOCAL_LIS2MDL
#include "LIS2MDLapi.h"

#include <stdlib.h>

#include "util/helpers.h" // printf, delay
#include <i2c.h>          // hi2c1 / hi2c2 / hi2c3

namespace {

// HAL_I2C_Mem_* timeout.
constexpr uint32_t IO_TIMEOUT_MS = 100;

// Which bus the magnetometer hangs off. The 2026 flight computer put the whole
// sensor stack on I2C2 — LSM6DSR, LPS22HH, LPS22HB, MS5611 and the LIS2MDL all
// shared it, with only the INA219 on I2C3 (MRT_setup.h, identical in the
// megaloop and rtos variants). One line to change if the harness moves.
I2C_HandleTypeDef* const SENSOR_BUS = &hi2c2;

// The two platform functions the ST register layer calls. Signatures are
// fixed by stmdev_write_ptr / stmdev_read_ptr in lis2mdl_reg.h; 0 means OK and
// non-zero is an error, which is the convention the rest of that layer uses.
int32_t hal_i2c_write(void* bus, uint8_t reg, const uint8_t* buf, uint16_t len) {
    // HAL takes a non-const pData even though a write never modifies it.
    return HAL_I2C_Mem_Write(static_cast<I2C_HandleTypeDef*>(bus),
                             LIS2MDL_I2C_ADD,
                             reg,
                             I2C_MEMADD_SIZE_8BIT,
                             const_cast<uint8_t*>(buf),
                             len,
                             IO_TIMEOUT_MS) == HAL_OK
               ? 0 : -1;
}

int32_t hal_i2c_read(void* bus, uint8_t reg, uint8_t* buf, uint16_t len) {
    return HAL_I2C_Mem_Read(static_cast<I2C_HandleTypeDef*>(bus),
                            LIS2MDL_I2C_ADD,
                            reg,
                            I2C_MEMADD_SIZE_8BIT,
                            buf,
                            len,
                            IO_TIMEOUT_MS) == HAL_OK
               ? 0 : -1;
}

} // end namespace

LIS2MDL_API::LIS2MDL_API(bool verbose)
    : LIS2MDL_API(SENSOR_BUS,
                  LIS2MDL_ID,
                  hal_i2c_write,
                  hal_i2c_read,
                  delay,
                  printf,
                  verbose) {}

LIS2MDL_API::LIS2MDL_API(void* i2c_bus,
                         uint8_t address,
                         I2CWriteFn i2c_write,
                         I2CReadFn  i2c_read,
                         DelayFn    delay,
                         PrintfFn   printf_fn,
                         bool verbose){
    _dev.printf = printf_fn;               // Init() calls self->printf before setting it
    _bus = i2c_bus;

    // Only reachable through the injected constructor, since the flight one
    if (i2c_bus == nullptr) {
        if (verbose && printf_fn != nullptr) {
            printf_fn("LIS2MDL: no I2C bus given\n");
        }
        _ok = false;
        return;
    }

    _ok = MRT_LIS2MDL_Init(&_dev,          // C self
                           i2c_bus,
                           address,
                           BOOT_TIME,
                           i2c_write,
                           i2c_read,
                           delay,
                           verbose);
}

LIS2MDL_API::~LIS2MDL_API() {
    free(_dev.ctx_handle);
    _dev.ctx_handle = nullptr;
}

Vec3 LIS2MDL_API::getMagneticDataVec3() {
    const float* data = _dev.getMagneticData(&_dev);
    return Vec3(data[0], data[1], data[2]);
}

bool LIS2MDL_API::get_constructor_status() const {
    return _ok;
}

uint8_t LIS2MDL_API::whoami() const {
    return _dev.whoamI;
}

void* LIS2MDL_API::which_bus() const {
    return _bus;
}
