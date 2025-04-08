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
// FIRMWARE_VERSION is set in Config.h and is not a changeable value
bool logPerformanceMetrics = false;
bool logAPerformanceMetric = false; // Not configurable
int enableSensReduction = 1; // 0 = Disabled, 1 = Enabled
int sensReductionDurationMilliseconds = 8;
int sensReductionAmmountX = 100; // 100 = full movement (no reduction), 0 = full lockout (full reduction)
int sensReductionAmmountY = 100; // 100 = full movement (no reduction), 0 = full lockout (full reduction)
int enableSpinning = 0; // 0=Disabled, 1=Enabled
int spinAmountPerRotation = 0;
int spinNumberOfRotations = 0;
int spinDelayBetweenRotationsMilliseconds = 0;
bool spinLockoutMouseUntilCompletion = false;
int spinBeforeAfterMouseEvent = 0; // 0=Before, 1=After, 2=Both
int disablePassthroughForMMB = 0; //0 = False, 1 = True
int disablePassthroughForRMB = 0; //0 = False, 1 = True, 2 = Only if MMB has been pressed within duration, 3 = Only pass through if double tapped (and or held) within duration
int disablePassthroughForLMB = 0; //0 = False, 1 = True, 2 = Only if MMB has been pressed within duration, 3 = Only pass through if double tapped (and or held) within duration
int disablePassthroughForMB4 = 0; //0 = False, 1 = True, 2 = Only if MMB has been pressed within duration, 3 = Only pass through if double tapped (and or held) within duration
int disablePassthroughForMB5 = 0; //0 = False, 1 = True, 2 = Only if MMB has been pressed within duration, 3 = Only pass through if double tapped (and or held) within duration