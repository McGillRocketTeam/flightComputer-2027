#pragma once
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <nonvolatile/24xx08/24xx08_address.h>

#define EEPROM_BLOCK_SIZE (EEPROM_SIZE_BYTES / EEPROM_NUM_BLOCKS)

#define FIELD_SIZE(field) sizeof(((MRT_EEPROM_MAP *)0)->field)

#define PACKED_STRUCT typedef struct __attribute__((packed))

// Master structs defining contents, and ordering of the EEPROM.
// Try to ensure that multi-byte fields line up to be on a single page (16 bytes on 24xx08)

PACKED_STRUCT{
    uint8_t flight_stage;
    float radio_freq;
    uint8_t radio_cr;
    uint8_t radio_sf;
    uint8_t radio_bw;

    uint8_t fc_state_reset;
    uint8_t fc_state_iwdg;
    uint8_t fc_state_WU;
} EEPROM_B0;

PACKED_STRUCT{
    uint8_t prop_armed;
    uint8_t recov_armed;
    uint8_t sd_open;
    uint8_t camera_rec;

    float recov_pad_altitude;
    uint8_t recov_flag; // Pad altitude set

    float true_apogee;
    uint32_t true_apogee_time;
} EEPROM_B1;

PACKED_STRUCT{
    uint16_t rtc_day;
    uint8_t rtc_hour;
    uint8_t rtc_minute;
    uint8_t rtc_second;
    uint16_t rtc_subsec;
} EEPROM_B2;

PACKED_STRUCT{
} EEPROM_B3;

// Assertions that each block is the correct size
static_assert(sizeof(EEPROM_B0) <= EEPROM_BLOCK_SIZE, "EEPROM BLOCK 0 EXCEEDS CONFIGURED MAXIMUM SIZE");
static_assert(sizeof(EEPROM_B1) <= EEPROM_BLOCK_SIZE, "EEPROM BLOCK 1 EXCEEDS CONFIGURED MAXIMUM SIZE");
static_assert(sizeof(EEPROM_B2) <= EEPROM_BLOCK_SIZE, "EEPROM BLOCK 2 EXCEEDS CONFIGURED MAXIMUM SIZE");
static_assert(sizeof(EEPROM_B3) <= EEPROM_BLOCK_SIZE, "EEPROM BLOCK 3 EXCEEDS CONFIGURED MAXIMUM SIZE");

#define EEPROM_ADDR(blockIdx, blockType, field) \
    (static_cast<uint16_t>((blockIdx << 8) + offsetof(blockType, field)))

#define EEPROM_SIZE(blockType, field) (uint8_t) sizeof(blockType::field)

enum class NV_Param : uint8_t{
    FLIGHT_STAGE,
    RADIO_FREQ,
    RADIO_CR,
    RADIO_SF,
    RADIO_BW,

    FC_STATE_RESET,
    FC_STATE_IWDG,
    FC_STATE_WU,

    PROP_ARMED,
    RECOV_ARMED,
    SD_OPEN,
    CAMERA_REC,

    RECOV_PAD_ALTITUDE,
    RECOV_FLAG,

    TRUE_APOGEE,
    TRUE_APOGEE_TIME,

    RTC_DAY,
    RTC_HOUR,
    RTC_MINUTE,
    RTC_SECOND,
    RTC_SUBSEC,

    // Total Parameter Num Tracking
    NUM_PARAMS,
};

// In order to compile, the following arrays must list the enum values in EXACTLY ascending (declared) order

