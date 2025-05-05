#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h> // Required for Serial definition

// --- Logging Configuration for Device Enumeration ---

// Define the Serial port to use for DeviceEnumerator and Manager logs
// Examples: Serial, Serial1, SerialUSB (if native)
#define DEBUG_SERIAL_ENUM Serial

// Define the baud rate for the debug serial port
#define DEBUG_SERIAL_BAUD_ENUM 115200

// Set to true to enable detailed logging messages from DeviceEnumerator
// and minimal logs from DeviceEnumeratorManager.
// Set to false to disable these logs completely.
#define ENABLE_LOGGING_DEVICE_ENUMERATOR true // Set to true or false as needed

// --- Timeout Configuration ---
// Timeout in milliseconds (Not used in this non-blocking approach's setup phase)
// #define ENUMERATION_TIMEOUT_MS 15000


#endif // CONFIG_H