#include "RF/e2bp.h"

E2bp::E2bp(uint16_t cepin, uint16_t cspin, const uint8_t* address,
           const uint8_t* channel)
    : RFConfigurator(cepin, cspin) {
    this->address = address;
    this->channel = channel;
    randomSeed(analogRead(0));  // initialize random generator with noise
}

bool E2bp::hack() {
    Serial.println("Hack !");
    this->begin();

    // for(uint16_t i=0; i<=0xff; i++) {
    //  delay(100);
    // trySomething(0x06);
    //}

    setupRFModule();
    // trySomething(0x00);

    // this->setupRFModule();
    return true;
}

void E2bp::setupRFModule() {
    uint8_t payload1[9] = {0x35, 0x04, 0x00, 0x20, 0xcc,
                           0x17, 0x00, 0x00, 0x00};
    uint8_t payload2[9] = {0x53, 0x04, 0x00, 0x20, 0xcc,
                           0x17, 0x00, 0x00, 0x00};
    // uint8_t randomByte;

    myRF->begin();

    // randomByte = random(0, 0xff);
    // payload1[6] = randomByte;
    // randomByte = random(0, 0xff);
    // payload2[6] = randomByte;

    trySomething(payload1, 9);
    //delay(20);
    //trySomething(payload2, 9);
    //delay(20);
    //myRF->flush_tx();
}

bool E2bp::trySomething(const uint8_t* payload, uint8_t len) {
    Serial.print("Payload=");
    for (uint8_t i = 0; i < len; i++) {
        Serial.print(payload[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
    uint8_t address[5] = {0xcc, 0x17, 0xcc, 0x17, 0x17};
    uint8_t channel = 0x29;
    byte recvBuf[2] = {0xff, 0xff};
    unsigned long startTime = millis();
    // char printBuf[32];

    ce(LOW);
    flush_tx();
    myRF->write_payload(payload, len, W_TX_PAYLOAD);
    myRF->write_register(RF_CH, 0b00101001);
    myRF->write_register(RF_SETUP, 0b00100011);
    myRF->write_register(RX_ADDR_P0, address, 5);
    myRF->write_register(TX_ADDR, address, len);
    myRF->write_register(RX_PW_P0, 0b00000010);
    myRF->write_register(EN_RXADDR, 0b00000001);
    myRF->write_register(EN_AA, 0b00000000);
    myRF->write_register(NRF_STATUS, 0b01110000);
    myRF->write_register(SETUP_RETR, 0b00000000);
    myRF->write_register(NRF_CONFIG, 0b00001110);
    delayMicroseconds(410);

    flush_rx();
    for(uint8_t i=0; i<10; i++) {
        myRF->write_register(NRF_CONFIG, 0b00001110);
        ce(HIGH);
        delayMicroseconds(15);
        ce(LOW);
        delayMicroseconds(685);
        myRF->write_register(NRF_CONFIG, 0b00001111);
        ce(HIGH);
        myRF->write_register(NRF_STATUS, 0b01110000);
        spiTrans(REUSE_TX_PL);
        delayMicroseconds(1000);
        ce(LOW);
    }

   return true;
}