#include "FirmwareSettings.h"
#include "Config.h" // Include if you need constants or global configurations

FirmwareSettings::FirmwareSettings() {}

void FirmwareSettings::updateSettings(const uint8_t* data) {
    uint8_t settingId = data[0];
    int16_t settingValue = data[1] | (data[2] << 8);  // Combine two bytes into a 16-bit integer

    switch (settingId) {
        case 0:
            FTDI_DEVICE.print("V: ");
            FTDI_DEVICE.println(FIRMWARE_VERSION, 2);
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
            disablePassthroughForMMB = settingValue;
            printSettingChange("disablePassthroughForMMB", String(disablePassthroughForMMB));
            break;
        case 7:
            disablePassthroughForRMB = settingValue;
            printSettingChange("disablePassthroughForRMB", String(disablePassthroughForRMB));
            break;
        case 8:
            disablePassthroughForLMB = settingValue;
            printSettingChange("disablePassthroughForLMB", String(disablePassthroughForLMB));
            break;
        case 9:
            disablePassthroughForMB4 = settingValue;
            printSettingChange("disablePassthroughForMB4", String(disablePassthroughForMB4));
            break;
        case 10:
            disablePassthroughForMB5 = settingValue;
            printSettingChange("disablePassthroughForMB5", String(disablePassthroughForMB5));
            break;
        case 11:
            enableDeltaLogging = settingValue;
            printSettingChange("enableDeltaLogging", String(enableDeltaLogging));
            break;
        default:
            FTDI_DEVICE.println("I: Unknown setting ID received -> " + String(settingId));
            FTDI_DEVICE.println("I: Unknown setting ID Value received -> " + String(settingValue));
            break;
    }
}

void FirmwareSettings::printSettingChange(const String& settingName, const String& value) const {
    FTDI_DEVICE.println("I: " + settingName + ": " + value);
}
