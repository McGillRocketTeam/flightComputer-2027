#pragma once
#include <sd_card/data/MRT_sd_data.h>
#include <sd_card/SD_ASTRA_frame_builder.h>
#include "fatfs.h"

#define SD_BUF_LEN 256
#define FILENAME ("FCLOG_A" ATOMIC_VERSION ".MRT")
#define BYTES_BEFORE_SYNC 4096

enum ERROR_SDCARD{
    SDCARD_OK = 0,
    SDCARD_ERROR,
    SDCARD_FILENOTFOUND,
    SDCARD_NODRIVE,
    SDCARD_NOTMOUNTED,
};
class SDCard{
    public:
    SDCard();

    bool isMounted();
    bool isOpen();

    FRESULT mount();
    bool openLogFile();
    FRESULT closeLogFile();

    // Packet method calls fill, and puts result to buf
    bool packet(uint16_t bitmask);
    ERROR_SDCARD fill(uint16_t bitmask);

    // Writes from buf -> SD Card
    bool write(size_t len);

    bool inserted = false;
    bool mounted  = false;
    bool opened   = false;

    // Bytes since last sync
    uint16_t cached = 0;

    // FATFS Objects
    FATFS file_system;
    FIL   file;

    // Pointer structs holding data sources
    // Ideally these all point to long lifecycle objects in Telemetry
    flight_ptr_SD_t flight_ptr;
    fc_ptr_SD_t fc_ptr;
    gps_ptr_SD_t gps_ptr;
    barometer_ptr_SD_t barometer_ptr;
    imu_ptr_SD_t imu_ptr;
    magnetometer_ptr_SD_t magnetometer_ptr;
    propulsion_ptr_SD_t propulsion_ptr;
    flags_ptr_SD_t flags_ptr;

    flight_atomic_SD_t flight_atomic;
    fc_atomic_SD_t fc_atomic;
    gps_atomic_SD_t gps_atomic;
    barometer_atomic_SD_t barometer_atomic;
    imu_atomic_SD_t imu_atomic;
    magnetometer_atomic_SD_t magnetometer_atomic;
    propulsion_atomic_SD_t propulsion_atomic;
    flags_atomic_SD_t flags_atomic;

    uint16_t active_atomics = {0};

    uint8_t buf[SD_BUF_LEN];

    SD_FrameBuilder fb{buf, SD_BUF_LEN};

    void bind_flight(flight_ptr_SD_t ptr){              this->flight_ptr = ptr;         active_atomics |= (1u << SD_AT_FLIGHT);}
    void bind_fc(fc_ptr_SD_t ptr){                      this->fc_ptr = ptr;             active_atomics |= (1u << SD_AT_FC);}
    void bind_gps(gps_ptr_SD_t ptr){                    this->gps_ptr = ptr;            active_atomics |= (1u << SD_AT_GPS);}
    void bind_barometer(barometer_ptr_SD_t ptr){        this->barometer_ptr = ptr;      active_atomics |= (1u << SD_AT_BAROMETER);}
    void bind_imu(imu_ptr_SD_t ptr){                    this->imu_ptr = ptr;            active_atomics |= (1u << SD_AT_IMU);}
    void bind_magnetometer(magnetometer_ptr_SD_t ptr){  this->magnetometer_ptr = ptr;   active_atomics |= (1u << SD_AT_MAGNETOMETER);}
    void bind_propulsion(propulsion_ptr_SD_t ptr){      this->propulsion_ptr = ptr;     active_atomics |= (1u << SD_AT_PROPULSION);}
    void bind_flags(flags_ptr_SD_t ptr){                this->flags_ptr = ptr;          active_atomics |= (1u << SD_AT_FLAGS);}

};