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

	Serial.begin(115200);
	Serial.println("MSG: Arduino Leonardo Started.");

	if (Usb.Init() == -1)
		Serial.println("MSG: Host Shield did not start.");

  Serial.println("MSG: USB Host Shield Started.");
	
	delay(200);

	if (!Hid.SetReportParser(0, &Mou))
		ErrorMessage<uint8_t > (PSTR("SetReportParser"), 1);
}

void loop() {
  	Usb.Task();
}

void onButtonDown(uint16_t buttonId) {
	Mouse.press(buttonId);
	Serial.print("MSG: Button ");
	switch (buttonId) {
			case MOUSE_LEFT:
				Serial.print("MOUSE_LEFT");
				break;
			case MOUSE_RIGHT:
				Serial.print("MOUSE_RIGHT");
				break;
			case MOUSE_MIDDLE:
				Serial.print("MOUSE_MIDDLE");
				break;
			case MOUSE_BUTTON4:
				Serial.print("MOUSE_BUTTON4");
				break;
			case MOUSE_BUTTON5:
				Serial.print("MOUSE_BUTTON5");
				break;
			default:
				Serial.print("OTHER_BUTTON");
				break;
		}
		Serial.println(" pressed");
}

void onButtonUp(uint16_t buttonId) {
	Mouse.release(buttonId);
	Serial.print("MSG: Button ");
	switch (buttonId) {
		case MOUSE_LEFT:
			Serial.print("MOUSE_LEFT");
			break;

		case MOUSE_RIGHT:
			Serial.print("MOUSE_RIGHT");
			break;

		case MOUSE_MIDDLE:
			Serial.print("MOUSE_MIDDLE");
			break;

		case MOUSE_BUTTON4:
			Serial.print("MOUSE_BUTTON4");
			break;

		case MOUSE_BUTTON5:
			Serial.print("MOUSE_BUTTON5");
			break;
		default:
			Serial.print("OTHER_BUTTON");
			break;
	}
	Serial.println(" released");
}

void onMouseMove(int16_t xMovement, int16_t yMovement, int8_t scrollValue) {
  Mouse.move(xMovement, yMovement, scrollValue);
}
