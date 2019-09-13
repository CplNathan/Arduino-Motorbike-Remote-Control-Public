#pragma once

#ifndef PinConfig_H
#define PinConfig_H

/*
   Static Defines
*/
#define GLOBALBAUDRATE 115200 // Leave as is, this is reduced when cpu clock speed is reduced. at PS/16 = 1 = ~2400 BAUD

/*
   Pin Declerations
*/
#define STARTER 6
#define IGNITION 7
#define HORN 10
#define SCREAMER 11 // Not used
#define HAZARDSA 12
#define HAZARDSB 13

#define BLUETOOTH_RX 8
#define BLUETOOTH_TX 9

#define BLUETOOTH_STATE 2

#define ALARM_TRIPA 3
#define ALARM_TRIPB 4
#define ALARM_TRIPC 5
#define ALARM_SENSORPOWER A0 // Because the sensors draw a small current when idle to save battery life we turn them off when the alarm is not primed, this pin is high when primed and low when not primed.

/* 
* Pins that can trigger the alarm, must be in the range of D0 to D7 (UNO only, unknown for other boards)
* This can be any inputs asuming they are setup properly in main.
*/
static const byte AlarmPins[3] =
{
   ALARM_TRIPA,
   ALARM_TRIPB,
   ALARM_TRIPC
};
static const int AlarmPinsLen = sizeof(AlarmPins);

/*
* Unused analog pins that can be powered down to save powered
*/
static const int lowPins[5] = {A1, A2, A3, A4, A5};
static const int lowPinsLen = sizeof(lowPins) / sizeof(lowPins[0]);

#endif