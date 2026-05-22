#include "Config.h"
#include "MouseEventSpoofer.h"
#include "CompositeHID.h"
#include "SunBoxLogger.h"

MouseEventSpoofer::MouseEventSpoofer(USBMouseHIDReportInterceptor* usbInterceptor, SerialMouseHIDReportInterceptor* serialInterceptor)
: usbInterceptor(usbInterceptor), serialInterceptor(serialInterceptor),
  activationTimestamp4MouseButtonExclusion(0), activationTimestamp4MouseMovementLockout(0) {
    for (uint8_t i = 0; i < DELTA_BUFFER_SIZE; i++) {
        deltaBufferX[i] = 0;
        deltaBufferY[i] = 0;
    }
}

void MouseEventSpoofer::spoofEvent() {
    bool hasUSBData = usbInterceptor->hasData();
    bool hasSerialData = serialInterceptor->hasData();

    if (!hasUSBData && !hasSerialData) return;

    // Button + movement state
    uint8_t usbPreviousMouseButtons = 0;
    uint8_t usbMouseButtons = 0;
    uint8_t unmodifiedUsbMouseButtons = 0;
    uint8_t serialPreviousMouseButtons = 0;
    uint8_t serialMouseButtons = 0;
    uint8_t unmodifiedSerialMouseButtons = 0;

    int16_t usbXMovement = 0;
    int16_t usbYMovement = 0;
    int8_t  usbScrollWheel = 0;
    int16_t serialXMovement = 0;
    int16_t serialYMovement = 0;
    int8_t  serialScrollWheel = 0;

    if (hasUSBData) {
        usbPreviousMouseButtons   = usbInterceptor->getPreviousMouseButtons();
        usbMouseButtons           = usbInterceptor->getMouseButtons();
        unmodifiedUsbMouseButtons = usbMouseButtons;
        usbXMovement              = usbInterceptor->getXMovement();
        usbYMovement              = usbInterceptor->getYMovement();
        usbScrollWheel            = (int8_t)usbInterceptor->getScrollWheel();
        usbInterceptor->reset();
    }

    if (hasSerialData) {
        serialPreviousMouseButtons   = serialInterceptor->getPreviousMouseButtons();
        serialMouseButtons           = serialInterceptor->getMouseButtons();
        unmodifiedSerialMouseButtons = serialMouseButtons;
        serialScrollWheel            = (int8_t)serialInterceptor->getScrollWheel();
        serialXMovement              = serialInterceptor->getXMovement();
        serialYMovement              = serialInterceptor->getYMovement();
        serialInterceptor->reset();

        // wheel == 1 is the host's signal to arm the sens-reduction window
        if (serialScrollWheel == 1) {
            activationTimestamp4MouseMovementLockout = millis() + sensReductionDurationMilliseconds;
            serialScrollWheel = 0;  // consume it; never forward to host
        }
    }

    // M: <hex> when the raw USB button byte changes (matches Teensy)
    if (hasUSBData) emitButtonChangeLog(unmodifiedUsbMouseButtons);

    // Capture raw inputs for SYN log before any modification
    int16_t synRawUsbX    = usbXMovement;
    int16_t synRawUsbY    = usbYMovement;
    int16_t synRawSerialX = hasSerialData ? serialXMovement : 0;
    int16_t synRawSerialY = hasSerialData ? serialYMovement : 0;

    // MMB exclusion window + passthrough
    if (usbMouseButtons & MOUSE_MIDDLE) {
        activationTimestamp4MouseButtonExclusion = millis();
        if (disablePassthroughForMMB == 1) {
            usbMouseButtons &= ~MOUSE_MIDDLE;
        }
    }

    // Per-button passthrough filtering on USB
    handleMouseButtonConfigCheck(usbMouseButtons, unmodifiedUsbMouseButtons, usbPreviousMouseButtons, MOUSE_RIGHT,   disablePassthroughForRMB, lastRMBPressTime);
    handleMouseButtonConfigCheck(usbMouseButtons, unmodifiedUsbMouseButtons, usbPreviousMouseButtons, MOUSE_LEFT,    disablePassthroughForLMB, lastLMBPressTime);
    handleMouseButtonConfigCheck(usbMouseButtons, unmodifiedUsbMouseButtons, usbPreviousMouseButtons, MOUSE_BUTTON4, disablePassthroughForMB4, lastMB4PressTime);
    handleMouseButtonConfigCheck(usbMouseButtons, unmodifiedUsbMouseButtons, usbPreviousMouseButtons, MOUSE_BUTTON5, disablePassthroughForMB5, lastMB5PressTime);

    // Same filtering applied to serial buttons
    handleMouseButtonConfigCheck(serialMouseButtons, unmodifiedSerialMouseButtons, serialPreviousMouseButtons, MOUSE_RIGHT,   disablePassthroughForRMB, lastRMBPressTime);
    handleMouseButtonConfigCheck(serialMouseButtons, unmodifiedSerialMouseButtons, serialPreviousMouseButtons, MOUSE_LEFT,    disablePassthroughForLMB, lastLMBPressTime);
    handleMouseButtonConfigCheck(serialMouseButtons, unmodifiedSerialMouseButtons, serialPreviousMouseButtons, MOUSE_BUTTON4, disablePassthroughForMB4, lastMB4PressTime);
    handleMouseButtonConfigCheck(serialMouseButtons, unmodifiedSerialMouseButtons, serialPreviousMouseButtons, MOUSE_BUTTON5, disablePassthroughForMB5, lastMB5PressTime);

    // Combined X/Y with sens reduction
    int16_t finalX = usbXMovement;
    int16_t finalY = usbYMovement;
    modifyMovementWithSerialData(finalX, finalY, serialXMovement, serialYMovement);

    // Anti-detect sign-flip sanitizer (only meaningful when serial contributed)
    if (hasSerialData) {
        antiDetect.sanitizeOutput(synRawUsbX, synRawUsbY,
                                  serialXMovement, serialYMovement,
                                  finalX, finalY);
    }

    // Combine buttons via handoff state machine for LMB/RMB, OR for others
    uint8_t finalButtons = 0;
    for (uint8_t buttonMask = 1; buttonMask <= 0x10; buttonMask <<= 1) {
        if (buttonMask == MOUSE_LEFT) {
            finalButtons |= processButtonHandoff(lmbHandoff, buttonMask,
                                                 serialMouseButtons, usbMouseButtons,
                                                 usbPreviousMouseButtons);
        } else if (buttonMask == MOUSE_RIGHT) {
            finalButtons |= processButtonHandoff(rmbHandoff, buttonMask,
                                                 serialMouseButtons, usbMouseButtons,
                                                 usbPreviousMouseButtons);
        } else {
            if ((usbMouseButtons & buttonMask) || (serialMouseButtons & buttonMask)) {
                finalButtons |= buttonMask;
            }
        }
    }

    // SYN log line — emitted whenever serial data was processed
    if (hasSerialData) {
        bool sensActive = (enableSensReduction == 1 && millis() <= activationTimestamp4MouseMovementLockout);
        logger.infof("SYN:%lu,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                     millis(),
                     synRawUsbX, synRawUsbY,
                     synRawSerialX, synRawSerialY,
                     sensActive ? 1 : 0,
                     (int)sensReductionAmmountX, (int)sensReductionAmmountY,
                     finalX, finalY,
                     0, 0,   // budgetX, budgetY (reserved)
                     0, 0);  // accumX, accumY (reserved)
    }

    // Apply combined button state to the host
    submitFinalButtonState(finalButtons);

    // Send movement (and the surviving wheel; serial wheel was cleared if it was a sens-trigger)
    int16_t wheel = (int16_t)usbScrollWheel + (int16_t)serialScrollWheel;
    onMouseMove(finalX, finalY, (int8_t)wheel);

    // Buffer raw USB deltas for delta logging (when enabled)
    if (enableDeltaLogging && hasUSBData) {
        bufferDelta(synRawUsbX, synRawUsbY);
    }
}

