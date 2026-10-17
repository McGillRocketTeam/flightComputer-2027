#include "frame_helpers.h"

uint32_t payload_length_from_bitmap(uint32_t bm) {
    uint32_t total = 0;
    for (int i = 0; i < AT_TOTAL; ++i)
        if (bm & (1u << i)) total += AT_SIZE[i];
    return total;
}

uint32_t atomic_offset(uint32_t bm, int k) {
    uint32_t off = 0;
    for (int i = 0; i < k; ++i)
        if (bm & (1u << i)) off += AT_SIZE[i];
    return off;
}
