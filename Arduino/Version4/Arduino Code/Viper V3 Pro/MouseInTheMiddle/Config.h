#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

/* Define the version */
#define FIRMWARE_VERSION 1.0

/* Settings for logging via Serial1 since CDC (onboard serial is disabled) */
#define DEBUG_MODE false // Set to 'false' when you don't want to print debug info.
#define FTDI_DEVICE Serial1
#define BAUD 115200

/* Sets the timeframe that will be used to exclude side thumb buttons from being passed to the PC when MMB is clicked */
const unsigned long BUTTON_EXCLUSION_DURATION_MS = 1300; // Exclusion Timeframe within button activation (1200 = 1.3sec)

// Setting that gets toggled during serial read for preformance metrics analysis
extern bool logPerformanceMetrics; // Configurable setting to enable or disable logging on serial events
extern bool logAPerformanceMetric; // Enables itself when serial events are available

/* BELOW ARE OPERATION SETTINGS LIKE MOUSE LOCKOUT AND HARDWARE SPINBOT */
extern bool enableLockout;
extern int mouseLockoutDurationMilliseconds;

extern int enableSpinning; // 0=Disabled, 1=Enabled
extern int spinAmountPerRotation;
extern int spinNumberOfRotations;
extern int spinDelayBetweenRotationsMilliseconds;
extern bool spinLockoutMouseUntilCompletion;
extern int spinBeforeAfterMouseEvent; // 0=Before, 1=After, 2=Both


#endif
