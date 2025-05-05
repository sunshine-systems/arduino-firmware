#ifndef DEVICE_ENUMERATOR_MANAGER_H
#define DEVICE_ENUMERATOR_MANAGER_H

#include <Arduino.h>
#include <USBHost_t36.h>
#include "config.h"          // For configuration constants (logging, serial)
#include "DeviceEnumerator.h" // Include the enumerator class definition

class DeviceEnumeratorManager {
public:
    // --- Constructor ---
    DeviceEnumeratorManager();

    // --- Initialization Method ---
    // Initializes the USB Host controller and internal state. Call this from setup().
    void begin();

    // --- Status Check & Print Method ---
    // Checks enumeration status, prints data once on completion/error,
    // handles resetting print flag on disconnect. Call this repeatedly from loop().
    void checkStatusAndPrint();

    // --- Core Component Accessors ---
    // Provide access to the underlying objects needed by the main loop.
    USBHost& getHost();
    DeviceEnumerator& getEnumerator();
    // You could add one for the Hub if needed: USBHub& getHub();

    // --- Status Check Methods ---
    // Simple wrappers around the enumerator's status methods,
    // or combinations thereof.
    bool isEnumerationComplete() const; // Checks if STATE_DONE
    bool isInErrorState() const;        // Checks if STATE_ERROR
    bool isEnumerationFinished() const; // Checks if STATE_DONE OR STATE_ERROR

    // --- Data Accessor ---
    const UsbDeviceData* getEnumeratedData() const;


private:
    // --- Core Components (Owned by Manager) ---
    USBHost          myusb_;      // The main USB Host object
    DeviceEnumerator enumerator_; // Our custom enumerator driver
    USBHub           hub1_;       // Keep Hub driver active

    // --- Internal State ---
    bool storedDataPrinted_; // Flag to track if data has been printed for the current device
    bool devicePresentOrHandled_; // Tracks if we've seen a device or handled its enumeration/disconnection
};

#endif // DEVICE_ENUMERATOR_MANAGER_H