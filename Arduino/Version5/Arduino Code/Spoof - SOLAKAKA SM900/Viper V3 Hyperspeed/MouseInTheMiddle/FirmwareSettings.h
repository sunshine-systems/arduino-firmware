#ifndef FIRMWARE_SETTINGS_H
#define FIRMWARE_SETTINGS_H

#include <Arduino.h>

class FirmwareSettings {
public:
    // Constructor
    FirmwareSettings();

    // Method to update settings based on received data
    void updateSettings(const uint8_t* data);

private:
    // Helper method to print setting changes
    void printSettingChange(const String& settingName, const String& value) const;
};

#endif // FIRMWARE_SETTINGS_H
