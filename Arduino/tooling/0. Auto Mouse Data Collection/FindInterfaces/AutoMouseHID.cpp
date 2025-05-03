// AutoMouseHID.cpp
#include "AutoMouseHID.h"

void AutoMouseHIDParser::Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) {
    if (!buf) return;

    // Print raw data
    Serial1.print(F("Raw Data: "));
    for (uint8_t i = 0; i < len; i++) {
        if (buf[i] < 0x10) Serial1.print("0");
        Serial1.print(buf[i], HEX);
        Serial1.print(" ");
    }
    Serial1.println();
}