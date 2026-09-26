#pragma once
#include <stdint.h>
#include <map>

typedef uint8_t CommandCode;

class Commands {
   public:
    Commands();

    bool registerCommand(CommandCode command_code, int (*&command_function)());

    bool runCommand(CommandCode command_code);

   private:
    std::map<CommandCode, int (*&)()> commandMap;
};