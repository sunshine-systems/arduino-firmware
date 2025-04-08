#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Structure to define a device
struct DeviceInfo {
    const char* name;
    const char* pid;
    uint8_t reportSize;
};

// List of supported devices
const DeviceInfo supportedDevices[] = {
    {"Razer Viper V2 Pro", "00A6", 8},
    {"Logitech Pro X Superlight", "C547", 13}
    // Add more devices here
};

extern DeviceInfo* connectedDevice;
extern bool hasReadConnectedDevice; 

const int numSupportedDevices = sizeof(supportedDevices) / sizeof(DeviceInfo);

/* Settings for logging via Serial1 since CDC (onboard serial is disabled) */
#define DEBUG_MODE false // Set to 'false' when you don't want to print debug info.
#define FT232RL Serial1
#define BAUD 115200

/* Sets the timeframe that will be used to exclude side thumb buttons from being passed to the PC when MMB is clicked */
const unsigned long BUTTON_EXCLUSION_DURATION_MS = 1300; // Exclusion Timeframe within button activation (1200 = 1.3sec)

// Additional configurable settings over serial
extern bool enableLockout;
extern int lockoutDuration;
extern bool enableSpinOnLMB;
extern int spinOnLMBRotations;
extern int spinOnLMBAmountToSpin;

#endif
