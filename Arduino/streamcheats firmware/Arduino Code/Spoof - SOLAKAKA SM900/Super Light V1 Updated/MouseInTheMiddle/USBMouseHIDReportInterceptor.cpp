// USBMouseHIDReportParser.cpp
#include "USBMouseHIDReportInterceptor.h"
#include "MathAndConversions.h"
#include "Config.h"
#include "SunBoxLogger.h"

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
    previousMouseButtons = mouseButtons;
    mouseButtons = data[0];
    xMovement = mathAndConversions.parseXandY_C547(data[3], data[2]);
    yMovement = mathAndConversions.parseXandY_C547(data[5], data[4]);
    scrollWheel = data[6];
    isDataAvailable = true;

    if (DEBUG_MODE) {
        logger.debugf("Mouse data - Buttons:0x%02X X:%d Y:%d Wheel:%d",
                      mouseButtons, xMovement, yMovement, scrollWheel);
    }
}