#pragma once

#ifndef ManagerBase_H
#define ManagerBase_H

#include <Arduino.h>

/* Do not include this file */

class ManagerBase
{
public:
    virtual void Tick(unsigned long DeltaTime){

    };
};

#endif