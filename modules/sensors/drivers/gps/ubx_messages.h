#ifndef INC_UBX_MESSAGES_H_
#define INC_UBX_MESSAGES_H_

#include <stdint.h>

#define UBX_SYNC_CHAR1 0xB5U
#define UBX_SYNC_CHAR2 0x62U

// --- message classes --- //
typedef enum {
    MSG_CLASS_UBX_NAV = 0x01, // navigation results messages
    MSG_CLASS_UBX_RXM =
        0x02, // receiver manager messages: satellite status, RTC status
    MSG_CLASS_UBX_INF = 0x04, // information messages
    MSG_CLASS_UBX_ACK = 0x05, // ack/nak responses to CFG input messages
    MSG_CLASS_UBX_CFG = 0x06, // configuration input messages
    MSG_CLASS_UBX_UPD = 0x09, // firmware update messages
    MSG_CLASS_UBX_MON = 0x0A, // monitoring messages (communication status,
                              // stack usage, task status)
    MSG_CLASS_UBX_AID = 0x0B, // AssistNow aiding messages (ephemeris, almanac,
                              // other A-GPS data input)
    MSG_CLASS_UBX_TIM =
        0x0D, // timing messages (time pulse output, time mark results)
    MSG_CLASS_UBX_ESF = 0x10, // external sensor fusion messages
    MSG_CLASS_UBX_MGA = 0x13, // multiple GNSS assistance messages
    MSG_CLASS_UBX_LOG =
        0x21, // logging messages (creation, deletion, info, retrieval)
    MSG_CLASS_UBX_SEC = 0x27, // security feature messages
    MSG_CLASS_UBX_HNR = 0x28, // high navigation rate messages
} ubx_message_class_t;

// --- message IDs per class --- //
typedef enum {
    MSG_ID_UBX_ACK_ACK = 0x01,
    MSG_ID_UBX_ACK_NAK = 0x00,
} ubx_ack_message_id_t;

typedef enum {
    MSG_ID_UBX_CFG_ANT = 0x13,   // antenna control
    MSG_ID_UBX_CFG_BATCH = 0x93, // batching configuration
    MSG_ID_UBX_CFG_CFG = 0x09,   // clear/save/load configurations
    MSG_ID_UBX_CFG_DAT = 0x06,   // user-defined datum
    MSG_ID_UBX_CFG_DGNSS = 0x70, // DGNSS configuration
    MSG_ID_UBX_CFG_DOSC = 0x61,  // disciplined oscillator configuration
    MSG_ID_UBX_CFG_ESFALG =
        0x56, // IMU mount misalignment (ESF = external sensor fusion)
    MSG_ID_UBX_CFG_ESFA = 0x4C,     // accelerometer sensor
    MSG_ID_UBX_CFG_ESFG = 0x4D,     // gyroscope sensor
    MSG_ID_UBX_CFG_ESFWT = 0x82,    // wheel-tick configuration
    MSG_ID_UBX_CFG_ESRC = 0x60,     // external sync sources
    MSG_ID_UBX_CFG_GEOFENCE = 0x69, // geofence configuration
    MSG_ID_UBX_CFG_GNSS = 0x3E,     // GNSS system configuration
    MSG_ID_UBX_CFG_HNR = 0x5C,      // high navigation rate settings
    MSG_ID_UBX_CFG_INF = 0x02,      // information message configuration
    MSG_ID_UBX_CFG_ITFM = 0x39, // jamming/interference monitor configuration
    MSG_ID_UBX_CFG_LOGFILTER = 0x47, // data logger configuration
    MSG_ID_UBX_CFG_MSG = 0x01, // message polling and message rate configuration
    MSG_ID_UBX_CFG_NAV5 = 0x24,  // navigation engine settings
    MSG_ID_UBX_CFG_NAVX5 = 0x23, // navigation engine expert settings
    MSG_ID_UBX_CFG_NMEA = 0x17,  // NMEA protocol configuration
    MSG_ID_UBX_CFG_ODO = 0x1E,   // odometer configuration
    MSG_ID_UBX_CFG_PM2 = 0x3B,   // extended power management configuration
    MSG_ID_UBX_CFG_PMS = 0x86,   // power mode setup
    MSG_ID_UBX_CFG_PRT = 0x00,   // port configuration (UART, SPI, USB, I2C/DDC)
    MSG_ID_UBX_CFG_PWR = 0x57,   // put receiver in a defined power state
    MSG_ID_UBX_CFG_RATE = 0x08,  // navigation/measurement rate settings
    MSG_ID_UBX_CFG_RINV = 0x34,  // contents of remote inventory
    MSG_ID_UBX_CFG_RST = 0x04,   // reset receiver / clear backup data
    MSG_ID_UBX_CFG_RXM = 0x11,   // RXM configuration
    MSG_ID_UBX_CFG_SBAS = 0x16,  // SBAS configuration
    MSG_ID_UBX_CFG_SENIF = 0x88, // I2C sensor interface configuration
    MSG_ID_UBX_CFG_SLAS = 0x8D,  // SLAS configuration
    MSG_ID_UBX_CFG_SMGR = 0x62,  // sync manager configuration
    MSG_ID_UBX_CFG_SPT = 0x64,   // configure and start a new sensor
    MSG_ID_UBX_CFG_TMODE2 = 0x3D, // time mode settings 2
    MSG_ID_UBX_CFG_TMODE3 = 0x71, // time mode settings 3
    MSG_ID_UBX_CFG_TP5 = 0x31,    // time pulse parameters and polling
    MSG_ID_UBX_CFG_TXSLOT = 0x53, // transmit buffer time slots configuration
    MSG_ID_UBX_CFG_USB = 0x1B,    // USB configuration
} ubx_cfg_message_id_t;

