#include "MouseInterfaceFinder.h"

// Array to store detected mouse interfaces
MouseInterfaceInfo mouseInterfaces[MAX_MOUSE_INTERFACES];
uint8_t mouseInterfaceCount = 0;
static uint8_t currentInterface = 0; // Track which interface we're currently processing

void PrintDescriptors(uint8_t addr) {
    uint8_t rcode = 0;
    uint8_t num_conf = 0;

    rcode = getdevdescr(addr, num_conf);
    if (rcode) {
        Serial1.print(F("\r\nError getting device descriptor: "));
        Serial1.println(rcode, HEX);
        return;
    }

    for (uint8_t i = 0; i < num_conf; i++) {
        rcode = getconfdescr(addr, i);
        if (rcode) {
            Serial1.print(F("\r\nError getting config descriptor: "));
            Serial1.println(rcode, HEX);
        }
    }
}

uint8_t getdevdescr(uint8_t addr, uint8_t &num_conf) {
    USB_DEVICE_DESCRIPTOR buf;
    uint8_t rcode;

    rcode = Usb.getDevDescr(addr, 0, 0x12, (uint8_t *)&buf);
    if (rcode) {
        return rcode;
    }

    Serial1.println(F("\r\n*** Device Descriptor ***"));
    Serial1.print(F("VID: 0x"));
    Serial1.println(buf.idVendor, HEX);
    Serial1.print(F("PID: 0x"));
    Serial1.println(buf.idProduct, HEX);
    Serial1.print(F("Class: 0x"));
    Serial1.println(buf.bDeviceClass, HEX);
    Serial1.print(F("Subclass: 0x"));
    Serial1.println(buf.bDeviceSubClass, HEX);
    Serial1.print(F("Protocol: 0x"));
    Serial1.println(buf.bDeviceProtocol, HEX);

    num_conf = buf.bNumConfigurations;
    return 0;
}

uint8_t getconfdescr(uint8_t addr, uint8_t conf) {
    uint8_t buf[512];
    uint8_t* buf_ptr = buf;
    uint8_t rcode;

    // Retrieve configuration descriptor header
    rcode = Usb.getConfDescr(addr, 0, 4, conf, buf);
    if (rcode) {
        Serial1.print(F("Error retrieving configuration descriptor: "));
        Serial1.println(rcode, HEX);
        return rcode;
    }

    uint16_t total_length = buf[2] | (buf[3] << 8);
    if (total_length > 512) {
        Serial1.println(F("Config descriptor too long - truncating"));
        total_length = 512;
    }

    // Retrieve the full configuration descriptor
    rcode = Usb.getConfDescr(addr, 0, total_length, conf, buf);
    if (rcode) {
        Serial1.print(F("Error retrieving full configuration descriptor: "));
        Serial1.println(rcode, HEX);
    }

    // Parse descriptors
    while (buf_ptr < buf + total_length) {
        uint8_t descr_length = *buf_ptr;
        uint8_t descr_type = *(buf_ptr + 1);

        switch (descr_type) {
            case USB_DESCRIPTOR_INTERFACE: {
                USB_INTERFACE_DESCRIPTOR* intf_ptr = (USB_INTERFACE_DESCRIPTOR*)buf_ptr;
                Serial1.println(F("\n*** Interface Descriptor ***"));
                Serial1.print(F("  Number: "));
                Serial1.println(intf_ptr->bInterfaceNumber);
                Serial1.print(F("  Class: 0x"));
                Serial1.println(intf_ptr->bInterfaceClass, HEX);
                Serial1.print(F("  Subclass: 0x"));
                Serial1.println(intf_ptr->bInterfaceSubClass, HEX);
                Serial1.print(F("  Protocol: 0x"));
                Serial1.println(intf_ptr->bInterfaceProtocol, HEX);
                Serial1.print(F("  Endpoints: "));
                Serial1.println(intf_ptr->bNumEndpoints);

                // Store the current interface number globally to track which interface we're processing
                currentInterface = intf_ptr->bInterfaceNumber;

                // Check if this is a mouse interface
                if (intf_ptr->bInterfaceClass == 0x03 && 
                    intf_ptr->bInterfaceSubClass == 0x01 &&
                    intf_ptr->bInterfaceProtocol == 0x02) {
                    if (mouseInterfaceCount < MAX_MOUSE_INTERFACES) {
                        mouseInterfaces[mouseInterfaceCount].interfaceNumber = intf_ptr->bInterfaceNumber;
                        mouseInterfaces[mouseInterfaceCount].protocol = intf_ptr->bInterfaceProtocol;
                        mouseInterfaces[mouseInterfaceCount].endpointAddress = 0; // Set later
                        mouseInterfaceCount++;
                    }
                }
                break;
            }
            case USB_DESCRIPTOR_ENDPOINT: {
                USB_ENDPOINT_DESCRIPTOR* ep_ptr = (USB_ENDPOINT_DESCRIPTOR*)buf_ptr;
                Serial1.println(F("    *** Endpoint Descriptor ***"));
                Serial1.print(F("      Address: 0x"));
                Serial1.println(ep_ptr->bEndpointAddress, HEX);
                Serial1.print(F("      Attributes: 0x"));
                Serial1.println(ep_ptr->bmAttributes, HEX);
                Serial1.print(F("      Max Packet Size: "));
                Serial1.println(ep_ptr->wMaxPacketSize);

                // Update endpoint address for the correct interface
                for (uint8_t i = 0; i < mouseInterfaceCount; i++) {
                    if (mouseInterfaces[i].interfaceNumber == currentInterface) {
                        mouseInterfaces[i].endpointAddress = ep_ptr->bEndpointAddress;
                        break;
                    }
                }
                break;
            }
            default:
                break;
        }

        buf_ptr += descr_length;
        if (descr_length == 0) {
            Serial1.println(F("Invalid descriptor length encountered."));
            break;
        }
    }

    return 0;
}

void PrintMouseInterfaces() {
    if (mouseInterfaceCount == 0) {
        Serial1.println(F("No mouse interface found."));
    } else {
        Serial1.println(F("\nDetected Mouse Interfaces:"));
        for (uint8_t i = 0; i < mouseInterfaceCount; i++) {
            Serial1.print(F("Interface: "));
            Serial1.print(mouseInterfaces[i].interfaceNumber);
            Serial1.print(F(", Protocol: "));
            Serial1.print(mouseInterfaces[i].protocol);
            Serial1.print(F(", Endpoint: 0x"));
            Serial1.println(mouseInterfaces[i].endpointAddress, HEX);
        }
    }
}
