class RazerViperV2Pro16BitDataParser {
public:

  uint8_t formatDataIntoHIDReport(uint8_t *buf)
  {
//        Serial1.print("MSG: Arduino formatDataIntoHIDReport() -> ");
//        for (uint8_t i = 0; i < 8; i++) {
//          Serial1.print(' ');
//          Serial1.print(buf[i], HEX);
//        }
//        Serial1.println(' ');
    // // Destructure the current report.
    // int8_t buttons = buf[0]; // Button Values (LMB, RMB, MB4, MB5, Scroll Wheel Click)
    // int8_t xMovement = buf[1]; // X Movement within -127..128 (0x80..0x7F)
    // int8_t yMovement = buf[2]; // Y Movement within -127..128 (0x80..0x7F)
    // int8_t scrollWheel = buf[3]; // Scroll Wheel Up & Down -127 or 128 (0x80 or 0x7F)
    // int8_t xMovementOverflow1 = buf[4]; // X Movement Overflow 1, used to calculate move beyond the -127..128 range into -32768..32767
    // int8_t xMovementOverflow2 = buf[5]; // X Movement Overflow 2, used to calculate move beyond the -127..128 range into -32768..32767
    // int8_t yMovementOverflow1 = buf[6]; // Y Movement Overflow 1, used to calculate move beyond the -127..128 range into -32768..32767
    // int8_t yMovementOverflow2 = buf[7]; // Y Movement Overflow 2, used to calculate move beyond the -127..128 range into -32768..32767

    // // Process the overflow values for the mouse movement beyond -127..128 range
    // int16_t upperX, upperY;
    // upperX = parseX(buf);
    // upperY = parseY(buf);

    // // Format the data into something the HIDReport understands.
    // uint8_t newHidReport[6];
    // newHidReport[0] = buttons;
    // newHidReport[1] = xMovement; // Lower value of the x movement
    // newHidReport[2] = static_cast<uint8_t>(upperX >> 8); // Upper 8 bits of X movement
    // newHidReport[3] = xMovement; // Lower value of the y movement
    // newHidReport[4] = static_cast<uint8_t>(upperY >> 8); // Upper 8 bits of Y movement
    // newHidReport[5] = scrollWheel;

    // return newHidReport;
  }

  int16_t parseX(const uint8_t* data) {
    int16_t x;
    uint8_t overflowByte = data[4];

    if (overflowByte == 0x80 || overflowByte == 0x7F) {
      x = twosComplement(data[5], data[4]);
    } else {
      x = static_cast<int16_t>(data[4]);
      if (x & (1 << 7)) {
        x -= 1 << 8;
      }
    }

    return x;
  }

  int16_t parseY(const uint8_t* data) {
    int16_t y;
    uint8_t overflowByte = data[6];

    if (overflowByte == 0x80 || overflowByte == 0x7F) {
      y = twosComplement(data[7], data[6]);
    } else {
      y = static_cast<int16_t>(data[6]);
      if (y & (1 << 7)) {
        y -= 1 << 8;
      }
    }

    return y;
  }
  int16_t twosComplement(uint8_t highByte, uint8_t lowByte) {
    int16_t value = (highByte << 8) | lowByte;
    if (value & (1 << 15)) {
      value -= 1 << 16;
    }
    return value;
  }
};
