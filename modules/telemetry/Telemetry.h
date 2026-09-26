#pragma once
#include <stdint.h>

#include "modules/sensors/CurrentSensor.h"
#include "modules/sensors/GPS.h"
#include "modules/sensors/IMU.h"
#include "modules/sensors/Magnetometer.h"

class Telemetry {
   public:
    Telemetry();

    bool addCurrentSensor(CurrentSensor& sensor);
    bool addMagnetometer(Magnetometer& sensor);
    bool addGPS(GPS& sensor);
    bool addIMU(IMU& sensor);

    void pollTelemetry();

    enum flight_stage {
        PAD = 0,
        BOOSTER_ASCENT,
        PRIMARY_COAST,
        SUSTAINER_ASCENT,
        SECONDARY_COAST,
        DROGUE_DESCENT,
        MAIN_DESCENT,
        LANDED,
        INVALID = -1,
    };

    struct {
        flight_stage flight_stage;
        float altitude_from_ground_ft;
        float altitude_rate_of_change_ft_s;
        float apogee_from_ground_ft;
        float main_deployment_from_ground_ft;
        float drogue_deployment_from_ground_ft;
    } rocket_telem;

    struct {
        int16_t battery_voltage_dV;
        int16_t battery_current_mA;
        bool CAN_bus_active;
        bool SD_card_file_open;
        bool SD_card_deletion_armed;
    } fc_telem;

    struct {
        bool gps_fix;
        int32_t GPS_longitude;
        int32_t GPS_latitude;
        int32_t GPS_altitude_mm;
        float time_since_GPS_ack_s;
    } gps_telem;

    struct {
        float acceleration_x_milli_G;
        float acceleration_y_milli_G;
        float acceleration_z_milli_G;
        float gyro_rate_x_mdeg_s;
        float gyro_rate_y_mdeg_s;
        float gyro_rate_z_mdeg_s;
    } imu_telem;

    struct {
        float magnetic_orientation_x_mgauss;  // Towards the GPS module
        float magnetic_orientation_y_mgauss;  // Left from the processor
        float magnetic_orientation_z_mgauss;  // Upwards from the processoR
    } mag_telem;

    struct {
        float atmospheric_temperature_C;
        float atmospheric_pressure_hpa;
    } barometer_telem;

    struct {
        bool drogue_armed_SW;
        bool drogue_energized_SW;
        bool main_armed_SW;
        bool main_energized_SW;
    } recovery_telem;

    struct {
        int8_t radio_snr;
        uint8_t radio_rssi;
    } radio_telem;

    struct {
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
    } propulsion_telem;
};