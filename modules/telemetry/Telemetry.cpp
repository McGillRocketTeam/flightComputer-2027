#include <telemetry/Atomics.h>
#include <telemetry/Telemetry.h>

Telemetry::Telemetry() : builder(nullptr, 0){
    
};

std::function<uint8_t*(Telemetry*)>
    Telemetry::atomic_fill_functions [AT_TOTAL] = {
        fill_recovery_atomic,
        fill_prop_states_atomic,
        fill_prop_atomic,
        fill_flight_stage_atomic,
        fill_fc_internal_atomic,
        fill_altitude_atomic,
        fill_altitude_events_atomic,
        fill_acceleration_atomic,
        fill_gyro_atomic,
        fill_gps_atomic,
        fill_radio_atomic,
        fill_sd_atomic,
        [](Telemetry*){return nullptr;}, // Payload Status Placeholder
        [](Telemetry*){return nullptr;}, // Payload Data Placeholder 
        [](Telemetry*){return nullptr;}, // Payload Adapter 0 Placeholder
        [](Telemetry*){return nullptr;}, // Payload Adapter 1 Placeholder
        [](Telemetry*){return nullptr;}, // Payload Adapter 2 Placeholder
        [](Telemetry*){return nullptr;}, // Payload Adapter 3 Placeholder
        fill_gps_debug_atomic,
};

FrameBuilder* Telemetry::getASTRAFrame(FrameHeader header) {
    uint8_t buf[sizeof(FrameHeader) +
                payload_length_from_bitmap(header.atomics_bitmap)];

    // Reset the FrameBuilder, this allows us to change the buffer size
    builder.~FrameBuilder();
    new (&builder) FrameBuilder(buf, sizeof(buf));

    for (int i = 0; i < AT_TOTAL; i++) {
        if (header.atomics_bitmap & 1 << i) {
            // Atomic present in bitmap
            builder.addAtomic(i, atomic_fill_functions[i](this), AT_SIZE[i]);
        }
    }

    return &builder;
}