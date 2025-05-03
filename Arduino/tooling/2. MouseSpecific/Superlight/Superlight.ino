#include <hiduniversal.h>
#include "mouse16bit.h"
#include "hidmouserptparser.h"
#include "RazerViperV2Pro16BitDataParser.h"

USB Usb;
HIDUniversal Hid(&Usb);
HIDMouseReportParser Mou(nullptr);
RazerViperV2Pro16BitDataParser razerViperDataParser;

void setup() {
	Mouse.begin();

	Serial1.begin(2000000);
	Serial1.println("MSG: Arduino Leonardo Started.");

	if (Usb.Init() == -1)
		Serial1.println("MSG: Host Shield did not start.");

  Serial1.println("MSG: USB Host Shield Started.");
	
	delay(200);

	if (!Hid.SetReportParser(0, &Mou))
		ErrorMessage<uint8_t > (PSTR("SetReportParser"), 1);
}

void loop() {
  	Usb.Task();
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
