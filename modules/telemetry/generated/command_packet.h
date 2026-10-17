#pragma once

#include <cstdint>

// Command string supports up to 4 characters
// It must be null terminated

typedef struct __attribute__((__packed__)) command_packet_data {
    uint8_t command_id;
    char command_string[6];
} command_packet_data;

typedef union command_packet {
    command_packet_data data;
    uint8_t bytes[sizeof(command_packet_data)];
} command_packet;

static_assert(sizeof(command_packet_data) % 4 == 3,
    "command_packet_data must be ensure 4 byte alignment in command_packet_extended_data");

// Be very careful, we need the float args to start 4byte aligned
typedef struct __attribute__((__packed__)) command_packet_extended_data {
    command_packet base;
    uint8_t argc;
    float args[3];
} command_packet_extended_data;

typedef union command_packet_extended {
    command_packet_extended_data data;
    uint8_t bytes[sizeof(command_packet_extended_data)];
} command_packet_extended;
