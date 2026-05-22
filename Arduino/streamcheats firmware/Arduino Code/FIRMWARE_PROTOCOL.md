# V5 Firmware Wire Protocol

The "V5" in `FIRMWARE_VERSION` refers to **this serial protocol**, spoken between the PC-side Python host and the Arduino Leonardo firmware over the FTDI link (115200 baud, no parity, 8N1). All sketches under `Arduino Code/Spoof - .../<device>/MouseInTheMiddle/` implement this same protocol regardless of which mouse they wrap.

## Frame format (PC → firmware)

Every PC-to-firmware frame is **9 bytes** total: a 1-byte length prefix followed by an 8-byte payload. The length prefix tells the firmware how many of the payload bytes are meaningful and what kind of frame it is.

```
+--------+-------------------------------------+
| length |              payload[8]             |
+--------+-------------------------------------+
   1 B                    8 B
```

| `length` | Meaning |
|---|---|
| `8` | Spoofed HID mouse report — see [HID frame](#hid-frame-length--8) |
| `3` | Settings update — see [Settings frame](#settings-frame-length--3) |
| anything else | Unrecognised — currently silently ignored |

Reference implementation: [`SerialMouseHIDReportInterceptor::sniffForSpoofableDataOverSerial`](./Spoof%20-%20SOLAKAKA%20SM900/Viper%20V3%20Pro/MouseInTheMiddle/SerialMouseHIDReportInterceptor.cpp).

### HID frame (`length == 8`)

```
byte  field                notes
----  -------------------  ---------------------------------------------
 0    mouseButtons         bitfield: LMB=0x01 RMB=0x02 MMB=0x04
                                     MB4=0x08 MB5=0x10
 1    xRanged              clamped X in int8 range, or 0x7F / 0x80 to
                           signal that the real value lives in bytes 4-5
 2    yRanged              same as above for Y (real value in bytes 6-7)
 3    flags                bit 0 = mouse-movement lockout active
 4-5  xFull (int16 LE)     full-resolution signed X movement
 6-7  yFull (int16 LE)     full-resolution signed Y movement
```

The "ranged" bytes exist because the upstream HID descriptors used by the spoof are 8-bit; the firmware uses the int16 fields to decide whether to fan a large movement out across multiple HID reports.

Reference implementations:
- Encoder: [`FirmwareInterface.create_spoofed_hid_report`](../Python%20Code/src/FirmwareInterface.py)
- Decoder: [`SerialMouseHIDReportInterceptor::processAndSetHIDReportData`](./Spoof%20-%20SOLAKAKA%20SM900/Viper%20V3%20Pro/MouseInTheMiddle/SerialMouseHIDReportInterceptor.cpp)

### Settings frame (`length == 3`)

```
byte  field          notes
----  -------------  ---------------------------------------------
 0    settingId      see table below
 1-2  value (int16 LE, signed)
 3-7  ignored        (padding to keep the frame 9 bytes total)
```

| `settingId` | Name | Notes |
|---:|---|---|
| 0 | `FIRMWARE_VERSION` | Read-only; writing it just causes the firmware to echo its version back on serial as `V: <version>` |
| 1 | `logPerformanceMetrics` | bool |
| 2 | `enableSensReduction` | bool |
| 3 | `sensReductionDurationMilliseconds` | ms |
| 4 | `sensReductionAmmountX` | 0 = full lockout, 100 = no reduction |
| 5 | `sensReductionAmmountY` | as above |
| 6 | `enableSpinning` | bool |
| 7 | `spinAmountPerRotation` | counts per rotation |
| 8 | `spinNumberOfRotations` | |
| 9 | `spinDelayBetweenRotationsMilliseconds` | ms |
| 10 | `spinLockoutMouseUntilCompletion` | bool |
| 11 | `spinBeforeAfterMouseEvent` | 0 = before, 1 = after, 2 = both |
| 12 | `disablePassthroughForMMB` | passthrough mode (see below) |
| 13 | `disablePassthroughForRMB` | passthrough mode |
| 14 | `disablePassthroughForLMB` | passthrough mode |
| 15 | `disablePassthroughForMB4` | passthrough mode |
| 16 | `disablePassthroughForMB5` | passthrough mode |

Passthrough modes:
- `0` — pass the button through normally
- `1` — never pass through
- `2` — only pass through if MMB was pressed within `BUTTON_EXCLUSION_DURATION_MS`
- `3` — only pass through if the button was double-tapped within `BUTTON_DOUBLE_TAP_TO_PASSTHROUGH_DURATION_MS`

Reference implementations:
- Encoder: [`FirmwareInterface.create_settings_report`](../Python%20Code/src/FirmwareInterface.py)
- Decoder: [`FirmwareSettings::updateSettings`](./Spoof%20-%20SOLAKAKA%20SM900/Viper%20V3%20Pro/MouseInTheMiddle/FirmwareSettings.cpp)

## Log lines (firmware → PC)

Outgoing serial is line-based ASCII with a one-letter prefix; the Python host dispatches on the prefix (see `FirmwareInterface.process_incoming_data`). Conventions are summarised in [`MouseInTheMiddle.md`](../../../MouseInTheMiddle.md):

| Prefix | Meaning |
|---|---|
| `S:` | Startup / status |
| `E:` | Error |
| `I:` | Info / debug (only when `DEBUG_MODE` is true) |
| `M:` | Mouse HID report log |
| `V:` | Firmware version reply (response to settingId `0`) |

## Versioning

Bumping `FIRMWARE_VERSION` in `Config.h` means **this protocol changed**. Per-device HID parsing changes (e.g. tweaking `MathAndConversions.h` for a new mouse) do **not** count as a schema bump.
