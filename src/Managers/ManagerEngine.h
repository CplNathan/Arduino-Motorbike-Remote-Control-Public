#pragma once

#ifndef ManagerEngine_H
#define ManagerEngine_H

#include "ManagerBase.h"

class ManagerRelay;
class ManagerAlarm;

enum class EngineFail
{
    EngineSuccess,
    EngineAlarmPrimed,
    EngineLocked, // Not used
    EngineStarting,
    EngineAlreadyOn,
    EngineAlreadyOff,
    EngineUnknown
};

class ManagerEngine : public ManagerBase
{
public:
    virtual void Tick(unsigned long DeltaTime) override;

    ManagerEngine(ManagerRelay *RelayManager, ManagerAlarm *AlarmManager)
    {
        this->RelayManager = RelayManager;
        this->AlarmManager = AlarmManager;

        bReady = true;
    };

    EngineFail StartEngine();
    EngineFail StopEngine();

    inline bool IsEngineRunning()
    {
        return EngineRunning;
    }

protected:
    unsigned long previousMillisEngine;

    bool EngineRunning = false;

private:
    ManagerRelay *RelayManager;
    ManagerAlarm *AlarmManager;
};

#endif