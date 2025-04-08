// USBMouseHIDReportParser.cpp
#include "USBMouseHIDReportInterceptor.h"
#include "MathAndConversions.h"
#include "Config.h"

USBMouseHIDReportInterceptor::USBMouseHIDReportInterceptor() {}

void USBMouseHIDReportInterceptor::Parse(USBHID* hid, bool is_rpt_id, uint8_t len, uint8_t* buf)
{
    // If we havent read the usb device and mapped it to global variables yet break early
    if (!hasReadConnectedDevice) return;

    // Process normally
    if (strcmp(connectedDevice->pid, "00A6") == 0) {
        if (len == connectedDevice->reportSize)
            processAndSetHIDReportData00A6(buf);
    } else if (strcmp(connectedDevice->pid, "C547") == 0) {
        if (len, connectedDevice->reportSize)
            processAndSetHIDReportDataC547(buf);
    } else {
        FT232RL.print("E: Cannot Process data, invalid device pid found: ");
        FT232RL.println(connectedDevice->pid);
    }
}

// Checks if theres data available for the spoofer class
bool USBMouseHIDReportInterceptor::hasData(){
    return isDataAvailable;
}

// reset mouse movement to prevent ghosting
void USBMouseHIDReportInterceptor::reset(){
    isDataAvailable = false;
}


/* The below functions are mouse-specific hid data parsers */

void USBMouseHIDReportInterceptor::processAndSetHIDReportData00A6(const uint8_t* data)
{
    if (DEBUG_MODE) {
        FT232RL.print("I: ");
        for (uint8_t i = 0; i < 8; i++) {
            FT232RL.print(data[i], HEX);
        }
        FT232RL.println(" ");
    }

    previousMouseButtons = mouseButtons;
    mouseButtons = data[0];
    xMovement = mathAndConversions.parseX_00A6(data);
    yMovement = mathAndConversions.parseY_00A6(data);
    scrollWheel = data[3];
    isDataAvailable = true;  // Set dataReceived to true when new data is set
}

void USBMouseHIDReportInterceptor::processAndSetHIDReportDataC547(const uint8_t* data)
{
    if (DEBUG_MODE) {
        FT232RL.print("I: ");
        for (uint8_t i = 0; i < 13; i++) {
            FT232RL.print(data[i], HEX);
        }
        FT232RL.println(" ");
    }

    // Restructure in the format of Razer Viper V2 Pro
    // uint8_t new_data[8];
    // new_data[0] = data[0]; // Mouse buttons
    // new_data[1] = data[2]; // Lower X from superlight
    // new_data[2] = data[4]; // Lower Y from superlight
    // new_data[3] = data[6]; // Scroll Wheel
    // new_data[4] = data[2]; // Lower X from superlight again
    // new_data[5] = data[3]; // Upper X from superlight (also overflow indicator)
    // new_data[6] = data[4]; // Lower Y from superlight again
    // new_data[7] = data[5]; // Upper Y from superlight (also overflow indicator)


    previousMouseButtons = mouseButtons;
    mouseButtons = data[0];
    xMovement = mathAndConversions.parseXandY_C547(data[3], data[2]);
    yMovement = mathAndConversions.parseXandY_C547(data[5], data[4]);
    scrollWheel = data[6];
    isDataAvailable = true;  // Set dataReceived to true when new data is set
}