Superlight HID Report README
This document provides a detailed analysis of the HID (Human Interface Device) reports for a Superlight mouse. The reports are based on a 13-byte structure, focusing particularly on mouse movements and button states.

Report Structure Overview
Each report consists of 13 bytes, each serving a specific purpose:

Interpretation Notes
    The X and Y movement data use a two's complement format in a little-endian byte order. This means the values should be read in reverse order (right to left) and then interpreted as signed integers.
    The actual meaning and use of the unknown/unused bytes may vary based on different models or configurations and are not active in the standard mouse movements.

Byte 1: Mouse Button States
Bytes 2-3: X-Axis Movement
Bytes 4-5: Y-Axis Movement
Bytes 6-7: Unknown/Unused
Byte 8: Scroll Wheel Movement
Bytes 9-13: Unknown/Unused
Detailed Byte Breakdown

Mouse Button States (Byte 1)
    00: No button pressed
    01: Left Mouse Button (LMB)
    02: Right Mouse Button (RMB)
    04: Middle Mouse Button (MMB)
    08: Mouse Button 4 (MB4)
    10: Mouse Button 5 (MB5)

X-Axis Movement (Bytes 2-3)
    Two's Complement, 16-bit, Little-Endian format
    Range: -32768 (8000 Hex) to 32767 (7FFF Hex)

Y-Axis Movement (Bytes 4-5)
    Same format and range as X-Axis Movement

Scroll Wheel Movement (Byte 8)
    ff: Scroll Down (one step)
    01: Scroll Up (one step)

Unknown/Unused (Bytes 6-7, 9-13)
    Typically 00 or constant values

Quick Movement Left and Slightly Up
Report: 00 00 43 ff ce ff 00 00 01 93 40 00 00
    Interpretation:
    No buttons pressed
    X-Axis: 43 ff (read as ff43 Hex) = -189 (Decimal), movement to the left
    Y-Axis: ce ff (read as ffce Hex) = -50 (Decimal), slight upward movement

Rapid Movement Right
    Report: 00 00 b9 00 32 00 00 00 01 93 40 00 00
    Interpretation:
    No buttons pressed
    X-Axis: b9 00 (read as 00b9 Hex) = 185 (Decimal), movement to the right
    Y-Axis: 32 00 (read as 0032 Hex) = 50 (Decimal), slight downward movement
