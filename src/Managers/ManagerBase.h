#pragma once

#ifndef ManagerBase_H
#define ManagerBase_H

/* Do not include this file */
#include <Arduino.h>

class ManagerBase
{
public:
    virtual void Tick(unsigned long DeltaTime){

    };

    virtual bool GetReadyState() { return bReady; };

protected:
    bool bReady = false;
};

#endif