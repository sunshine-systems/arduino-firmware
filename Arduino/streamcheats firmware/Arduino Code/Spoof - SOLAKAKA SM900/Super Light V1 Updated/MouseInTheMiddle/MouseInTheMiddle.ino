#include "MouseInTheMiddle.h"
#include "Config.h"
#include "SunBoxLogger.h"

void setup() {
  FTDI_DEVICE.begin(BAUD); // Initialize serial communication
  FTDI_DEVICE.setTimeout(1); // Set reading timeout to 1ms
  logger.begin();

  // Seed random() for the LMB/RMB button-handoff gap jitter.
  randomSeed(micros());

  logger.startup("Arduino Leonardo Started.");

  // Initialize USB Host Shield
  if (Usb.Init() == -1) {
    logger.error("Host Shield did not start.");
  } else {
    logger.startup("USB Host Shield Started.");
  }

  delay(200);

  // Set the report parser for the USB Host Shield
  if (!Hid.SetReportParser(0, &usbInterceptor)) {
    ErrorMessage<uint8_t >(PSTR("SetReportParser"), 1);
  } else {
    logger.startup("ReportParser set successfully.");
  }

  delay(500);
}

void loop() {
  Usb.Task(); // Regularly process USB tasks
  serialInterceptor.sniffForSpoofableDataOverSerial();
  mouseEventSpoofer.spoofEvent();
}
