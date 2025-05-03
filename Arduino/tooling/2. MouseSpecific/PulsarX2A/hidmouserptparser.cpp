#include "hidmouserptparser.h"
#include "The16BitDataParser.h"

The16BitDataParser the16BitDataParser;

HIDMouseReportParser::HIDMouseReportParser(void*) : previousButtonsState(0), mmbTimestamp(0) {}

void HIDMouseReportParser::Parse(USBHID* hid, bool is_rpt_id, uint8_t len, uint8_t* buf)
{
    // Print report data
    Serial1.print("MRPT: ");
    for (uint8_t i = 0; i < len; i++) {
        Serial1.print(buf[i], HEX);
        Serial1.print(" ");
    }
    Serial1.println();

    // MMB is pressed, record the timestamp
    if (buf[0] & 4)
    {
        mmbTimestamp = millis();
        buf[0] &= ~4;
        Serial1.println("MSG: MMB Detected, excluding it from the report.");
    }

    // Check if MB5 events should be excluded from the hid report
    if (!(previousButtonsState & 0x10) && (millis() - mmbTimestamp) <= 1200)
    {
        buf[0] &= ~0x10; // Exclude MB5 by clearing its bit
        Serial1.println("MSG: MB5 Detected; excluding it from the mouse report.");
    }

    // Handle button events
    for (uint16_t buttonId = 1; buttonId <= 16; buttonId <<= 1)
    {
        if (buf[0] & buttonId)
        {
            if (!(previousButtonsState & buttonId))
            {
                onButtonDown(buttonId);
            }
        }
        else
        {
            if (previousButtonsState & buttonId)
            {
                onButtonUp(buttonId);
            }
        }
    }

    // Parse Data for the Mouse16Bit hid report
    previousButtonsState = buf[0];
    int16_t xMovement, yMovement;
    int8_t scrollValue = buf[5];
    xMovement = the16BitDataParser.parseX(buf);
    yMovement = the16BitDataParser.parseY(buf);

    if (xMovement != 0 || yMovement != 0 || scrollValue != 0)
    {
        onMouseMove(xMovement, yMovement, scrollValue);
    }
}
