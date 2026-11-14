// AUTO-GENERATED FILE; DO NOT EDIT (Source: generate_sd_data.py)
// Last Generated: 2026-11-06 18:45:11.885903

#pragma once
#include <stdint.h>
#include <stddef.h>
#include <assert.h>

#define ATOMIC_VERSION "0"
// Designed for usage with 16-bit atomic bitmask


#define SAFE_COPY(src, dest) ((src) != nullptr ? ((dest) = *(src), true) : false)

enum SDAtomicTypes {
    SD_AT_FLIGHT = (0),
    SD_AT_FC = (1),
    SD_AT_GPS = (2),
    SD_AT_BAROMETER = (3),
    SD_AT_IMU = (4),
    SD_AT_MAGNETOMETER = (5),
    SD_AT_PROPULSION = (6),
    SD_AT_FLAGS = (7),
    SD_AT_TOTAL = (8),
};

#define SD_ATOMIC_BITMASK_LEN 2
static_assert(SD_AT_TOTAL <= (SD_ATOMIC_BITMASK_LEN * 8), "MORE THAN 16 SD ATOMICS DEFINED");

struct __attribute__((__packed__)) flight_data_SD_t {
    uint8_t flight_stage;
    float alt_agl;
    float alt_asl;
    float vertical_speed;
    float apogee;
    float main_alt;
    float booster_burnout_alt;
};

struct __attribute__((__packed__)) flight_ptr_SD_t {
    uint8_t* flight_stage;
    float* alt_agl;
    float* alt_asl;
    float* vertical_speed;
    float* apogee;
    float* main_alt;
    float* booster_burnout_alt;
};

union flight_atomic_SD_t {
    flight_data_SD_t data;
    uint8_t bytes[25];
};

struct __attribute__((__packed__)) fc_data_SD_t {
    uint32_t time_since_sw_reset;
    uint8_t battery_voltage;
    uint8_t current_draw;
    uint32_t time_since_radio_contact;
    uint16_t radio_frequency;
};

struct __attribute__((__packed__)) fc_ptr_SD_t {
    uint32_t* time_since_sw_reset;
    uint8_t* battery_voltage;
    uint8_t* current_draw;
    uint32_t* time_since_radio_contact;
    uint16_t* radio_frequency;
};

union fc_atomic_SD_t {
    fc_data_SD_t data;
    uint8_t bytes[12];
};

struct __attribute__((__packed__)) gps_data_SD_t {
    int32_t gps_lat;
    int32_t gps_long;
    uint16_t gps_alt;
    uint32_t time_since_fix;
};

struct __attribute__((__packed__)) gps_ptr_SD_t {
    int32_t* gps_lat;
    int32_t* gps_long;
    uint16_t* gps_alt;
    uint32_t* time_since_fix;
};

union gps_atomic_SD_t {
    gps_data_SD_t data;
    uint8_t bytes[14];
};

struct __attribute__((__packed__)) barometer_data_SD_t {
    float baro_pressure;
    uint8_t baro_temp;
};

struct __attribute__((__packed__)) barometer_ptr_SD_t {
    float* baro_pressure;
    uint8_t* baro_temp;
};

union barometer_atomic_SD_t {
    barometer_data_SD_t data;
    uint8_t bytes[5];
};

struct __attribute__((__packed__)) imu_data_SD_t {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
};

struct __attribute__((__packed__)) imu_ptr_SD_t {
    int16_t* accel_x;
    int16_t* accel_y;
    int16_t* accel_z;
    int16_t* gyro_x;
    int16_t* gyro_y;
    int16_t* gyro_z;
};

union imu_atomic_SD_t {
    imu_data_SD_t data;
    uint8_t bytes[12];
};

struct __attribute__((__packed__)) magnetometer_data_SD_t {
    int16_t mag_x;
    int16_t mag_y;
    int16_t mag_z;
};

struct __attribute__((__packed__)) magnetometer_ptr_SD_t {
    int16_t* mag_x;
    int16_t* mag_y;
    int16_t* mag_z;
};

union magnetometer_atomic_SD_t {
    magnetometer_data_SD_t data;
    uint8_t bytes[6];
};

struct __attribute__((__packed__)) propulsion_data_SD_t {
    uint16_t tank_top_pressure;
    uint16_t tank_bot_pressure;
    int8_t tank_top_temp;
    int8_t tank_bot_temp;
};

struct __attribute__((__packed__)) propulsion_ptr_SD_t {
    uint16_t* tank_top_pressure;
    uint16_t* tank_bot_pressure;
    int8_t* tank_top_temp;
    int8_t* tank_bot_temp;
};

union propulsion_atomic_SD_t {
    propulsion_data_SD_t data;
    uint8_t bytes[6];
};


