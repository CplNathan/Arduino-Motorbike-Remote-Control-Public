#include "ManagerEngine.h"
#include "ManagerRelay.h"
#include "ManagerAlarm.h"

void ManagerEngine::Tick(unsigned long DeltaTime)
{
    if (EngineRunning)
    {
        if (DeltaTime - previousMillisEngine >= MAXENGINEONTIME)
        {
            StopEngine();
        }
    }
}

EngineFail ManagerEngine::StartEngine()
{
    if (EngineRunning)
    {
        return EngineFail::EngineAlreadyOn;
    }
    if (AlarmManager->IsAlarmPrimed() || AlarmManager->IsAlarmRunning())
    {
        return EngineFail::EngineAlarmPrimed;
    }
    if (RelayManager->IsEngineStarting())
    {
        return EngineFail::EngineStarting;
    }

    if (RelayManager->SetEngineState(true))
    {
        EngineRunning = true;
        previousMillisEngine = (unsigned long)millis();
        return EngineFail::EngineSuccess;
    }

    return EngineFail::EngineUnknown;
}

EngineFail ManagerEngine::StopEngine()
{
    /* Again, for 'safety' always execute this incase of some edge case scenario */
    RelayManager->SetEngineState(false);
    EngineFail ReturnValue = EngineRunning ? EngineFail::EngineSuccess : EngineFail::EngineAlreadyOff; // Already on should only mean that the engine was supposed to be on but because there is no feedback we don't know if it is, it could have stalled or failed. Issue command but warn user anyway.
    EngineRunning = false;
    return ReturnValue;
}