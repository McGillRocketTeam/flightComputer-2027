#include <commands/commands.h>
#include "stm32f4xx.h"


bool Commands::registerCommand(CommandID command_code, CommandFn command_function){
    if(command_code < 0 || command_code >= NUM_COMMANDS){
        // Reject Invalid Commands
        return false;
    }

    if(commandMap[command_code] != nullptr){
        // Reject already bound commands
        return false;
    }

    commandMap[command_code] = command_function;
    return true;
}

void changeFreq(CHANGE_FREQ_ArgPack);
void zeroAltimeter(ZERO_ALTIMETER_ArgPack);
void forceAtomics(FORCE_ATOMICS_ArgPack);

bool Commands::runCommand(CommandID command_code, uint8_t* data, size_t len){
    if (!checkValid(command_code, data, len)) {
        printf("COMMAND INVALID\r\n");
        return false;
    }

    switch (command_code) {
        case (NO_OP):
            // No Op
            return true;
        case (RESET_AV):
            // STUBBED FOR NOW
            NVIC_SystemReset();
            return true;
        case (LAUNCH):
            return true;
        case (CHANGE_FREQ): {
            // As Examples of arg commands
            CHANGE_FREQ_ArgPack args;
            memcpy(&args, data, len);
            changeFreq(args);
        } break;
        case (ZERO_ALTIMETER): {
            ZERO_ALTIMETER_ArgPack args;
            memcpy(&args, data, len);
            zeroAltimeter(args);
        } break;
        case (FORCE_ATOMICS): {
            FORCE_ATOMICS_ArgPack args;
            memcpy(&args, data, len);
            forceAtomics(args);
        } break;
    };
}

bool checkValid(CommandID id, uint8_t* data, size_t len) {
    if (id < 0 || id >= NUM_COMMANDS) {
        // Reject invalid commands
        return false;
    }

    size_t desired_len = argPackLengths[id];

    if (desired_len <= 0) {
        // Reject non-argumented commands
        return false;
    }

    if (desired_len != len) {
        // Reject if we do not have the correct amount of data
        return false;
    }
    return true;
}