struct __attribute__((__packed__)) flags_data_SD_t {
    bool CAN_bus_active : 1;
    bool camera_recording : 1;
    bool gps_fix : 1;
    bool recovery_armed : 1;
    bool main_armed : 1;
    bool main_energized : 1;
    bool main_continuity : 1;
    bool drogue_armed : 1;
    bool drogue_energized : 1;
    bool drogue_continuity : 1;
    bool propulsion_armed : 1;
    bool propulsion_on : 1;
    bool mov_armed : 1;
    bool mov_energized : 1;
    bool mov_continuity : 1;
    bool fdov_armed : 1;
    bool fdov_energized : 1;
    bool fdov_continuity : 1;
    bool vent_armed : 1;
    bool vent_energized : 1;
    bool vent_continuity : 1;
};

union flags_atomic_SD_t {
    flags_data_SD_t data;
    uint8_t bytes[3];
};

struct __attribute__((__packed__)) flags_ptr_SD_t {
    bool* CAN_bus_active;
    bool* camera_recording;
    bool* gps_fix;
    bool* recovery_armed;
    bool* main_armed;
    bool* main_energized;
    bool* main_continuity;
    bool* drogue_armed;
    bool* drogue_energized;
    bool* drogue_continuity;
    bool* propulsion_armed;
    bool* propulsion_on;
    bool* mov_armed;
    bool* mov_energized;
    bool* mov_continuity;
    bool* fdov_armed;
    bool* fdov_energized;
    bool* fdov_continuity;
    bool* vent_armed;
    bool* vent_energized;
    bool* vent_continuity;
};


inline bool fill_flight(flight_ptr_SD_t& ptr, flight_atomic_SD_t& out){
    bool ok = true;
    ok &= SAFE_COPY(ptr.flight_stage, out.data.flight_stage);
    ok &= SAFE_COPY(ptr.alt_agl, out.data.alt_agl);
    ok &= SAFE_COPY(ptr.alt_asl, out.data.alt_asl);
    ok &= SAFE_COPY(ptr.vertical_speed, out.data.vertical_speed);
    ok &= SAFE_COPY(ptr.apogee, out.data.apogee);
    ok &= SAFE_COPY(ptr.main_alt, out.data.main_alt);
    ok &= SAFE_COPY(ptr.booster_burnout_alt, out.data.booster_burnout_alt);
    return ok;
};

inline bool fill_fc(fc_ptr_SD_t& ptr, fc_atomic_SD_t& out){
    bool ok = true;
    ok &= SAFE_COPY(ptr.time_since_sw_reset, out.data.time_since_sw_reset);
    ok &= SAFE_COPY(ptr.battery_voltage, out.data.battery_voltage);
    ok &= SAFE_COPY(ptr.current_draw, out.data.current_draw);
    ok &= SAFE_COPY(ptr.time_since_radio_contact, out.data.time_since_radio_contact);
    ok &= SAFE_COPY(ptr.radio_frequency, out.data.radio_frequency);
    return ok;
};

inline bool fill_gps(gps_ptr_SD_t& ptr, gps_atomic_SD_t& out){
    bool ok = true;
    ok &= SAFE_COPY(ptr.gps_lat, out.data.gps_lat);
    ok &= SAFE_COPY(ptr.gps_long, out.data.gps_long);
    ok &= SAFE_COPY(ptr.gps_alt, out.data.gps_alt);
    ok &= SAFE_COPY(ptr.time_since_fix, out.data.time_since_fix);
    return ok;
};

inline bool fill_barometer(barometer_ptr_SD_t& ptr, barometer_atomic_SD_t& out){
    bool ok = true;
    ok &= SAFE_COPY(ptr.baro_pressure, out.data.baro_pressure);
    ok &= SAFE_COPY(ptr.baro_temp, out.data.baro_temp);
    return ok;
};

inline bool fill_imu(imu_ptr_SD_t& ptr, imu_atomic_SD_t& out){
    bool ok = true;
    ok &= SAFE_COPY(ptr.accel_x, out.data.accel_x);
    ok &= SAFE_COPY(ptr.accel_y, out.data.accel_y);
    ok &= SAFE_COPY(ptr.accel_z, out.data.accel_z);
    ok &= SAFE_COPY(ptr.gyro_x, out.data.gyro_x);
    ok &= SAFE_COPY(ptr.gyro_y, out.data.gyro_y);
    ok &= SAFE_COPY(ptr.gyro_z, out.data.gyro_z);
    return ok;
};

inline bool fill_magnetometer(magnetometer_ptr_SD_t& ptr, magnetometer_atomic_SD_t& out){
    bool ok = true;
    ok &= SAFE_COPY(ptr.mag_x, out.data.mag_x);
    ok &= SAFE_COPY(ptr.mag_y, out.data.mag_y);
    ok &= SAFE_COPY(ptr.mag_z, out.data.mag_z);
    return ok;
};

