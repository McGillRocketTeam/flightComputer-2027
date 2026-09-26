#include "frame_view.h"
#include "command_packet.h"
#ifdef __cplusplus

FrameView::FrameView() : _base(nullptr), _len(0) {}
FrameView::FrameView(const uint8_t* bytes, size_t len) : _base(bytes), _len(len) {}
void FrameView::reset(const uint8_t* bytes, size_t len) { _base = bytes; _len = len; }

ParseError FrameView::validate() const {
    if (_len == sizeof(command_packet_data))
        return ParseError::COMMAND_PACKET;
    if (_len == sizeof(command_packet_extended))
        return ParseError::COMMAND_PACKET_EXTENDED;
    if (_len < sizeof(FrameHeader))
        return ParseError::TooShort;
    const auto* h = header();
    const uint32_t need = payload_length_from_bitmap(h->atomics_bitmap);
    if (sizeof(FrameHeader) + need > _len) return ParseError::PayloadTooShort;
    for (int i = 0; i < AT_TOTAL; ++i)
        if ((h->atomics_bitmap & (1u << i)) && AT_SIZE[i] == 0)
            return ParseError::UnknownAtomicSize;
    return ParseError::Ok;
}

const FrameHeader* FrameView::header() const {
    return reinterpret_cast<const FrameHeader*>(_base);
}
const uint8_t* FrameView::payload() const {
    return _base + sizeof(FrameHeader);
}
size_t FrameView::payload_len() const {
    return (_len >= sizeof(FrameHeader)) ? (_len - sizeof(FrameHeader)) : 0;
}

bool FrameView::cts() const {
    return (_len >= sizeof(FrameHeader)) && header_has_cts(header());
}
bool FrameView::ack() const {
    return (_len >= sizeof(FrameHeader)) && header_has_ack(header());
}
bool FrameView::bad() const {
    return (_len >= sizeof(FrameHeader)) && header_has_bad(header());
}
bool FrameView::nak() const {
    return (_len >= sizeof(FrameHeader)) && header_has_nak(header());
}

int FrameView::ack_id() const {
    return header_get_ack_id(header());
}

bool FrameView::hasAtomic(int idx) const {
    if (idx < 0 || idx >= AT_TOTAL || _len < sizeof(FrameHeader)) return false;
    return (header()->atomics_bitmap & (1u << idx)) != 0;
}

const uint8_t* FrameView::atomicPtr(int idx) const {
    if (!hasAtomic(idx)) return nullptr;
    const uint32_t off = atomic_offset(header()->atomics_bitmap, idx);
    if (off + AT_SIZE[idx] > payload_len()) return nullptr;
    return payload() + off;
}
#endif
