#ifndef MOUSEINTHEMIDDLE_H
#define MOUSEINTHEMIDDLE_H

#include "Config.h"  // Includes the extern declarations for Usb and Hid
#include "Mouse16Bit.h"
#include "USBMouseHIDReportInterceptor.h"
#include "SerialMouseHIDReportInterceptor.h"
#include "MouseEventSpoofer.h"
#include "MathAndConversions.h"
#include "FirmwareSettings.h"

// No need to redefine Usb and Hid here, they are extern in Config.h

extern USBMouseHIDReportInterceptor usbInterceptor;
extern FirmwareSettings firmwareSettings;
extern SerialMouseHIDReportInterceptor serialInterceptor;
extern MouseEventSpoofer mouseEventSpoofer;
extern MathAndConversions mathAndConversions;

#endif // MOUSEINTHEMIDDLE_H
