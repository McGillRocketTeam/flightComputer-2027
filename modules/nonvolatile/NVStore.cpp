#include <nonvolatile/NVStore.h>
#include <nonvolatile/24xx08/24xx08.h>

template <typename T>
STATUS_NVSTORE NVStore::writeParameter(NV_Param param, T& data){
    if(!data){
        return NVSTORE_ERROR;
    };

    uint16_t addr    = nv_param_addresses[(uint8_t) param];
    uint8_t size     = nv_param_sizes[(uint8_t) param];

    if(size == 0 || size != sizeof(T)){
        return NVSTORE_ERROR;
    }

    if(write_page(addr, (uint8_t*) data, size)){
        return NVSTORE_ERROR;
    }

    return NVSTORE_OK;
}

template <typename T>
STATUS_NVSTORE NVStore::readParameter(NV_Param param, T& out){
    if(!out){
        return NVSTORE_ERROR;
    };

    uint16_t addr    = nv_param_addresses[(uint8_t) param];
    uint8_t size     = nv_param_sizes[(uint8_t) param];

    if(size == 0 || size != sizeof(T)){
        return NVSTORE_ERROR;
    }

    if(read_data(addr, (uint8_t*) out, size)){
        return NVSTORE_ERROR;
    }

    return NVSTORE_OK;
}

STATUS_NVSTORE NVStore::loadAll(){
    STATUS_NVSTORE res = NVSTORE_OK;

    for(uint8_t i = 0; i < (uint8_t) NV_Param::NUM_PARAMS; i++){
        void* loc = volatile_locations[i];
        if(loc){
            if(readParameter((NV_Param)i, loc)){
                res = NVSTORE_ERROR;
            }
        }
    }

    return res;
}

STATUS_NVSTORE NVStore::saveAll(){
    STATUS_NVSTORE res = NVSTORE_OK;

    for(uint8_t i = 0; i < (uint8_t) NV_Param::NUM_PARAMS; i++){
        if(writeParameter((NV_Param)i, volatile_locations[i])){
            res = NVSTORE_ERROR;
        }
    }

    return res;
}

template <typename T>
uint8_t NVStore::bind(NV_Param param, T* location){
    if(!location){
        return -1;
    }

    volatile_locations[(uint8_t) param] = location;
    
    return 0;
}