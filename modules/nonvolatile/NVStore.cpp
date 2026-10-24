#include <nonvolatile/NVStore.h>
#include <nonvolatile/24xx08/24xx08.h>

NVStore::NVStore(){
}

STATUS_NVSTORE NVStore::writeParameterRaw(NV_Param param, const void* data){
    if(!data){
        return NVSTORE_ERROR;
    };

    if((uint8_t)param >= (uint8_t) NV_Param::NUM_PARAMS){
        return NVSTORE_ERROR;
    }

    uint16_t addr    = nv_param_addresses[(uint8_t) param];
    uint8_t size     = nv_param_sizes[(uint8_t) param];

    if(write_page(addr, (uint8_t*) data, size)){
        return NVSTORE_ERROR;
    }

    return NVSTORE_OK;
}

STATUS_NVSTORE NVStore::readParameterRaw(NV_Param param, void* out){
    if(!out){
        return NVSTORE_ERROR;
    };

    if((uint8_t)param >= (uint8_t) NV_Param::NUM_PARAMS){
        return NVSTORE_ERROR;
    }

    uint16_t addr    = nv_param_addresses[(uint8_t) param];
    uint8_t size     = nv_param_sizes[(uint8_t) param];

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
            if(readParameterRaw((NV_Param)i, loc)){
                res = NVSTORE_ERROR;
            }
        }
    }

    return res;
}

STATUS_NVSTORE NVStore::saveAll(){
    STATUS_NVSTORE res = NVSTORE_OK;

    for(uint8_t i = 0; i < (uint8_t) NV_Param::NUM_PARAMS; i++){
        if(writeParameterRaw((NV_Param)i, volatile_locations[i])){
            res = NVSTORE_ERROR;
        }
    }

    return res;
}