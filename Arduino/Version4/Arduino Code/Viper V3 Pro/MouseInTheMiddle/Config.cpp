#include "Config.h"

// Initialize settings with default values
// FIRMWARE_VERSION is set in Config.h and is not a changeable value
bool logPerformanceMetrics = false;
bool logAPerformanceMetric = false; // Not configurable
bool enableLockout = 1;
int mouseLockoutDurationMilliseconds = 16;
int enableSpinning = 0; // 0=Disabled, 1=Enabled
int spinAmountPerRotation = 0;
int spinNumberOfRotations = 0;
int spinDelayBetweenRotationsMilliseconds = 0;
bool spinLockoutMouseUntilCompletion = false;
int spinBeforeAfterMouseEvent = 0; // 0=Before, 1=After, 2=Both