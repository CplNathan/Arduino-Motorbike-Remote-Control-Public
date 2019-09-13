#ifndef Helpers_H
#define Helpers_H

#include "PCH.h"
#include <inttypes.h>

class Helpers
{
public:
    static inline void setpciEnable(byte pin, bool EnablePCI)
    {
        if (EnablePCI)
        {
            pciEnablePin(pin);
        }
        else
        {
            pciDisablePin(pin);
        }
    };

    static inline void pciEnablePin(byte pin)
    {
        *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
        PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
        PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
    };

    static inline void pciDisablePin(byte pin)
    {
        PCICR  &= ~bit (digitalPinToPCICRbit(pin));
    };
};

#endif
