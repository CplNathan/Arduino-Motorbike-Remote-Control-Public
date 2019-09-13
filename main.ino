/* Include PCH (contians most commonly used headers) */
#include "src/PCH.h"
/* Include Managers */
#include "src/Managers/ManagerRelay.h"
#include "src/Managers/ManagerEngine.h"
#include "src/Managers/ManagerAlarm.h"
/* Include Libraries */
#include "src/SoftwareSerial/SoftwareSerial.h" /* custom */

SoftwareSerial BT(BLUETOOTH_RX, BLUETOOTH_TX);

ManagerRelay RelayManager = ManagerRelay();
ManagerAlarm AlarmManager = ManagerAlarm(&RelayManager);
ManagerEngine EngineManager = ManagerEngine(&RelayManager, &AlarmManager);

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

  pinMode(ALARM_TRIPA, INPUT_PULLUP);
  pinMode(ALARM_TRIPB, INPUT_PULLUP);
  pinMode(ALARM_TRIPC, INPUT_PULLUP);
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

  BT.begin(GLOBALBAUDRATE);
  Serial.println(F("Controller Ready"));

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
  //power_usart0_disable();
  power_timer1_disable();
  power_timer2_disable();
  power_twi_disable();
}

ISR(PCINT2_vect) // handle pin change interrupt for D0 to D7 here
{
  AlarmManager.AlarmISR(); /* Checks pins to see if alarm is tripped etc and acts appropriately */
  sleep_disable();
}

#pragma region MainLoop
char incomingByte;

bool expectingPassword;
String incomingPassword, incomingCommand;

void loop()
{
  /*
        Read Commands
    */
  expectingPassword = false;
  while (BT.available() > 0)
  {
    incomingByte = BT.read();
    if (incomingByte == '\n' || incomingByte == -1)
      break;

    if ((char)incomingByte == ',')
    {
      expectingPassword = true;
    }
    else if (expectingPassword)
    {
      incomingPassword += (char)incomingByte;
    }
    else
    {
      incomingCommand += (char)incomingByte;
    }
  }

  /*
        Process Commands
  */
  if (incomingCommand.length() > 0)
  {
    if (incomingPassword != PASSWORD)
    {
      BT.println(F("Unauthorized Command"));
      incomingCommand = "";
      incomingPassword = "";
      return;
    }

    bool isStart = incomingCommand.startsWith("START");
    bool isStop = incomingCommand.startsWith("STOP");

    if (isStart || isStop)
    {
      EngineFail Response = EngineFail::EngineSuccess;

      if (isStart)
      {
        Response = EngineManager.StartEngine();
      }
      if (isStop)
      {
        Response = EngineManager.StopEngine();
      }

      if (Response == EngineFail::EngineSuccess)
      {
        BT.println(isStart ? F("Starting engine") : F("Stopping engine"));
      }
      else if (Response == EngineFail::EngineAlarmPrimed)
      {
        BT.println(F("Please disarm the alarm first."));
      }
      else if (Response == EngineFail::EngineAlreadyOff)
      {
        BT.println(F("Engine already stopped, the command has been sent anyway."));
      }
      else if (Response == EngineFail::EngineAlreadyOn)
      {
        BT.println(F("Engine already running."));
      }
      else if (Response == EngineFail::EngineStarting)
      {
        BT.println(F("Engine is currently starting."));
      }
      else if (Response == EngineFail::EngineUnknown)
      {
        BT.println(F("Unknown failure when starting"));
      }
    }

    if (incomingCommand.startsWith("ALARM"))
    {
      AlarmFail Response = AlarmManager.StartAlarm();
      if (Response == AlarmFail::AlarmArmed)
      {
        BT.println(F("Alarm armed"));
      }
      else if (Response == AlarmFail::AlarmFailed)
      {
        BT.println(F("Alarm already running"));
      }
    }

    if (incomingCommand.startsWith("TOGGLEALARM"))
    {
      AlarmFail Response = AlarmManager.ToggleAlarmPrimed();
      if (Response == AlarmFail::AlarmArmed)
      {
        BT.println(F("Alarm armed"));
      }
      else if (Response == AlarmFail::AlarmDisarmed)
      {
        BT.println(F("Alarm disarmed"));
      }
      else if (Response == AlarmFail::AlarmFailed)
      {
        BT.println(F("Alarm failed to toggle"));
      }
    }

    if (incomingCommand.startsWith("STATUS"))
    {
      String s1 = "Alarm Primed: " + String(AlarmManager.IsAlarmPrimed() ? "Yes" : "No");
      String s2 = "\nAlarm Sounding: " + String(AlarmManager.IsAlarmRunning() ? "Yes" : "No");
      String s3 = "\nEngine Running: " + String(EngineManager.IsEngineRunning() ? "Yes" : "No");
      String s4 = "\nHazards Running: " + String(RelayManager.IsHazardsRunning() ? "Yes" : "No");
      String s5 = "\nAlarm tripped " + String(AlarmManager.GetTrippedCounter()) + " time(s) since last connection";
      String sfinal = s1 + s2 + s3 + s4 + s5;

      BT.println(F("-Printing Status-"));
      BT.println(sfinal);
      BT.println(F("-End Printing Status-"));
    }

    if (incomingCommand.startsWith("HAZARDS"))
    {
      if (RelayManager.ToggleHazardsBlinking(LockedOption::LockedUser))
      {
        BT.println(F("Toggling Hazards"));
      }
      else
      {
        BT.println(F("Hazards are locked by another task"));
      }
    }

    incomingCommand = "";
    incomingPassword = "";
  }

  unsigned long currentMillis = (unsigned long)millis();

  /*
      Tick Order
      The tick order should be preserved as the engine manager relies on the alarm manager letting it know if its primed etc so it must first be ticked to know this etc...
      once all states have been determined it is then time for the relay manager to tick and take action on the current states.
  */
  AlarmManager.Tick(currentMillis);
  EngineManager.Tick(currentMillis);

  RelayManager.Tick(currentMillis); /* Should be last */

  if (CanSleep(currentMillis, true))
  {
    /* Wakes up here */
    yield();
  }
}

bool CanSleep(unsigned long DeltaTime, bool EnterSleepIfTrue)
{
  if (digitalRead(BLUETOOTH_STATE)) {
    AlarmManager.LastConnectionNow(DeltaTime);
    return false;
  }
  if (AlarmManager.IsAlarmRunning())
    return false;
  if (EngineManager.IsEngineRunning())
    return false;
  if (RelayManager.IsEngineStarting())
    return false;

  if (DeltaTime - AlarmManager.GetLastConnection() >= SLEEPTIMEOUT)
  {
    if (EnterSleepIfTrue)
    {
      EnterSleep(DeltaTime);
      AlarmManager.LastConnectionNow(DeltaTime);
    }
    return true;
  }
  else
  {
    return false;
  }
}

void EnterSleep(unsigned long DeltaTime)
{
  if (!CanSleep(DeltaTime, false))
    return;

  noInterrupts();

  Serial.println(F("Entering Sleep"));

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

  Serial.println(F("Exiting Sleep"));

  /* Reset information about how many times it has been tripped if a connection has been detected, then save the old value to be printed */
  if (digitalRead(BLUETOOTH_STATE))
    AlarmManager.ResetTrippedCounter();
}