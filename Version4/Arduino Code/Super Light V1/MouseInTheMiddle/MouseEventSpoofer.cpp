#include "Config.h"
#include "MouseEventSpoofer.h"
#include "Mouse16Bit.h"

MouseEventSpoofer::MouseEventSpoofer(USBMouseHIDReportInterceptor* usbInterceptor, SerialMouseHIDReportInterceptor* serialInterceptor)
: usbInterceptor(usbInterceptor), serialInterceptor(serialInterceptor), spinPerformed(false) {}

void MouseEventSpoofer::spoofEvent() {
    bool hasUSBData = usbInterceptor->hasData();
    bool hasSerialData = serialInterceptor->hasData();

    // prevents processing if there's no data
    if (!hasUSBData && !hasSerialData) {
        // no data, do nothing
        return;
    }

    /* ================= Declare button variables ============= */
    uint8_t usbPreviousMouseButtons = 0;
    uint8_t usbMouseButtons = 0;
    uint8_t serialPreviousMouseButtons = 0;
    uint8_t serialMouseButtons = 0;

    /* ================= Mouse Event Data ============= */
    int16_t usbXMovement = 0;
    int16_t usbYMovement = 0;
    uint8_t usbScrollWheel = 0;
    int16_t serialXMovement = 0;
    int16_t serialYMovement = 0;
    uint8_t serialScrollWheel = 0;

    if (hasUSBData) {
        usbPreviousMouseButtons = usbInterceptor->getPreviousMouseButtons();
        usbMouseButtons = usbInterceptor->getMouseButtons();
        usbXMovement = usbInterceptor->getXMovement();
        usbYMovement = usbInterceptor->getYMovement();
        usbScrollWheel = usbInterceptor->getScrollWheel();
        usbInterceptor->reset();

        // Handle MMB press
        if (usbMouseButtons & MOUSE_MIDDLE) {
            logMouseEvent(usbMouseButtons);
            activationTimestamp4MouseButtonExclusion = millis();
            usbMouseButtons &= ~MOUSE_MIDDLE;
        }

        // Handle BUTTON4 exclusion
        if (shouldExcludeButton(usbMouseButtons, usbPreviousMouseButtons, MOUSE_BUTTON4)) {
            logMouseEvent(usbMouseButtons);
            usbMouseButtons &= ~MOUSE_BUTTON4;
        }

        // Handle BUTTON5 exclusion
        if (shouldExcludeButton(usbMouseButtons, usbPreviousMouseButtons, MOUSE_BUTTON5)) {
            logMouseEvent(usbMouseButtons);
            usbMouseButtons &= ~MOUSE_BUTTON5;
        }

        // Notify Serial Device about RMB events (up and down)
        handleMouseButtonEvent(usbMouseButtons, usbPreviousMouseButtons, MOUSE_RIGHT);

        // Notify Serial Device about LMB events (up and down)
        handleMouseButtonEvent(usbMouseButtons, usbPreviousMouseButtons, MOUSE_LEFT);
    }

    if (hasSerialData) {
        serialPreviousMouseButtons = serialInterceptor->getPreviousMouseButtons();
        serialMouseButtons = serialInterceptor->getMouseButtons();
        serialScrollWheel = serialInterceptor->getScrollWheel();
        serialXMovement = serialInterceptor->getXMovement();
        serialYMovement = serialInterceptor->getYMovement();
        serialInterceptor->reset();

        // Set the lockout timestamp if the program sends a mouse movement event
        if (serialScrollWheel == 1) {
            activationTimestamp4MouseMovementLockout = millis() + mouseLockoutDurationMilliseconds;
        }
    }

    // Check if we are in the lockout period & Discard USB mouse movements
    if (millis() <= activationTimestamp4MouseMovementLockout) {
        usbXMovement = 0;
        usbYMovement = 0;
    }

    modifyMovementWithSerialData(usbXMovement, usbYMovement, serialXMovement, serialYMovement);

    // Handle button events and move the mouse
    handleButtonEvents(usbMouseButtons, usbPreviousMouseButtons, serialMouseButtons, serialPreviousMouseButtons);
    onMouseMove(usbXMovement, usbYMovement, usbScrollWheel);
}

