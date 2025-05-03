// MouseInTheMiddle.h

#ifndef MOUSEINTHEMIDDLE_H
#define MOUSEINTHEMIDDLE_H

#include <hiduniversal.h>
#include "Mouse16Bit.h"
#include "USBMouseHIDReportInterceptor.h"
#include "SerialMouseHIDReportInterceptor.h"
#include "MouseEventSpoofer.h"
#include "EdianConversions.h"

//extern EdianConversions edianConversions;

USB Usb;
HIDUniversal Hid(&Usb);
USBMouseHIDReportInterceptor usbInterceptor;
SerialMouseHIDReportInterceptor serialInterceptor;
MouseEventSpoofer mouseEventSpoofer(&usbInterceptor, &serialInterceptor);
EdianConversions edianConversions;


#endif // MOUSEINTHEMIDDLE_H