void MouseEventSpoofer::submitFinalButtonState(uint8_t finalButtons) {
    uint8_t changed = finalButtons ^ lastOutputButtons;
    if (!changed) return;

    for (uint8_t buttonMask = 1; buttonMask <= 0x10; buttonMask <<= 1) {
        if (!(changed & buttonMask)) continue;
        if (finalButtons & buttonMask) {
            CompositeHID.press(buttonMask);
        } else {
            CompositeHID.release(buttonMask);
        }
    }
    lastOutputButtons = finalButtons;
}

void MouseEventSpoofer::emitButtonChangeLog(uint8_t unmodifiedUsbButtons) {
    if (unmodifiedUsbButtons != lastLoggedUsbButtons) {
        logger.mousef("%02X", unmodifiedUsbButtons);
        lastLoggedUsbButtons = unmodifiedUsbButtons;
    }
}

void MouseEventSpoofer::bufferDelta(int16_t rawX, int16_t rawY) {
    deltaBufferX[deltaFrameCount] = rawX;
    deltaBufferY[deltaFrameCount] = rawY;
    deltaFrameCount++;
    if (deltaFrameCount >= DELTA_BUFFER_SIZE) {
        char buf[120];
        int pos = 0;
        for (uint8_t i = 0; i < DELTA_BUFFER_SIZE; i++) {
            if (i > 0 && pos < (int)sizeof(buf)) buf[pos++] = ':';
            pos += snprintf(buf + pos, sizeof(buf) - pos, "%d,%d",
                            deltaBufferX[i], deltaBufferY[i]);
            if (pos >= (int)sizeof(buf)) { pos = sizeof(buf) - 1; break; }
        }
        buf[pos] = '\0';
        logger.mouse(buf);
        deltaFrameCount = 0;
    }
}

