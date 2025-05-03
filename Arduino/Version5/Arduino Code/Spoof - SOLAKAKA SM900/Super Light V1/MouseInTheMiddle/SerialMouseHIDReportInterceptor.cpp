// SerialMouseHIDReportInterceptor.cpp
#include "SerialMouseHIDReportInterceptor.h"
#include "Config.h"
#include "MathAndConversions.h"

SerialMouseHIDReportInterceptor::SerialMouseHIDReportInterceptor(FirmwareSettings* firmwareSettings)
: firmwareSettings(firmwareSettings) {}

void SerialMouseHIDReportInterceptor::sniffForSpoofableDataOverSerial() {
    static uint8_t commandBuffer[8];  // Buffer size to accommodate data (max 8 bytes)
    static uint8_t bufferIndex = 0;
    static uint8_t expectedLength = 0;  // Length of the message

    uint8_t dataBuffer[9];  // Buffer to hold the entire 9 bytes of data

    // Read the entire 9 bytes if available
    if (FTDI_DEVICE.available() >= 9) {
        for (uint8_t i = 0; i < 9; i++) {
            dataBuffer[i] = FTDI_DEVICE.read();
        }

        // The first byte is the length prefix
        expectedLength = dataBuffer[0];

        // Copy the relevant bytes into the commandBuffer based on the length prefix
        for (uint8_t i = 0; i < expectedLength; i++) {
            commandBuffer[i] = dataBuffer[i + 1];
        }

        logAPerformanceMetric = true;

        if (expectedLength == 8) {
            // If the length byte is 8, it's a HID message
            processAndSetHIDReportData(commandBuffer);
        } else if (expectedLength == 3) {
            // If the length byte is 3, it's a settings message
            firmwareSettings->updateSettings(commandBuffer);
        }

        // Reset bufferIndex for the next message
        bufferIndex = 0;
    }
}

// Process and set spoofed HID report data to be used in the MouseEventSpoofer
void SerialMouseHIDReportInterceptor::processAndSetHIDReportData(const uint8_t* data)
{
    previousMouseButtons = mouseButtons;
    mouseButtons = data[0];
    scrollWheel = data[3];
    xMovement = mathAndConversions.parseX_00A6(data);
    yMovement = mathAndConversions.parseY_00A6(data);
    isDataAvailable = true;  // Set dataReceived to true when new data is set
}

// Used to determine if we read ANY data in this loop at all
bool SerialMouseHIDReportInterceptor::hasData(){
    return isDataAvailable;
}

// Reset Serial Mouse movement to prevent ghosting
void SerialMouseHIDReportInterceptor::reset(){
    isDataAvailable = false;
}