// AutoMouseHID.h
#ifndef AUTOMOUSEHID_H
#define AUTOMOUSEHID_H

#include <hidboot.h>
#include "MouseInterfaceFinder.h"

class AutoMouseHIDParser : public HIDReportParser {
private:
    int16_t xMovement;
    int16_t yMovement;

public:
    AutoMouseHIDParser() : xMovement(0), yMovement(0) {}
    void Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) override;
};

#endif // AUTOMOUSEHID_H