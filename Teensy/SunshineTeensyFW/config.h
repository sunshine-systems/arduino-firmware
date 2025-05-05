#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h> // Required for Serial definition

// --- Logging Configuration for Device Enumeration ---

// Define the Serial port to use for DeviceEnumerator and Manager logs
// Examples: Serial, Serial1, SerialUSB (if native)
#define DEBUG_SERIAL_PORT Serial1

// Define the baud rate for the debug serial port
#define DEBUG_BAUD_RATE 115200


#endif // CONFIG_H