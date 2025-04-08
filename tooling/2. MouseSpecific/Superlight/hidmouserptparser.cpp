#include "hidmouserptparser.h"
#include "RazerViperV2Pro16BitDataParser.h"

RazerViperV2Pro16BitDataParser razerViperV2Pro16BitDataParser;

HIDMouseReportParser::HIDMouseReportParser(void*) : previousButtonsState(0), mmbTimestamp(0) {}

void HIDMouseReportParser::Parse(USBHID* hid, bool is_rpt_id, uint8_t len, uint8_t* buf)
{
    Serial1.print("MRPT:");
    for (uint8_t i = 0; i < len; i++)
    {
        Serial1.print(' ');
        Serial1.print(buf[i], HEX);
    }
    Serial1.println();

}
