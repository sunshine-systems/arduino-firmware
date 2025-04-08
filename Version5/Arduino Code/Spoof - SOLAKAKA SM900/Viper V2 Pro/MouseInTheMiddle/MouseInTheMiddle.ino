#include "MouseInTheMiddle.h"
#include "Config.h"

void setup() {
  FTDI_DEVICE.begin(BAUD); // Initialize serial communication
  FTDI_DEVICE.setTimeout(1); // Set reading timeout to 1ms
  FTDI_DEVICE.println("S: Arduino Leonardo Started.");

  // Initialize USB Host Shield
  if (Usb.Init() == -1) {
    FTDI_DEVICE.println("E: Host Shield did not start.");
  } else {
    FTDI_DEVICE.println("S: USB Host Shield Started.");
  }

  delay(200);

  // Set the report parser for the USB Host Shield
  if (!Hid.SetReportParser(0, &usbInterceptor)) {
    ErrorMessage<uint8_t >(PSTR("SetReportParser"), 1);
  } else {
    FTDI_DEVICE.println("S: ReportParser set successfully.");
  }

  delay(500);
}

void loop() {
  Usb.Task(); // Regularly process USB tasks
  serialInterceptor.sniffForSpoofableDataOverSerial();
  mouseEventSpoofer.spoofEvent();
}