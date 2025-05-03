#include <Arduino.h>
#include <USBHost_t36.h> // Core USB Host library

// --- Configuration ---
// >>> Select your debug output port here <<<
#define DEBUG_SERIAL Serial // Use Serial1 for external adapter
// #define DEBUG_SERIAL Serial    // Use standard USB Serial Monitor

const unsigned long DEBUG_BAUD = 115200; // Baud rate for the chosen debug port

// --- USB Host Objects ---
USBHost myusb;

// *** Instantiate MINIMAL drivers ***
USBHub hub1(myusb);            // Keep hub support
MouseController mouse1(myusb); // Specific mouse driver (will be checked separately)
USBHIDParser hid1(myusb);      // Generic HID parser (inherits correctly for the array)

// --- Driver Tracking Array (Excludes MouseController) ---
USBDriver *drivers_to_monitor[] = {
    &hub1,
    &hid1 // USBHIDParser is okay here
};

// Corresponding names for printing
const char * driver_names[] = {
    "Hub1",
    "HIDParser1" // Generic HID Driver
};

// Calculate the number of drivers we are actively monitoring in the array
const int CNT_MONITORED_DEVICES = sizeof(drivers_to_monitor) / sizeof(drivers_to_monitor[0]);

// Keep track of the active state for each monitored driver in the array
bool driver_active[CNT_MONITORED_DEVICES];

// Separate tracking for MouseController
bool mouse1_active = false;

// --- Setup Function ---
void setup() {
    // Start the chosen debug serial port
    DEBUG_SERIAL.begin(DEBUG_BAUD);

    // If using standard Serial, wait a bit for the monitor to connect
    #if DEBUG_SERIAL == Serial
        unsigned long startWait = millis();
        while (!Serial && (millis() - startWait < 4000)) {
             yield(); // Allow background tasks
        }
        delay(100);
    #else
        delay(100); // Small delay for Serial1 just in case
    #endif

    // Initialize active states to false
    for (int i = 0; i < CNT_MONITORED_DEVICES; i++) {
        driver_active[i] = false;
    }
    mouse1_active = false; // Initialize mouse state

    // Print startup messages to the chosen debug port
    DEBUG_SERIAL.println("\n-------------------------------------------------");
    DEBUG_SERIAL.println("Teensy 4.1 - Debug Output & USB Host");
    DEBUG_SERIAL.println("              Minimal Driver Test v2");
    DEBUG_SERIAL.println("-------------------------------------------------");
    DEBUG_SERIAL.print("Debug output port started at ");
    DEBUG_SERIAL.print(DEBUG_BAUD);
    DEBUG_SERIAL.println(" baud.");
    DEBUG_SERIAL.printf("Monitoring %d array drivers + MouseController.\n", CNT_MONITORED_DEVICES);

    // Start the USB Host stack
    myusb.begin();
    DEBUG_SERIAL.println("USB Host Controller Started. Connect mouse (set to 1kHz).");
}

// --- Loop Function ---
void loop() {
    myusb.Task(); // Keep the USB stack running

    // --- 1. Check Status of Drivers in the Array (Hub, HIDParser) ---
    for (uint8_t i = 0; i < CNT_MONITORED_DEVICES; i++) {
        if (*drivers_to_monitor[i] != driver_active[i]) {
            if (driver_active[i]) {
                // Disconnected
                DEBUG_SERIAL.printf("\n*** Driver %s - disconnected ***\n", driver_names[i]);
                driver_active[i] = false;
            } else {
                // Connected
                DEBUG_SERIAL.printf("\n*** Driver %s - connected ***\n", driver_names[i]);
                // Explicitly use USBHIDInput scope for ambiguous members if needed, though less likely here
                DEBUG_SERIAL.printf("  VID: 0x%04X, PID: 0x%04X\n", drivers_to_monitor[i]->idVendor(), drivers_to_monitor[i]->idProduct());
                driver_active[i] = true;
                const uint8_t *psz = drivers_to_monitor[i]->manufacturer();
                if (psz && *psz) DEBUG_SERIAL.printf("  Manufacturer: %s\n", psz);
                psz = drivers_to_monitor[i]->product();
                if (psz && *psz) DEBUG_SERIAL.printf("  Product: %s\n", psz);
                 psz = drivers_to_monitor[i]->serialNumber();
                if (psz && *psz) DEBUG_SERIAL.printf("  Serial #: %s\n", psz);
            }
            DEBUG_SERIAL.println("-------------------------------------");
        }
    }

    // --- 2. Check Status of MouseController Separately ---
    // Explicitly call the boolean operator from the USBHIDInput base class
    bool current_mouse1_state = mouse1.USBHIDInput::operator bool();
    if (current_mouse1_state != mouse1_active) {
         if (mouse1_active) {
             // Disconnected
             DEBUG_SERIAL.printf("\n*** Driver Mouse1 - disconnected ***\n");
             mouse1_active = false;
         } else {
             // Connected
             DEBUG_SERIAL.printf("\n*** Driver Mouse1 - connected ***\n");
             // Explicitly use USBHIDInput:: scope resolution for ambiguous members
             DEBUG_SERIAL.printf("  VID: 0x%04X, PID: 0x%04X\n", mouse1.USBHIDInput::idVendor(), mouse1.USBHIDInput::idProduct());
             mouse1_active = true;
             const uint8_t *psz = mouse1.USBHIDInput::manufacturer();
             if (psz && *psz) DEBUG_SERIAL.printf("  Manufacturer: %s\n", psz);
             psz = mouse1.USBHIDInput::product();
             if (psz && *psz) DEBUG_SERIAL.printf("  Product: %s\n", psz);
             psz = mouse1.USBHIDInput::serialNumber();
             if (psz && *psz) DEBUG_SERIAL.printf("  Serial #: %s\n", psz);
         }
         DEBUG_SERIAL.println("-------------------------------------");
    }

    // --- 3. Optional: Echo Debug Serial Input ---
    if (DEBUG_SERIAL.available()) {
        char receivedChar = DEBUG_SERIAL.read();
        DEBUG_SERIAL.print("Debug RX Echo: ");
        if (isprint(receivedChar)) { DEBUG_SERIAL.print(receivedChar); } else { DEBUG_SERIAL.print("0x"); if (receivedChar < 0x10) DEBUG_SERIAL.print("0"); DEBUG_SERIAL.print(receivedChar, HEX); }
        DEBUG_SERIAL.println();
    }
}