#pragma once
#include <stdint.h>
#include <functional>

#include "sd_card/SDCard.h"
#include "sensors/CurrentSensor.h"
#include "sensors/GPS.h"
#include "sensors/IMU.h"
#include "sensors/Magnetometer.h"

#include <ASTRA.h>

enum flight_stage_t {
    PAD = 0,
    BOOSTER_ASCENT = 1,
    PRIMARY_COAST = 2,
    SUSTAINER_ASCENT = 3,
    SECONDARY_COAST = 4,
    DROGUE_DESCENT = 5,
    MAIN_DESCENT = 6,
    LANDED = 7,
    INVALID = -1,
};

class Telemetry {
   public:
    Telemetry();

    bool addCurrentSensor(CurrentSensor& sensor);
    bool addMagnetometer(Magnetometer& sensor);
    bool addGPS(GPS& sensor);
    bool addIMU(IMU& sensor);
    bool addSD(SDCard& sd_card);

    void pollTelemetry();

    FrameBuilder* getASTRAFrame(FrameHeader header);

    FrameBuilder builder;

    static std::function<uint8_t*(Telemetry*)> atomic_fill_functions [AT_TOTAL];

    uint16_t writeSD();
    struct rocket_telem {
        flight_stage_t flight_stage;
        float altitude_from_ground_ft;
        float altitude_rate_of_change_ft_s;
        float apogee_from_ground_ft;
        float main_deployment_from_ground_ft;
        float drogue_deployment_from_ground_ft;
    };

    struct fc_telem {
        int16_t battery_voltage_dV;
        int16_t battery_current_mA;
        bool CAN_bus_active;
        bool SD_card_file_open;
        bool SD_card_deletion_armed;
    };

    struct gps_telem {
        bool gps_fix;
        int32_t GPS_longitude;
        int32_t GPS_latitude;
        int32_t GPS_altitude_mm;
        float time_since_GPS_ack_s;
    };

    struct imu_telem {
        float acceleration_x_milli_G;
        float acceleration_y_milli_G;
        float acceleration_z_milli_G;
        float gyro_rate_x_mdeg_s;
        float gyro_rate_y_mdeg_s;
        float gyro_rate_z_mdeg_s;
    };

    struct mag_telem {
        float magnetic_orientation_x_mgauss;  // Towards the GPS module
        float magnetic_orientation_y_mgauss;  // Left from the processor
        float magnetic_orientation_z_mgauss;  // Upwards from the processoR
    };

    struct barometer_telem {
        float atmospheric_temperature_C;
        float atmospheric_pressure_hpa;
    };

    struct recovery_telem {
        bool drogue_armed_SW;
        bool drogue_energized_SW;
        bool main_armed_SW;
        bool main_energized_SW;
    };

    struct radio_telem {
        int8_t radio_snr;
        uint8_t radio_rssi;
    };
    struct propulsion_telem {
        bool logical_propulsion_arming;
        bool logical_launch_arming;
        bool electrical_propulsion_arming;
        bool propulsion_top_valves_continuity;
        bool propulsion_bottom_valves_continuity;

        bool fdov_armed_SW;
        bool fdov_energized_SW;
        bool mov_armed_SW;
        bool mov_energized_SW;
        bool vent_armed_SW;
        bool vent_energized_SW;

        uint8_t propulsion_vent_valve_readings;
        uint8_t propulsion_fdov_valve_readings;
        uint8_t propulsion_mov_valve_readings;
        uint8_t recov_main_readings;
        uint8_t recov_drogue_readings;

        /* Fuel Tanks */
        // Raw PT voltages
        uint16_t fuel_tank_top_pressure_psi;
        uint16_t fuel_tank_bottom_pressure_psi;

        int16_t fuel_tank_top_temperature_C;
        int16_t fuel_tank_bottom_temperature_C;
    };

    rocket_telem rocket_telem;
    fc_telem fc_telem;
    gps_telem gps_telem;
    imu_telem imu_telem;
    mag_telem mag_telem;
    barometer_telem barometer_telem;
    recovery_telem recovery_telem;
    radio_telem radio_telem;
    propulsion_telem propulsion_telem;
};