#ifndef _MOUSEEVENTSPOOFER_H_
#define _MOUSEEVENTSPOOFER_H_

#include "USBMouseHIDReportInterceptor.h"
#include "SerialMouseHIDReportInterceptor.h"

#define MOUSE_LEFT      1
#define MOUSE_RIGHT     2
#define MOUSE_MIDDLE    4
#define MOUSE_BUTTON4   8
#define MOUSE_BUTTON5   0x10

class MouseEventSpoofer {
    public:
        MouseEventSpoofer(USBMouseHIDReportInterceptor* usbInterceptor, SerialMouseHIDReportInterceptor* serialInterceptor);
        void spoofEvent();

    private:
        void submitButtonStates();
        void modifyMovementWithSerialData(int16_t &usbXMovement, int16_t &usbYMovement, int16_t serialXMovement, int16_t serialYMovement);
        void onMouseMove(int16_t xMovement, int16_t yMovement, int8_t scrollValue);
        void logMouseEvent(uint8_t mouseButtons);  // Ensure this function is declared
        bool shouldExcludeButton(uint8_t currentButtons, uint8_t previousButtons, uint8_t buttonMask); // Ensure this function is declared
        void handleMouseButtonEvent(uint8_t currentButtons, uint8_t previousButtons, uint8_t buttonMask); // Ensure this function is declared
        void handleMouseButtonConfigCheck(uint8_t &usbMouseButtons, uint8_t &unmodifiedUsbMouseButtons, uint8_t &usbPreviousMouseButtons, uint8_t buttonMask, int disablePassthroughOption, unsigned long &lastPressTime);
        void handleButtonEvents(uint8_t usbButtons, uint8_t previousUsbButtonsState, uint8_t serialButtons, uint8_t previousSerialButtonsState);

        // Member variables
        USBMouseHIDReportInterceptor* usbInterceptor;
        SerialMouseHIDReportInterceptor* serialInterceptor;
        uint8_t finalButtonStates;
        uint8_t previousUsbButtonsState;
        unsigned long activationTimestamp4MouseButtonExclusion;
        unsigned long activationTimestamp4MouseMovementLockout;

        // Use integers for fixed-point arithmetic tracking
        int sensReductionXAccumulator = 0; // Accumulator for X-axis movement
        int sensReductionYAccumulator = 0; // Accumulator for Y-axis movement

        // Timestamps for double-tap logic
        unsigned long lastRMBPressTime = 0;
        unsigned long lastLMBPressTime = 0;
        unsigned long lastMB4PressTime = 0;
        unsigned long lastMB5PressTime = 0;
};

#endif // _MOUSEEVENTSPOOFER_H_
