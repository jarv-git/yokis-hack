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
    delay(200);

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

    // randomByte = random(0, 0xff);
    // payload1[6] = randomByte;
    // randomByte = random(0, 0xff);
    // payload2[6] = randomByte;

    trySomething(payload1, 9);
    delay(20);
    trySomething(payload2, 9);
    delay(20);
    myRF->flush_tx();
}

bool E2bp::trySomething(const uint8_t* payload, uint8_t len) {
    Serial.print("Payload=");
    for (uint8_t i = 0; i < 9; i++) {
        Serial.print(payload[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
    uint8_t address[5] = {0xcc, 0x17, 0xcc, 0x17, 0x17};
    uint8_t channel = 0x29;
    byte recvBuf[2] = {0x00, 0x00};
    // char printBuf[32];

    myRF->flush_tx();
    myRF->setAutoAck(false);
    myRF->setCRCLength(RF24_CRC_16);
    // myRF->disableCRC();
    myRF->setDataRate(RF24_250KBPS);
    myRF->setPALevel(RF24_PA_LOW);
    myRF->setChannel(channel);
    myRF->setPayloadSize(9);
    myRF->setAddressWidth(5);
    myRF->openWritingPipe(address);
    myRF->stopListening();
    myRF->printDetails();
    delay(5);

    myRF->write(payload, len);

    delay(4);
    myRF->flush_rx();
    myRF->write_register(NRF_CONFIG, 0xe);
    delayMicroseconds(700);
    myRF->write_register(NRF_CONFIG, 0xf);
    myRF->write_register(NRF_STATUS, 0x70);
    myRF->stopListening();
    // Resend payload
    myRF->reUseTX();
    delay(1);

    //myRF->powerDown();

/*
    myRF->disableCRC();
    myRF->setPayloadSize(0x02);
    myRF->openReadingPipe(0, address);
    for (;;) {
        // Try to get a response from the device
        //myRF->startListening();
        delay(5);
        if (myRF->available()) {
            myRF->read(recvBuf, 2);
            Serial.print("Data received: ");
            for (uint8_t i = 0; i < 2; i++) {
                Serial.print(recvBuf[i], HEX);
                Serial.print(" ");
            }
            Serial.println("");
            break;
        }
    }
*/

    /*
        myRF->setPayloadSize(0x02);
        myRF->openReadingPipe(0, address);
        myRF->startListening();
        //myRF->printDetails();
        while (true) {
            if (myRF->available()) {
                myRF->read_payload(recvBuf, 2);
                Serial.print("Data received: ");
                for (uint8_t i = 0; i < 2; i++) {
                    Serial.print(recvBuf[i], HEX);
                    Serial.print(" ");
                }
                Serial.println("");
            }
        }  // while
    */
    /*
        myRF->setPayloadSize(2);
        myRF->setAddressWidth(5);
        myRF->openReadingPipe(0, address);
        myRF->startListening();
        while (true) {
            if (myRF->available()) {
                myRF->read(recvBuf, 2);
                Serial.print("Data received: ");
                for (uint8_t i = 0; i < 2; i++) {
                    Serial.print(recvBuf[i], HEX);
                    Serial.print(" ");
                }
                Serial.println("");
            }
        }
    */
}