#include "CompositeHID.h"

#if defined(USBCON)
#define REPORT_ID_CONSUMER_CONTROL 2

// Mouse Report Descriptor
static const uint8_t _mouseReportDescriptor[] PROGMEM = {
    // Mouse
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x02,        // Usage (Mouse)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    0x09, 0x01,        //   Usage (Pointer)
    0xA1, 0x00,        //   Collection (Physical)
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (Button 1)
    0x29, 0x05,        //     Usage Maximum (Button 5)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x95, 0x05,        //     Report Count (5)
    0x75, 0x01,        //     Report Size (1)
    0x81, 0x02,        //     Input (Data, Variable, Absolute)
    0x95, 0x01,        //     Report Count (1)
    0x75, 0x03,        //     Report Size (3)
    0x81, 0x01,        //     Input (Constant)
    0x05, 0x01,        //     Usage Page (Generic Desktop)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x16, 0x01, 0x80,  //     Logical Minimum (-32767)
    0x26, 0xFF, 0x7F,  //     Logical Maximum (32767)
    0x75, 0x10,        //     Report Size (16)
    0x95, 0x02,        //     Report Count (2)
    0x81, 0x06,        //     Input (Data, Variable, Relative)
    0x09, 0x38,        //     Usage (Wheel)
    0x15, 0x81,        //     Logical Minimum (-127)
    0x25, 0x7F,        //     Logical Maximum (127)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x06,        //     Input (Data, Variable, Relative)
    0xC0,              //   End Collection
    0xC0               // End Collection
};

// Keyboard Report Descriptor
static const uint8_t _keyboardReportDescriptor[] PROGMEM = {
    // Keyboard
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x03,        //   Report ID (3)
    0x05, 0x07,        //   Usage Page (Key Codes)
    0x19, 0xE0,        //   Usage Minimum (224)
    0x29, 0xE7,        //   Usage Maximum (231)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x08,        //   Report Count (8)
    0x81, 0x02,        //   Input (Data, Variable, Absolute)
    0x95, 0x06,        //   Report Count (6)
    0x75, 0x08,        //   Report Size (8)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x65,        //   Logical Maximum (101)
    0x05, 0x07,        //   Usage Page (Key Codes)
    0x19, 0x00,        //   Usage Minimum (0)
    0x29, 0x65,        //   Usage Maximum (101)
    0x81, 0x00,        //   Input (Data, Array)
    0xC0               // End Collection
};

// Consumer Control Report Descriptor
static const uint8_t _consumerControlReportDescriptor[] PROGMEM = {
    0x05, 0x0C,        // Usage Page (Consumer)
    0x09, 0x01,        // Usage (Consumer Control)
    0xA1, 0x01,        // Collection (Application)
    0x85, REPORT_ID_CONSUMER_CONTROL,  // Report ID (2)
    0x15, 0x00,        // Logical Minimum (0)
    0x26, 0x9C, 0x02,  // Logical Maximum (668)
    0x19, 0x00,        // Usage Minimum (0)
    0x2A, 0x9C, 0x02,  // Usage Maximum (668)
    0x75, 0x10,        // Report Size (16)
    0x95, 0x01,        // Report Count (1)
    0x81, 0x00,        // Input (Data, Array)
    0xC0               // End Collection
};

// Vendor-Specific Descriptor
static const uint8_t _vendorReportDescriptor1[] PROGMEM = {
    0x06, 0x00, 0xFF,      // Usage Page (Vendor Defined 65280)
    0x09, 0x01,            // Usage (Vendor Usage 1)
    0xA1, 0x01,            // Collection (Application)
    0x85, 0x06,            //   Report ID (6)
    0x15, 0x00,            //   Logical Minimum (0)
    0x26, 0xFF, 0x00,      //   Logical Maximum (255)
    0x75, 0x08,            //   Report Size (8)
    0x95, 0x02,            //   Report Count (2)
    0x09, 0x2F,            //   Usage (Vendor Usage 47)
    0x81, 0x02,            //   Input (Data, Variable, Absolute)
    0xC0                   // End Collection
};

// System Control Descriptor
static const uint8_t _systemControlReportDescriptor[] PROGMEM = {
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x80,        // Usage (System Control)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x08,        //   Report ID (8)
    0x09, 0x81,        //   Usage (System Power Down)
    0x09, 0x82,        //   Usage (System Sleep)
    0x09, 0x83,        //   Usage (System Wake Up)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x03,        //   Report Count (3)
    0x81, 0x02,        //   Input (Data, Variable, Absolute)
    0x95, 0x05,        //   Report Count (5)
    0x81, 0x03,        //   Input (Constant, Array, Absolute)
    0xC0               // End Collection
};

CompositeHID_::CompositeHID_(void) : PluggableUSBModule(2, 2, epType) {  // Changed to 2 endpoints
    epType[0] = EP_TYPE_INTERRUPT_IN;
    epType[1] = EP_TYPE_INTERRUPT_IN;
    protocol = HID_REPORT_PROTOCOL;
    idle = 1;
    
    mouseButtons = 0;
    keyboardModifiers = 0;
    memset(keyboardKeys, 0, sizeof(keyboardKeys));
    
    PluggableUSB().plug(this);
}

int CompositeHID_::begin(void) {
    return 0;
}

void CompositeHID_::SendReport(uint8_t id, const void* data, int len) {
    uint8_t ep = (id == REPORT_ID_MOUSE) ? pluggedEndpoint : (pluggedEndpoint + 1);

    auto ret = USB_Send(ep, &id, 1);
    if (ret < 0) {
        return;
    }
    
    ret = USB_Send(ep | TRANSFER_RELEASE, data, len);
}


