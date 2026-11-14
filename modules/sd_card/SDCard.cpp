#include <sd_card/SDCard.h>
#include "util/helpers.h"

SDCard::SDCard(){}

bool SDCard::isMounted(){
    inserted = HAL_GPIO_ReadPin(IN_SD_CARD_DETECT_GPIO_Port, IN_SD_CARD_DETECT_Pin) == GPIO_PIN_RESET;

    return inserted && mounted;
};

bool SDCard::isOpen(){
    return opened;
};

FRESULT SDCard::mount(){
    inserted = HAL_GPIO_ReadPin(IN_SD_CARD_DETECT_GPIO_Port, IN_SD_CARD_DETECT_Pin) == GPIO_PIN_RESET;

    FRESULT res = FR_DISK_ERR;

    if(inserted){
        res = f_mount(&file_system, "", 1);
        mounted = (res == FR_OK);
    }

    return res;
};

bool SDCard::openLogFile(){
    const char* path = FILENAME;

    FRESULT open_result = f_open(&file, path, FA_WRITE | FA_OPEN_ALWAYS);
    // This can default to success, unless it is short circuited by a failed open, it will be read before being used
    FRESULT jump_result = FR_OK;

    if(!open_result){
        jump_result = f_lseek(&file, f_size(&file));
    }

    cached = 0;

    opened = !open_result && !jump_result;

    return !open_result && !jump_result;
};

FRESULT SDCard::closeLogFile(){
    opened = false;
    cached = 0;
    return f_close(&file);
}

// Writes data to SD Card's currently open file from buf
bool SDCard::write(size_t len){
    if(!isMounted() || !isOpen()){
        return false;
    }

    UINT    bytes_written;
    FRESULT write_result = f_write(&file, buf, len, &bytes_written);

    FRESULT sync_result = FR_OK;

    if (!write_result && len + cached > BYTES_BEFORE_SYNC){
        sync_result = f_sync(&file);
        if(!sync_result){
            cached = 0;
        } else {
            cached += bytes_written;
        }
    } else {
        cached += bytes_written;
    }

    return !write_result && !sync_result && (bytes_written == len);
};

bool SDCard::packet(uint16_t bitmask){
    if(!isMounted() || !isOpen()){
        return false;
    }

    fb.reset();

    // Zero out all atomics
    flight_atomic = {0};
    fc_atomic = {0};
    gps_atomic = {0};
    barometer_atomic = {0};
    imu_atomic = {0};
    magnetometer_atomic = {0};
    propulsion_atomic = {0};
    flags_atomic = {0};

    // Pull relevant packets from stored pointers
    fill(bitmask);

    // Clear buffer
    memset(buf, 0, SD_BUF_LEN);

    printf("Filling Atomics\n");

    for(int i = 0; i < 16; i++){
        if((bitmask & (1u<<i)) && (active_atomics & (1u<<i))){
            switch((SDAtomicTypes) i){
                case SD_AT_FLIGHT:
                    fb.addAtomic_SD(i, &flight_atomic, sizeof(flight_atomic));
                    break;
                case SD_AT_FC:
                    fb.addAtomic_SD(i, &fc_atomic, sizeof(fc_atomic));
                    break;
                case SD_AT_GPS:
                    fb.addAtomic_SD(i, &gps_atomic, sizeof(gps_atomic));
                    break;
                case SD_AT_BAROMETER:
                    fb.addAtomic_SD(i, &barometer_atomic, sizeof(barometer_atomic));
                    break;
                case SD_AT_IMU:
                    fb.addAtomic_SD(i, &imu_atomic, sizeof(imu_atomic));
                    break;
                case SD_AT_MAGNETOMETER:
                    fb.addAtomic_SD(i, &magnetometer_atomic, sizeof(magnetometer_atomic));
                    break;
                case SD_AT_PROPULSION:
                    fb.addAtomic_SD(i, &propulsion_atomic, sizeof(propulsion_atomic));
                    break;
                case SD_AT_FLAGS:
                    fb.addAtomic_SD(i, &flags_atomic, sizeof(flags_atomic));
                    break;
                default:
                    break;
            };
        }
    }

    printf("Bitmask: %d\n", bitmask);

    size_t used = fb.finalize_SD();

    return write(used);
}

ERROR_SDCARD SDCard::fill(uint16_t bitmask){
    for(int i = 0; i < 16; i++){
        // If atomic requested and bound
        if((bitmask & (1u<<i)) && (active_atomics & (1u<<i))){
            switch((SDAtomicTypes) i){
                case SD_AT_FLIGHT:
                    fill_flight(flight_ptr, flight_atomic);
                    break;
                case SD_AT_FC:
                    fill_fc(fc_ptr, fc_atomic);
                    break;
                case SD_AT_GPS:
                    fill_gps(gps_ptr, gps_atomic);
                    break;
                case SD_AT_BAROMETER:
                    fill_barometer(barometer_ptr, barometer_atomic);
                    break;
                case SD_AT_IMU:
                    fill_imu(imu_ptr, imu_atomic);
                    break;
                case SD_AT_MAGNETOMETER:
                    fill_magnetometer(magnetometer_ptr, magnetometer_atomic);
                    break;
                case SD_AT_PROPULSION:
                    fill_propulsion(propulsion_ptr, propulsion_atomic);
                    break;
                case SD_AT_FLAGS:
                    fill_flags(flags_ptr, flags_atomic);
                    break;
                default:
                    break;
            }
        }
    }
    return SDCARD_OK;
}