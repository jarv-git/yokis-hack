#ifndef __SCANNER_H__
#define __SCANNER_H__

#include <Arduino.h>
#include "RF/configurator.h"

class Scanner : public RFConfigurator {
   private:
    const uint8_t *address;
    const uint8_t *channel;

    void printRecvData(const uint8_t *recvbuf, uint8_t len);

   public:
    // Constructor
    Scanner(uint16_t, uint16_t, const uint8_t *, const uint8_t *);
    void setupRFModule() override;
    bool scan();
};

#endif