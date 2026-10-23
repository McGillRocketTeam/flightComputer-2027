// AUTO-GENERATED FILE; DO NOT EDIT (Source: generate_eeprom_map.py)
// Last Generated: 2026-10-23 12:11:44.981761

#pragma once
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <nonvolatile/24xx08/24xx08_address.h>
#define PACKED_STRUCT typedef struct __attribute__((packed))

// Check Generator Config against driver. These must match for generator to reject block and page violations correctly
static_assert(EEPROM_NUM_BLOCKS == 4, "Generator NUM_BLOCKS does not match");
static_assert(EEPROM_SIZE_BYTES == 1024, "Generator Block Size does not match");
static_assert(EEPROM_PAGE_SIZE == 16, "Generator Page Size does not match");

// START PARAM STRUCT
// Param Struct should not be used in implementation, it is provided as type reference
PACKED_STRUCT{
    // Block 0
    uint8_t  flight_stage;
    float    radio_freq;
    uint8_t  radio_cr;
    uint8_t  radio_sf;
    uint8_t  radio_bw;
    uint8_t  fc_state_reset;
    uint8_t  fc_state_iwdg;
    uint8_t  fc_state_WU;

    // Block 1
    uint8_t  prop_armed;
    uint8_t  recov_armed;
    uint8_t  sd_open;
    uint8_t  camera_rec;
    float    true_apogee;
    uint32_t true_apogee_time;
    float    recov_pad_altitude;
    uint8_t  recov_flag;

    // Block 2
    uint16_t rtc_day;
    uint8_t  rtc_hour;
    uint8_t  rtc_minute;
    uint8_t  rtc_second;
    uint16_t rtc_subsec;

    // Block 3
    char TEST_PARAM[5];
} EEPROM_STRUCT;
// END PARAM STRUCT

// Ensure struct cannot compile exceeding memory size. Generator does check this, this is redundancy
static_assert(sizeof(EEPROM_STRUCT) <= EEPROM_SIZE_BYTES, "EEPROM TOTAL MEMORY EXCEEDED: GENERATOR SHOULD HAVE PREVENTED THIS");

// START PARAM ENUM
enum class NV_Param : uint8_t {
    FLIGHT_STAGE,       // Block: 0
    RADIO_FREQ,         // Block: 0
    RADIO_CR,           // Block: 0
    RADIO_SF,           // Block: 0
    RADIO_BW,           // Block: 0
    FC_STATE_RESET,     // Block: 0
    FC_STATE_IWDG,      // Block: 0
    FC_STATE_WU,        // Block: 0
    PROP_ARMED,         // Block: 1
    RECOV_ARMED,        // Block: 1
    SD_OPEN,            // Block: 1
    CAMERA_REC,         // Block: 1
    TRUE_APOGEE,        // Block: 1
    TRUE_APOGEE_TIME,   // Block: 1
    RECOV_PAD_ALTITUDE, // Block: 1
    RECOV_FLAG,         // Block: 1
    RTC_DAY,            // Block: 2
    RTC_HOUR,           // Block: 2
    RTC_MINUTE,         // Block: 2
    RTC_SECOND,         // Block: 2
    RTC_SUBSEC,         // Block: 2
    TEST_PARAM,         // Block: 3
    NUM_PARAMS,
};
// END PARAM ENUM

// START PARAM ADDRESS TABLE
static uint16_t nv_param_addresses [(uint8_t) NV_Param::NUM_PARAMS]{
    [(uint8_t) NV_Param::FLIGHT_STAGE]        = 0x0,
    [(uint8_t) NV_Param::RADIO_FREQ]          = 0x1,
    [(uint8_t) NV_Param::RADIO_CR]            = 0x5,
    [(uint8_t) NV_Param::RADIO_SF]            = 0x6,
    [(uint8_t) NV_Param::RADIO_BW]            = 0x7,
    [(uint8_t) NV_Param::FC_STATE_RESET]      = 0x8,
    [(uint8_t) NV_Param::FC_STATE_IWDG]       = 0x9,
    [(uint8_t) NV_Param::FC_STATE_WU]         = 0xA,
    [(uint8_t) NV_Param::PROP_ARMED]          = 0x100,
    [(uint8_t) NV_Param::RECOV_ARMED]         = 0x101,
    [(uint8_t) NV_Param::SD_OPEN]             = 0x102,
    [(uint8_t) NV_Param::CAMERA_REC]          = 0x103,
    [(uint8_t) NV_Param::TRUE_APOGEE]         = 0x104,
    [(uint8_t) NV_Param::TRUE_APOGEE_TIME]    = 0x108,
    [(uint8_t) NV_Param::RECOV_PAD_ALTITUDE]  = 0x10C,
    [(uint8_t) NV_Param::RECOV_FLAG]          = 0x110,
    [(uint8_t) NV_Param::RTC_DAY]             = 0x200,
    [(uint8_t) NV_Param::RTC_HOUR]            = 0x202,
    [(uint8_t) NV_Param::RTC_MINUTE]          = 0x203,
    [(uint8_t) NV_Param::RTC_SECOND]          = 0x204,
    [(uint8_t) NV_Param::RTC_SUBSEC]          = 0x205,
    [(uint8_t) NV_Param::TEST_PARAM]          = 0x300,
};
// END PARAM ADDRESS TABLE

// START PARAM SIZE TABLE
static uint16_t nv_param_sizes [(uint8_t) NV_Param::NUM_PARAMS]{
    [(uint8_t) NV_Param::FLIGHT_STAGE]        = 1,
    [(uint8_t) NV_Param::RADIO_FREQ]          = 4,
    [(uint8_t) NV_Param::RADIO_CR]            = 1,
    [(uint8_t) NV_Param::RADIO_SF]            = 1,
    [(uint8_t) NV_Param::RADIO_BW]            = 1,
    [(uint8_t) NV_Param::FC_STATE_RESET]      = 1,
    [(uint8_t) NV_Param::FC_STATE_IWDG]       = 1,
    [(uint8_t) NV_Param::FC_STATE_WU]         = 1,
    [(uint8_t) NV_Param::PROP_ARMED]          = 1,
    [(uint8_t) NV_Param::RECOV_ARMED]         = 1,
    [(uint8_t) NV_Param::SD_OPEN]             = 1,
    [(uint8_t) NV_Param::CAMERA_REC]          = 1,
    [(uint8_t) NV_Param::TRUE_APOGEE]         = 4,
    [(uint8_t) NV_Param::TRUE_APOGEE_TIME]    = 4,
    [(uint8_t) NV_Param::RECOV_PAD_ALTITUDE]  = 4,
    [(uint8_t) NV_Param::RECOV_FLAG]          = 1,
    [(uint8_t) NV_Param::RTC_DAY]             = 2,
    [(uint8_t) NV_Param::RTC_HOUR]            = 1,
    [(uint8_t) NV_Param::RTC_MINUTE]          = 1,
    [(uint8_t) NV_Param::RTC_SECOND]          = 1,
    [(uint8_t) NV_Param::RTC_SUBSEC]          = 2,
    [(uint8_t) NV_Param::TEST_PARAM]          = 5,
};
// END PARAM SIZE TABLE

// END GENERATED CONTENT
