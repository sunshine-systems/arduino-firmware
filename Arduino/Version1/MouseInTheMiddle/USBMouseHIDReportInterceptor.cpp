// USBMouseHIDReportParser.cpp
#include "USBMouseHIDReportInterceptor.h"
#include "EdianConversions.h"

USBMouseHIDReportInterceptor::USBMouseHIDReportInterceptor() {}

void USBMouseHIDReportInterceptor::Parse(USBHID* hid, bool is_rpt_id, uint8_t len, uint8_t* buf)
{

    // If the length of data received is equal to HID_REPORT_SIZE, copy the data to hidReportData.
    if (len == HID_REPORT_SIZE) {
//        Serial1.print("MRPT: ");
//        for (uint8_t i = 0; i < len; i++) {
//            Serial1.print(' ');
//            Serial1.print(buf[i], HEX);
//        }
//        Serial1.println(" ");
        processAndSetHIDReportData(buf);
    }

}

/*
    This converts the 8 byte report into a 4 byte report
    Byte 0 = Mouse Buttons
    Byte 1 = Mouse X movement within the range of -32767...32767
    Byte 2 = Mouse X movement within the range of -32767...32767
    Byte 3 = Scroll Wheel up & down

*/
void USBMouseHIDReportInterceptor::processAndSetHIDReportData(const uint8_t* data)
{
    previousMouseButtons = mouseButtons;
    mouseButtons = data[0];
    xMovement = edianConversions.parseX(data);
    yMovement = edianConversions.parseY(data);
    scrollWheel = data[3];
    isDataAvailable = true;  // Set dataReceived to true when new data is set
}

bool USBMouseHIDReportInterceptor::hasData(){
    return isDataAvailable;
}

// reset mouse movement to prevent ghosting
void USBMouseHIDReportInterceptor::reset(){
    isDataAvailable = false;
}
