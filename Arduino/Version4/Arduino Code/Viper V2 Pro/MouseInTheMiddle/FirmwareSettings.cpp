#include "FirmwareSettings.h"
#include "Config.h" // Include if you need constants or global configurations

FirmwareSettings::FirmwareSettings() {}

void FirmwareSettings::updateSettings(const uint8_t* data) {
    uint8_t settingId = data[0];
    int16_t settingValue = data[1] | (data[2] << 8);  // Combine two bytes into a 16-bit integer

    switch (settingId) {
        case 0:
            FTDI_DEVICE.print("V: ");
            FTDI_DEVICE.println(FIRMWARE_VERSION);
            break;
        case 1:
            logPerformanceMetrics = settingValue;
            printSettingChange("logPerformanceMetrics", String(settingValue));
            break;
        case 2:
            enableLockout = settingValue;
            printSettingChange("enableLockout", String(settingValue));
            break;
        case 3:
            mouseLockoutDurationMilliseconds = settingValue;
            printSettingChange("mouseLockoutDurationMilliseconds", String(mouseLockoutDurationMilliseconds));
            break;
        case 4:
            enableSpinning = settingValue;
            printSettingChange("enableSpinning", String(enableSpinning));
            break;
        case 5:
            spinAmountPerRotation = settingValue;
            printSettingChange("spinAmountPerRotation", String(spinAmountPerRotation));
            break;
        case 6:
            spinNumberOfRotations = settingValue;
            printSettingChange("spinNumberOfRotations", String(spinNumberOfRotations));
            break;
        case 7:
            spinDelayBetweenRotationsMilliseconds = settingValue;
            printSettingChange("spinDelayBetweenRotationsMilliseconds", String(spinDelayBetweenRotationsMilliseconds));
            break;
        case 8:
            spinLockoutMouseUntilCompletion = settingValue;
            printSettingChange("spinLockoutMouseUntilCompletion", String(spinLockoutMouseUntilCompletion));
            break;
        case 9:
            spinBeforeAfterMouseEvent = settingValue;
            printSettingChange("spinBeforeAfterMouseEvent", String(spinBeforeAfterMouseEvent));
            break;
        default:
            FTDI_DEVICE.print("Unknown setting ID received -> ");
            FTDI_DEVICE.println(settingId);
            break;
    }
}

void FirmwareSettings::printSettingChange(const String& settingName, const String& value) const {
    FTDI_DEVICE.print("Setting changed - ");
    FTDI_DEVICE.print(settingName);
    FTDI_DEVICE.print(": ");
    FTDI_DEVICE.println(value);
}
