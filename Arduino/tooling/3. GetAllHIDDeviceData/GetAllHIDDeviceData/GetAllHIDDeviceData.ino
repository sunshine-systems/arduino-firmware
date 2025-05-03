#include <usbhub.h>
#include <hidcomposite.h>
#include <usbhid.h>
#include <SPI.h>

USB Usb;

// Subclass of HIDComposite with the SelectInterface method implemented
class CustomHIDComposite : public HIDComposite {
public:
    uint8_t selectedInterface;
    uint8_t selectedProtocol;

    CustomHIDComposite(USB *p) : HIDComposite(p), selectedInterface(0), selectedProtocol(0) {}

    // Implement the pure virtual function from HIDComposite
    bool SelectInterface(uint8_t iface, uint8_t proto) override {
        // We only care about Interface 2 and Protocol 2
        if (iface == 2 && proto == 2) {
            Serial1.print("Selecting Interface ");
            Serial1.print(iface);
            Serial1.print(", Protocol ");
            Serial1.println(proto);

            // Store selected interface and protocol
            selectedInterface = iface;
            selectedProtocol = proto;

            return true;  // Accept this interface and protocol
        }
        return false;  // Reject other interfaces
    }
};

CustomHIDComposite HidComposite(&Usb);

// Custom parser class that implements HIDReportParser
class MyMouseParser : public HIDReportParser {
public:
    CustomHIDComposite* hidComposite;

    // Constructor to initialize the parser with HIDComposite reference
    MyMouseParser(CustomHIDComposite* composite) : hidComposite(composite) {}

    // Override the Parse function from HIDReportParser
    void Parse(USBHID* hid, bool is_rpt_id, uint8_t len, uint8_t* buf) override {
        // Log only when from Interface 2 and Protocol 2
        if (hidComposite->selectedInterface == 2 && hidComposite->selectedProtocol == 2) {
            Serial1.print("Raw Data from Interface ");
            Serial1.print(hidComposite->selectedInterface);
            Serial1.print(", Protocol ");
            Serial1.print(hidComposite->selectedProtocol);
            Serial1.print(": ");
            for (uint8_t i = 0; i < len; i++) {
                Serial1.print(buf[i], HEX);
                Serial1.print(" ");
            }
            Serial1.println();
        }
    }
};

MyMouseParser mouseParser(&HidComposite);

void setup() {
    Serial1.begin(115200);
    Serial1.println("MSG: Arduino Leonardo Started.");

    // Initialize USB host shield
    if (Usb.Init() == -1) {
        Serial1.println("MSG: USB Host Shield did not start.");
        while (1);  // Halt execution if USB initialization fails
    }

    Serial1.println("MSG: USB Host Shield Initialized.");

    // Set the report parser only for Interface 2, Protocol 2
    if (HidComposite.SetReportParser(2, &mouseParser)) {
        Serial1.println("MSG: Report parser set for Interface 2, Protocol 2.");
    } else {
        Serial1.println("Error: Failed to set report parser for Interface 2, Protocol 2.");
    }
}

void loop() {
    Usb.Task();  // Perform USB tasks and read HID reports
}
