#include <hiduniversal.h>
#include "mouse16bit.h"
#include "hidmouserptparser.h"
#include "serialcommands.h"
#include "RazerViperV2Pro16BitDataParser.h"

USB Usb;
HIDUniversal Hid(&Usb);
HIDMouseReportParser Mou(nullptr);
RazerViperV2Pro16BitDataParser razerViperDataParser;

// Define the callback function
void processSerialData(uint8_t *buf, uint8_t size) {

//        Serial1.print("MSG: Arduino processSerialData() -> ");
//        for (uint8_t i = 0; i < size; i++) {
//          Serial1.print(' ');
//          Serial1.print(buf[i], HEX);
//        }
//        Serial1.println(' ');

        // uint8_t spoofedHidReport = razerViperDataParser.formatDataIntoHIDReport(buf);
        Mou.Parse(&Hid, true, size, buf);

}

SerialCommands serialCommands(processSerialData);

void setup() {
	Mouse.begin();

	Serial1.begin(115200);
	Serial1.println("MSG: Arduino Leonardo Started.");

	if (Usb.Init() == -1)
		Serial1.println("MSG: Host Shield did not start.");

  Serial1.println("MSG: USB Host Shield Started.");
	
	delay(200);

	if (!Hid.SetReportParser(0, &Mou))
		ErrorMessage<uint8_t > (PSTR("SetReportParser"), 1);
}

void loop() {
    serialCommands.processInput();
  	Usb.Task();
}

void onButtonDown(uint16_t buttonId) {
  	Mouse.press(buttonId);
 	Serial1.print("MSG: Button ");
 	switch (buttonId) {
		case MOUSE_LEFT:
			Serial1.print("MOUSE_LEFT");
			break;

		case MOUSE_RIGHT:
			Serial1.print("MOUSE_RIGHT");
			break;

		case MOUSE_MIDDLE:
			Serial1.print("MOUSE_MIDDLE");
			break;

		case MOUSE_BUTTON4:
			Serial1.print("MOUSE_BUTTON4");
			break;

		case MOUSE_BUTTON5:
			Serial1.print("MOUSE_BUTTON5");
			break;

		default:
			Serial1.print("OTHER_BUTTON");
			break;
}
	Serial1.println(" pressed");
}

void onButtonUp(uint16_t buttonId) {
	Mouse.release(buttonId);
	Serial1.print("MSG: Button ");
	switch (buttonId) {
		case MOUSE_LEFT:
			Serial1.print("MOUSE_LEFT");
			break;

		case MOUSE_RIGHT:
			Serial1.print("MOUSE_RIGHT");
			break;

		case MOUSE_MIDDLE:
			Serial1.print("MOUSE_MIDDLE");
			break;

		case MOUSE_BUTTON4:
			Serial1.print("MOUSE_BUTTON4");
			break;

		case MOUSE_BUTTON5:
			Serial1.print("MOUSE_BUTTON5");
			break;
		default:
			Serial1.print("OTHER_BUTTON");
			break;
	}
	Serial1.println(" released");
}

void onMouseMove(int16_t xMovement, int16_t yMovement, int8_t scrollValue) {
  Mouse.move(xMovement, yMovement, scrollValue);
}