void CompositeHID_::buttons(uint8_t b) {
    if (b != mouseButtons) {
        mouseButtons = b;
        move(0, 0, 0);
    }
}

void CompositeHID_::move(int16_t x, int16_t y, signed char wheel) {
    uint8_t m[6];
    m[0] = mouseButtons;
    m[1] = x & 0xFF;
    m[2] = static_cast<uint8_t>(x >> 8);
    m[3] = y & 0xFF;
    m[4] = static_cast<uint8_t>(y >> 8);
    m[5] = wheel;
    SendReport(REPORT_ID_MOUSE, m, 6);
}

void CompositeHID_::sendKeyboard(uint8_t modifiers, uint8_t keycode) {
    uint8_t report[8];
    report[0] = modifiers;
    report[1] = 0; // Reserved
    report[2] = keycode;
    report[3] = 0;
    report[4] = 0;
    report[5] = 0;
    report[6] = 0;
    report[7] = 0;
    SendReport(REPORT_ID_KEYBOARD, report, 8);
}

void CompositeHID_::sendConsumerControl(uint16_t usage) {
    uint8_t report[3];
    report[0] = REPORT_ID_CONSUMER_CONTROL;
    report[1] = usage & 0xFF;
    report[2] = (usage >> 8) & 0xFF;
    SendReport(REPORT_ID_CONSUMER_CONTROL, report, sizeof(report));
}

void CompositeHID_::press(uint8_t button) {
    buttons(mouseButtons | button);
}

void CompositeHID_::release(uint8_t button) {
    buttons(mouseButtons & ~button);
}

void CompositeHID_::click(uint8_t button) {
    press(button);
    release(button);
}

bool CompositeHID_::isPressed(uint8_t button) {
    return (mouseButtons & button) != 0;
}

int CompositeHID_::getInterface(uint8_t* interfaceCount) {
    uint8_t currentInterface = *interfaceCount;
    *interfaceCount += 2;  // Two interfaces: one for mouse, one for combined HID

    // Mouse Interface
    HIDDescriptor mouseInterface = {
        D_INTERFACE(currentInterface, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_BOOT_INTERFACE, HID_PROTOCOL_MOUSE),
        D_HIDREPORT(sizeof(_mouseReportDescriptor)),
        D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01)
    };
    int res = USB_SendControl(0, &mouseInterface, sizeof(mouseInterface));
    if (res < 0) {
        return res;
    }

    // Combined Keyboard, Consumer Control, Vendor-Specific, and System Control Interface
    HIDDescriptor combinedInterface = {
        D_INTERFACE(currentInterface + 1, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_BOOT_INTERFACE, HID_PROTOCOL_KEYBOARD),
        D_HIDREPORT(sizeof(_keyboardReportDescriptor) 
                    + sizeof(_consumerControlReportDescriptor)
                    + sizeof(_vendorReportDescriptor1)
                    + sizeof(_systemControlReportDescriptor)),
        D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint + 1), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01)
    };
    res = USB_SendControl(0, &combinedInterface, sizeof(combinedInterface));
    return res;
}

int CompositeHID_::getDescriptor(USBSetup& setup) {
    if (setup.bmRequestType != REQUEST_DEVICETOHOST_STANDARD_INTERFACE) return 0;
    if (setup.wValueH != HID_REPORT_DESCRIPTOR_TYPE) return 0;

    if (setup.wIndex == pluggedInterface) {
        // Mouse descriptor
        return USB_SendControl(TRANSFER_PGM, _mouseReportDescriptor, sizeof(_mouseReportDescriptor));
    } 
    else if (setup.wIndex == pluggedInterface + 1) {
        // Combined descriptors for Keyboard, Consumer Control, Vendor-Specific, and System Control
        USB_SendControl(TRANSFER_PGM, _keyboardReportDescriptor, sizeof(_keyboardReportDescriptor));
        USB_SendControl(TRANSFER_PGM, _consumerControlReportDescriptor, sizeof(_consumerControlReportDescriptor));
        USB_SendControl(TRANSFER_PGM, _vendorReportDescriptor1, sizeof(_vendorReportDescriptor1));
        return USB_SendControl(TRANSFER_PGM, _systemControlReportDescriptor, sizeof(_systemControlReportDescriptor));
    }

    return 0;
}

bool CompositeHID_::setup(USBSetup& setup) {
    uint8_t r = setup.bRequest;
    uint8_t requestType = setup.bmRequestType;

    if (requestType == REQUEST_DEVICETOHOST_CLASS_INTERFACE) {
        if (r == HID_GET_REPORT) {
            // TODO: HID GET_REPORT
            return true;
        }
        if (r == HID_GET_PROTOCOL) {
            // TODO: HID GET_PROTOCOL
            return true;
        }
    }
    
    if (requestType == REQUEST_HOSTTODEVICE_CLASS_INTERFACE) {
        if (r == HID_SET_PROTOCOL) {
            protocol = setup.wValueL;
            return true;
        }
        if (r == HID_SET_IDLE) {
            idle = setup.wValueL;
            return true;
        }
        if (r == HID_SET_REPORT) {
            // TODO: HID SET_REPORT
            return true;
        }
    }
    
    return false;
}

uint8_t CompositeHID_::getShortName(char *name) {
    name[0] = 'H';
    name[1] = 'I';
    name[2] = 'D';
    return 3;
}

CompositeHID_ CompositeHID;

#endif /* if defined(USBCON) */