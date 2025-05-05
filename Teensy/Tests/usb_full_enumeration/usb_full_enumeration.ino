// ==========================================================================
// Teensy 4.1 - Full Enumeration Test (v25 - Refactored + Report Desc Fix)
// ==========================================================================
// Main sketch file

#include <Arduino.h>
#include <USBHost_t36.h>
#include "DeviceEnumerator.h" // Include our custom class header

// --- USB Host Objects ---
USBHost myusb;

// *** Instantiate Drivers ***
DeviceEnumerator enumerator(myusb); // Our custom driver ONLY for enumeration
// <<< Standard drivers DISABLED >>>
USBHub hub1(myusb); // Keep hub driver
// MouseController mouse1(myusb);  // DISABLED
// USBHIDParser hid1(myusb);      // DISABLED


// --- Driver Tracking Array (Hub only) ---
USBDriver *drivers_to_monitor[] = { &hub1 }; // Only hub
const char * driver_names[] = { "Hub1" };
const int CNT_MONITORED_DEVICES = sizeof(drivers_to_monitor) / sizeof(drivers_to_monitor[0]);
bool driver_active[CNT_MONITORED_DEVICES];

// --- Global Flag for Printing ---
bool storedDataPrinted = false;

// --- Global Pointer (Needed if other .cpp files were to access enumerator) ---
// DeviceEnumerator* global_enumerator_ptr = nullptr; // Not strictly needed now, but good practice

// --- Setup Function ---
void setup() {
    Serial.begin(115200); // Use Serial directly
    #if defined(USB_SERIAL) // Wait for Serial Monitor connection if using Native USB Serial
        unsigned long startWait = millis();
        while (!Serial && (millis() - startWait < 4000)) { yield(); } // 4 second timeout
        delay(200); // Allow slightly more time for serial monitor to fully connect
    #else // For Hardware Serial, just a small delay
        delay(200);
    #endif

    // --- Initialize Global Pointer ---
    // global_enumerator_ptr = &enumerator; // Assign address

    for (int i = 0; i < CNT_MONITORED_DEVICES; i++) {
        driver_active[i] = false;
    }

    Serial.println("\n-------------------------------------------------");
    Serial.println("Teensy 4.1 - Full Enumeration Test (v25 - Refactored)"); // Updated Title
    Serial.println("-------------------------------------------------");
    Serial.print("Debug output port started at ");
    Serial.print(115200); // Hardcode baud rate here
    Serial.println(" baud.");
    Serial.printf("Monitoring %d standard drivers (Hub Only).\n", CNT_MONITORED_DEVICES);
    Serial.println("DeviceEnumerator is active and will analyze devices.");
    Serial.println("Standard MouseController and HIDParser are DISABLED.");
    Serial.println("-------------------------------------------------");

    myusb.begin();
    Serial.println("USB Host Controller Started.");
    Serial.println("Connect a USB device to the Host port...");
}

// --- Loop Function ---
void loop() {
    myusb.Task();      // Process USB Host tasks - CRITICAL for callbacks
    enumerator.Task(); // Process our custom driver's tasks

    // --- Check Enumerator Status ---
    static unsigned long last_check = 0;
    static uint32_t loop_count = 0;
    loop_count++;
    if (millis() - last_check > 2000) { // Print status every 2 seconds
        Serial.print("Loop Status Check ["); Serial.print(loop_count); Serial.print("]: Enumerator Device Ptr: ");
        if (enumerator.getCurrentDevice()) {
             Serial.printf("0x%08lX (Valid)\n", (uint32_t)enumerator.getCurrentDevice());
        } else {
             Serial.println("NULL");
        }
        Serial.printf("  Enumeration Done? %s, Error State? %s, Printed? %s\n",
                            (enumerator.isEnumerationDone() ? "Yes" : "No"),
                            (enumerator.isErrorState() ? "Yes" : "No"),
                            (storedDataPrinted ? "Yes" : "No"));
        last_check = millis();
    }


    // --- Print Stored Data Once ---
    if ((enumerator.isEnumerationDone() || enumerator.isErrorState()) && !storedDataPrinted && enumerator.getStoredDeviceData()) {
        Serial.println("\n>>> Enumeration Complete or Error State Reached - Printing Stored Data <<<");
        enumerator.printStoredData(Serial); // Pass Serial object to print function
        storedDataPrinted = true;
    }

    // --- Reset Print Flag on Disconnect ---
    if (enumerator.getStoredDeviceData() == nullptr && storedDataPrinted) {
         Serial.println("\n>>> Enumerator Data Cleared (Device Disconnected?) - Resetting Print Flag <<<");
         storedDataPrinted = false;
    }


    // --- Check Status of Hub Driver (Optional) ---
    for (uint8_t i = 0; i < CNT_MONITORED_DEVICES; i++) { // Loop only checks Hub1 now
        if (*drivers_to_monitor[i] != driver_active[i]) {
            if (driver_active[i]) {
                // Serial.printf("\n>>> Standard Driver %s - disconnected <<<\n", driver_names[i]); // Less verbose
                driver_active[i] = false;
            } else {
                Serial.printf("\n>>> Standard Driver %s - connected <<<\n", driver_names[i]);
                driver_active[i] = true;
            }
        }
    }
    // --- MouseController / HIDParser Checks Are Removed ---
}