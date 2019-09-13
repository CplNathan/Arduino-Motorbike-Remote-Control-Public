#pragma once

#ifndef UserConfig_H
#define UserConfig_H

/*
   Configuration
*/
#define MAXENGINEONTIME 120000 // 2 minutes (ms) (time that the engine stays on for after remote start before deactivating)
#define MAXALARMONTIME 60000 // 1 minute (ms) (time that the alarm remains on for before turning off)
#define ALARMFREQUENCY 250 // 0.25 seconds (ms) (rate at which the alarm pulses)
#define SLEEPTIMEOUT 10000 // 10 seconds (ms) (time that it takes to enter sleep after a connection ends)
#define PASSWORD F("-password-") // Master password that is required to run commands

#endif