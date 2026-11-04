#pragma once
#include "util/helpers.h"
#include "stm32f427xx.h"
#include "i2c.h"
#include "gpio.h"

// ANSI COLORS
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"

// Terminal labels
typedef enum terminal_label { INFO, COMMAND, CAN, PERIPHERAL, STATE_R } terminal_label;

// Labels colors
#define INFO_P_COLOR       ANSI_COLOR_RESET
#define COMMAND_P_COLOR    ANSI_COLOR_YELLOW
#define CAN_P_COLOR        ANSI_COLOR_BLUE
#define PERIPHERAL_P_COLOR ANSI_COLOR_MAGENTA
#define STATE_R_P_COLOR    ANSI_COLOR_CYAN

// States colors
#define ERROR_P_COLOR   ANSI_COLOR_RED
#define WARNING_P_COLOR ANSI_COLOR_YELLOW
#define OK_P_COLOR      ANSI_COLOR_GREEN

void MRT_setup();

// I2C fix for busy bit
void I2C_busy_bit_fix(void);

void MRT_terminalPrint(terminal_label label, const char* peripheral, const char* state, const char* information);