#include <commands/commands.h>
#include "stm32f4xx.h"
#include <util/helpers.h>
#include <radios/Radios.h>

template <typename T>
bool Commands::bindModule(T* module){
    T mod = std::get<T*>(boundModules);

    if(!mod){
        return false;
    }

    mod = module;
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
            Radio* radio = std::get<Radio*>(boundModules);
            if(radio && handleArgs(args, data, len)){
                return radio->changeFreq(args);
            }
            return false;
        }
        case (ZERO_ALTIMETER):
        case (FORCE_ATOMICS):
        default: return false;
    };
}

bool Commands::checkValid(CommandID id, uint8_t* data, size_t len) {
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

// Handles args and prevents de-ref null data pointer
template <typename T>
bool Commands::handleArgs(T& args, uint8_t* data, size_t len){
    if(!data){
        return false;
    } else {
        memcpy(&args, data, len);
        return true;
    }
}