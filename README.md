# Arduino Motorbike Remote Control
A tool that allows you to remotely control a Motorbike via Bluetooth.

Includes remote start and anti-theft alarm functionality, is designed to meet my needs however has configuration options that allow you to tweak basic values.
### Target Audience
This project is aimed at someone who has a basic foundation in programming and electrical systems and is interested in implementing a similar system. It is probably best to use this repo to see how someone else has successfully implemented this system.

# Prerequisites
- Arduino (tested on Uno only)
- Relays, Sensors, Wires, etc
- Laptop (or something to program the Arduino with)
- Bluetooth module with a STATE pin

# Notices & Configuration
While this project has some configuration options it is specifically tailored towards my needs and is designed to work with the Arduino Uno so it is important that you know why this is and what limitations it poses.

## Notices
### Hardware Interrupt Pins
A good example of this would be that the Uno only has two dedicated hardware interrupt pins: D2 & D3 so when using three pins as alarm sensors and one to wake the device from sleep when the Bluetooth module detects a connection you quickly run out of dedicated hardware pins.

To circumvent this issue I have modified the registers to allow interrupts on a group of pins, in this case, D0 through D7 on PCINT2_vect.
```
ISR(PCINT2_vect) // handle pin change interrupt for D0 to D7 here
{
  AlarmManager.AlarmISR(); /* Checks pins to see if alarm is tripped etc and acts appropriately */
  sleep_disable();
}
```
This conflicts with the default SoftwareSerial library so I have modified it and commented out the duplicate definition, however this does mean that SoftwareSerial cannot be used for pins D0 to D7.
### Software Serial
Another issue with the Uno is that pins D0 and D1 are used for serial communication as well as being dedicated RX and TX pins, causing issues such as being unable to program the board and unable to view the serial monitor while the Bluetooth module is plugged in.

As a fix I have opted to use the Software Serial library (NewSoftSerial) as an alternative, however this isn't without its issues... with my particular setup I have issues with BAUD rates other than 115200, this may be an issue with my bootleg Bluetooth module or some other issue with the library or the Arduino. :shrug:

## Configuration
In the /cfg/UserConfig.h file you can find the following config options:
```
#define MAXENGINEONTIME 120000 // 2 minutes (ms) (time that the engine stays on for after remote start before deactivating)
#define MAXALARMONTIME 60000 // 1 minute (ms) (time that the alarm remains on for before turning off)
#define ALARMFREQUENCY 250 // 0.25 seconds (ms) (rate at which the alarm pulses)
#define SLEEPTIMEOUT 10000 // 10 seconds (ms) (time that it takes to enter sleep after a connection ends)
#define PASSWORD F("-your password-") // Master password that is required to run commands
```
Here it allows you to edit a few options, the only important one is the password definition. It is wrapped in an F() macro so that it is stored in flash memory.

The password must be sent with every command like so 'command,password' with return line enabled.

# Usage
## Commands
The currently supported commands are:
```
START - starts the engine
STOP - stops the engine
ALARM - triggers the alarm manually
TOGGLEALARM - primes and disarms the alarm - if it is triggered it stops and disarms
STATUS - prints the status to Bluetooth
HAZARDS - toggles hazard blinking
```
## Bluetooth Apps
There are many ways of sending commands to the device I recommend an app called Serial Bluetooth Terminal on Android. It has many features including storing commands, support for bluetooth low energy, and support for a wide range of configurations, etc.

I have also designed a custom stylized app for Tizen to interface with the Arduino.

# Contributing
Contributions are welcome, for more information including areas that I have identified that could be improved please see the CONTRIBURING.md file.
## Donations
If you find this project useful it would be great if you could support me by sending a couple quid my way. Thanks :D

# Important
Please make sure to do your research, follow local guidelines, take appropriate precautions, and audit this code yourself as there may be edge case scenarios that I haven't accounted for.

The starter functions have NO FEEDBACK, in my case this isn't important because I have it set up to use the starter switch and this has appropriate fail-safes in place such as: side stand detection, gear & clutch detection so it won't start if it is unsafe to do so.

Also never wire a relay directly to the starter, the starter motor can draw hundreds of amps and will melt anything not rated for the correct load... instead tap into the starter switch ensuring that it has appropriate fail-safes.