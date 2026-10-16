#pragma once
#include <stdint.h>
#include <telemetry/generated/command_packet_examples.h>

using CommandFn = int (*)();

class Commands {
   public:
    Commands();

    bool registerCommand(CommandID command_code, CommandFn command_function);

    bool runCommand(CommandID command_code){return runCommand(command_code, nullptr, 0);};

    bool runCommand(CommandID command_code, uint8_t* data, size_t len);

   private:
    CommandFn commandMap[NUM_COMMANDS];

    bool checkValid(CommandID id, uint8_t* data, size_t len);

    template <typename T>
    T Commands::handleArgs(T& args, uint8_t* data, size_t len);
};