#include <radios/Radios.h>
#include <main.h>
#include <spi.h>
#include <util/HAL.h>
#include <util/helpers.h>

Radio::Radio(FrequencyBand freq)
    : freq(freq)
    , stmHAL(RADIO_SPI)
    , mod(&stmHAL, 
        STM32_PIN(SX_CS_GPIO_Port, SX_CS_Pin),
        STM32_PIN(SX_DIO_GPIO_Port, SX_DIO_Pin),
        STM32_PIN(SX_RST_GPIO_Port, SX_RST_Pin),
        STM32_PIN(SX_BUSY_GPIO_Port, SX_BUSY_Pin))
    {

    switch (freq){
        case FrequencyBand::FREQ_433:
            radio433 = std::make_unique<SX1268>(&mod);
            break;
        case FrequencyBand::FREQ_900:
            radio900 = std::make_unique<SX1262>(&mod);
            break;
    }

    atomicRules = {
        /* Recov */           AtomicRule(nullptr),
        /* Prop States */     AtomicRule(nullptr),
        /* Prop */            AtomicRule(nullptr),
        /* Flight Stage */    AtomicRule(nullptr),
        /* FC Internal */     AtomicRule(nullptr),
        /* Altitude */        AtomicRule(nullptr),
        /* Altitude Events */ AtomicRule(nullptr),
        /* Acceleration */    AtomicRule(nullptr),
        /* Gyro */            AtomicRule(nullptr),
        /* GPS */             AtomicRule(nullptr),
        /* Radio */           AtomicRule(nullptr),
        /* SD */              AtomicRule(nullptr),
        /* Payload Status */  AtomicRule(nullptr),
        /* Payload Data */    AtomicRule(nullptr),
        /* PL-0 */            AtomicRule(nullptr),
        /* PL-1 */            AtomicRule(nullptr),
        /* PL-2 */            AtomicRule(nullptr),
        /* PL-3 */            AtomicRule(nullptr),
        /* GPS Debug */       AtomicRule(nullptr),
    };
}

int16_t Radio::init(){
    int16_t status = RADIOLIB_ERR_UNKNOWN;

    if(freq == FrequencyBand::FREQ_433 && radio433){
        status = radio433->begin((float)freq, RADIO_BW, RADIO_SF, RADIO_CR, RADIO_SYNCWORD, 0, RADIO_PREAMBLE_LEN, 0, true);
        radio = radio433.get();
    } else if (freq == FrequencyBand::FREQ_900 && radio900){
        status = radio900->begin((float)freq, RADIO_BW, RADIO_SF, RADIO_CR, RADIO_SYNCWORD, 0, RADIO_PREAMBLE_LEN, 0, true);
        radio = radio900.get();
    }

    // Bind DIO interrupt so that it gives the semaphore
    setCallback(STM32_PIN(SX_DIO_GPIO_Port,SX_DIO_Pin), &Radio::giveRadioSemaphoreTrampoline, this);

    radioSemaphore = xSemaphoreCreateBinaryStatic(&radioSemBuffer);

    return status;
}

void Radio::giveRadioSemaphore(){
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(radioSemaphore, &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

void Radio::giveRadioSemaphoreTrampoline(void* ctx){
    static_cast<Radio*>(ctx)->giveRadioSemaphore();
}

void Radio::update(){
    if(xSemaphoreTake(radioSemaphore, MAX_SEMAPHORE_TIMEOUT) == pdFALSE){
        // If we could not take the semaphore, short return.
        return;
    }

    switch (state) {
        case (ASTRA_PREINIT):
            break;
        case (ASTRA_READY):
            // Send sequence number 0 acknowledgement as first transmission
            // This is a routine on top of the ASTRA protocol that the GSC uses to detect success of a commanded reset
            setIDForAck(0);

            transmit();
            state = ASTRA_TX_NOCTS;
            break;
        case (ASTRA_TX_NOCTS):
            // If we just sent a NOCTS packet, than the only outward state transition is coming due for a CTS packet.
            // This transition only affects what the transmission will say, and what happens after, we still transmit.
            if(packet_counter % cts_period == 0){
                setCTS();
                state = ASTRA_TX_CTS;
            }
            transmit();
            break;
        case (ASTRA_TX_CTS):
            // We just sent a CTS packet, therefore, we must enter receive mode
            receive();
            state = ASTRA_RX;
            break;
        case (ASTRA_RX):
            getReceived();
            // Handle Radios contact time tracking
            
            // Then, after our receive, we return to normal CTS/NOCTS operations
            state = ASTRA_TX_NOCTS;
            break;
    }
}

void Radio::transmit(){
    // Generate ASTRA Header
    FrameHeader header;

    header.ack_id = id_to_ack;
    header.seq = packet_counter;

    uint32_t atomics = 0;
    for(int i = 0; i < AT_TOTAL; i++){
        if(atomicRules[i].get()){
            atomics |= 1 << i;
        }
    }

    // Get ASTRA Payload from Telemetry if able, otherwise just send empty header
    uint8_t* data = (uint8_t*)&header;
    size_t len = sizeof(FrameHeader);

    if(telemetry){
        FrameBuilder* builder = telemetry->getASTRAFrame(header);
        len = builder->finalize(header.seq, header.flags, header.ack_id);

        data = builder->out;
    }

    radio->startTransmit(data, len);
}

void Radio::receive(){
    int16_t state = radio->startReceive(MAX_RX_TIMEOUT);
    if(state == RADIOLIB_ERR_NONE){
        println("Started Receive");
    } else {
        println("Error on RX Start");
    }
}

void Radio::getReceived(){
    size_t len = radio->getPacketLength();

    uint8_t data[len];

    int16_t state = radio->readData(data, len);

    if(state != RADIOLIB_ERR_NONE){
        println("ERROR READING RECEIVED");
        return;
    }

    command_packet_extended packet;

    // Copy packet data, and args if present out
    if(len == sizeof(command_packet) || len == sizeof(command_packet_extended)){
        size_t len_command = sizeof(packet.data.base);
        memcpy(&(packet.data.base), data, len_command);

        if(len == sizeof(command_packet_extended)){
            size_t len_argc = sizeof(packet.data.argc);
            size_t len_args = sizeof(packet.data.args);

            memcpy(&(packet.data.argc), data + len_command, len_argc);
            memcpy(&(packet.data.args), data + len_command + len_argc, len_args);
        }
    }

    // PARSE COMMAND HERE
}