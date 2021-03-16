#include "ManagerAlarm.h"
#include "ManagerRelay.h"
#include "../Helpers.h"

void ManagerAlarm::Tick(unsigned long DeltaTime)
{
    if (AlarmRunning)
    {
        if (DeltaTime - previousMillisAlarm >= MAXALARMONTIME)
        {
            StopAlarm();
        }
    }
}

void ManagerAlarm::AlarmISR()
{
    if (AlarmRunning || !AlarmPrimed)
        return;

    bool Tripped = false;
    for (int i = 0; i < AlarmPinsLen; i++)
    {
        if (digitalRead(AlarmPins[i]))
        {
            Tripped = true;
            break;
        }
    }

    if (Tripped)
    {
        TrippedCounter++;
        StartAlarm();
    }
}

void ManagerAlarm::SetInterrupts(bool State)
{
    if (State)
    {
        digitalWrite(ALARM_SENSORPOWER, HIGH);
        delay(500);
        for (int i = 0; i < AlarmPinsLen; i++)
        {
            Helpers::setpciEnable(AlarmPins[i], true);
        }
    }
    else
    {
        for (int i = 0; i < AlarmPinsLen; i++)
        {
            Helpers::setpciEnable(AlarmPins[i], false);
        }
        digitalWrite(ALARM_SENSORPOWER, LOW);
    }
}

AlarmFail ManagerAlarm::StartAlarm()
{
    if (AlarmRunning)
        return AlarmFail::AlarmFailed;

    SetInterrupts(false);

    unsigned long currentMillis = (unsigned long)millis();

    previousMillisAlarm = currentMillis;

    AlarmRunning = true;
    RelayManager->SetHazardsBlinking(true, LockedOption::LockedAlarm);
    RelayManager->SetHornBlinking(true, LockedOption::LockedAlarm);

    return AlarmFail::AlarmArmed;
}

AlarmFail ManagerAlarm::StopAlarm()
{
    AlarmRunning = false;
    RelayManager->SetHazardsBlinking(false, LockedOption::LockedAlarm);
    RelayManager->SetHornBlinking(false, LockedOption::LockedAlarm);
    SetInterrupts(AlarmPrimed);

    return AlarmFail::AlarmDisarmed;
}

AlarmFail ManagerAlarm::ToggleAlarmPrimed()
{
    if (AlarmPrimed || AlarmRunning)
    {
        AlarmPrimed = false;
        SetInterrupts(false);
        AlarmFail Response = StopAlarm();
        RelayManager->SetAlarmStatusIndicator(false, LockedOption::LockedAlarm);
        return Response;
    }
    else
    {
        SetInterrupts(true);
        AlarmPrimed = true;
        RelayManager->SetAlarmStatusIndicator(true, LockedOption::LockedAlarm);
        return AlarmFail::AlarmArmed;
    }
}