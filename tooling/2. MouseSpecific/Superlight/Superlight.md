# Superlight HID Report Data Structure README

## Overview
This README provides a detailed explanation of the data structure for HID (Human Interface Device) reports specific to the Superlight mouse. Each report is composed of 13 bytes, each serving distinct roles, primarily in conveying information about mouse button states, X and Y-axis movements, and scroll wheel actions.

## Report Structure
Each HID report from the Superlight mouse is 13 bytes long. The structure and purpose of each byte are as follows:

### Byte Breakdown

1. **Byte 1 - Mouse Button States**
   - Indicates the state of the mouse buttons.
   - Value Mapping:
     - `00`: No button pressed
     - `01`: Left Mouse Button (LMB)
     - `02`: Right Mouse Button (RMB)
     - `04`: Middle Mouse Button (MMB, usually the scroll wheel click)
     - `08`: Mouse Button 4 (MB4)
     - `10`: Mouse Button 5 (MB5)

2. **Bytes 2 & 3 - X-Axis Movement**
   - Represents horizontal movement (left/right) of the mouse.
   - Two's complement, 16-bit integer in little-endian format.
   - Range: -32768 (`8000` Hex) to 32767 (`7FFF` Hex).

3. **Bytes 4 & 5 - Y-Axis Movement**
   - Represents vertical movement (up/down) of the mouse.
   - Same format and range as the X-axis movement.

4. **Byte 8 - Scroll Wheel Movement**
   - Reflects the action of the scroll wheel.
   - Value Mapping:
     - `ff`: Scroll Down (one step)
     - `01`: Scroll Up (one step)

5. **Bytes 6, 7, and 9 to 13 - Unknown/Unused**
   - These bytes are typically `00` or show constant values.
   - Their specific purpose may vary based on different mouse models or configurations.

## Example HID Reports

### Report 1: Quick Movement to the Left with Slight Upward Motion
- **HID Report**: `00 00 43 ff ce ff 00 00 01 93 40 00 00`
- **Interpretation**:
  - No buttons pressed.
  - X-Axis (`43 ff` read as `ff43`): Leftward movement (-189 in decimal).
  - Y-Axis (`ce ff` read as `ffce`): Slight upward movement (-50 in decimal).

### Report 2: Rapid Movement to the Right
- **HID Report**: `00 00 b9 00 32 00 00 00 01 93 40 00 00`
- **Interpretation**:
  - No buttons pressed.
  - X-Axis (`b9 00` read as `00b9`): Rightward movement (185 in decimal).
  - Y-Axis (`32 00` read as `0032`): Slight downward movement (50 in decimal).

## Interpretation Notes
- In these reports, X and Y-axis movement data are interpreted using two's complement notation in little-endian format. It is crucial to read the values in reverse order (right to left) and then interpret them as signed integers.
- The function of unknown/unused bytes may change based on the specific configuration of the device and might not be active in standard mouse functions.