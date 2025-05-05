#include "DeviceEnumeratorManager.h"
#include "config.h" // Include configuration for logging, serial

// --- Constructor ---
DeviceEnumeratorManager::DeviceEnumeratorManager() :
    myusb_(),
    enumerator_(myusb_), // Pass host reference to enumerator
    hub1_(myusb_),       // Pass host reference to hub
    storedDataPrinted_(false),
    devicePresentOrHandled_(false)
{ }

// --- Initialization Method ---
void DeviceEnumeratorManager::begin() {
    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
        DEBUG_SERIAL_ENUM.println("DeviceEnumeratorManager: Initializing USB Host Controller...");
    }
    myusb_.begin();
    storedDataPrinted_ = false; // Reset state on initialization
    devicePresentOrHandled_ = false;
    if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
        DEBUG_SERIAL_ENUM.println("DeviceEnumeratorManager: USB Host Controller Started.");
    }
}

// --- Status Check & Print Method ---
void DeviceEnumeratorManager::checkStatusAndPrint() {

    Device_t* currentDev = enumerator_.getCurrentDevice();
    // *** CORRECTED: Use actual method isErrorState() ***
    bool isFinished = enumerator_.isEnumerationDone() || enumerator_.isErrorState();

    // 1. Detect device presence (first time seeing it this connection cycle)
    if (!devicePresentOrHandled_ && currentDev != nullptr) {
        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
            DEBUG_SERIAL_ENUM.println("Manager: Device detected by enumerator.");
        }
        devicePresentOrHandled_ = true;
        storedDataPrinted_ = false; // Ensure print flag is reset for new device
    }

    // 2. Handle printing data upon enumeration finishing
    if (devicePresentOrHandled_ && isFinished && !storedDataPrinted_) {
        if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
            DEBUG_SERIAL_ENUM.println("\n>>> Manager: Enumeration Finished <<<");
            // Use isEnumerationDone() for SUCCESS check (already correct)
            DEBUG_SERIAL_ENUM.printf("    Result: %s\n", enumerator_.isEnumerationDone() ? "SUCCESS" : "FAILED (Error State)");
            DEBUG_SERIAL_ENUM.println("    --- Printing Stored Data (if any) ---");
        }

        const UsbDeviceData* data = enumerator_.getStoredDeviceData();
        if (data && (data->idVendor != 0 || data->idProduct != 0)) {
            enumerator_.printStoredData(DEBUG_SERIAL_ENUM);
        } else {
             if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
                DEBUG_SERIAL_ENUM.println("    (No valid data stored or available to print)");
             }
        }
         if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
             DEBUG_SERIAL_ENUM.println("    -------------------------------------");
         }
        storedDataPrinted_ = true; // Mark as printed/handled for this device instance
    }

    // 3. Handle disconnection (Reset flags)
    if (devicePresentOrHandled_ && currentDev == nullptr) {
         if (ENABLE_LOGGING_DEVICE_ENUMERATOR) {
            DEBUG_SERIAL_ENUM.println("\n>>> Manager: Device disconnected or enumerator reset. <<<");
         }
         storedDataPrinted_ = false;
         devicePresentOrHandled_ = false;
    }
}


// --- Core Component Accessors ---
USBHost& DeviceEnumeratorManager::getHost() {
    return myusb_;
}

DeviceEnumerator& DeviceEnumeratorManager::getEnumerator() {
    return enumerator_;
}

// --- Status Check Methods ---
bool DeviceEnumeratorManager::isEnumerationComplete() const {
    return enumerator_.isEnumerationDone();
}

bool DeviceEnumeratorManager::isInErrorState() const { // Keep method name consistent in manager
    // *** CORRECTED: Call actual enumerator method isErrorState() ***
    return enumerator_.isErrorState();
}

bool DeviceEnumeratorManager::isEnumerationFinished() const {
    // *** CORRECTED: Use actual method isErrorState() in combined check ***
    return enumerator_.isEnumerationDone() || enumerator_.isErrorState();
}

// --- Data Accessor ---
const UsbDeviceData* DeviceEnumeratorManager::getEnumeratedData() const {
    const UsbDeviceData* data = enumerator_.getStoredDeviceData();
    if (data && (data->idVendor != 0 || data->idProduct != 0)) {
        return data;
    }
    return nullptr;
}