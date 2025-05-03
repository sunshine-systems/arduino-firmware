// Mouse16Bit.cpp
#include "Mouse16Bit.h"

#if defined(_USING_HID)

static const uint8_t _hidReportDescriptor[] PROGMEM = {
    0x05, 0x01,       /* Usage Page (Generic Desktop),       */
    0x09, 0x02,       /* Usage (Mouse),                      */
    0xA1, 0x01,       /*  Collection (Application),          */
    0x85, 0x01,       /*     Report Id 1  Mouse Motion       */
    0x09, 0x01,       /*   Usage (Pointer),                  */
    0xA1, 0x00,       /*  Collection (Physical),             */
    0x05, 0x09,       /*     Usage Page (Buttons),           */
    0x19, 0x01,       /*     Usage Minimum (01),             */
    0x29, 0x05,       /*     Usage Maximum (03),  5 buttons  */
    0x15, 0x00,       /*     Logical Minimum (0),            */
    0x25, 0x01,       /*     Logical Maximum (1),            */
    0x75, 0x01,       /*     Report Size (1),                */
    0x95, 0x05,       /*     Report Count (5),               */
    0x81, 0x02,       /*     Input (Data, Variable, Absolute)*/
    0x75, 0x03,       /*     Report Size (3), 3 bit padding  */
    0x95, 0x01,       /*     Report Count (1),               */
    0x81, 0x01,       /*     Input (Constant),               */
    0x05, 0x01,       /*     Usage Page (Generic Desktop),   */
    0x09, 0x30,       /*     Usage (X),                      */
    0x09, 0x31,       /*     Usage (Y),                      */
    0x16, 0x01, 0x80, /*     Logical Minimum (-32767),       */
    0x26, 0xFF, 0x7F, /*     Logical Maximum (32767),        */
    0x75, 0x10,       /*     Report Size (16),               */
    0x95, 0x02,       /*     Report Count (2),               */
    0x81, 0x06,       /*     Input (Data, Variable, Relative)*/
    0x09, 0x38,       /*     Usage (Scroll),                 */
    0x15, 0x81,       /*     Logical Minimum(-127)           */
    0x25, 0x7F,       /*     Logical Maximum(127)            */
    0x75, 0x08,       /*     Report Size(8)                  */
    0x95, 0x01,       /*     Report Count(1)                 */
    0x81, 0x06,       /*     Input(Data,Var,Rel)             */
    0xC0,             /*  End Collection,                    */
    0xC0,             /* End Collection                      */
};

//================================================================================
//================================================================================
//	Mouse

Mouse_::Mouse_(void) : _buttons(0)
{
    static HIDSubDescriptor node(_hidReportDescriptor, sizeof(_hidReportDescriptor));
    HID().AppendDescriptor(&node);
}

void Mouse_::begin(void)
{
}

void Mouse_::end(void)
{
}

void Mouse_::click(uint8_t b)
{
    _buttons = b;
    move(0, 0, 0);
    _buttons = 0;
    move(0, 0, 0);
}

void Mouse_::move(int16_t x, int16_t y, signed char wheel)
{
    uint8_t m[6];
    m[0] = _buttons;                    // Button states
    m[1] = x & 0xFF;                    // Lower 8 bits of X movement
    m[2] = static_cast<uint8_t>(x >> 8); // Upper 8 bits of X movement (MSB)
    m[3] = y & 0xFF;                    // Lower 8 bits of Y movement
    m[4] = static_cast<uint8_t>(y >> 8); // Upper 8 bits of Y movement (MSB)
    m[5] = wheel;                       // Wheel value

    HID().SendReport(1, m, 6);
}



void Mouse_::buttons(uint8_t b)
{
    if (b != _buttons)
    {
        _buttons = b;
        move(0, 0, 0);
    }
}

void Mouse_::press(uint8_t b)
{
    buttons(_buttons | b);
}

void Mouse_::release(uint8_t b)
{
    buttons(_buttons & ~b);
}

bool Mouse_::isPressed(uint8_t b)
{
    if ((b & _buttons) > 0)
        return true;
    return false;
}

Mouse_ Mouse;

#endif
