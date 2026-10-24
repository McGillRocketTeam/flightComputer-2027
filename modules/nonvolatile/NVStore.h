#pragma once
#include <nonvolatile/MRT_eeprom_map.h>

typedef enum {
    NVSTORE_OK = 0,
    NVSTORE_ERROR = -1,
} STATUS_NVSTORE;

class NVStore {
   public:
    NVStore();

    STATUS_NVSTORE writeParameterRaw(NV_Param param, const void* data);

    STATUS_NVSTORE readParameterRaw(NV_Param param, void* data);

    template <typename T>
    STATUS_NVSTORE writeParameter(NV_Param param, T& data);

    template <typename T>
    STATUS_NVSTORE readParameter(NV_Param param, T& out);

    // Reads all parameters configured with a volatile location from NV -> V.
    // Loads FROM EEPROM
    STATUS_NVSTORE loadAll();

    // Writes all parameters configured with a volatile location from V -> NV.
    // Saves TO EEPROM
    STATUS_NVSTORE saveAll();

    // Binds a field for bulk save/restore
    // Associates a pointer with an NV_Param field
    template <typename T>
    STATUS_NVSTORE bind(NV_Param param, T* location);

   private:
    void* volatile_locations[(uint8_t)NV_Param::NUM_PARAMS]{nullptr};
};

// Implementations of generics:
template <typename T>
inline STATUS_NVSTORE NVStore::writeParameter(NV_Param param, T& data) {
    const uint8_t index = (uint8_t)param;

    if (index >= (uint8_t)NV_Param::NUM_PARAMS) {
        return NVSTORE_ERROR;
    }
    if ((uint16_t)sizeof(T) != nv_param_sizes[index]) {
        return NVSTORE_ERROR;
    }

    return writeParameterRaw(param, static_cast<const void*>(&data));
}

template <typename T>
inline STATUS_NVSTORE NVStore::readParameter(NV_Param param, T& data) {
    const uint8_t index = (uint8_t)param;

    if (index >= (uint8_t)NV_Param::NUM_PARAMS) {
        return NVSTORE_ERROR;
    }
    if ((uint16_t)sizeof(T) != nv_param_sizes[index]) {
        return NVSTORE_ERROR;
    }

    return readParameterRaw(param, static_cast<void*>(&data));
}

template <typename T>
inline STATUS_NVSTORE NVStore::bind(NV_Param param, T* location) {
    if (!location) {
        return NVSTORE_ERROR;
    }

    const uint8_t index = (uint8_t)param;

    if (index >= (uint8_t)NV_Param::NUM_PARAMS) {
        return NVSTORE_ERROR;
    }

    if ((uint16_t)sizeof(T) != nv_param_sizes[index]) {
        return NVSTORE_ERROR;
    }

    volatile_locations[index] = location;

    return NVSTORE_OK;
}