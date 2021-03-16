#include <Arduino.h>
#include <AltSoftSerial.h>

#include "src/Helpers.h"
/* Include Managers */
#include "src/Managers/ManagerRelay.h"
#include "src/Managers/ManagerEngine.h"
#include "src/Managers/ManagerAlarm.h"
#include "src/Managers/ManagerCommunication.h"

#include "cfg/UserConfig.h"
#include "cfg/PinConfig.h"

#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/interrupt.h>

/*
Board	Transmit Pin	Receive Pin	Unusable PWM
Teensy 3.5 / 3.6	21	20	22
Teensy 3.0 / 3.1 / 3.2	21	20	22
Teensy 2.0	9	10	(none)
Teensy++ 2.0	25	4	26, 27
Arduino Uno, Duemilanove,
LilyPad, Mini (& other ATMEGA328)	9	8	10
Arduino Leonardo, Yun, Micro	5	13	(none)
Arduino Mega	46	48	44, 45
Wiring-S	5	6	4
Sanguino	13	14	12
*/

AltSoftSerial Bluetooth;

ManagerRelay RelayManager = ManagerRelay();
ManagerAlarm AlarmManager = ManagerAlarm(&RelayManager);
ManagerEngine EngineManager = ManagerEngine(&RelayManager, &AlarmManager);
ManagerCommunication CommunicationManager = ManagerCommunication(&Bluetooth);

void setup()
{
    /*
      Clock Setups
  */
    if (F_CPU == 8000000)
        clock_prescale_set(clock_div_2);
    if (F_CPU == 4000000)
        clock_prescale_set(clock_div_4);
    if (F_CPU == 2000000)
        clock_prescale_set(clock_div_8);
    if (F_CPU == 1000000)
        clock_prescale_set(clock_div_16);

    /*
      Pin Setups
  */
    pinMode(STARTER, OUTPUT);
    pinMode(IGNITION, OUTPUT);
    pinMode(HORN, OUTPUT);
    pinMode(HAZARDSA, OUTPUT);
    pinMode(HAZARDSB, OUTPUT);

    pinMode(ALARM_TRIPA, INPUT_PULLUP); // PCI enabled inside the AlarmManager
    pinMode(ALARM_TRIPB, INPUT_PULLUP); // PCI enabled inside the AlarmManager
    pinMode(ALARM_TRIPC, INPUT_PULLUP); // PCI enabled inside the AlarmManager
    pinMode(ALARM_SENSORPOWER, OUTPUT);

    pinMode(BLUETOOTH_STATE, INPUT_PULLUP);
    Helpers::pciEnablePin(BLUETOOTH_STATE);

    digitalWrite(STARTER, HIGH);
    digitalWrite(IGNITION, HIGH);
    digitalWrite(HORN, HIGH);
    digitalWrite(HAZARDSA, HIGH);
    digitalWrite(HAZARDSB, HIGH);

    Serial.begin(GLOBALBAUDRATE);
    while (!Serial)
    {
        ; // wait for serial port to connect. Needed for native USB debugging only
    }

    Bluetooth.begin(GLOBALBAUDRATE);

    /*
     Power Savings
    */
    for (int i = 0; i < lowPinsLen; i++)
    {
        pinMode(lowPins[i], OUTPUT);
        digitalWrite(lowPins[i], LOW);
    }

    power_spi_disable();
    power_adc_disable();
    power_timer1_disable();
    power_timer2_disable();
    power_twi_disable();
}

ISR(PCINT2_vect) // handle pin change interrupt for D0 to D7 here
{
    AlarmManager.AlarmISR();
    sleep_disable();
}

#pragma region MainLoop
void loop()
{
    unsigned long currentMillis = (unsigned long)millis();

    /*
      Tick Order
      The tick order should be preserved as the engine manager relies on the alarm manager letting it know if its primed etc so it must first be ticked to know this etc...
      once all states have been determined it is then time for the relay manager to tick and take action on the current states.
  */
    CommunicationManager.Tick(currentMillis);
    Task CurrentTask = CommunicationManager.GetCommand();

    if (!CurrentTask.bExecuted)
    {
        // I was actually going to do something special with bitmasks but instead I have this. I suppose it is useful if I ever add more in the future.
        bool bOn = (CurrentTask.Command & Command::STATE_On) == Command::STATE_On;
        bool bReset = (CurrentTask.Command & Command::STATE_Reset) == Command::STATE_Reset;
        bool bToggle = (CurrentTask.Command & Command::STATE_Toggle) == Command::STATE_Toggle;
        bool bSpecial = (CurrentTask.Command & Command::STATE_Special) == Command::STATE_Special;

        // Engine
        if ((CurrentTask.Command & Command::CMD_Engine) == Command::CMD_Engine)
        {
            EngineFail Response = EngineFail::EngineSuccess;

            if (bOn && !bReset) // Failsafe
            {
                Response = EngineManager.StartEngine();
            }
            else
            {
                Response = EngineManager.StopEngine();
            }
        }

        // Alarm
        if ((CurrentTask.Command & Command::CMD_Alarm) == Command::CMD_Alarm)
        {

            if (bOn || bReset || bToggle)
            {
                AlarmFail Response = AlarmManager.ToggleAlarmPrimed();
            }
            else if (bSpecial)
            {
                AlarmFail Response = AlarmManager.StartAlarm();
            }
        }

        // Hazards
        if ((CurrentTask.Command & Command::CMD_Hazards) == Command::CMD_Hazards)
        {
            if (bOn || bReset || bToggle)
            {
                RelayManager.ToggleHazardsBlinking(LockedOption::LockedUser);
            }
        }
    }

    AlarmManager.Tick(currentMillis);
    EngineManager.Tick(currentMillis);
    RelayManager.Tick(currentMillis);

    if (CanSleep(currentMillis))
    {
        EnterSleep(currentMillis);
        AlarmManager.LastConnectionNow(currentMillis);

        /* Wakes up here */
        yield();
    }
}

bool CanSleep(unsigned long DeltaTime)
{
    if (digitalRead(BLUETOOTH_STATE))
    {
        AlarmManager.LastConnectionNow(DeltaTime);
        return false;
    }
    if (AlarmManager.IsAlarmRunning())
        return false;
    if (EngineManager.IsEngineRunning())
        return false;
    if (RelayManager.IsEngineStarting())
        return false;
    if (RelayManager.IsHazardsRunning())
        return false;

    return DeltaTime - AlarmManager.GetLastConnection() >= SLEEPTIMEOUT;
}

void EnterSleep(unsigned long DeltaTime)
{
    if (!CanSleep(DeltaTime))
        return;

    noInterrupts();

    EngineManager.StopEngine();
    AlarmManager.StopAlarm();
    RelayManager.SetHornBlinking(false, LockedOption::FORCEUNLOCKSLEEP);
    RelayManager.SetHazardsBlinking(false, LockedOption::FORCEUNLOCKSLEEP);

    Serial.flush();
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    sleep_enable();

    interrupts();

    delay(1000);
    sleep_cpu();

    /* Reset information about how many times it has been tripped if a connection has been detected, then save the old value to be printed */
    if (digitalRead(BLUETOOTH_STATE))
        AlarmManager.ResetTrippedCounter();
}