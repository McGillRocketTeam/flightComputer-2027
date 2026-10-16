#pragma once
#include <stdint.h>
#include <tuple>
#include <telemetry/generated/command_packet_examples.h>
#define COMMAND_LAMBDA(function) [](uint8_t* data, size_t len) -> int function // Define used to simplify the lambda boilerplate for a no-arg command


using CommandFn = int (*)(uint8_t*, size_t);

class Radio;

class Commands {
   public:
    Commands() = default;
    
    template <typename T>
    bool bindModule(T* module);

    uint16_t runCommand(CommandID command_code){return runCommand(command_code, nullptr, 0);};

    uint16_t runCommand(CommandID command_code, uint8_t* data, size_t len);


   private:
    bool checkValid(CommandID id, uint8_t* data, size_t len);

    // Tuple containing a pointer to each module that can be bound
    // The validity of these pointers determines whether the command is configured
    std::tuple<Radio*> boundModules;

    template <typename T>
    bool handleArgs(T& args, uint8_t* data, size_t len);
};