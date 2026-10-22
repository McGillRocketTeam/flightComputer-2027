#pragma once
#include <nonvolatile/MRT_eeprom_map.h>

typedef enum {
    NVSTORE_OK = 0,
    NVSTORE_ERROR = -1,
} STATUS_NVSTORE;

class NVStore {
   public:

    NVStore();

    template <typename T>
    STATUS_NVSTORE writeParameter(NV_Param param, T& data);

    template <typename T>
    STATUS_NVSTORE readParameter(NV_Param param, T& out);

    // Reads all parameters configured with a volatile location from NV -> V. Loads FROM EEPROM
    STATUS_NVSTORE loadAll();

    // Writes all parameters configured with a volatile location from V -> NV. Saves TO EEPROM
    STATUS_NVSTORE saveAll();

    // Binds a field for bulk save/restore
    // Associates a pointer with an NV_Param field
    template <typename T>
    uint8_t bind(NV_Param param, T* location);

    private:
        void* volatile_locations [(uint8_t) NV_Param::NUM_PARAMS];
};