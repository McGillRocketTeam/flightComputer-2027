#pragma once
#ifdef __cplusplus
#include <cstdint>

// ---- flags (1 = set) ----

// Clear To Send flag mask
#define FLAG_CTS 0x01
// ACKnowledgement flag mask
#define FLAG_ACK 0x02
// BAD command acknowledged
#define FLAG_BAD 0x04
// Nop AcKnowledgement 
#define FLAG_NAK 0x8

#define MAX_NUMBER_OF_ATOMIC_PACKETS 32

// ---- 8-byte frame header ----
typedef struct  __attribute__((packed)) {
    uint16_t seq;            // 2B sequence number
    uint8_t  flags;          // bit0=CTS, bit1=ACK, bit2=BAD 3-7 reserved
    uint8_t  ack_id;         // 0 if unused
    uint32_t atomics_bitmap; // bit i => atomic i present in payload
} FrameHeader;

static_assert(sizeof(FrameHeader) == 8, "FrameHeader must be 8 bytes");

typedef union {
    FrameHeader data;
    uint8_t     bytes[sizeof(FrameHeader)];
} FrameHeaderPacket;

static inline bool header_has_cts(const FrameHeader* h) { return (h->flags & FLAG_CTS) != 0; }
static inline bool header_has_ack(const FrameHeader* h) { return (h->flags & FLAG_ACK) != 0; }
static inline bool header_has_bad(const FrameHeader* h) { return (h->flags & FLAG_BAD) != 0; }
static inline bool header_has_nak(const FrameHeader* h) { return (h->flags & FLAG_NAK) != 0; }
static inline uint8_t header_get_ack_id(const FrameHeader* h) { return h->ack_id; }
#endif
