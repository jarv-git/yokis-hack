#include "RF/scanner.h"

Scanner::Scanner(uint16_t cepin, uint16_t cspin, const uint8_t* address, const uint8_t* channel) : RFConfigurator(cepin, cspin) {
    this->address = address;
    this->channel = channel;
}

void Scanner::setupRFModule() {

}

bool Scanner::scan() {
    uint8_t recvBuf[9];
    uint8_t len;
    Serial.println("Scanning packet for button pressed / release");
    Serial.println("Press any key to exit");
    myRF->begin();
    delay(5);

    myRF->setAutoAck(false);
    myRF->disableCRC();
    myRF->setChannel(*channel);
    myRF->setDataRate(RF24_250KBPS);
    myRF->setPALevel(RF24_PA_LOW);
    myRF->setAddressWidth(5);
    //myRF->setPayloadSize(9);x
    myRF->openReadingPipe(0, address);

    myRF->startListening();
    while(true){
        if(Serial.available()) {
            Serial.println("Exiting.");
            break; // exit scanner
        } else if(myRF->available()) {
            // Scan and print received PL
            len = 9;
            myRF->read(recvBuf, len);
            if(recvBuf[0] == 0x35 || recvBuf[0] == 0x53) {
                // command payload
            }
            printRecvData(recvBuf, len);
        }

        delay(1);
    }

    // Empty serial input buffer if needed
    while (Serial.available()) {
        Serial.read();
    }
    myRF->closeReadingPipe(0);

    return true;
}

void Scanner::printRecvData(const uint8_t* recvBuf, uint8_t len) {
    char buf[32];
    Serial.print("Data received: ");
    for(uint8_t i=0; i<len; i++) {
        sprintf(buf, "%02x ", recvBuf[i]);
        Serial.print(buf);
    }
    Serial.println();
}