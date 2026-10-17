// AUTO-GENERATED. Do not edit by hand. Generated on: 2026-08-03 21:46
#pragma once
#ifdef __cplusplus
#include <cstdint>

// ---------- AtomicType Enum ----------
enum AtomicType {
    AT_RECOV_ATOMIC         = 0,  // bit 0
    AT_PROP_STATES_ATOMIC   = 1,  // bit 1
    AT_PROP_ATOMIC          = 2,  // bit 2
    AT_FLIGHT_STAGE_ATOMIC  = 3,  // bit 3
    AT_FC_INTERNAL_ATOMIC   = 4,  // bit 4
    AT_ALTITUDE_ATOMIC      = 5,  // bit 5
    AT_ALTITUDE_EVENTS_ATOMIC = 6,  // bit 6
    AT_ACCELERATION_ATOMIC  = 7,  // bit 7
    AT_GYRO_ATOMIC          = 8,  // bit 8
    AT_GPS_ATOMIC           = 9,  // bit 9
    AT_RADIO_ATOMIC         = 10,  // bit 10
    AT_SD_ATOMIC            = 11,  // bit 11
    AT_PAYLOAD_STATUS_ATOMIC = 12,  // bit 12
    AT_PAYLOAD_DATA_ATOMIC  = 13,  // bit 13
    AT_PAYLOAD_ADAPTER_0_ATOMIC = 14,  // bit 14
    AT_PAYLOAD_ADAPTER_1_ATOMIC = 15,  // bit 15
    AT_PAYLOAD_ADAPTER_2_ATOMIC = 16,  // bit 16
    AT_PAYLOAD_ADAPTER_3_ATOMIC = 17,  // bit 17
    AT_GPS_DEBUG_ATOMIC     = 18,  // bit 18
    AT_TOTAL                             // total count
};

// ---------- recov_atomic atomic ----------
typedef struct __attribute__((__packed__)) recov_atomic_data
{
    bool drogue_armed_SW : 1;
    bool drogue_armed_HW : 1;
    bool drogue_energized_SW : 1;
    bool drogue_energizedGate_HW : 1;
    bool drogue_energizedCurrent_HW : 1;
    bool drogue_continuity_HW : 1;
    bool main_armed_SW : 1;
    bool main_armed_HW : 1;
    bool main_energized_SW : 1;
    bool main_energizedGate_HW : 1;
    bool main_energizedCurrent_HW : 1;
    bool main_continuity_HW : 1;
} recov_atomic_data;

typedef union recov_atomic_packet {
    recov_atomic_data data;
    uint8_t bytes[sizeof(recov_atomic_data)];
} recov_atomic_packet;

// ---------- prop_states_atomic atomic ----------
typedef struct __attribute__((__packed__)) prop_states_atomic_data
{
    bool mov_hall_state : 1;
    bool fdov_armed_SW : 1;
    bool fdov_armed_HW : 1;
    bool fdov_energized_SW : 1;
    bool fdov_energizedGate_HW : 1;
    bool fdov_energizedCurrent_HW : 1;
    bool fdov_continuity_HW : 1;
    bool mov_armed_SW : 1;
    bool mov_armed_HW : 1;
    bool mov_energized_SW : 1;
    bool mov_energizedGate_HW : 1;
    bool mov_energizedCurrent_HW : 1;
    bool mov_continuity_HW : 1;
    bool prop_energized_electric : 1;
    bool vent_armed_SW : 1;
    bool vent_armed_HW : 1;
    bool vent_energized_SW : 1;
    bool vent_energizedGate_HW : 1;
    bool vent_energizedCurrent_HW : 1;
    bool vent_continuity_HW : 1;
} prop_states_atomic_data;

typedef union prop_states_atomic_packet {
    prop_states_atomic_data data;
    uint8_t bytes[sizeof(prop_states_atomic_data)];
} prop_states_atomic_packet;

// ---------- prop_atomic atomic ----------
typedef struct __attribute__((__packed__)) prop_atomic_data
{
    uint16_t cc_pressure;
    uint16_t tank_pressure;
    int16_t tank_temp_celsius;
    int16_t vent_temp_celsius;
} prop_atomic_data;

typedef union prop_atomic_packet {
    prop_atomic_data data;
    uint8_t bytes[sizeof(prop_atomic_data)];
} prop_atomic_packet;

// ---------- flight_stage_atomic atomic ----------
typedef struct __attribute__((__packed__)) flight_stage_atomic_data
{
    uint8_t flight_stage;
} flight_stage_atomic_data;

typedef union flight_stage_atomic_packet {
    flight_stage_atomic_data data;
    uint8_t bytes[sizeof(flight_stage_atomic_data)];
} flight_stage_atomic_packet;

// ---------- fc_internal_atomic atomic ----------
typedef struct __attribute__((__packed__)) fc_internal_atomic_data
{
    float fc_pressure_hpa;
    uint32_t fc_time_since_start;
    int16_t fc_temp_celsius_em2;
    int16_t battery_current_draw_mA;
    uint8_t battery_voltage_dV;
    uint8_t fc_rssi_dBm;
    int8_t fc_snr_dB;
    bool vx_sense_HW : 1;
    bool vx_sense_SW : 1;
} fc_internal_atomic_data;

