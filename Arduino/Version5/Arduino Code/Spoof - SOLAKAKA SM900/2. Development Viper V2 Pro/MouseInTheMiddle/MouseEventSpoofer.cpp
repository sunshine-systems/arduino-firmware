#include "Config.h"
#include "MouseEventSpoofer.h"
#include "CompositeHID.h"

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
    uint8_t unmodifiedUsbMouseButtons = 0;
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
        unmodifiedUsbMouseButtons = usbInterceptor->getMouseButtons();
        usbXMovement = usbInterceptor->getXMovement();
        usbYMovement = usbInterceptor->getYMovement();
        usbScrollWheel = usbInterceptor->getScrollWheel();
        usbInterceptor->reset();

        // Handle MMB press
        if (usbMouseButtons & MOUSE_MIDDLE) {
            // Log the event
            logMouseEvent(usbMouseButtons);

            // Update exclusion timestamp
            activationTimestamp4MouseButtonExclusion = millis();

            // Check passthrough condition based on disablePassthroughForMMB
            if (disablePassthroughForMMB == 0) {
                // No passthrough blocking, continue as normal
            } else if (disablePassthroughForMMB == 1) {
                // Passthrough disabled, clear the byte
                usbMouseButtons &= ~MOUSE_MIDDLE;
            }
        }

        // handle logging the button events only if there is a change
        handleMouseButtonEvent(usbMouseButtons, usbPreviousMouseButtons, MOUSE_RIGHT);
        handleMouseButtonEvent(usbMouseButtons, usbPreviousMouseButtons, MOUSE_LEFT);
        handleMouseButtonEvent(usbMouseButtons, usbPreviousMouseButtons, MOUSE_BUTTON4);
        handleMouseButtonEvent(usbMouseButtons, usbPreviousMouseButtons, MOUSE_BUTTON5);

        // Handle modifying the RMB, LMB, MB4, MB5
        handleMouseButtonConfigCheck(usbMouseButtons, unmodifiedUsbMouseButtons, usbPreviousMouseButtons, MOUSE_RIGHT, disablePassthroughForRMB, lastRMBPressTime);
        handleMouseButtonConfigCheck(usbMouseButtons, unmodifiedUsbMouseButtons, usbPreviousMouseButtons, MOUSE_LEFT, disablePassthroughForLMB, lastLMBPressTime);
        handleMouseButtonConfigCheck(usbMouseButtons, unmodifiedUsbMouseButtons, usbPreviousMouseButtons, MOUSE_BUTTON4, disablePassthroughForMB4, lastMB4PressTime);
        handleMouseButtonConfigCheck(usbMouseButtons, unmodifiedUsbMouseButtons, usbPreviousMouseButtons, MOUSE_BUTTON5, disablePassthroughForMB5, lastMB5PressTime);


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
            activationTimestamp4MouseMovementLockout = millis() + sensReductionDurationMilliseconds;
        }
    }

    // Modify our X,Y movement with SerialData and or other factors like lockout, sens reduction etc.
    modifyMovementWithSerialData(usbXMovement, usbYMovement, serialXMovement, serialYMovement);

    // Pre-spin handling (before LMB is pressed)
    performSpinEvent(true, usbMouseButtons, usbPreviousMouseButtons, serialMouseButtons, serialPreviousMouseButtons);

    // Handle button events
    handleButtonEvents(usbMouseButtons, usbPreviousMouseButtons, serialMouseButtons, serialPreviousMouseButtons);

    // Post-spin handling (after LMB is pressed)
    performSpinEvent(false, usbMouseButtons, usbPreviousMouseButtons, serialMouseButtons, serialPreviousMouseButtons);

    // Move the mouse normally
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
                    CompositeHID.release(buttonMask);
                }
                continue;
            }
            // If either USB or serial indicate press, then press the button
            if (usbButtons & buttonMask || serialButtons & buttonMask) {
                CompositeHID.press(buttonMask);
            }
        } else {
            // Handle all other buttons normally
            if (usbButtons & buttonId) {
                if (!(previousUsbButtonsState & buttonId)) {
                    CompositeHID.press(buttonMask);
                }
            } else {
                if (previousUsbButtonsState & buttonId) {
                    CompositeHID.release(buttonMask);
                }
            }
        }
    }
}

void MouseEventSpoofer::modifyMovementWithSerialData(int16_t &usbXMovement, int16_t &usbYMovement, int16_t serialXMovement, int16_t serialYMovement) {
    // Convert int16_t to long for higher precision calculations using integers
    long usbX = static_cast<long>(usbXMovement);
    long usbY = static_cast<long>(usbYMovement);

    if (enableSensReduction == 1 && millis() <= activationTimestamp4MouseMovementLockout) {
        // Apply separate X and Y axis sensitivity reduction using fixed-point-like scaling
        if (sensReductionAmmountX >= 0 && sensReductionAmmountX <= 100) {
            usbX = (usbX * sensReductionAmmountX);  // Scale up by 100 for precision
            sensReductionXAccumulator += usbX % 100; // Track the leftover
            usbX = usbX / 100;  // Get the reduced integer part

            // Apply accumulated X-axis movement when overflow happens
            if (abs(sensReductionXAccumulator) >= 100) {
                usbXMovement += sensReductionXAccumulator / 100; // Apply accumulated leftover
                sensReductionXAccumulator %= 100;  // Keep only the remainder
            }
        }

        if (sensReductionAmmountY >= 0 && sensReductionAmmountY <= 100) {
            usbY = (usbY * sensReductionAmmountY);  // Scale up by 100 for precision
            sensReductionYAccumulator += usbY % 100; // Track the leftover
            usbY = usbY / 100;  // Get the reduced integer part

            // Apply accumulated Y-axis movement when overflow happens
            if (abs(sensReductionYAccumulator) >= 100) {
                usbYMovement += sensReductionYAccumulator / 100; // Apply accumulated leftover
                sensReductionYAccumulator %= 100;  // Keep only the remainder
            }
        }

        // Convert back to int16_t after adding serial movement
        usbXMovement = static_cast<int16_t>(usbX + serialXMovement);
        usbYMovement = static_cast<int16_t>(usbY + serialYMovement);

    } else {
        // Case for no lockout or sensitivity reduction, just add serial movement
        usbXMovement += serialXMovement;
        usbYMovement += serialYMovement;
    }
}

