#include "Config.h"
#include "USBMouseHIDReportInterceptor.h"
#include "SerialMouseHIDReportInterceptor.h"
#include "MouseEventSpoofer.h"
#include "FirmwareSettings.h"
#include "MathAndConversions.h"

// Define the USB and HID composite objects
USB Usb;  // Define the USB object
CustomHIDComposite Hid(&Usb);  // Define the CustomHIDComposite object

// Define the interceptors and other objects
USBMouseHIDReportInterceptor usbInterceptor;
FirmwareSettings firmwareSettings;
SerialMouseHIDReportInterceptor serialInterceptor(&firmwareSettings);
MouseEventSpoofer mouseEventSpoofer(&usbInterceptor, &serialInterceptor);
MathAndConversions mathAndConversions;

// Initialize settings with default values
bool logPerformanceMetrics = false;
bool logAPerformanceMetric = false;
bool enableLockout = 1;
int mouseLockoutDurationMilliseconds = 16;
int enableSpinning = 0;
int spinAmountPerRotation = 0;
int spinNumberOfRotations = 0;
int spinDelayBetweenRotationsMilliseconds = 0;
bool spinLockoutMouseUntilCompletion = false;
int spinBeforeAfterMouseEvent = 0;
