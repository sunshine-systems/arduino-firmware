#ifndef _ANTI_DETECT_H_
#define _ANTI_DETECT_H_

#include <Arduino.h>

// Port of the Teensy SunshineUSBProxy AntiDetect sanitizer.
// Prevents serial (synthetic) input from flipping the output sign against
// the user's raw USB direction, with a small tolerance for sustained
// corrections.
class AntiDetect {
public:
    AntiDetect();
    void begin();

    // Call after combining USB + serial to sanitize the output.
    //   rawUsbX/Y    = original USB values BEFORE any modification
    //   serialX/Y    = serial (aimbot) deltas that were added
    //   combinedX/Y  = blended output (usb*sens/100 + serial), modified in place
    void sanitizeOutput(int16_t rawUsbX, int16_t rawUsbY,
                        int16_t serialX, int16_t serialY,
                        int16_t& combinedX, int16_t& combinedY);

private:
    int16_t sanitizeAxis(int16_t rawUsb, int16_t combined, int16_t serial, uint8_t& opposingFrames);

    uint8_t opposingFramesX;
    uint8_t opposingFramesY;

    static const uint8_t OPPOSING_THRESHOLD = 2;
};

#endif