typedef enum {
    MSG_ID_UBX_INF_DEBUG = 0x04,   // ASCII output with debug contents
    MSG_ID_UBX_INF_ERROR = 0x00,   // ASCII output with error contents
    MSG_ID_UBX_INF_NOTICE = 0x02,  // ASCII output with informational contents
    MSG_ID_UBX_INF_TEST = 0x03,    // ASCII output with test contents
    MSG_ID_UBX_INF_WARNING = 0x01, // ASCII output with warning contents
} ubx_inf_message_id_t;

typedef enum {
    MSG_ID_UBX_NAV_PVT = 0x07,
} ubx_nav_message_id_t;

// --- message lengths per message --- //
typedef enum {
    MSG_LEN_UBX_ACK_ACK = 2,
    MSG_LEN_UBX_ACK_NAK = 2,
} ubx_ack_message_len_t;

// unless noted by name, lengths are for get and set
// base length means message length must be calculated at run time
typedef enum {
    MSG_LEN_UBX_CFG_ANT = 4,
    MSG_LEN_UBX_CFG_BATCH = 8,
    MSG_LEN_UBX_CFG_CFG = 12,
    MSG_LEN_UBX_CFG_CFG2 = 13, // UBX-CFG-CFG with optional deviceMask
    MSG_LEN_UBX_CFG_DAT_SET = 44,
    MSG_LEN_UBX_CFG_DAT_GET = 52,
    MSG_LEN_UBX_CFG_DGNSS = 4,
    MSG_LEN_UBX_CFG_DOSC = 4, // base length
    MSG_LEN_UBX_CFG_ESFALG = 12,
    MSG_LEN_UBX_CFG_ESFA = 20,
    MSG_LEN_UBX_CFG_ESGF = 20,
    MSG_LEN_UBX_CFG_ESFWT = 32,
    MSG_LEN_UBX_CFG_ESRC = 4,     // base length
    MSG_LEN_UBX_CFG_GEOFENCE = 8, // base length
    MSG_LEN_UBX_CFG_GNSS = 4,     // base length
    MSG_LEN_UBX_CFG_HNR = 4,
    MSG_LEN_UBX_CFG_INF_POLL = 1,
    MSG_LEN_UBX_CFG_INF = 0, // base length
    MSG_LEN_UBX_CFG_ITFM = 8,
    MSG_LEN_UBX_CFG_LOGFILTER = 12,
    MSG_LEN_UBX_CFG_MSG_POLL = 2,
    MSG_LEN_UBX_CFG_MSG_MULTIPLE = 8,
    MSG_LEN_UBX_CFG_MSG_SINGLE = 3,
    MSG_LEN_UBX_CFG_NAV5 = 36,
    MSG_LEN_UBX_CFG_NAVX5 = 40,
    MSG_LEN_UBX_CFG_NMEA = 4,
    MSG_LEN_UBX_CFG_NMEA_V0 = 12,
    MSG_LEN_UBX_CFG_NMEA_V1_EXT = 20,
    MSG_LEN_UBX_CFG_ODO = 20,
    MSG_LEN_UBX_CFG_PM2 = 44,
    MSG_LEN_UBX_CFG_PM2_EXT = 48,
    MSG_LEN_UBX_CFG_PMS = 8,
    MSG_LEN_UBX_CFG_PRT_POLL = 1,
    MSG_LEN_UBX_CFG_PRT = 20,
    MSG_LEN_UBX_CFG_PWR = 8,
    MSG_LEN_UBX_CFG_RATE = 6,
    MSG_LEN_UBX_CFG_RINV = 1, // base length
    MSG_LEN_UBX_CFG_RST = 4,
    MSG_LEN_UBX_CFG_RXM = 2,
    MSG_LEN_UBX_CFG_SBAS = 8,
    MSG_LEN_UBX_CFG_SENIF = 6,
    MSG_LEN_UBX_CFG_SLAS = 4,
    MSG_LEN_UBX_CFG_SMGR = 20,
    MSG_LEN_UBX_CFG_SPT = 12,
    MSG_LEN_UBX_CFG_TMODE2 = 28,
    MSG_LEN_UBX_CFG_TMODE3 = 40,
    MSG_LEN_UBX_CFG_TP5_POLL_TP0 = 0,
    MSG_LEN_UBX_CFG_TP5_POLL_TP = 1,
    MSG_LEN_UBX_CFG_TP5 = 32,
    MSG_LEN_UBX_CFG_TXSLOT = 16,
    MSG_LEN_UBX_CFG_USB = 108,
} ubx_cfg_message_len_t;

