#include "RF/e2bp.h"

E2bp::E2bp(uint16_t cepin, uint16_t cspin, const uint8_t* address, const uint8_t* channel) : RFConfigurator(cepin, cspin) {
    this->address = address;
    this->channel = channel;
}

bool E2bp::hack() {
    Serial.println("Hack !");
    this->begin();
    this->setupRFModule();
    return true;
}

void E2bp::setupRFModule() {
    byte packet[9] = {0x00, 0x00, 0x1c, 0x17, 0xcc, 0x20, 0x00, 0x04, 0x35};

    myRF->flush_tx();
    myRF->setChannel(*channel);
    myRF->setDataRate(RF24_250KBPS);
    myRF->setPALevel(RF24_PA_LOW);
    myRF->setAddressWidth(5);
    myRF->setPayloadSize(0x02);
    myRF->openWritingPipe(address);
    myRF->disableDynamicPayloads();
    myRF->setCRCLength(RF24_CRC_16);
    myRF->setAutoAck(false);
    myRF->stopListening();
    myRF->printDetails();
    
    myRF->startFastWrite(packet, 9, false, true);

    
    myRF->openReadingPipe(0, address);
    myRF->startListening();
    delay(5);

    byte buf[2];
    char printBuf[16];
    while(true) {
        if(myRF->available()) {
            myRF->read(buf, 2);
            Serial.print("Received: ");
            sprintf(printBuf, "%02x %02x", buf[0], buf[1]);
            Serial.println(printBuf);
            break;
        }
        delay(5);
    }
}