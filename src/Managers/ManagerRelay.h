#pragma once

#ifndef ManagerRelay_H
#define ManagerRelay_H

#include "ManagerBase.h"
#include "../../cfg/UserConfig.h"
#include "../../cfg/PinConfig.h"

enum class LockedOption
{
    Unlocked,
    LockedAlarm,
    LockedUser,
    FORCEUNLOCKSLEEP /* Used for sleeping */
};

struct BlinkBase
{
    bool Blink;
    bool CurrentlyHigh;
    unsigned long previousMillis;
    LockedOption Locked;

    void Tick(unsigned long DeltaTime)
    {
    }

    bool CanChangeStatus(LockedOption InLocked)
    {
        if (Locked == InLocked || Locked == LockedOption::Unlocked || InLocked == LockedOption::LockedAlarm || InLocked == LockedOption::FORCEUNLOCKSLEEP) /* Alarm can force override on all */
        {
            return true;
        }
        return false;
    }

    bool SetBlink(bool State, LockedOption InLocked)
    {
        if (CanChangeStatus(InLocked)) /* Alarm can force override on all */
        {
            Blink = State;
            Locked = State ? InLocked : LockedOption::Unlocked;
            return true;
        }

        return false;
    }

    BlinkBase()
    {
        Blink = false;
        CurrentlyHigh = false;
        Locked = LockedOption::Unlocked;
    }
};

struct Hazards : BlinkBase
{
    void Tick(unsigned long DeltaTime)
    {
        if (BlinkArmPattern)
        {
            if (DeltaTime - previousMillisState >= ALARMFREQUENCY)
            {
                previousMillisState = DeltaTime;
                switch (BlinkPatternState)
                {
                case 2:
                /* Fall Through */
                case 0:
                {
                    digitalWrite(HAZARDSA, LOW);
                    digitalWrite(HAZARDSB, LOW);
                    break;
                }
                case 3:
                /* Fall Through */
                case 1:
                {
                    digitalWrite(HAZARDSA, HIGH);
                    digitalWrite(HAZARDSB, HIGH);
                    break;
                }
                }
                if (BlinkPatternState >= 4)
                {
                    BlinkPatternState = 0;
                    BlinkArmPattern = false;
                    Locked = LockedOption::Unlocked;
                }
                else
                {
                    BlinkPatternState++;
                }
            }
        }
        else if (BlinkDissarmPattern)
        {
            if (DeltaTime - previousMillisState >= ALARMFREQUENCY * 2)
            {
                previousMillisState = DeltaTime;
                switch (BlinkPatternState)
                {
                case 0:
                {
                    digitalWrite(HAZARDSA, LOW);
                    digitalWrite(HAZARDSB, LOW);
                    break;
                }
                case 1:
                {
                    digitalWrite(HAZARDSA, HIGH);
                    digitalWrite(HAZARDSB, HIGH);
                    break;
                }
                }
                if (BlinkPatternState >= 2)
                {
                    BlinkPatternState = 0;
                    BlinkDissarmPattern = false;
                    Locked = LockedOption::Unlocked;
                }
                else
                {
                    BlinkPatternState++;
                }
            }
        }
        else if (Blink)
        {
            if (DeltaTime - previousMillis >= ALARMFREQUENCY)
            {
                previousMillis = DeltaTime;
                CurrentlyHigh = !CurrentlyHigh;
                digitalWrite(HAZARDSA, CurrentlyHigh);
                digitalWrite(HAZARDSB, CurrentlyHigh);
            }
        }
        else
        {
            CurrentlyHigh = false;
            digitalWrite(HAZARDSA, HIGH);
            digitalWrite(HAZARDSB, HIGH);
        }
    }

    bool ToggleAlarmPattern(bool State, LockedOption InLocked)
    {
        if (CanChangeStatus(InLocked)) /* Alarm can force override on all */
        {
            SetBlink(false, InLocked); // Turn off other blinker states if they are running.
            Locked = InLocked;

            if (State)
            {
                BlinkArmPattern = true;
            }
            else
            {
                BlinkDissarmPattern = true;
            }

            return true;
        }

        return false;
    }

    bool BlinkArmPattern = false;
    bool BlinkDissarmPattern = false;
    int BlinkPatternState = 0;
    unsigned long previousMillisState;
};

struct Horn : BlinkBase
{
    void Tick(unsigned long DeltaTime)
    {
        if (Blink)
        {
            if (DeltaTime - previousMillis >= ALARMFREQUENCY)
            {
                previousMillis = DeltaTime;
                CurrentlyHigh = !CurrentlyHigh;
                digitalWrite(HORN, !CurrentlyHigh); /* Inverted so that current draw at any one point is lower */
            }
        }
        else
        {
            CurrentlyHigh = false;
            digitalWrite(HORN, HIGH);
        }
    }
};

struct Engine
{
    void Tick(unsigned long DeltaTime)
    {
        if (Starting && !Started)
        {
            if (DeltaTime - previousMillisState >= 1000) /* Hard coded 2s for each state */
            {
                previousMillisState = DeltaTime;
                switch (EngineStartState)
                {
                case 0:
                {
                    digitalWrite(IGNITION, LOW);
                    EngineStartState++;
                    break;
                }
                case 1:
                {
                    digitalWrite(STARTER, LOW);
                    EngineStartState++;
                    break;
                }
                case 2:
                {
                    digitalWrite(STARTER, HIGH);
                    EngineStartState = 0;
                    Starting = false;
                    Started = true;
                    break;
                }
                }
            }
        }
        else if (!Starting && !Started)
        {
            digitalWrite(IGNITION, HIGH);
            digitalWrite(STARTER, HIGH);
        }
    }

    bool SetEngine(bool Enable)
    {
        if (Enable)
        {
            if (Started || Starting)
                return false;
            Starting = true;
            return true;
        }
        else
        {
            Starting = false;
            Started = false;
            return true;
        }
    }

    int EngineStartState;
    unsigned long previousMillisState;

    bool Started;
    bool Starting;
};

class ManagerRelay : public ManagerBase
{
public:
    virtual void Tick(unsigned long DeltaTime) override;

    bool SetHornBlinking(bool Enable, LockedOption LockedBy = LockedOption::Unlocked);
    bool SetHazardsBlinking(bool Enable, LockedOption LockedBy = LockedOption::Unlocked);
    bool ToggleHazardsBlinking(LockedOption LockedBy);

    inline bool IsHazardsRunning()
    {
        return HazardsState.Blink;
    }

    inline bool IsHornRunning()
    {
        return HornState.Blink;
    }

    inline bool IsEngineStarting()
    {
        return EngineState.Starting;
    }

    /* Only to be called from ManagerEngine */
    bool SetEngineState(bool Enable);

    bool SetAlarmStatusIndicator(bool Activating, LockedOption LockedBy);

private:
    Hazards HazardsState;
    Horn HornState;
    Engine EngineState;
};

#endif