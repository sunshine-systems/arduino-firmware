// SerialMouseHIDReportInterceptor.h
#ifndef SERIALMOUSEHIDREPORTINTERCEPTOR_H
#define SERIALMOUSEHIDREPORTINTERCEPTOR_H

#include <Arduino.h>

class SerialMouseHIDReportInterceptor {
    private:
        uint8_t previousMouseButtons;
        uint8_t mouseButtons;
        uint8_t scrollWheel;
        int16_t xMovement;
        int16_t yMovement;

        void setPreviousMouseButtons(uint8_t buttons) {previousMouseButtons = buttons; };
        void setMouseButtons(uint8_t buttons) { mouseButtons = buttons; };
        void setScrollWheel(uint8_t wheel) { scrollWheel = wheel; };
        void setXMovement(int16_t x) { xMovement = x; };
        void setYMovement(int16_t y) { yMovement = y; };

        bool isDataAvailable;

        void processAndSetHIDReportData(const uint8_t* data);
        void updateSettings(const uint8_t* data);

    public:
        SerialMouseHIDReportInterceptor();

        void sniffForSpoofableDataOverSerial();
        bool hasData();
        void reset(); // used to prevent "mouse movement ghosting"

        uint8_t getPreviousMouseButtons() const { return previousMouseButtons; };
        uint8_t getMouseButtons() const { return mouseButtons; };
        uint8_t getScrollWheel() const { return scrollWheel; };
        int16_t getXMovement() const { return xMovement; };
        int16_t getYMovement() const { return yMovement; };

};

#endif // SERIALMOUSEHIDREPORTINTERCEPTOR_H