uint8_t MouseEventSpoofer::processButtonHandoff(
    ButtonHandoff &handoff, uint8_t buttonMask,
    uint8_t serialButtons, uint8_t usbButtons, uint8_t prevUsbButtons) {

    bool synHeld  = (serialButtons & buttonMask) != 0;
    bool usbHeld  = (usbButtons    & buttonMask) != 0;
    bool usbEdge  = usbHeld && !(prevUsbButtons & buttonMask);

    switch (handoff.state) {
        case HANDOFF_IDLE:
            if (synHeld && !usbHeld) {
                handoff.state = HANDOFF_SYNTHETIC_HOLD;
                return buttonMask;
            }
            return (synHeld || usbHeld) ? buttonMask : 0;

        case HANDOFF_SYNTHETIC_HOLD:
            if (!synHeld) {
                handoff.state = HANDOFF_IDLE;
                return usbHeld ? buttonMask : 0;
            }
            if (usbEdge) {
                handoff.state = HANDOFF_RELEASE;
                handoff.releaseStartMs = millis();
                handoff.gapDurationMs = HANDOFF_GAP_MIN_MS +
                    random(0, HANDOFF_GAP_MAX_MS - HANDOFF_GAP_MIN_MS + 1);
                return 0;
            }
            return buttonMask;

        case HANDOFF_RELEASE:
            if (millis() - handoff.releaseStartMs >= handoff.gapDurationMs) {
                handoff.state = HANDOFF_USER_CONTROL;
                return usbHeld ? buttonMask : 0;
            }
            return 0;

        case HANDOFF_USER_CONTROL:
            if (!synHeld && !usbHeld) {
                handoff.state = HANDOFF_IDLE;
                return 0;
            }
            return usbHeld ? buttonMask : 0;
    }
    return 0;
}

void MouseEventSpoofer::modifyMovementWithSerialData(int16_t &usbXMovement, int16_t &usbYMovement, int16_t serialXMovement, int16_t serialYMovement) {
    long usbX = static_cast<long>(usbXMovement);
    long usbY = static_cast<long>(usbYMovement);

    if (enableSensReduction == 1 && millis() <= activationTimestamp4MouseMovementLockout) {
        if (sensReductionAmmountX >= 0 && sensReductionAmmountX <= 100) {
            usbX = (usbX * sensReductionAmmountX);
            sensReductionXAccumulator += usbX % 100;
            usbX = usbX / 100;
            if (abs(sensReductionXAccumulator) >= 100) {
                usbX += sensReductionXAccumulator / 100;
                sensReductionXAccumulator %= 100;
            }
        }
        if (sensReductionAmmountY >= 0 && sensReductionAmmountY <= 100) {
            usbY = (usbY * sensReductionAmmountY);
            sensReductionYAccumulator += usbY % 100;
            usbY = usbY / 100;
            if (abs(sensReductionYAccumulator) >= 100) {
                usbY += sensReductionYAccumulator / 100;
                sensReductionYAccumulator %= 100;
            }
        }
        usbXMovement = static_cast<int16_t>(usbX + serialXMovement);
        usbYMovement = static_cast<int16_t>(usbY + serialYMovement);
    } else {
        usbXMovement += serialXMovement;
        usbYMovement += serialYMovement;
    }
}

void MouseEventSpoofer::onMouseMove(int16_t xMovement, int16_t yMovement, int8_t scrollValue) {
    CompositeHID.move(xMovement, yMovement, scrollValue);
}

bool MouseEventSpoofer::shouldExcludeButton(uint8_t currentButtons, uint8_t previousButtons, uint8_t buttonMask) {
    return (currentButtons & buttonMask) && !(previousButtons & buttonMask) &&
           (millis() - activationTimestamp4MouseButtonExclusion) <= BUTTON_EXCLUSION_DURATION_MS;
}

void MouseEventSpoofer::handleMouseButtonConfigCheck(uint8_t &buttons, uint8_t unmodifiedButtons, uint8_t previousButtons,
                                                     uint8_t buttonMask, int disablePassthroughOption, unsigned long &lastPressTime) {
    unsigned long currentTime = millis();

    if (unmodifiedButtons & buttonMask) {
        if (disablePassthroughOption == 0) {
            // pass through
        } else if (disablePassthroughOption == 1) {
            buttons &= ~buttonMask;
        } else if (disablePassthroughOption == 2) {
            if (shouldExcludeButton(unmodifiedButtons, previousButtons, buttonMask)) {
                buttons &= ~buttonMask;
            }
        } else if (disablePassthroughOption == 3) {
            if (!(previousButtons & buttonMask) && (unmodifiedButtons & buttonMask)) {
                if (lastPressTime && (currentTime - lastPressTime <= BUTTON_DOUBLE_TAP_TO_PASSTHROUGH_DURATION_MS)) {
                    lastPressTime = 0;
                } else {
                    buttons &= ~buttonMask;
                    lastPressTime = currentTime;
                }
            } else if ((previousButtons & buttonMask) && (unmodifiedButtons & buttonMask)) {
                if (lastPressTime != 0) {
                    buttons &= ~buttonMask;
                }
            }
            if (shouldExcludeButton(unmodifiedButtons, previousButtons, buttonMask)) {
                buttons &= ~buttonMask;
            }
        }
    } else if (previousButtons & buttonMask) {
        lastPressTime = currentTime;
    }
}
