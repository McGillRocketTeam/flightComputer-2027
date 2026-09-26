#pragma once
#ifdef __cplusplus
#include <cstdint>
#include "telemetry_packets.h"

uint32_t payload_length_from_bitmap(uint32_t bm);
uint32_t atomic_offset(uint32_t bm, int k);
#endif