typedef union fc_internal_atomic_packet {
    fc_internal_atomic_data data;
    uint8_t bytes[sizeof(fc_internal_atomic_data)];
} fc_internal_atomic_packet;

// ---------- altitude_atomic atomic ----------
typedef struct __attribute__((__packed__)) altitude_atomic_data
{
    float barometer_altitude_from_pad_ft;
    float barometer_altitude_from_sea_level_ft;
    float vertical_speed_ft_per_sec;
} altitude_atomic_data;

typedef union altitude_atomic_packet {
    altitude_atomic_data data;
    uint8_t bytes[sizeof(altitude_atomic_data)];
} altitude_atomic_packet;

// ---------- altitude_events_atomic atomic ----------
typedef struct __attribute__((__packed__)) altitude_events_atomic_data
{
    float apogee_from_ground_ft;
    float drouge_deployment_from_ground_ft;
    float main_deployment_from_ground_ft;
} altitude_events_atomic_data;

typedef union altitude_events_atomic_packet {
    altitude_events_atomic_data data;
    uint8_t bytes[sizeof(altitude_events_atomic_data)];
} altitude_events_atomic_packet;

// ---------- acceleration_atomic atomic ----------
typedef struct __attribute__((__packed__)) acceleration_atomic_data
{
    int16_t acceleration_x_mg;
    int16_t acceleration_y_mg;
    int16_t acceleration_z_mg;
} acceleration_atomic_data;

typedef union acceleration_atomic_packet {
    acceleration_atomic_data data;
    uint8_t bytes[sizeof(acceleration_atomic_data)];
} acceleration_atomic_packet;

// ---------- gyro_atomic atomic ----------
typedef struct __attribute__((__packed__)) gyro_atomic_data
{
    int16_t gyro_rate_x_mdeg_per_s;
    int16_t gyro_rate_y_mdeg_per_s;
    int16_t gyro_rate_z_mdeg_per_s;
} gyro_atomic_data;

typedef union gyro_atomic_packet {
    gyro_atomic_data data;
    uint8_t bytes[sizeof(gyro_atomic_data)];
} gyro_atomic_packet;

// ---------- gps_atomic atomic ----------
typedef struct __attribute__((__packed__)) gps_atomic_data
{
    float gps_latitude_deg_e7;
    float gps_longitude_deg_e7;
    float gps_altitude_mm;
    float gps_time_last_update_s;
} gps_atomic_data;

typedef union gps_atomic_packet {
    gps_atomic_data data;
    uint8_t bytes[sizeof(gps_atomic_data)];
} gps_atomic_packet;

// ---------- radio_atomic atomic ----------
typedef struct __attribute__((__packed__)) radio_atomic_data
{
    char call_sign[6];
} radio_atomic_data;

typedef union radio_atomic_packet {
    radio_atomic_data data;
    uint8_t bytes[sizeof(radio_atomic_data)];
} radio_atomic_packet;

// ---------- sd_atomic atomic ----------
typedef struct __attribute__((__packed__)) sd_atomic_data
{
    bool sd_card_file_open : 1;
    bool sd_card_deletion_armed : 1;
} sd_atomic_data;

typedef union sd_atomic_packet {
    sd_atomic_data data;
    uint8_t bytes[sizeof(sd_atomic_data)];
} sd_atomic_packet;

// ---------- payload_status_atomic atomic ----------
typedef struct __attribute__((__packed__)) payload_status_atomic_data
{
    uint32_t payload_time_elapsed;
    bool payload_ok : 1;
    bool payload_sampling : 1;
} payload_status_atomic_data;

typedef union payload_status_atomic_packet {
    payload_status_atomic_data data;
    uint8_t bytes[sizeof(payload_status_atomic_data)];
} payload_status_atomic_packet;

// ---------- payload_data_atomic atomic ----------
typedef struct __attribute__((__packed__)) payload_data_atomic_data
{
    uint16_t payload_freq_x;
    uint16_t payload_freq_y;
    uint16_t payload_freq_z;
    uint16_t payload_amp_x;
    uint16_t payload_amp_y;
    uint16_t payload_amp_z;
} payload_data_atomic_data;

typedef union payload_data_atomic_packet {
    payload_data_atomic_data data;
    uint8_t bytes[sizeof(payload_data_atomic_data)];
} payload_data_atomic_packet;

// ---------- payload_adapter_0_atomic atomic ----------
typedef struct __attribute__((__packed__)) payload_adapter_0_atomic_data
{
    int16_t payload_adapter_0_raw_x;
    int16_t payload_adapter_0_raw_y;
    int16_t payload_adapter_0_raw_z;
} payload_adapter_0_atomic_data;

typedef union payload_adapter_0_atomic_packet {
    payload_adapter_0_atomic_data data;
    uint8_t bytes[sizeof(payload_adapter_0_atomic_data)];
} payload_adapter_0_atomic_packet;

