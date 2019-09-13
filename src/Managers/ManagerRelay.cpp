#include "ManagerRelay.h"

void ManagerRelay::Tick(unsigned long DeltaTime)
{
    EngineState.Tick(DeltaTime);
    HazardsState.Tick(DeltaTime);
    HornState.Tick(DeltaTime);
}

bool ManagerRelay::SetHornBlinking(bool Enable, LockedOption LockedBy)
{
    return HornState.SetBlink(Enable, LockedBy);
}

bool ManagerRelay::SetHazardsBlinking(bool Enable, LockedOption LockedBy)
{
    return HazardsState.SetBlink(Enable, LockedBy);
}

bool ManagerRelay::ToggleHazardsBlinking(LockedOption LockedBy)
{
    return SetHazardsBlinking(!HazardsState.Blink, LockedBy);
}

bool ManagerRelay::SetEngineState(bool Enable)
{
    return EngineState.SetEngine(Enable);
}

bool ManagerRelay::SetAlarmStatusIndicator(bool Activating, LockedOption LockedBy)
{
    return HazardsState.PlaySequenceSM(Activating, LockedBy);
}