// Addresses of each parameter
static uint16_t nv_param_addresses [(uint8_t) NV_Param::NUM_PARAMS]{
    [(uint8_t) NV_Param::FLIGHT_STAGE]       = EEPROM_ADDR(0, EEPROM_B0, flight_stage),
    [(uint8_t) NV_Param::RADIO_FREQ]         = EEPROM_ADDR(0, EEPROM_B0, radio_freq),
    [(uint8_t) NV_Param::RADIO_CR]           = EEPROM_ADDR(0, EEPROM_B0, radio_cr),
    [(uint8_t) NV_Param::RADIO_SF]           = EEPROM_ADDR(0, EEPROM_B0, radio_sf),
    [(uint8_t) NV_Param::RADIO_BW]           = EEPROM_ADDR(0, EEPROM_B0, radio_bw),
    [(uint8_t) NV_Param::FC_STATE_RESET]     = EEPROM_ADDR(0, EEPROM_B0, fc_state_reset),
    [(uint8_t) NV_Param::FC_STATE_IWDG]      = EEPROM_ADDR(0, EEPROM_B0, fc_state_iwdg),
    [(uint8_t) NV_Param::FC_STATE_WU]        = EEPROM_ADDR(0, EEPROM_B0, fc_state_WU),

    [(uint8_t) NV_Param::PROP_ARMED]         = EEPROM_ADDR(0, EEPROM_B1, prop_armed),
    [(uint8_t) NV_Param::RECOV_ARMED]        = EEPROM_ADDR(0, EEPROM_B1, recov_armed),
    [(uint8_t) NV_Param::SD_OPEN]            = EEPROM_ADDR(0, EEPROM_B1, sd_open),
    [(uint8_t) NV_Param::CAMERA_REC]         = EEPROM_ADDR(0, EEPROM_B1, camera_rec),
    [(uint8_t) NV_Param::RECOV_PAD_ALTITUDE] = EEPROM_ADDR(0, EEPROM_B1, recov_pad_altitude),
    [(uint8_t) NV_Param::RECOV_FLAG]         = EEPROM_ADDR(0, EEPROM_B1, recov_flag),
    [(uint8_t) NV_Param::TRUE_APOGEE]        = EEPROM_ADDR(0, EEPROM_B1, true_apogee),
    [(uint8_t) NV_Param::TRUE_APOGEE_TIME]   = EEPROM_ADDR(0, EEPROM_B1, true_apogee_time),

    [(uint8_t) NV_Param::RTC_DAY]            = EEPROM_ADDR(0, EEPROM_B2, rtc_day),
    [(uint8_t) NV_Param::RTC_HOUR]           = EEPROM_ADDR(0, EEPROM_B2, rtc_hour),
    [(uint8_t) NV_Param::RTC_MINUTE]         = EEPROM_ADDR(0, EEPROM_B2, rtc_minute),
    [(uint8_t) NV_Param::RTC_SECOND]         = EEPROM_ADDR(0, EEPROM_B2, rtc_second),
    [(uint8_t) NV_Param::RTC_SUBSEC]         = EEPROM_ADDR(0, EEPROM_B2, rtc_subsec),
};

// Sizes of each parameter
static uint8_t nv_param_sizes [(uint8_t) NV_Param::NUM_PARAMS]{
    [(uint8_t) NV_Param::FLIGHT_STAGE]       = EEPROM_SIZE(EEPROM_B0, flight_stage),
    [(uint8_t) NV_Param::RADIO_FREQ]         = EEPROM_SIZE(EEPROM_B0, radio_freq),
    [(uint8_t) NV_Param::RADIO_CR]           = EEPROM_SIZE(EEPROM_B0, radio_cr),
    [(uint8_t) NV_Param::RADIO_SF]           = EEPROM_SIZE(EEPROM_B0, radio_sf),
    [(uint8_t) NV_Param::RADIO_BW]           = EEPROM_SIZE(EEPROM_B0, radio_bw),
    [(uint8_t) NV_Param::FC_STATE_RESET]     = EEPROM_SIZE(EEPROM_B0, fc_state_reset),
    [(uint8_t) NV_Param::FC_STATE_IWDG]      = EEPROM_SIZE(EEPROM_B0, fc_state_iwdg),
    [(uint8_t) NV_Param::FC_STATE_WU]        = EEPROM_SIZE(EEPROM_B0, fc_state_WU),

    [(uint8_t) NV_Param::PROP_ARMED]         = EEPROM_SIZE(EEPROM_B1, prop_armed),
    [(uint8_t) NV_Param::RECOV_ARMED]        = EEPROM_SIZE(EEPROM_B1, recov_armed),
    [(uint8_t) NV_Param::SD_OPEN]            = EEPROM_SIZE(EEPROM_B1, sd_open),
    [(uint8_t) NV_Param::CAMERA_REC]         = EEPROM_SIZE(EEPROM_B1, camera_rec),
    [(uint8_t) NV_Param::RECOV_PAD_ALTITUDE] = EEPROM_SIZE(EEPROM_B1, recov_pad_altitude),
    [(uint8_t) NV_Param::RECOV_FLAG]         = EEPROM_SIZE(EEPROM_B1, recov_flag),
    [(uint8_t) NV_Param::TRUE_APOGEE]        = EEPROM_SIZE(EEPROM_B1, true_apogee),
    [(uint8_t) NV_Param::TRUE_APOGEE_TIME]   = EEPROM_SIZE(EEPROM_B1, true_apogee_time),

    [(uint8_t) NV_Param::RTC_DAY]            = EEPROM_SIZE(EEPROM_B2, rtc_day),
    [(uint8_t) NV_Param::RTC_HOUR]           = EEPROM_SIZE(EEPROM_B2, rtc_hour),
    [(uint8_t) NV_Param::RTC_MINUTE]         = EEPROM_SIZE(EEPROM_B2, rtc_minute),
    [(uint8_t) NV_Param::RTC_SECOND]         = EEPROM_SIZE(EEPROM_B2, rtc_second),
    [(uint8_t) NV_Param::RTC_SUBSEC]         = EEPROM_SIZE(EEPROM_B2, rtc_subsec),
};