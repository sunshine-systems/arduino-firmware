#ifndef COMPOSITE_HID_h
#define COMPOSITE_HID_h

#include "HID.h"

#if defined(USBCON)

#define _USING_HID

// Report IDs
#define REPORT_ID_MOUSE     1
#define REPORT_ID_KEYBOARD  3

// Mouse button definitions
#define MOUSE_LEFT    0x01
#define MOUSE_RIGHT   0x02
#define MOUSE_MIDDLE  0x04
#define MOUSE_BACK    0x08
#define MOUSE_FORWARD 0x10
#define MOUSE_ALL     (MOUSE_LEFT | MOUSE_RIGHT | MOUSE_MIDDLE | MOUSE_BACK | MOUSE_FORWARD)

class CompositeHID_ : public PluggableUSBModule {
public:
    CompositeHID_(void);
    int begin(void);
    
    // Mouse Functions
    void move(int16_t x, int16_t y, signed char wheel = 0);
    void click(uint8_t b = MOUSE_LEFT);
    void press(uint8_t b = MOUSE_LEFT);
    void release(uint8_t b = MOUSE_LEFT);
    bool isPressed(uint8_t b = MOUSE_LEFT);

    // Keyboard Functions
    void sendKeyboard(uint8_t modifiers, uint8_t keycode);
    void sendConsumerControl(uint16_t usage);

protected:
    virtual bool setup(USBSetup& setup);
    virtual int getInterface(uint8_t* interfaceCount);
    virtual int getDescriptor(USBSetup& setup);
    virtual uint8_t getShortName(char* name);

private:
    uint8_t epType[2];     // Need two endpoints
    uint8_t protocol;
    uint8_t idle;
    
    uint8_t mouseButtons;
    uint8_t keyboardModifiers;
    uint8_t keyboardKeys[6];
    
    void SendReport(uint8_t id, const void* data, int len);
    void buttons(uint8_t b);
    
};

extern CompositeHID_ CompositeHID;

#endif
#endif