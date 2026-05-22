#ifndef _MOUSEEVENTSPOOFER_H_
#define _MOUSEEVENTSPOOFER_H_

#include "USBMouseHIDReportInterceptor.h"
#include "SerialMouseHIDReportInterceptor.h"
#include "AntiDetect.h"

#define MOUSE_LEFT      0x01
#define MOUSE_RIGHT     0x02
#define MOUSE_MIDDLE    0x04
#define MOUSE_BUTTON4   0x08
#define MOUSE_BUTTON5   0x10

// Button handoff state machine (mirrors Teensy SunBoxSyntheticHandleOutput).
// Forces a randomized release gap when the user presses a button the
// synthetic (serial) input is already holding, so the takeover looks natural.
enum ButtonHandoffState {
    HANDOFF_IDLE,
    HANDOFF_SYNTHETIC_HOLD,
    HANDOFF_RELEASE,
    HANDOFF_USER_CONTROL
};

struct ButtonHandoff {
    ButtonHandoffState state = HANDOFF_IDLE;
    unsigned long releaseStartMs = 0;
    unsigned long gapDurationMs = 0;  // randomized per-handoff (18-75ms)
};

static const unsigned long HANDOFF_GAP_MIN_MS = 18;
static const unsigned long HANDOFF_GAP_MAX_MS = 75;

class MouseEventSpoofer {
    public:
        MouseEventSpoofer(USBMouseHIDReportInterceptor* usbInterceptor, SerialMouseHIDReportInterceptor* serialInterceptor);
        void spoofEvent();

    private:
        void modifyMovementWithSerialData(int16_t &usbXMovement, int16_t &usbYMovement, int16_t serialXMovement, int16_t serialYMovement);
        void onMouseMove(int16_t xMovement, int16_t yMovement, int8_t scrollValue);
        bool shouldExcludeButton(uint8_t currentButtons, uint8_t previousButtons, uint8_t buttonMask);
        void handleMouseButtonConfigCheck(uint8_t &buttons, uint8_t unmodifiedButtons, uint8_t previousButtons, uint8_t buttonMask, int disablePassthroughOption, unsigned long &lastPressTime);
        uint8_t processButtonHandoff(ButtonHandoff &handoff, uint8_t buttonMask,
                                     uint8_t serialButtons, uint8_t usbButtons,
                                     uint8_t prevUsbButtons);
        void submitFinalButtonState(uint8_t finalButtons);
        void emitButtonChangeLog(uint8_t unmodifiedUsbButtons);
        void bufferDelta(int16_t rawX, int16_t rawY);

        // Member variables
        USBMouseHIDReportInterceptor* usbInterceptor;
        SerialMouseHIDReportInterceptor* serialInterceptor;
        unsigned long activationTimestamp4MouseButtonExclusion;
        unsigned long activationTimestamp4MouseMovementLockout;

        // Fixed-point accumulators for sens reduction
        int sensReductionXAccumulator = 0;
        int sensReductionYAccumulator = 0;

        // Double-tap timestamps
        unsigned long lastRMBPressTime = 0;
        unsigned long lastLMBPressTime = 0;
        unsigned long lastMB4PressTime = 0;
        unsigned long lastMB5PressTime = 0;

        // Button-change detector for M: log
        uint8_t lastLoggedUsbButtons = 0;

        // Last submitted final button state (so we only press/release on change)
        uint8_t lastOutputButtons = 0;

        // LMB / RMB handoff state machines
        ButtonHandoff lmbHandoff;
        ButtonHandoff rmbHandoff;

        // Anti-detect sign-flip sanitizer
        AntiDetect antiDetect;

        // Delta buffer for M: x,y:x,y:... output (every 10 USB frames)
        static const uint8_t DELTA_BUFFER_SIZE = 10;
        int16_t deltaBufferX[DELTA_BUFFER_SIZE];
        int16_t deltaBufferY[DELTA_BUFFER_SIZE];
        uint8_t deltaFrameCount = 0;
};

#endif // _MOUSEEVENTSPOOFER_H_
