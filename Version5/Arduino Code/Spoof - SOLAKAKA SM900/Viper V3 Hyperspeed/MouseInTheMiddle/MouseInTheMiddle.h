// MouseInTheMiddle.h

#ifndef MOUSEINTHEMIDDLE_H
#define MOUSEINTHEMIDDLE_H

#include <hiduniversal.h>
#include "CompositeHID.h"
#include "USBMouseHIDReportInterceptor.h"
#include "SerialMouseHIDReportInterceptor.h"
#include "MouseEventSpoofer.h"
#include "MathAndConversions.h"
#include "FirmwareSettings.h"

//extern MathAndConversions mathAndConversions;

USB Usb;
HIDUniversal Hid(&Usb);
USBMouseHIDReportInterceptor usbInterceptor;
FirmwareSettings firmwareSettings;
SerialMouseHIDReportInterceptor serialInterceptor(&firmwareSettings);
MouseEventSpoofer mouseEventSpoofer(&usbInterceptor, &serialInterceptor);
MathAndConversions mathAndConversions;

#endif // MOUSEINTHEMIDDLE_H