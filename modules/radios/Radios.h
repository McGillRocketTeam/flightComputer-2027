#pragma once
#include <RadioLib.h>
#include <radios/STM32HAL.h>
#include <radios/AtomicRules.h>
#include <telemetry/Telemetry.h>

#include <FreeRTOS.h>
#include <semphr.h>
#include <memory>

#define RADIO_SPI &hspi2 //TODO: MOVE TO MAIN

#define RADIO_SF 8
#define RADIO_BW RADIOLIB_SX126X_LORA_BW_250_0
#define RADIO_CR RADIOLIB_SX126X_LORA_CR_4_8
#define RADIO_PREAMBLE_LEN 12
#define RADIO_SYNCWORD 0x12

#define MAX_RX_TIMEOUT 2000
#define MAX_SEMAPHORE_TIMEOUT MAX_RX_TIMEOUT + 10

enum FrequencyBand : uint32_t{
    FREQ_433 = 435,
    FREQ_900 = 903,
};

enum AstraState{
    ASTRA_PREINIT = 0,
    ASTRA_READY,
    ASTRA_TX_NOCTS,
    ASTRA_TX_CTS,
    ASTRA_RX,
};

class Radio{
    public:
    Radio(FrequencyBand freq);
    void addTelemetry(Telemetry* telemetry){this->telemetry = telemetry;};

    // Begins radio on configured frequency
    int16_t init();

    // Runs ASTRA state machine
    void update(); 

    // Puts radio into receive mode
    void receive();

    // Processes received data
    void getReceived();

    // Transmits an ASTRA packet
    void transmit();

    void setCTSPeriod(uint8_t period) {cts_period = period;};

    void setCTS(){cts = true;};
    void setIDForAck(uint16_t seq){
        id_to_ack = seq;
        ack = true;
    }; 

    std::vector<AtomicRule> atomicRules;

    uint32_t packet_counter = 0;

    private:
    AstraState state = AstraState::ASTRA_PREINIT;

    uint8_t cts_period = 4;

    bool cts = false;
    bool ack = false;
    uint16_t id_to_ack = 0;

    Telemetry* telemetry = nullptr;

    FrequencyBand freq;

    STM32HAL_Radios stmHAL;
    Module mod;

    std::unique_ptr<SX1268> radio433;
    std::unique_ptr<SX1262> radio900;

    SX126x* radio;

    void giveRadioSemaphore();
    static void giveRadioSemaphoreTrampoline(void* ctx);

    StaticSemaphore_t radioSemBuffer;
    SemaphoreHandle_t radioSemaphore;
};