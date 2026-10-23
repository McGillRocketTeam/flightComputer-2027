#include <nonvolatile/24xx08/24xx08.h>
#include <stm32f4xx_hal.h>
#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))


// Takes a raw 10-bit memory address, and converts it to wire format for a 24xx08
// Adds control code, & arranges bytes correctly
// Used so as to keep memory locations in the memory map correct, and unaffected by device impl
static I2C_Address parse_address(uint16_t addr){
    I2C_Address out;
    // Address is as follows: (C indicating Control Code, B Block Address, X Don't Care, and O is R/W op, set by HAL)
    // C3 C2 C1 C0 X B1 B0 O
    // So operations are as follows, clear the word address (lower 8) (>>8), clear any bits but B1 and B0 (& 0x03), and add space for O
    out.dev_addr = CONTROL_CODE_24xx08 | (((addr >> 8) & 0x03) <<1);
    out.word_addr = addr & 0xFF; // Only the lower byte

    return out;
}

status_24xx08_t write_byte(uint16_t addr, const uint8_t data) {
    return write_page(addr, &data, 1);
}

status_24xx08_t write_page(uint16_t addr, const uint8_t* data, size_t len) {
    if (!data || len == 0 || addr >= EEPROM_SIZE_BYTES) {
        return STATUS_24xx08_ERROR;
    }

    I2C_Address dw = parse_address(addr);

    if (BLOCK_SIZE - dw.word_addr < len) {
        // We are trying to write more data than available in this block
        // Reject, for now, multi-block data is only supported as multiple calls
        // Can't see a reason to change this -AH
        return STATUS_24xx08_ERROR_NO_SPACE;
    }

    uint8_t buf[1 + PAGE_SIZE]; // Write buffer, space allocated for 1 word address + 16 data bytes


    // Start by sending data until the next page boundary, or until data
    // depleted
    do {
        uint8_t len_actual = MIN(PAGE_SIZE - (dw.word_addr % PAGE_SIZE), len);

        buf[0] = dw.word_addr;
        memcpy(&buf[1], data, len_actual);

        if (HAL_I2C_Master_Transmit(I2C_BUS_24xx08, dw.dev_addr, buf,
                                    len_actual + 1, 10) != HAL_OK) {
            return STATUS_24xx08_ERROR;
        }

        uint32_t start_time = HAL_GetTick();

        while(HAL_I2C_IsDeviceReady(I2C_BUS_24xx08, dw.dev_addr, 1, 2)) {
            if(HAL_GetTick() - start_time > 10) {return STATUS_24xx08_TIMEOUT;}
            // While the device is not ready, check again, until timeout has expired
            // TODO: Replace when implementing DMA
        }

        dw.word_addr += len_actual;
        data += len_actual;
        len -= len_actual;

    } while (len > 0);

    return STATUS_24xx08_OK;
}

status_24xx08_t read_data(uint16_t addr, uint8_t* out, size_t len) {
    if(addr >= EEPROM_SIZE_BYTES){
        // Invalid Address
        return STATUS_24xx08_ERROR;
    }

    if(len > EEPROM_SIZE_BYTES - addr){
        return STATUS_24xx08_ERROR_NO_SPACE;
    }

    if(!out || len == 0){
        return STATUS_24xx08_ERROR;
    }

    I2C_Address dw = parse_address(addr);

    // This is a random address read. Not deemed necessary to implement Current Address separately. Would save 1 byte OTW
    if(HAL_I2C_Mem_Read(I2C_BUS_24xx08, dw.dev_addr, dw.word_addr, I2C_MEMADD_SIZE_8BIT, out, len, 10 + len) != HAL_OK){
        return STATUS_24xx08_ERROR;
    }
    // TODO: Add some proper timing estimation

    return STATUS_24xx08_OK;
}