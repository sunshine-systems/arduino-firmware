// MathAndConversions.h
class MathAndConversions {
public:

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

  int16_t parseX_A885(const uint8_t* data) {
    // Check the sign byte (data[2])
    if (data[2] == 0xFF) {
      // Negative value
      return -static_cast<int16_t>(256 - data[1]);
    } else if (data[2] != 0) {
      // Extended positive range
      return static_cast<int16_t>((data[2] << 8) | data[1]);
    } else {
      // Regular positive value
      return static_cast<int16_t>(data[1]);
    }
  }

  int16_t parseY_A885(const uint8_t* data) {
    // The same logic as parseX but using data[3] and data[4]
    if (data[4] == 0xFF) {
      // Negative value
      return -static_cast<int16_t>(256 - data[3]);
    } else if (data[4] != 0) {
      // Extended positive range
      return static_cast<int16_t>((data[4] << 8) | data[3]);
    } else {
      // Regular positive value
      return static_cast<int16_t>(data[3]);
    }
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