inline bool fill_propulsion(propulsion_ptr_SD_t& ptr, propulsion_atomic_SD_t& out){
    bool ok = true;
    ok &= SAFE_COPY(ptr.tank_top_pressure, out.data.tank_top_pressure);
    ok &= SAFE_COPY(ptr.tank_bot_pressure, out.data.tank_bot_pressure);
    ok &= SAFE_COPY(ptr.tank_top_temp, out.data.tank_top_temp);
    ok &= SAFE_COPY(ptr.tank_bot_temp, out.data.tank_bot_temp);
    return ok;
};

inline bool fill_flags(flags_ptr_SD_t& ptr, flags_atomic_SD_t& out){
    bool ok = true;
    if(ptr.CAN_bus_active != nullptr){out.data.CAN_bus_active = *ptr.CAN_bus_active; } else {ok = false;}
    if(ptr.camera_recording != nullptr){out.data.camera_recording = *ptr.camera_recording; } else {ok = false;}
    if(ptr.gps_fix != nullptr){out.data.gps_fix = *ptr.gps_fix; } else {ok = false;}
    if(ptr.recovery_armed != nullptr){out.data.recovery_armed = *ptr.recovery_armed; } else {ok = false;}
    if(ptr.main_armed != nullptr){out.data.main_armed = *ptr.main_armed; } else {ok = false;}
    if(ptr.main_energized != nullptr){out.data.main_energized = *ptr.main_energized; } else {ok = false;}
    if(ptr.main_continuity != nullptr){out.data.main_continuity = *ptr.main_continuity; } else {ok = false;}
    if(ptr.drogue_armed != nullptr){out.data.drogue_armed = *ptr.drogue_armed; } else {ok = false;}
    if(ptr.drogue_energized != nullptr){out.data.drogue_energized = *ptr.drogue_energized; } else {ok = false;}
    if(ptr.drogue_continuity != nullptr){out.data.drogue_continuity = *ptr.drogue_continuity; } else {ok = false;}
    if(ptr.propulsion_armed != nullptr){out.data.propulsion_armed = *ptr.propulsion_armed; } else {ok = false;}
    if(ptr.propulsion_on != nullptr){out.data.propulsion_on = *ptr.propulsion_on; } else {ok = false;}
    if(ptr.mov_armed != nullptr){out.data.mov_armed = *ptr.mov_armed; } else {ok = false;}
    if(ptr.mov_energized != nullptr){out.data.mov_energized = *ptr.mov_energized; } else {ok = false;}
    if(ptr.mov_continuity != nullptr){out.data.mov_continuity = *ptr.mov_continuity; } else {ok = false;}
    if(ptr.fdov_armed != nullptr){out.data.fdov_armed = *ptr.fdov_armed; } else {ok = false;}
    if(ptr.fdov_energized != nullptr){out.data.fdov_energized = *ptr.fdov_energized; } else {ok = false;}
    if(ptr.fdov_continuity != nullptr){out.data.fdov_continuity = *ptr.fdov_continuity; } else {ok = false;}
    if(ptr.vent_armed != nullptr){out.data.vent_armed = *ptr.vent_armed; } else {ok = false;}
    if(ptr.vent_energized != nullptr){out.data.vent_energized = *ptr.vent_energized; } else {ok = false;}
    if(ptr.vent_continuity != nullptr){out.data.vent_continuity = *ptr.vent_continuity; } else {ok = false;}
    return ok;
};

extern const uint16_t SD_AT_SIZE[SD_AT_TOTAL];

#define SD_FLAG_CAN_BUS_ACTIVE (1u<<0)
#define SD_FLAG_CAMERA_RECORDING (1u<<1)
#define SD_FLAG_GPS_FIX (1u<<2)
#define SD_FLAG_RECOVERY_ARMED (1u<<3)
#define SD_FLAG_MAIN_ARMED (1u<<4)
#define SD_FLAG_MAIN_ENERGIZED (1u<<5)
#define SD_FLAG_MAIN_CONTINUITY (1u<<6)
#define SD_FLAG_DROGUE_ARMED (1u<<7)
#define SD_FLAG_DROGUE_ENERGIZED (1u<<8)
#define SD_FLAG_DROGUE_CONTINUITY (1u<<9)
#define SD_FLAG_PROPULSION_ARMED (1u<<10)
#define SD_FLAG_PROPULSION_ON (1u<<11)
#define SD_FLAG_MOV_ARMED (1u<<12)
#define SD_FLAG_MOV_ENERGIZED (1u<<13)
#define SD_FLAG_MOV_CONTINUITY (1u<<14)
#define SD_FLAG_FDOV_ARMED (1u<<15)
#define SD_FLAG_FDOV_ENERGIZED (1u<<16)
#define SD_FLAG_FDOV_CONTINUITY (1u<<17)
#define SD_FLAG_VENT_ARMED (1u<<18)
#define SD_FLAG_VENT_ENERGIZED (1u<<19)
#define SD_FLAG_VENT_CONTINUITY (1u<<20)
