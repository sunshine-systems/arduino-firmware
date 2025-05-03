# Razer Viper V2 Pro HID Report Structure :mouse:

## Overview
This README outlines the structure of HID (Human Interface Device) reports for the Razer Viper V2 Pro mouse. The HID reports from this device consist of 8 bytes, each designated to convey specific information about mouse interactions, including button presses, movements, and scroll actions.

## Report Structure
Each HID report for the Razer Viper V2 Pro mouse is structured as follows:

### Byte Breakdown

1. **Byte 0 - Mouse Buttons**
   - Contains the state of various mouse buttons.
   - Button Mapping:
     - Left click
     - Right click
     - Mouse Button 4
     - Mouse Button 5
     - Scroll Wheel Click

2. **Byte 1 - X Movement**
   - Represents horizontal movement of the mouse (left/right).
   - Range: -127 to 128

3. **Byte 2 - Y Movement**
   - Represents vertical movement of the mouse (up/down).
   - Same range as X Movement.

4. **Byte 3 - Scroll Wheel Movement**
   - Indicates the action of the scroll wheel (up and down).

5. **Bytes 4 & 5 - Overflow Value for X**
   - Used for an extended range of X-axis movement.
   - When these bytes are present, Byte 1 (X Movement) is ignored.
   - Represents an endian conversion of the X-axis movement.

6. **Bytes 6 & 7 - Overflow Value for Y**
   - Used for an extended range of Y-axis movement.
   - When these bytes are present, Byte 2 (Y Movement) is ignored.
   - Represents an endian conversion of the Y-axis movement.

## Overflow Values and Endian Conversion
- The overflow values for X and Y movements allow for capturing larger ranges of movement than the -127 to 128 range.
- Endian conversion in this context refers to how the multi-byte values (Bytes 4 & 5 for X, Bytes 6 & 7 for Y) are read and interpreted.
- The device driver prioritizes the overflow values over the standard X and Y movement bytes (Byte 1 and 2) when they are present, allowing for more precise tracking during rapid or extensive mouse movements.

## Interpretation Notes
- Understanding the endian conversion and the role of overflow values is crucial for accurately interpreting extended movements.
- The standard X and Y movement bytes are

sufficient for most mouse actions, but in cases of rapid or extensive movement, the overflow bytes provide enhanced precision.
- The mouse buttons are mapped in Byte 0, which indicates the state of each button in a specific bit pattern.