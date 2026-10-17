#ifdef __cplusplus
#include <cstring>
#include <cstdint>
#include "frame_builder.h"
#include "frame_header.h"
#include "telemetry_packets.h"

FrameBuilder::FrameBuilder(uint8_t* buf, size_t capacity)
    : out(buf),
      cap(capacity),
      used(sizeof(FrameHeader)),
      temp_buffer(capacity),
      temp_used(0)
{
    std::memset(atomic_pointers, 0, sizeof(atomic_pointers));
}

bool FrameBuilder::addAtomic(int idx, const void* src, size_t sz) {
    if (idx < 0 || idx >= AT_TOTAL) return false;
    if (AT_SIZE[idx] != sz) return false;
    if (temp_used + sz > cap) return false;
    if (h.atomics_bitmap & (1u << idx)) return false;

    std::memcpy(temp_buffer.data() + temp_used, src, sz);

    atomic_pointers[idx] = temp_buffer.data() + temp_used;
    temp_used += sz;
    h.atomics_bitmap |= (1u << idx);

    return true;
}

size_t FrameBuilder::finalize(uint16_t seq, uint8_t flags, uint8_t ack_id) {
    h.seq    = seq;
    h.flags  = flags;
    h.ack_id = ack_id;

    std::memcpy(out, &h, sizeof(FrameHeader));

    size_t current_offset = sizeof(FrameHeader);

    for (int idx = 0; idx < MAX_NUMBER_OF_ATOMIC_PACKETS; ++idx) {
        if (atomic_pointers[idx] != nullptr) {
            size_t sz = AT_SIZE[idx];
            std::memcpy(out + current_offset, atomic_pointers[idx], sz);
            current_offset += sz;
        }
    }

    used = current_offset;
    return current_offset;
}
#endif