// ---------- payload_adapter_1_atomic atomic ----------
typedef struct __attribute__((__packed__)) payload_adapter_1_atomic_data
{
    int16_t payload_adapter_1_raw_x;
    int16_t payload_adapter_1_raw_y;
    int16_t payload_adapter_1_raw_z;
} payload_adapter_1_atomic_data;

typedef union payload_adapter_1_atomic_packet {
    payload_adapter_1_atomic_data data;
    uint8_t bytes[sizeof(payload_adapter_1_atomic_data)];
} payload_adapter_1_atomic_packet;

// ---------- payload_adapter_2_atomic atomic ----------
typedef struct __attribute__((__packed__)) payload_adapter_2_atomic_data
{
    int16_t payload_adapter_2_raw_x;
    int16_t payload_adapter_2_raw_y;
    int16_t payload_adapter_2_raw_z;
} payload_adapter_2_atomic_data;

typedef union payload_adapter_2_atomic_packet {
    payload_adapter_2_atomic_data data;
    uint8_t bytes[sizeof(payload_adapter_2_atomic_data)];
} payload_adapter_2_atomic_packet;

// ---------- payload_adapter_3_atomic atomic ----------
typedef struct __attribute__((__packed__)) payload_adapter_3_atomic_data
{
    int16_t payload_adapter_3_raw_x;
    int16_t payload_adapter_3_raw_y;
    int16_t payload_adapter_3_raw_z;
} payload_adapter_3_atomic_data;

typedef union payload_adapter_3_atomic_packet {
    payload_adapter_3_atomic_data data;
    uint8_t bytes[sizeof(payload_adapter_3_atomic_data)];
} payload_adapter_3_atomic_packet;

// ---------- gps_debug_atomic atomic ----------
typedef struct __attribute__((__packed__)) gps_debug_atomic_data
{
    uint32_t gps_hacc_mm;
    uint32_t gps_vacc_mm;
    int32_t gps_ground_speed_mmps;
    int32_t gps_heading_motion_deg_e5;
    uint16_t gps_pdop_centi;
    int16_t gps_satellite_1_azimuth_deg;
    int16_t gps_satellite_2_azimuth_deg;
    int16_t gps_satellite_3_azimuth_deg;
    int16_t gps_satellite_4_azimuth_deg;
    int16_t gps_satellite_5_azimuth_deg;
    int16_t gps_satellite_6_azimuth_deg;
    int16_t gps_satellite_7_azimuth_deg;
    int16_t gps_satellite_8_azimuth_deg;
    uint8_t gps_fix_type;
    uint8_t gps_satellites;
    uint8_t gps_satellite_1_gnss_id;
    uint8_t gps_satellite_1_sv_id;
    uint8_t gps_satellite_1_cno_dbhz;
    int8_t gps_satellite_1_elevation_deg;
    uint8_t gps_satellite_2_gnss_id;
    uint8_t gps_satellite_2_sv_id;
    uint8_t gps_satellite_2_cno_dbhz;
    int8_t gps_satellite_2_elevation_deg;
    uint8_t gps_satellite_3_gnss_id;
    uint8_t gps_satellite_3_sv_id;
    uint8_t gps_satellite_3_cno_dbhz;
    int8_t gps_satellite_3_elevation_deg;
    uint8_t gps_satellite_4_gnss_id;
    uint8_t gps_satellite_4_sv_id;
    uint8_t gps_satellite_4_cno_dbhz;
    int8_t gps_satellite_4_elevation_deg;
    uint8_t gps_satellite_5_gnss_id;
    uint8_t gps_satellite_5_sv_id;
    uint8_t gps_satellite_5_cno_dbhz;
    int8_t gps_satellite_5_elevation_deg;
    uint8_t gps_satellite_6_gnss_id;
    uint8_t gps_satellite_6_sv_id;
    uint8_t gps_satellite_6_cno_dbhz;
    int8_t gps_satellite_6_elevation_deg;
    uint8_t gps_satellite_7_gnss_id;
    uint8_t gps_satellite_7_sv_id;
    uint8_t gps_satellite_7_cno_dbhz;
    int8_t gps_satellite_7_elevation_deg;
    uint8_t gps_satellite_8_gnss_id;
    uint8_t gps_satellite_8_sv_id;
    uint8_t gps_satellite_8_cno_dbhz;
    int8_t gps_satellite_8_elevation_deg;
    bool gps_satellite_1_used_in_fix : 1;
    bool gps_satellite_2_used_in_fix : 1;
    bool gps_satellite_3_used_in_fix : 1;
    bool gps_satellite_4_used_in_fix : 1;
    bool gps_satellite_5_used_in_fix : 1;
    bool gps_satellite_6_used_in_fix : 1;
    bool gps_satellite_7_used_in_fix : 1;
    bool gps_satellite_8_used_in_fix : 1;
    bool gps_fix_ok : 1;
} gps_debug_atomic_data;

typedef union gps_debug_atomic_packet {
    gps_debug_atomic_data data;
    uint8_t bytes[sizeof(gps_debug_atomic_data)];
} gps_debug_atomic_packet;

// ---------- Atomic Size Catalog (declaration) ----------
extern const uint16_t AT_SIZE[AT_TOTAL];

#endif