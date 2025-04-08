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

    private:
        void submitButtonStates();
        void modifyMovementWithSerialData(int16_t &usbXMovement, int16_t &usbYMovement, int16_t serialXMovement, int16_t serialYMovement);
        void onMouseMove(int16_t xMovement, int16_t yMovement, int8_t scrollValue);
        void logMouseEvent(uint8_t mouseButtons);
        bool shouldExcludeButton(uint8_t currentButtons, uint8_t previousButtons, uint8_t buttonMask);
        void handleMouseButtonEvent(uint8_t currentButtons, uint8_t previousButtons, uint8_t buttonMask);
        void handleButtonEvents(uint8_t usbButtons, uint8_t previousUsbButtonsState, uint8_t serialButtons, uint8_t previousSerialButtonsState);
        void performSpinEvent(bool isBeforeEvent, uint8_t usbMouseButtons, uint8_t usbPreviousMouseButtons, uint8_t serialMouseButtons, uint8_t serialPreviousMouseButtons);

        // Member variables
        USBMouseHIDReportInterceptor* usbInterceptor;
        SerialMouseHIDReportInterceptor* serialInterceptor;
        uint8_t finalButtonStates;
        uint8_t previousUsbButtonsState;
        uint8_t previousSerialButtonsState;
        unsigned long activationTimestamp4MouseButtonExclusion;
        unsigned long activationTimestamp4MouseMovementLockout;
        bool spinPerformed;
};

#endif // _MOUSEEVENTSPOOFER_H_
