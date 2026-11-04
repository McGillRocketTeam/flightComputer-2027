#pragma once
#include <cstring>

#include "sd_card/data/MRT_sd_data.h"
#include "telemetry/generated/frame_builder.h"

// Extension of the ASTRA FrameBuilder to make modifications for the slimmer SD
// ASTRA format
struct SD_FrameBuilder : FrameBuilder {
    using FrameBuilder::FrameBuilder;

    size_t finalize_SD() {
        // For SD ASTRA a few changes are made:
        // The atomic bitmask is only 2 bytes
        // The "frame header" consists soleley of that bitmask

        std::memcpy(out, &h.atomics_bitmap, SD_ATOMIC_BITMASK_LEN);

        size_t current_offset = SD_ATOMIC_BITMASK_LEN;

        for (int idx = 0; idx < MAX_NUMBER_OF_ATOMIC_PACKETS; ++idx) {
            if (atomic_pointers[idx] != nullptr) {
                size_t sz = SD_AT_SIZE[idx];
                std::memcpy(out + current_offset, atomic_pointers[idx], sz);
                current_offset += sz;
            }
        }

        used = current_offset;
        return current_offset;
    };

    bool addAtomic_SD(int idx, const void* src, size_t sz) {
        if (idx < 0 || idx >= SD_AT_TOTAL) return false;
        if (SD_AT_SIZE[idx] != sz) return false;
        if (temp_used + sz > cap) return false;
        if (h.atomics_bitmap & (1u << idx)) return false;

        std::memcpy(temp_buffer.data() + temp_used, src, sz);

        atomic_pointers[idx] = temp_buffer.data() + temp_used;
        temp_used += sz;
        h.atomics_bitmap |= (1u << idx);

        return true;
    };

    void reset() {
        h.atomics_bitmap = 0;
        temp_used = 0;
        std::memset(atomic_pointers, 0, sizeof(atomic_pointers));
    }
};