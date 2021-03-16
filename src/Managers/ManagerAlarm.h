#pragma once

#ifndef ManagerAlarm_H
#define ManagerAlarm_H

#include "ManagerBase.h"

class ManagerRelay;

enum class AlarmFail
{
    AlarmArmed,
    AlarmDisarmed,
    AlarmFailed
};

class ManagerAlarm : public ManagerBase
{
public:
    virtual void Tick(unsigned long DeltaTime) override;

    ManagerAlarm(ManagerRelay *RelayManager)
    {
        this->RelayManager = RelayManager;
        bReady = true;
    };

    void AlarmISR();

    AlarmFail ToggleAlarmPrimed();
    AlarmFail StartAlarm();
    AlarmFail StopAlarm();

    inline bool IsAlarmPrimed()
    {
        return AlarmPrimed;
    }

    inline bool IsAlarmRunning()
    {
        return AlarmRunning;
    }

    inline int GetTrippedCounter()
    {
        return OldTrippedCounter;
    }

    inline void ResetTrippedCounter()
    {
        OldTrippedCounter = TrippedCounter;
        TrippedCounter = 0;
    }

    inline void LastConnectionNow(unsigned long DeltaTime)
    {
        previousMillisLastConnection = DeltaTime;
    }

    inline unsigned long GetLastConnection()
    {
        return previousMillisLastConnection;
    }

protected:
    volatile bool AlarmRunning = false;
    volatile bool AlarmPrimed = false;

    volatile int OldTrippedCounter;
    volatile int TrippedCounter;

    volatile unsigned long previousMillisAlarm;
    volatile unsigned long previousMillisLastConnection;

    void SetInterrupts(bool State);

private:
    ManagerRelay *RelayManager;
};

#endif