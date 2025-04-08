#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <usbhub.h>
#include "CustomHIDComposite.h"

/* Sets the timeframe that will be used to exclude side thumb buttons from being passed to the PC when MMB is clicked */
const unsigned long BUTTON_EXCLUSION_DURATION_MS = 1300; // Exclusion Timeframe within button activation (1200 = 1.3sec)

/* Define the version */
#define FIRMWARE_VERSION 1.0

/* Settings for logging via Serial1 since CDC (onboard serial is disabled) */
#define DEBUG_MODE false
#define FTDI_DEVICE Serial1
#define BAUD 115200

/* Declare the USB and HID composite objects as extern */
extern USB Usb;
extern CustomHIDComposite Hid;

/* Extern other settings and interceptors */
extern bool logPerformanceMetrics;
extern bool logAPerformanceMetric;
extern bool enableLockout;
extern int mouseLockoutDurationMilliseconds;
extern int enableSpinning;
extern int spinAmountPerRotation;
extern int spinNumberOfRotations;
extern int spinDelayBetweenRotationsMilliseconds;
extern bool spinLockoutMouseUntilCompletion;
extern int spinBeforeAfterMouseEvent;

#endif // CONFIG_H
