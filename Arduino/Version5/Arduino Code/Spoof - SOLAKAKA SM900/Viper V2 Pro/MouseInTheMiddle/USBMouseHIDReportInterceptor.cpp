// USBMouseHIDReportParser.cpp
#include "USBMouseHIDReportInterceptor.h"
#include "MathAndConversions.h"
#include "Config.h"

USBMouseHIDReportInterceptor::USBMouseHIDReportInterceptor() {}

void USBMouseHIDReportInterceptor::Parse(USBHID* hid, bool is_rpt_id, uint8_t len, uint8_t* buf)
{
    processAndSetHIDReportData00A6(buf);
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
        FTDI_DEVICE.print("I: ");
        for (uint8_t i = 0; i < 8; i++) {
            FTDI_DEVICE.print(data[i], HEX);
        }
        FTDI_DEVICE.println("");
    }

    previousMouseButtons = mouseButtons;
    mouseButtons = data[0];
    xMovement = mathAndConversions.parseX_00A6(data);
    yMovement = mathAndConversions.parseY_00A6(data);
    scrollWheel = data[3];
    isDataAvailable = true;  // Set dataReceived to true when new data is set
}