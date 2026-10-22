#pragma once

// I2C Addressing for the 24xx08 EEPROM is as follows:
// 7 bit address + 1 bit operation
// B0 - B3: Control code (0b1010)
// B4: DNC (Chosen as 0)
// B5-B6: Block select (0-3)
//
// B7: Operation Code: R/W bit
//
// In total there are 8 distinct addresses that this device will respond to
// 0b10100000 to 0b10101111
// Or 0xA0 to 0xAF

#define CONTROL_CODE_24xx08 0b10100000

#define BLOCK0_24xx08 0b00 << 1
#define BLOCK1_24xx08 0b01 << 1
#define BLOCK2_24xx08 0b10 << 1
#define BLOCK3_24xx08 0b11 << 1

#define READ_24xx08 0b1
#define WRITE_24xx08 0b0

#define EEPROM_SIZE_BYTES 1024
#define EEPROM_NUM_BLOCKS 4u