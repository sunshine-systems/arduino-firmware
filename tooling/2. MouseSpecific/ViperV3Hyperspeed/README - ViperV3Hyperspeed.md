# Razer Viper V3 Hyperspeed HID Report Structure :mouse:

## Overview
This README outlines the structure of HID (Human Interface Device) reports for the Razer Viper V3 Hyperspeed mouse. The HID reports from this device consist of 8 bytes, each designated to convey specific information about mouse interactions, including button presses, movements, and scroll actions.

## Report Structure
Each HID report for the Razer Viper V3 Hyperspeed mouse is structured as follows:

### Byte Breakdown

1. **Byte 0 - Mouse Buttons**
   - Contains the state of various mouse buttons.
   - Button Mapping:
     - Left click
     - Right click
     - Mouse Button 4
     - Mouse Button 5
     - Scroll Wheel Click

2. **Byte 1 - Unknown/Undocumented**
   - No specific documentation available for this byte.

3. **Byte 2 - Unknown/Undocumented**
   - No specific documentation available for this byte.

4. **Byte 3 - Scroll Wheel Movement**
   - Indicates the action of the scroll wheel (up and down).

5. **Byte 4 - X Movement**
   - Represents horizontal movement of the mouse (left/right).
   - Range: -127 to 128

6. **Byte 5 - Overflow Value for X**
   - Used for an extended range of X-axis movement.
   - When this byte is present, Byte 4 (X Movement) is considered for extended movement.
   - Represents an additional value to the X-axis movement for precise tracking.

7. **Bytes 6 & 7 - Y Movement**
   - Represents vertical movement of the mouse (up/down).
   - Byte 6 for the primary value and Byte 7 for overflow.
   - When these bytes are present, they offer an extended range for Y-axis movement.
   - Represents an endian conversion of the Y-axis movement for enhanced precision.

## Overflow Values and Endian Conversion
- The overflow values for X and Y movements allow for capturing larger ranges of movement than the standard ranges.
- Endian conversion in this context refers to how the multi-byte values (Byte 5 for X, Bytes 6 & 7 for Y) are read and interpreted.
- The device driver prioritizes the overflow values over the standard X and Y movement bytes (Byte 4 and 6) when they are present, allowing for more precise tracking during rapid or extensive mouse movements.

## Interpretation Notes
- Understanding the endian conversion and the role of overflow values is crucial for accurately interpreting extended movements.
- The standard X and Y movement bytes are sufficient for most mouse actions, but in cases of rapid or extensive movement, the overflow bytes provide enhanced precision.
- The mouse buttons are mapped in Byte 0, which indicates the state of each button in a specific bit pattern.
