//FindInterfaces.ino
#include "MouseInterfaceFinder.h"
#include "AutoMouseHID.h"

// Global objects
USB Usb;
AutoMouseHIDParser Parser;
HIDBoot<USB_HID_PROTOCOL_MOUSE> HidMouse(&Usb);

void setup() {
    Serial1.begin(115200);
    Serial1.println(F("Auto Mouse Reader Starting..."));

    if (Usb.Init() == -1) {
        Serial1.println(F("USB Host Shield initialization failed"));
        while (1);
    }

    Serial1.println(F("Waiting for device..."));
    delay(200);
}

void loop() {
    static bool initialized = false;
    static bool parserSet = false;

    Usb.Task();

    if (Usb.getUsbTaskState() == USB_STATE_RUNNING) {
        if (!initialized) {
            PrintDescriptors(1);  // Print all descriptors
            PrintMouseInterfaces(); // Print detected mouse interfaces
            initialized = true;
        }
        
        if (!parserSet && mouseInterfaceCount > 0) {
            // Set the report parser using the detected interface number
            if (HidMouse.SetReportParser(0, &Parser)) {
                Serial1.print(F("Report parser set successfully for Interface "));
                Serial1.println(mouseInterfaces[0].interfaceNumber);
                parserSet = true;
            } else {
                Serial1.println(F("Failed to set report parser"));
            }
        }
    } else if (!initialized) {
        Serial1.println(F("Waiting for USB device..."));
        delay(500);
    }
}