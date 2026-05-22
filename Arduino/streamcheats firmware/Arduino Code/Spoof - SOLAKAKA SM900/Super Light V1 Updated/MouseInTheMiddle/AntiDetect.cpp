#include "AntiDetect.h"

AntiDetect::AntiDetect()
    : opposingFramesX(0)
    , opposingFramesY(0) {
}

void AntiDetect::begin() {
    opposingFramesX = 0;
    opposingFramesY = 0;
}

void AntiDetect::sanitizeOutput(int16_t rawUsbX, int16_t rawUsbY,
                                 int16_t serialX, int16_t serialY,
                                 int16_t& combinedX, int16_t& combinedY) {
    combinedX = sanitizeAxis(rawUsbX, combinedX, serialX, opposingFramesX);
    combinedY = sanitizeAxis(rawUsbY, combinedY, serialY, opposingFramesY);
}

int16_t AntiDetect::sanitizeAxis(int16_t rawUsb, int16_t combined, int16_t serial, uint8_t& opposingFrames) {
    if (serial == 0)   { opposingFrames = 0; return combined; }
    if (rawUsb == 0)   { opposingFrames = 0; return combined; }
    if (combined == 0) { opposingFrames = 0; return combined; }

    bool sameSign = (rawUsb > 0) == (combined > 0);
    if (sameSign) { opposingFrames = 0; return combined; }

    opposingFrames++;

    int16_t absSerial = abs(serial);
    int16_t absScaledUsb = abs(combined - serial);

    if (opposingFrames < OPPOSING_THRESHOLD) {
        if (absSerial <= absScaledUsb) {
            int16_t userSign = (rawUsb > 0) ? 1 : -1;
            int16_t dampened = absScaledUsb - absSerial;
            if (dampened < 1 && absScaledUsb >= 1) {
                dampened = 1;
            }
            return userSign * dampened;
        } else {
            return 0;
        }
    }

    return combined;
}
