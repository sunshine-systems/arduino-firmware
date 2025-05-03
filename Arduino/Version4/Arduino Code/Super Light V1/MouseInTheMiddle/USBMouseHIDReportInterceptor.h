// USBMouseHIDReportInterceptor.h
#ifndef USBMOUSEHIDREPORTINTERCEPTOR_H
#define USBMOUSEHIDREPORTINTERCEPTOR_H

#include <usbhid.h>

#define HID_REPORT_SIZE 8

class USBMouseHIDReportInterceptor : public HIDReportParser {
    private:
        uint8_t previousMouseButtons;
        uint8_t mouseButtons;
        int16_t xMovement;
        int16_t yMovement;
        uint8_t scrollWheel;

        void setMouseButtons(uint8_t buttons) { mouseButtons = buttons; }
        void setXMovement(int16_t x) { xMovement = x; }
        void setYMovement(int16_t y) { yMovement = y; }
        void setScrollWheel(uint8_t wheel) { scrollWheel = wheel; }

        bool isDataAvailable;

    public:
        USBMouseHIDReportInterceptor();
        virtual void Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf);

        void processAndSetHIDReportData00A6(const uint8_t* data);
        void processAndSetHIDReportDataC547(const uint8_t* data);
        bool hasData();
        void reset(); // used to prevent "mouse movement ghosting"
        uint8_t getPreviousMouseButtons() const { return previousMouseButtons; }
        uint8_t getMouseButtons() const { return mouseButtons; }
        int16_t getXMovement() const { return xMovement; }
        int16_t getYMovement() const { return yMovement; }
        uint8_t getScrollWheel() const { return scrollWheel; }
};

#endif // USBMOUSEHIDREPORTINTERCEPTOR_H
