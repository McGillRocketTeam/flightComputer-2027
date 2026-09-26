#pragma once
#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
#include <vector>
#include "frame_header.h"

struct FrameBuilder {
    FrameHeader h{0};
    uint8_t* out{nullptr};
    size_t cap{0};
    size_t used{0};

    std::vector<uint8_t> temp_buffer;
    size_t temp_used{0};

    const void* atomic_pointers[MAX_NUMBER_OF_ATOMIC_PACKETS]{};

    FrameBuilder(uint8_t* buf, size_t capacity);
    ~FrameBuilder() = default;

    // We should never copy the builder cause we hold pointers to certain atomics
    FrameBuilder(const FrameBuilder&) = delete;
    FrameBuilder& operator=(const FrameBuilder&) = delete;

    bool addAtomic(int idx, const void* src, size_t sz);
    size_t finalize(uint16_t seq, uint8_t flags, uint8_t ack_id);
};
#endif