#pragma once

#include <stdint.h>

#include <cstring>
#include <functional>
#include <map>
#include <memory>
#include <variant>

// Mapping between all configured commands and their IDs
// Used for readability, not technically necessary
// Includes all commands, even those outside of the configured mission
enum CommandID : uint8_t {
    NO_OP = 0,
    RESET_AV = 1,
    LAUNCH = 2,
    CHANGE_FREQ = 3,
    ZERO_ALTIMETER = 4,
    FORCE_ATOMICS = 5,
    NUM_COMMANDS = 6,
};

// Define command argument "packs", sets of arguments. Only for argumented
// commands
struct CHANGE_FREQ_ArgPack {
    uint32_t freq;
};

struct ZERO_ALTIMETER_ArgPack {
    float altitude;
};

struct FORCE_ATOMICS_ArgPack {
    uint32_t bitmap;
};

const size_t argPackLengths[NUM_COMMANDS] = {
    0,                               // NO-OP
    0,                               // RESET_AV
    0,                               // LAUNCH
    sizeof(CHANGE_FREQ_ArgPack),     // CHANGE_FREQ
    sizeof(ZERO_ALTIMETER_ArgPack),  // ZERO_ALTIMETER
    sizeof(FORCE_ATOMICS_ArgPack),   // FORCE_ATOMICS
};

// END EXAMPLE OF GENERATED COMMAND HEADER