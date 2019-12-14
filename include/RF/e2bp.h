#ifndef __E2BP_H__
#define __E2BP_H__

#include <Arduino.h>
#include "RF/configurator.h"

class E2bp : public RFConfigurator {
   private:
    const uint8_t *address;
    const uint8_t *channel;

   public:
    // Constructor
    E2bp(uint16_t, uint16_t, const uint8_t *, const uint8_t *);
    void setupRFModule();
    bool hack();
};

#endif  // __E2BP_H__