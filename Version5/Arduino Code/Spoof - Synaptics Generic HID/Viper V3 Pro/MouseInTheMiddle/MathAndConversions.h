// MathAndConversions.h
class MathAndConversions {
public:
  // Current implementation for 00C1 device
  int16_t parseX_00C1(const uint8_t* data) {
    // Check if high byte indicates negative value
    if (data[5] == 0xFF || data[5] == 0xFE) {
      // Use full two's complement for larger negative numbers
      return twosComplement_00A6(data[5], data[4]);
    } else {
      // For positive values, just return the low byte
      return data[4];
    }
  }

  int16_t parseY_00C1(const uint8_t* data) {
    // Check if high byte indicates negative value
    if (data[7] == 0xFF || data[7] == 0xFE) {
      // Use full two's complement for larger negative numbers
      return twosComplement_00A6(data[7], data[6]);
    } else {
      // For positive values, just return the low byte
      return data[6];
    }
  }

  // Legacy implementations remain the same...
  int16_t parseX_00A6(const uint8_t* data) {
    int16_t x;
    uint8_t overflowByte = data[1];

    if (overflowByte == 0x80 || overflowByte == 0x7F) {
      x = twosComplement_00A6(data[5], data[4]);
    } else {
      x = static_cast<int16_t>(data[1]);
      if (x & (1 << 7)) {
        x -= 1 << 8;
      }
    }

    return x;
  }

  int16_t parseY_00A6(const uint8_t* data) {
    int16_t y;
    uint8_t overflowByte = data[2];

    if (overflowByte == 0x80 || overflowByte == 0x7F) {
      y = twosComplement_00A6(data[7], data[6]);
    } else {
      y = static_cast<int16_t>(data[2]);
      if (y & (1 << 7)) {
        y -= 1 << 8;
      }
    }

    return y;
  }

  int16_t twosComplement_00A6(uint8_t highByte, uint8_t lowByte) {
    int16_t value = (highByte << 8) | lowByte;
    if (value & (1 << 15)) {
      value -= 1UL << 16;
    }
    return value;
  }
};

extern MathAndConversions mathAndConversions;