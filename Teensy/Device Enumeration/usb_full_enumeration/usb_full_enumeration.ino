// ==========================================================================
// Teensy 4.1 - Full Enumeration Test (v30a - Hybrid - Fixed Manager Calls)
// ==========================================================================

#include <Arduino.h>
#include <USBHost_t36.h>
#include "config.h"
#include "DeviceEnumerator.h"
#include "DeviceEnumeratorManager.h" // Include the manager class

// --- Global Manager Instance ---
// Owns USBHost, DeviceEnumerator, Hub, and enumeration state flags.
DeviceEnumeratorManager deviceManager;

// --- Global flag to track if we should halt ---
bool haltSystem = false;

// --- Setup Function ---
void setup() {
    // 1. Initialize Debug Serial Port
    DEBUG_SERIAL_ENUM.begin(DEBUG_SERIAL_BAUD_ENUM);
    #if defined(USB_SERIAL) || defined(SERIAL_USB)
        unsigned long startWait = millis();
        while (!DEBUG_SERIAL_ENUM && (millis() - startWait < 4000)) { yield(); }
        delay(200);
    #else
        delay(200);
    #endif

    DEBUG_SERIAL_ENUM.println("\n----------------------------------------------------------");
    DEBUG_SERIAL_ENUM.println("Teensy 4.1 - Enumeration Test (v30a - Hybrid Approach Fixed)");
    DEBUG_SERIAL_ENUM.println("----------------------------------------------------------");
    DEBUG_SERIAL_ENUM.printf("Debug output: Configured Port @ %lu baud\n", (unsigned long)DEBUG_SERIAL_BAUD_ENUM);
    DEBUG_SERIAL_ENUM.printf("DeviceEnumerator Logging: %s\n", ENABLE_LOGGING_DEVICE_ENUMERATOR ? "Enabled" : "Disabled");
    DEBUG_SERIAL_ENUM.println("----------------------------------------------------------");

    // 2. Initialize the Manager (which initializes USBHost and resets state)
    DEBUG_SERIAL_ENUM.println("setup(): Initializing DeviceEnumeratorManager...");
    deviceManager.begin();

    DEBUG_SERIAL_ENUM.println("setup(): Setup complete. Monitoring device in loop()...");
    DEBUG_SERIAL_ENUM.println("----------------------------------------------------------");
}

// --- Loop Function ---
void loop() {

    // Check if we need to halt execution due to previous error
    if (haltSystem) {
        delay(1000); // Keep delaying in halt state
        return;
    }

    // 1. CRITICAL: Call Task functions using manager's accessors
    //    This runs them in the loop's context.
    deviceManager.getHost().Task();
    deviceManager.getEnumerator().Task();
    // Note: Hub task is called implicitly by getHost().Task() if hub is active

    // 2. Let the manager check enumeration status, print data when ready,
    //    and handle disconnect state resets internally.
    deviceManager.checkStatusAndPrint();

    // 3. Check for HALT condition (only *after* status check might have printed error)
    //    Use the manager's corrected status check methods.
    if (deviceManager.isEnumerationFinished() && deviceManager.isInErrorState()) {
         DEBUG_SERIAL_ENUM.println("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
         DEBUG_SERIAL_ENUM.println("!!! loop(): DEVICE ENUMERATION FAILED! System halting. !!!");
         DEBUG_SERIAL_ENUM.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
         haltSystem = true; // Set flag to halt on next loop iteration
    }

    // 4. Small delay to prevent overly tight loop
    delay(1);
}