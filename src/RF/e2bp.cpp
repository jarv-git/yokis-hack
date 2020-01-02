#include "RF/e2bp.h"
#include "globals.h"
#include "utils.h"

E2bp::E2bp(uint16_t cepin, uint16_t cspin) : RFConfigurator(cepin, cspin) {
    device = new Device("e2bp");
    this->firstPayloadStatus = UNDEFINED;
    this->secondPayloadStatus = UNDEFINED;
}

E2bp::~E2bp() { delete device; }

void E2bp::reset() {
    this->firstPayloadStatus = UNDEFINED;
    this->secondPayloadStatus = UNDEFINED;
    this->loopContinue = true;
}

void E2bp::setDevice(const Device* device) { this->device->copy(device); }

const Device* E2bp::getDevice() { return this->device; }

bool E2bp::setDeviceStatus(DeviceStatus ds) {
    unsigned long timeout = millis() + 1000;
    uint8_t buf[9];

    reset();

    while (millis() <= timeout && secondPayloadStatus != ds) {
        firstPayloadStatus = UNDEFINED;
        setupRFModule();
        getFirstPayload(buf);
        sendPayload(buf);
        getSecondPayload(buf);
        sendPayload(buf);
        delay(10);
    }

    return true;
}

bool E2bp::on() { return setDeviceStatus(ON); }

bool E2bp::off() { return setDeviceStatus(OFF); }

bool E2bp::toggle() {
    uint8_t buf[9];
    unsigned long timeout = millis() + 1000;
    bool res = false;

    reset();
    setupRFModule();
    while (millis() < timeout && firstPayloadStatus == secondPayloadStatus) {
        firstPayloadStatus = UNDEFINED;
        getFirstPayload(buf);
        sendPayload(buf);
        getSecondPayload(buf);
        sendPayload(buf);
    }

    return res;
}

void E2bp::getFirstPayload(uint8_t* buf) {
    buf[0] = device->getBeginPacket();
    buf[1] = 0x04;
    buf[2] = 0x00;
    buf[3] = 0x20;
    buf[4] = device->getHardwareAddress()[0];
    buf[5] = device->getHardwareAddress()[1];
    buf[6] = random(0, 0xff);
    buf[7] = 0x00;
    buf[8] = 0x00;
}

void E2bp::getSecondPayload(uint8_t* buf) {
    buf[0] = device->getEndPacket();
    buf[1] = 0x04;
    buf[2] = 0x00;
    buf[3] = 0x20;
    buf[4] = device->getHardwareAddress()[0];
    buf[5] = device->getHardwareAddress()[1];
    buf[6] = random(0, 0xff);
    buf[7] = 0x00;
    buf[8] = 0x00;
}

bool E2bp::sendPayload(const uint8_t* payload) {
    if (IS_DEBUG_ENABLED) {
        Serial.print("Payload: ");
        for (uint8_t i = 0; i < 9; i++) {
            Serial.print(payload[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    }

    delay(20);
    setupPayload(payload);
    runMainLoop();

    return true;
}

void E2bp::setupRFModule() {
    begin();
    // write_register(RF_CH, device->getChannel());
    setChannel(device->getChannel());
    // write_register(RF_SETUP, 0b00100011);
    setDataRate(RF24_250KBPS);
    setPALevel(RF24_PA_LOW);
    // write_register(RX_ADDR_P0, device->getHardwareAddress(), 5);
    setAddressWidth(5);
    openReadingPipe(0, device->getHardwareAddress());
    // write_register(TX_ADDR, device->getHardwareAddress(), 5);
    // openWritingPipe(device->getHardwareAddress());
    // write_register(RX_PW_P0, 0x02);
    setPayloadSize(0x02);
    // write_register(EN_RXADDR, 1); // openreadingpipe set this already
    // write_register(EN_AA, 0);
    setAutoAck(false);
    // write_register(NRF_STATUS, 0b01110000); // no need to do this
    // write_register(SETUP_RETR, 0);
    setRetries(0, 0);
    // write_register(NRF_CONFIG, 0b00001110);
    openWritingPipe(device->getHardwareAddress());  // set to TX mode
    delay(4);  // It's literally what I sniffed on the SPI
    // flush_rx(); // done when calling begin()
    if (IS_DEBUG_ENABLED) {
        printDetails();
    }
}

void E2bp::runMainLoop() {
    unsigned long timeout = millis() + MAIN_LOOP_TIMEOUT_MILLIS;
    uint8_t buf[2];
    ce(LOW);
    // while not interrupted by RX or timeout
    while (loopContinue && millis() < timeout) {
        write_register(NRF_CONFIG, 0b00001110);  // PTX
        ce(HIGH);
        delayMicroseconds(15);
        ce(LOW);
        delayMicroseconds(685);

        write_register(NRF_CONFIG, 0b00001111);  // PRX
        ce(HIGH);
        write_register(NRF_STATUS, 0b01110000);  // Reset interrupts
        spiTrans(REUSE_TX_PL);
        delay(1);
        ce(LOW);
    }

    // We got here right after being interrupted
    if (available()) {
        read(buf, 2);
        Serial.print("Received: ");
        printBinaryRepresentation(buf[0], true);
        Serial.print(" ");
        printBinaryRepresentation(buf[1], true);
        Serial.println();
        if (firstPayloadStatus == UNDEFINED)
            firstPayloadStatus = buf[1] == 1 ? ON : OFF;
        else
            secondPayloadStatus = buf[1] == 1 ? ON : OFF;
    }
}

void E2bp::setupPayload(const uint8_t* payload) {
    // Prepare everything to send continuously the given payload
    loopContinue = true;
    setPayloadSize(PAYLOAD_LENGTH);
    //flush_tx();
    //write_payload(payload, PAYLOAD_LENGTH, W_TX_PAYLOAD);
    memcpy(currentPayload, payload, PAYLOAD_LENGTH);
    startFastWrite(currentPayload, PAYLOAD_LENGTH, false, false);
}

#if defined(ESP8266)
ICACHE_RAM_ATTR
#endif
void E2bp::stopMainLoop() {
    loopContinue = false;
    counter = 0;
}

#if defined(ESP8266)
ICACHE_RAM_ATTR
#endif
void E2bp::interruptTxOk() {
    if (IS_DEBUG_ENABLED) {
        Serial.println("E2bp - TX OK");
    }
    write_register(NRF_CONFIG, 0b00001111);  // PRX
    ce(HIGH);
    write_register(NRF_STATUS, 0b01110000);  // Reset interrupts
}

#if defined(ESP8266)
ICACHE_RAM_ATTR
#endif
void E2bp::interruptRxReady() {
    if (IS_DEBUG_ENABLED) {
        Serial.println("E2bp - RX READY");
    }
    counter++;
    stopMainLoop();
}

#if defined(ESP8266)
ICACHE_RAM_ATTR
#endif
void E2bp::interruptTxFailed() {
    // Ignore
    Serial.println("TX sent failed");
}