void MouseEventSpoofer::handleButtonEvents(uint8_t usbButtons, uint8_t previousUsbButtonsState, uint8_t serialButtons, uint8_t previousSerialButtonsState) {
    // For each button
    for (uint16_t buttonId = 1; buttonId <= 16; buttonId <<= 1) {
        uint8_t buttonMask;
        switch (buttonId) {
            case 1: buttonMask = MOUSE_LEFT; break;
            case 2: buttonMask = MOUSE_RIGHT; break;
            case 4: buttonMask = MOUSE_MIDDLE; break;
            case 8: buttonMask = MOUSE_BUTTON4; break;
            case 16: buttonMask = MOUSE_BUTTON5; break;
            default:
                FTDI_DEVICE.println(String("E: Unknown ButtonID -> ") + buttonId);
                continue;
        }

        if (buttonMask == MOUSE_LEFT || buttonMask == MOUSE_RIGHT) {
            // If USB mouse is still holding the button, ignore serial release event
            if ((usbButtons & buttonMask) && !(serialButtons & buttonMask) && (previousSerialButtonsState & buttonMask)) {
                continue;
            }
            // If both USB and serial indicate release, then release the button
            if (!(usbButtons & buttonMask) && !(serialButtons & buttonMask)) {
                if (previousUsbButtonsState & buttonMask || previousSerialButtonsState & buttonMask) {
                    Mouse.release(buttonMask);
                }
                continue;
            }
            // If either USB or serial indicate press, then press the button
            if (usbButtons & buttonMask || serialButtons & buttonMask) {
                Mouse.press(buttonMask);
            }
        } else {
            // Handle all other buttons normally
            if (usbButtons & buttonId) {
                if (!(previousUsbButtonsState & buttonId)) {
                    Mouse.press(buttonMask);
                }
            } else {
                if (previousUsbButtonsState & buttonId) {
                    Mouse.release(buttonMask);
                }
            }
        }
    }
}

void MouseEventSpoofer::modifyMovementWithSerialData(int16_t &usbXMovement, int16_t &usbYMovement, int16_t serialXMovement, int16_t serialYMovement) {
    if (serialXMovement != 0) {
        usbXMovement += serialXMovement;
    }

    if (serialYMovement != 0) {
        usbYMovement += serialYMovement;
    }
}

void MouseEventSpoofer::onMouseMove(int16_t xMovement, int16_t yMovement, int8_t scrollValue) {
    // Print the xMovement value
    Mouse.move(xMovement, yMovement, scrollValue);
}

void MouseEventSpoofer::logMouseEvent(uint8_t mouseButtons) {
    FTDI_DEVICE.print("M: ");
    FTDI_DEVICE.print(mouseButtons, HEX);
    FTDI_DEVICE.println();
}

bool MouseEventSpoofer::shouldExcludeButton(uint8_t currentButtons, uint8_t previousButtons, uint8_t buttonMask) {
    return (currentButtons & buttonMask) && !(previousButtons & buttonMask) &&
           (millis() - activationTimestamp4MouseButtonExclusion) <= BUTTON_EXCLUSION_DURATION_MS;
}

void MouseEventSpoofer::handleMouseButtonEvent(uint8_t currentButtons, uint8_t previousButtons, uint8_t buttonMask) {
    if (currentButtons & buttonMask) {
        if (!(previousButtons & buttonMask)) {
            logMouseEvent(currentButtons);
        }
    } else if (previousButtons & buttonMask) {
        logMouseEvent(currentButtons);
    }
}

void MouseEventSpoofer::performSpinEvent() {
    if (enableSpinning == 0) {
        return;
    }

    // Spin only if LMB is pressed and was not pressed in the previous event
    bool usbLMBPressed = (finalButtonStates & MOUSE_LEFT) && !(previousUsbButtonsState & MOUSE_LEFT);
    bool serialLMBPressed = (finalButtonStates & MOUSE_LEFT) && !(previousSerialButtonsState & MOUSE_LEFT);

    if ((usbLMBPressed || serialLMBPressed) && !spinPerformed) {
        spinPerformed = true;
        for (int i = 0; i < spinNumberOfRotations; ++i) {
            onMouseMove(spinAmountPerRotation, 0, 0);
            delay(spinDelayBetweenRotationsMilliseconds);
        }
    }
}