typedef enum {
    MSG_LEN_UBX_NAV_PVT = 92,
} ubx_nav_message_len_t;

// INF message have variable length delimited by '\n'. handle accordingly

// --- UBX-CFG-NAV5 message-specific parameters --- //
typedef enum {
    DYNAMIC_PLATFORM_MODEL_PORTABLE = 0,
    DYNAMIC_PLATFORM_MODEL_STATIONARY = 2,
    DYNAMIC_PLATFORM_MODEL_PEDESTRIAN = 3,
    DYNAMIC_PLATFORM_MODEL_AUTOMOTIVE = 4,
    DYNAMIC_PLATFORM_MODEL_SEA = 5,
    DYNAMIC_PLATFORM_MODEL_AIRBORNE_1G = 6,
    DYNAMIC_PLATFORM_MODEL_AIRBORNE_2G = 7,
    DYNAMIC_PLATFORM_MODEL_AIRBORNE_4G = 8,
    DYNAMIC_PLATFORM_MODEL_WRIST = 9,
    DYNAMIC_PLATFORM_MODEL_MOTORBIKE = 10,
    DYNAMIC_PLATFORM_MODEL_ROBOTIC_LAWN_MOWER = 11,
    DYNAMIC_PLATFORM_MODEL_ELECTRIC_KICK_SCOOTER = 12,
} ubx_cfg_nav5_dynamic_models_t;

typedef enum {
    FIXMODE_2D_ONLY = 1,
    FIXMODE_3D_ONLY = 2,
    FIXMODE_AUTO_2D_3D = 3,
} ubx_cfg_nav5_fixModes_t;

// bitmask for UBX-CFG-NAV5
#define BITMASK_UBX_CFG_NAV5_ALL 0x05FF
#define BITMASK_UBX_CFG_NAV5_NONE 0x0000
#define BITMASK_UBX_CFG_NAV5_dyn (1 << 0)
#define BITMASK_UBX_CFG_NAV5_minEl (1 << 1)
#define BITMASK_UBX_CFG_NAV5_posFixMode (1 << 2)
#define BITMASK_UBX_CFG_NAV5_drLim (1 << 3)
#define BITMASK_UBX_CFG_NAV5_posMask (1 << 4)
#define BITMASK_UBX_CFG_NAV5_timeMask (1 << 5)
#define BITMASK_UBX_CFG_NAV5_staticHoldMask (1 << 6)
#define BITMASK_UBX_CFG_NAV5_dgpsMask (1 << 7)
#define BITMASK_UBX_CFG_NAV5_cnoThreshold (1 << 8)
#define BITMASK_UBX_CFG_NAV5_utc (1 << 10)

// --- UBX-CFG-PRT message-specific parameters --- //

#define UBX_PORT_UART1           (0x01U)
#define UBX_PROTO_MASK_UBX       (1U << 0)
#define UBX_PROTO_MASK_NMEA      (1U << 1)
#define UBX_PROTO_MASK_RTCM      (1U << 2)

#endif /* INC_UBX_MESSAGES_H_ */
