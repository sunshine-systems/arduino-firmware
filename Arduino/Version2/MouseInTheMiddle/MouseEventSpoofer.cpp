#include "Config.h"
#include "MouseEventSpoofer.h"
#include "Mouse16Bit.h"

MouseEventSpoofer::MouseEventSpoofer(USBMouseHIDReportInterceptor* usbInterceptor, SerialMouseHIDReportInterceptor* serialInterceptor)
: usbInterceptor(usbInterceptor), serialInterceptor(serialInterceptor) {}

void MouseEventSpoofer::spoofEvent() {
    bool hasUSBData = usbInterceptor->hasData();
    bool hasSerialData = serialInterceptor->hasData();

    // prevents processing if theres no data
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


    if(hasUSBData) {
        usbPreviousMouseButtons = usbInterceptor->getPreviousMouseButtons();
        usbMouseButtons = usbInterceptor->getMouseButtons();
        usbXMovement = usbInterceptor->getXMovement();
        usbYMovement = usbInterceptor->getYMovement();
        usbScrollWheel = usbInterceptor->getScrollWheel();
        usbInterceptor->reset();

        // === This following section is for excluding MMB, MB4 and MB5 from hid reports

        // MMB is pressed, record the timestamp
        if (usbMouseButtons & MOUSE_MIDDLE) {
            // Log the data to FT232RL so the program can enable scripts
            FT232RL.print("M: ");
            FT232RL.print(usbMouseButtons, HEX);
            FT232RL.println();

            // Excluding MMB from the report & start the timer for logging
            activationTimestamp4MouseButtonExclusion = millis();
            usbMouseButtons &= ~MOUSE_MIDDLE;
        }

        // Check if BUTTON4 events should be excluded from the mouse report
        if ((usbMouseButtons & MOUSE_BUTTON4) && !(usbPreviousMouseButtons & MOUSE_BUTTON4) && (millis() - activationTimestamp4MouseButtonExclusion) <= BUTTON_EXCLUSION_DURATION_MS) {
            // Log the data to FT232RL so the program can enable scripts
            FT232RL.print("M: ");
            FT232RL.print(usbMouseButtons, HEX);
            FT232RL.println();

            // Exclude BUTTON4 by clearing its bit
            usbMouseButtons &= ~MOUSE_BUTTON4;
        }

        // Check if BUTTON5 events should be excluded from the mouse report
        if ((usbMouseButtons & MOUSE_BUTTON5) && !(usbPreviousMouseButtons & MOUSE_BUTTON5) && (millis() - activationTimestamp4MouseButtonExclusion) <= BUTTON_EXCLUSION_DURATION_MS) {
            // Log the data to FT232RL so the program can enable scripts
            FT232RL.print("M: ");
            FT232RL.print(usbMouseButtons, HEX);
            FT232RL.println();

            // Exclude BUTTON5 by clearing its bit
            usbMouseButtons &= ~MOUSE_BUTTON5;
        };

        // Notify Serial Device about RMB events (up and down)
        if (usbMouseButtons & MOUSE_RIGHT) {
            if (!(usbPreviousMouseButtons & MOUSE_RIGHT)) {
                // RMB was just pressed, send a message for press event
                FT232RL.print("M: ");
                FT232RL.print(usbMouseButtons, HEX);
                FT232RL.println();
            }
        } else if (usbPreviousMouseButtons & MOUSE_RIGHT) {
            // RMB was just released, send a message for release event
            FT232RL.print("M: ");
            FT232RL.print(usbMouseButtons, HEX);
            FT232RL.println();
        }

        // Notify Serial Device about RMB events (up and down)
        if (usbMouseButtons & MOUSE_LEFT) {
            if (!(usbPreviousMouseButtons & MOUSE_LEFT)) {
                // RMB was just pressed, send a message for press event
                FT232RL.print("M: ");
                FT232RL.print(usbMouseButtons, HEX);
                FT232RL.println();
            }
        } else if (usbPreviousMouseButtons & MOUSE_LEFT) {
            // RMB was just released, send a message for release event
            FT232RL.print("M: ");
            FT232RL.print(usbMouseButtons, HEX);
            FT232RL.println();
        }


    }

    if(hasSerialData) {
        serialPreviousMouseButtons = serialInterceptor->getPreviousMouseButtons();
        serialMouseButtons = serialInterceptor->getMouseButtons();
        serialScrollWheel = serialInterceptor->getScrollWheel();
        serialXMovement = serialInterceptor->getXMovement();
        serialYMovement = serialInterceptor->getYMovement();
        serialInterceptor->reset();

        // Set the lockout timestamp if the program sends a mouse movement event
        if (serialScrollWheel == 1) {
            activationTimestamp4MouseMovementLockout = millis() + lockoutDuration;
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
        switch(buttonId) {
            case 1: buttonMask = MOUSE_LEFT; break;
            case 2: buttonMask = MOUSE_RIGHT; break;
            case 4: buttonMask = MOUSE_MIDDLE; break;
            case 8: buttonMask = MOUSE_BUTTON4; break;
            case 16: buttonMask = MOUSE_BUTTON5; break;
            default:
                FT232RL.println(String("E: Unknown ButtonID -> ") + buttonId);
                continue;
        }

        if(buttonMask == MOUSE_LEFT) {
            // If USB mouse is still holding the button, ignore serial release event
            if ((usbButtons & MOUSE_LEFT) && !(serialButtons & MOUSE_LEFT) && (previousSerialButtonsState & MOUSE_LEFT)) {
                continue;
            }
            // If both USB and serial indicate release, then release the button
            if (!(usbButtons & MOUSE_LEFT) && !(serialButtons & MOUSE_LEFT)) {
                if (previousUsbButtonsState & MOUSE_LEFT || previousSerialButtonsState & MOUSE_LEFT) {
                    Mouse.release(buttonMask);
                }
                continue;
            }
            // If either USB or serial indicate press, then press the button
            if (usbButtons & MOUSE_LEFT || serialButtons & MOUSE_LEFT) {
                Mouse.press(buttonMask);
            }
        }
        else {
            // Handle all other buttons normally
            if (usbButtons & buttonId) {
                if (!(previousUsbButtonsState & buttonId)) {
                    Mouse.press(buttonMask);
                }
            }
            else {
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
