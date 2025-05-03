// MouseInTheMiddle.h

#ifndef MOUSEINTHEMIDDLE_H
#define MOUSEINTHEMIDDLE_H

#include <usbhub.h>
#include <hiduniversal.h>
#include "Mouse16Bit.h"
#include "USBMouseHIDReportInterceptor.h"
#include "SerialMouseHIDReportInterceptor.h"
#include "MouseEventSpoofer.h"
#include "MathAndConversions.h"

//extern MathAndConversions mathAndConversions;

USB Usb;
HIDUniversal Hid(&Usb);
USBMouseHIDReportInterceptor usbInterceptor;
SerialMouseHIDReportInterceptor serialInterceptor;
MouseEventSpoofer mouseEventSpoofer(&usbInterceptor, &serialInterceptor);
MathAndConversions mathAndConversions;

#endif // MOUSEINTHEMIDDLE_H