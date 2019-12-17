#include <Arduino.h>
#include "RF/e2bp.h"
#include "RF/pairing.h"
#include "RF/scanner.h"
#include "globals.h"
#include "printf.h"
#include "serial/genericCallback.h"
#include "serial/serialHelper.h"

#define CE_PIN 7
#define CSN_PIN 8

// singletons
SerialHelper* mySerial;
RF* pairingRF;
E2bp* bp;
Scanner* scanner;
const byte address[5] = {0xcc, 0x17, 0xcc, 0x17, 0x17};
const uint8_t channel = 0x29;

bool pairingCallback();
bool e2bpCallback();
bool scannerCallback();

void setup() {
    pairingRF = new RF(CE_PIN, CSN_PIN);
    bp = new E2bp(CE_PIN, CSN_PIN, address, &channel);
    scanner = new Scanner(CE_PIN, CSN_PIN, address, &channel);

    mySerial = new SerialHelper();
    
    mySerial->registerCallback(
        new GenericCallback('P',
                            "(P)air with a Yokis device - basically act as if "
                            "a Yokis remote try is pairing",
                            pairingCallback));
    mySerial->registerCallback(
        new GenericCallback('T',
                            "send a (T)oggle message - basically act as a "
                            "Yokis remote when button pressed",
                            e2bpCallback));
    mySerial->registerCallback(new GenericCallback(
        'S', "Scanner for button pressed/released on a Yokis remote (debug)",
        scannerCallback));

    printf_begin();
    mySerial->usage();
}

void loop() {
    mySerial->readFromSerial();
    delay(100);
}

bool pairingCallback() { return pairingRF->hackPairing(); }
bool e2bpCallback() { return bp->hack(); }
bool scannerCallback() { return scanner->scan(); }