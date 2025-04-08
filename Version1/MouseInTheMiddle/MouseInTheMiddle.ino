#include "MouseInTheMiddle.h"

void setup() {
	Mouse.begin();

	Serial1.begin(2000000);
	Serial1.println("MSG: Arduino Leonardo Started.");

	if (Usb.Init() == -1)
		Serial1.println("MSG: Host Shield did not start.");

  Serial1.println("MSG: USB Host Shield Started.");
	
	delay(200);

	if (!Hid.SetReportParser(0, &usbInterceptor))
		ErrorMessage<uint8_t > (PSTR("SetReportParser"), 1);
}

void loop() {
    serialInterceptor.sniffForSpoofableDataOverSerial();
  	Usb.Task();
  	mouseEventSpoofer.spoofEvent();
}