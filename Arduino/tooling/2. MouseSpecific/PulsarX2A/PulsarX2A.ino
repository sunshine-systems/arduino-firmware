#include <usbhub.h>
#include <hidcomposite.h>
#include "mouse16bit.h"
#include "hidmouserptparser.h"
#include "serialcommands.h"
#include "The16BitDataParser.h"

USB Usb;

// Subclass of HIDComposite to select the appropriate interface
class CustomHIDComposite : public HIDComposite {
public:
    uint8_t selectedInterface;
    uint8_t selectedProtocol;

    CustomHIDComposite(USB *p) : HIDComposite(p), selectedInterface(0), selectedProtocol(0) {}

    // Implement the pure virtual function from HIDComposite
    bool SelectInterface(uint8_t iface, uint8_t proto) override {
        // We are looking for the interface and protocol that provide the correct data.
        if (iface == 2 && proto == 2) { // Adjust iface and proto based on your mouse configuration
            Serial1.print("Selecting Interface ");
            Serial1.print(iface);
            Serial1.print(", Protocol ");
            Serial1.println(proto);

            // Store the selected interface and protocol
            selectedInterface = iface;
            selectedProtocol = proto;

            return true; // Accept this interface and protocol
        }
        return false; // Reject other interfaces and protocols
    }
};

CustomHIDComposite Hid(&Usb);  // Use CustomHIDComposite instead of HIDUniversal
HIDMouseReportParser Mou(nullptr);
The16BitDataParser that16BitDataParser;

// Define the callback function for processing serial data
void processSerialData(uint8_t *buf, uint8_t size) {
    Serial1.print("Processing Serial Data -> ");
    for (uint8_t i = 0; i < size; i++) {
        Serial1.print(buf[i], HEX);
        Serial1.print(" ");
    }
    Serial1.println();

    Mou.Parse(&Hid, true, size, buf);
}

SerialCommands serialCommands(processSerialData);

void setup() {
    Mouse.begin();  // Initialize the mouse functionality

    Serial1.begin(115200);
    Serial1.println("MSG: Arduino Leonardo Started.");

    // Initialize the USB host shield
    if (Usb.Init() == -1) {
        Serial1.println("MSG: Host Shield did not start.");
    } else {
        Serial1.println("MSG: Host Shield initialized successfully.");
    }

    Serial1.println("MSG: USB Host Shield Started.");
    delay(200);

    // Set the report parser for the correct interface and protocol
    if (!Hid.SetReportParser(2, &Mou)) { // Interface 2 is selected based on previous tests
        Serial1.println("MSG: Error: Failed to set report parser for interface 2.");
    } else {
        Serial1.println("MSG: Mouse report parser successfully set for interface 2.");
    }
}

void loop() {
    Usb.Task();  // Ensure this is called regularly to handle USB tasks
}

void onButtonDown(uint16_t buttonId) {
    Mouse.press(buttonId);
}

void onButtonUp(uint16_t buttonId) {
    Mouse.release(buttonId);
}

void onMouseMove(int16_t xMovement, int16_t yMovement, int8_t scrollValue) {
    Mouse.move(xMovement, yMovement, scrollValue);
}