void MouseEventSpoofer::onMouseMove(int16_t xMovement, int16_t yMovement, int8_t scrollValue) {
    // Print the xMovement value
    CompositeHID.move(xMovement, yMovement, scrollValue);
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

void MouseEventSpoofer::performSpinEvent(bool isBeforeEvent, uint8_t usbMouseButtons, uint8_t usbPreviousMouseButtons, uint8_t serialMouseButtons, uint8_t serialPreviousMouseButtons) {
    // Check if spinning is enabled
    if (enableSpinning == 0) {
        return;
    }

    // Check if the correct LMB event has occurred (LMB pressed for the first time)
    bool lmbPressed = !(usbPreviousMouseButtons & MOUSE_LEFT) && (usbMouseButtons & MOUSE_LEFT);
    bool lmbPressedSerial = !(serialPreviousMouseButtons & MOUSE_LEFT) && (serialMouseButtons & MOUSE_LEFT);

    // If LMB was not pressed on either USB or serial, we skip the spin
    if (!lmbPressed && !lmbPressedSerial) {
        return;
    }

    // Check if this is a before or after event spin
    if (isBeforeEvent) {
        if (spinBeforeAfterMouseEvent != 0 && spinBeforeAfterMouseEvent != 2) {
            return;  // Not a spin-before event
        }
    } else {
        if (spinBeforeAfterMouseEvent != 1 && spinBeforeAfterMouseEvent != 2) {
            return;  // Not a spin-after event
        }
        // Add 1ms delay before spinning for AFTER event
        delay(5);
    }

    // Check if we already performed a spin (avoid spinning multiple times on LMB hold)
    if (spinPerformed) {
        return;
    }

    // Perform the spin
    spinPerformed = true;
    for (int i = 0; i < spinNumberOfRotations; ++i) {
        onMouseMove(spinAmountPerRotation, 0, 0);
        delay(spinDelayBetweenRotationsMilliseconds);
    }

    // Reset spinPerformed if you want spinning to happen again (e.g., after LMB release)
    spinPerformed = false;  // Set to false if you want to allow spinning again later
}

void MouseEventSpoofer::handleMouseButtonConfigCheck(uint8_t &usbMouseButtons, uint8_t &unmodifiedUsbMouseButtons, uint8_t &usbPreviousMouseButtons, uint8_t buttonMask, int disablePassthroughOption, unsigned long &lastPressTime) {
    unsigned long currentTime = millis();

    if (usbMouseButtons & buttonMask) {
        // Button is pressed, now check the passthrough condition
        if (disablePassthroughOption == 0) {
            // Case 0: No passthrough logic, just handle up/down events

        } else if (disablePassthroughOption == 1) {
            // Case 1: Log the event and disable passthrough
            usbMouseButtons &= ~buttonMask;  // Block passthrough

        } else if (disablePassthroughOption == 2) {
            // Case 2: Handle normal up/down events, but block based on MMB timing

            // Check exclusion condition with MMB timing
            if (this->shouldExcludeButton(usbMouseButtons, usbPreviousMouseButtons, buttonMask)) {
                usbMouseButtons &= ~buttonMask;  // Disable passthrough if exclusion condition met
            }

        } else if (disablePassthroughOption == 3) {
            // Case 3: Handle normal up/down events with double-tap passthrough logic

            // Check for new press (button is pressed now, but was not pressed before)
            if (!(usbPreviousMouseButtons & buttonMask) && (usbMouseButtons & buttonMask)) {
                // Button was just pressed (previous state was released, now it's pressed)
                if (lastPressTime && (currentTime - lastPressTime <= BUTTON_DOUBLE_TAP_TO_PASSTHROUGH_DURATION_MS)) {
                    // Double-tap condition met, allowing passthrough
                    lastPressTime = 0;  // Reset last press time after successful double-tap
                } else {
                    // Either this is the first press or the time between presses is too long
                    usbMouseButtons &= ~buttonMask;  // Disable passthrough for this press
                    lastPressTime = currentTime;  // Update the press time for future detection
                }
            }
            // Check if the button is held down (present in both previous and current states)
            else if ((usbPreviousMouseButtons & buttonMask) && (usbMouseButtons & buttonMask)) {
                // The button is being held down
                if (lastPressTime == 0) {
                    // Double-tap was detected earlier, maintain passthrough
                } else {
                    // No valid double-tap, block passthrough
                    usbMouseButtons &= ~buttonMask;
                }
            } 
            // Handle button release
            else if (!(usbMouseButtons & buttonMask) && (usbPreviousMouseButtons & buttonMask)) {
                // Button release detected
                lastPressTime = currentTime;  // Mark release time for double-tap detection
            }

            // Check MMB exclusion window as well
            if (this->shouldExcludeButton(usbMouseButtons, usbPreviousMouseButtons, buttonMask)) {
                usbMouseButtons &= ~buttonMask;  // Disable passthrough if MMB exclusion is active
            }
        }

    } else if (usbPreviousMouseButtons & buttonMask) {
        lastPressTime = currentTime;  // Update time when the button was released
    }
}
