// SerialMouseHIDReportInterceptor.cpp
#include "SerialMouseHIDReportInterceptor.h"
#include "Config.h"
#include "MathAndConversions.h"

SerialMouseHIDReportInterceptor::SerialMouseHIDReportInterceptor() {}

// Assuming HID reports are 8 bytes and settings are 3 bytes
void SerialMouseHIDReportInterceptor::sniffForSpoofableDataOverSerial() {
    static uint8_t commandBuffer[8];  // Largest needed size for HID reports
    static uint8_t bufferIndex = 0;

    while (Serial1.available()) {
        uint8_t incomingByte = Serial1.read();
        if (incomingByte == '\n') {
            if (bufferIndex == 8) {  // HID report size
                processAndSetHIDReportData(commandBuffer);
            } else if (bufferIndex == 3) {  // Settings update size
                updateSettings(commandBuffer);
            } else {
                if (DEBUG_MODE) {
                    FT232RL.print("E: Incorrect data length: ");
                    FT232RL.println(bufferIndex);
                }
            }
            bufferIndex = 0; // Reset the buffer index
        } else {
            if (bufferIndex < sizeof(commandBuffer)) {
                commandBuffer[bufferIndex++] = incomingByte;
            } else {
                // Handling buffer overflow - reset index if byte limit is exceeded without receiving newline
                bufferIndex = 0;
            }
        }
    }
}


/*
    Byte 0 = Mouse Buttons
    Byte 1 = Mouse X movement within the range of -32767...32767
    Byte 2 = Mouse X movement within the range of -32767...32767
*/
void SerialMouseHIDReportInterceptor::processAndSetHIDReportData(const uint8_t* data)
{
    previousMouseButtons = mouseButtons;
    mouseButtons = data[0];
    scrollWheel = data[3];
    xMovement = mathAndConversions.parseX_00A6(data);
    yMovement = mathAndConversions.parseY_00A6(data);
    isDataAvailable = true;  // Set dataReceived to true when new data is set
}

bool SerialMouseHIDReportInterceptor::hasData(){
    return isDataAvailable;
}

// reset mouse movement to prevent ghosting
void SerialMouseHIDReportInterceptor::reset(){
    isDataAvailable = false;
}

void SerialMouseHIDReportInterceptor::updateSettings(const uint8_t* data) {
    uint8_t settingId = data[0];
    int16_t settingValue = data[1] | (data[2] << 8);  // Combine two bytes into a 16-bit integer

    // Handling enableLockout setting
    if (settingId == 1) {
        enableLockout = settingValue != 0;  // Treat non-zero as true
        FT232RL.print("I: Setting changed - enableLockout: ");
        FT232RL.println(enableLockout ? "True" : "False");
    }
    // Handling lockoutDuration setting
    else if (settingId == 2) {
        lockoutDuration = settingValue;
        FT232RL.print("I: Setting changed - lockoutDuration: ");
        FT232RL.println(lockoutDuration);
    }
    // Handling enableSpinOnLMB setting
    else if (settingId == 3) {
        enableSpinOnLMB = settingValue != 0;
        FT232RL.print("I: Setting changed - enableSpinOnLMB: ");
        FT232RL.println(enableSpinOnLMB ? "True" : "False");
    }
    // Handling spinOnLMBRotations setting
    else if (settingId == 4) {
        spinOnLMBRotations = settingValue;
        FT232RL.print("I: Setting changed - spinOnLMBRotations: ");
        FT232RL.println(spinOnLMBRotations);
    }
    // Handling spinOnLMBAmountToSpin setting
    else if (settingId == 5) {
        spinOnLMBAmountToSpin = settingValue;
        FT232RL.print("I: Setting changed - spinOnLMBAmountToSpin: ");
        FT232RL.println(spinOnLMBAmountToSpin);
    }
    else {
        // Handle unknown setting ID
        FT232RL.print("I: Unknown setting ID received -> ");
        FT232RL.println(settingId);
    }
}
