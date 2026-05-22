#include "MouseInTheMiddle.h"
#include "Config.h"

void setup() {
    FTDI_DEVICE.begin(BAUD);  // Initialize serial communication
    FTDI_DEVICE.setTimeout(1);  // Set reading timeout to 1ms
    FTDI_DEVICE.println("S: Arduino Leonardo Started.");

    // Initialize USB Host Shield
    if (Usb.Init() == -1) {
        FTDI_DEVICE.println("E: Host Shield did not start.");
    } else {
        FTDI_DEVICE.println("S: USB Host Shield Started.");
    }

    delay(200);

    // Set the report parser for the selected interface and protocol
    if (Hid.SetReportParser(0, &usbInterceptor)) {
        FTDI_DEVICE.println("S: Report parser set for Interface 2.");
    } else {
        FTDI_DEVICE.println("E: Failed to set report parser for Interface 2.");
    }

    delay(500);
}

void loop() {
    Usb.Task();  // Regularly process USB tasks
    // Process serial data for spoofing
    serialInterceptor.sniffForSpoofableDataOverSerial();
    // Process mouse event spoofing
    mouseEventSpoofer.spoofEvent();
}
