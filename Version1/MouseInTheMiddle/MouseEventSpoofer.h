// MouseEventSpoofer.h

#ifndef _MOUSEEVENTSPOOFER_H_
#define _MOUSEEVENTSPOOFER_H_

#include "USBMouseHIDReportInterceptor.h"
#include "SerialMouseHIDReportInterceptor.h"

#define MOUSE_LEFT 		1
#define MOUSE_RIGHT 	2
#define MOUSE_MIDDLE 	4
#define MOUSE_BUTTON4 	8
#define MOUSE_BUTTON5 	0x10

class MouseEventSpoofer {
public:
    MouseEventSpoofer(USBMouseHIDReportInterceptor* usbInterceptor, SerialMouseHIDReportInterceptor* serialInterceptor);
    void spoofEvent();
    void handleButtonEvents(uint8_t usbButtons, uint8_t previousUsbButtonsState, uint8_t serialButtons, uint8_t previousSerialButtonsState);
    void modifyMovementWithSerialData(int16_t &usbXMovement, int16_t &usbYMovement, int16_t serialXMovement, int16_t serialYMovement) ;
    void onMouseMove(int16_t xMovement, int16_t yMovement, int8_t scrollValue);

private:
    USBMouseHIDReportInterceptor* usbInterceptor;
    SerialMouseHIDReportInterceptor* serialInterceptor;
    unsigned long activationTimestamp4MouseButtonExclusion;
};

#endif // _MOUSEEVENTSPOOFER_H_
