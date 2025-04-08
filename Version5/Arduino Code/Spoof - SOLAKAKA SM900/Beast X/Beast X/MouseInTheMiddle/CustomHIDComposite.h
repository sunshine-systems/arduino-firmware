#ifndef CUSTOMHIDCOMPOSITE_H
#define CUSTOMHIDCOMPOSITE_H

#include <hidcomposite.h>

// Custom HIDComposite class to select the appropriate interface and protocol
class CustomHIDComposite : public HIDComposite {
public:
    uint8_t selectedInterface;
    uint8_t selectedProtocol;

    // Constructor
    CustomHIDComposite(USB *p) : HIDComposite(p), selectedInterface(0), selectedProtocol(0) {}

    // Implement the pure virtual function from HIDComposite
    bool SelectInterface(uint8_t iface, uint8_t proto) override {
        // Adjust iface and proto based on your mouse configuration
        if (iface == 1 && proto == 2) {  
            // // Print to serial which interface and protocol are selected
            // Serial1.print("I: Selecting Interface ");
            // Serial1.print(iface);
            // Serial1.print(", Protocol ");
            // Serial1.println(proto);

            // Store the selected interface and protocol
            selectedInterface = iface;
            selectedProtocol = proto;

            return true;  // Accept this interface and protocol
        }
        return false;  // Reject other interfaces and protocols
    }
};

#endif // CUSTOMHIDCOMPOSITE_H
