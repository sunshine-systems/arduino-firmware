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
            printSettingChange("logPerformanceMetrics", String(logPerformanceMetrics));
            break;
        case 2:
            enableSensReduction = settingValue;
            printSettingChange("enableSensReduction", String(enableSensReduction));
            break;
        case 3:
            sensReductionDurationMilliseconds = settingValue;
            printSettingChange("sensReductionDurationMilliseconds", String(sensReductionDurationMilliseconds));
            break;
        case 4:
            sensReductionAmmountX = settingValue;
            printSettingChange("sensReductionAmmountX", String(sensReductionAmmountX));
            break;
        case 5:
            sensReductionAmmountY = settingValue;
            printSettingChange("sensReductionAmmountY", String(sensReductionAmmountY));
            break;
        case 6:
            enableSpinning = settingValue;
            printSettingChange("enableSpinning", String(enableSpinning));
            break;
        case 7:
            spinAmountPerRotation = settingValue;
            printSettingChange("spinAmountPerRotation", String(spinAmountPerRotation));
            break;
        case 8:
            spinNumberOfRotations = settingValue;
            printSettingChange("spinNumberOfRotations", String(spinNumberOfRotations));
            break;
        case 9:
            spinDelayBetweenRotationsMilliseconds = settingValue;
            printSettingChange("spinDelayBetweenRotationsMilliseconds", String(spinDelayBetweenRotationsMilliseconds));
            break;
        case 10:
            spinLockoutMouseUntilCompletion = settingValue;
            printSettingChange("spinLockoutMouseUntilCompletion", String(spinLockoutMouseUntilCompletion));
            break;
        case 11:
            spinBeforeAfterMouseEvent = settingValue;
            printSettingChange("spinBeforeAfterMouseEvent", String(spinBeforeAfterMouseEvent));
            break;
        case 12:
            disablePassthroughForMMB = settingValue;
            printSettingChange("disablePassthroughForMMB", String(disablePassthroughForMMB));
            break;
        case 13:
            disablePassthroughForRMB = settingValue;
            printSettingChange("disablePassthroughForRMB", String(disablePassthroughForRMB));
            break;
        case 14:
            disablePassthroughForLMB = settingValue;
            printSettingChange("disablePassthroughForLMB", String(disablePassthroughForLMB));
            break;
        case 15:
            disablePassthroughForMB4 = settingValue;
            printSettingChange("disablePassthroughForMB4", String(disablePassthroughForMB4));
            break;
        case 16:
            disablePassthroughForMB5 = settingValue;
            printSettingChange("disablePassthroughForMB5", String(disablePassthroughForMB5));
            break;
        default:
            FTDI_DEVICE.print("Unknown setting ID received -> ");
            FTDI_DEVICE.println(settingId);
            FTDI_DEVICE.print("Unknown setting ID Value received -> ");
            FTDI_DEVICE.println(settingValue);
            break;
    }
}

void FirmwareSettings::printSettingChange(const String& settingName, const String& value) const {
    FTDI_DEVICE.print("Setting changed - ");
    FTDI_DEVICE.print(settingName);
    FTDI_DEVICE.print(": ");
    FTDI_DEVICE.println(value);
}
