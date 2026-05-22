# Stream Cheats Firmware (V5 schema)

The Arduino Leonardo firmware that intercepts USB mouse HID reports, talks to a PC over a serial link, and re-emits (spoofed) mouse events back to the host. The PC side lives in [`Python Code/`](./Python%20Code/).

## Layout

```
streamcheats firmware/
├── Arduino Code/
│   ├── FIRMWARE_PROTOCOL.md                    Wire schema (what V5 means)
│   ├── simplified-firmware-guide.md            How to capture HID data for a new mouse
│   ├── Spoof - SOLAKAKA SM900/                 Spoof variant A (composite HID)
│   └── Spoof - Synaptics Generic HID/          Spoof variant B (HID only)
├── Python Code/                                 Host-side serial driver
└── RUN Live Data Read Test.bat                  Launcher for the read-loop test
```

## Spoof variants

The two top-level `Spoof - *` directories contain **the same firmware speaking the same V5 protocol**. They only differ in:

1. **What USB device the Leonardo presents itself as to the host PC.**
   - `Spoof - SOLAKAKA SM900` impersonates a SOLAKAKA SM900 and uses a composite HID descriptor (mouse + keyboard, see `CompositeHID.cpp/.h`).
   - `Spoof - Synaptics Generic HID` impersonates a Synaptics Generic HID device and uses the stock Arduino mouse stack (`Mouse16Bit.cpp/.h`).
2. **Which upstream mice are supported.** Each variant has its own set of per-device subfolders (Viper V3 Pro, Glorious Model O, etc.). The set differs because not every mouse has been ported to both spoofs.

If you're modifying firmware behaviour, change it in **both** variants — they share logic by copy, not by include.

## Per-device folders

Inside each spoof variant, each subfolder (e.g. `Viper V3 Pro/MouseInTheMiddle/`) is a self-contained Arduino sketch for one specific upstream mouse. They differ only in the HID-report parsing layer (`MathAndConversions.h`, `USBMouseHIDReportInterceptor.cpp`) because each mouse has its own HID report format. The protocol, settings, and spoofing logic are identical across devices.

Special folders under `Spoof - SOLAKAKA SM900/`:
- `1. Test Spoof/` — minimal composite-HID test sketch, useful for verifying the host enumerates the spoofed device correctly before bringing in interception logic.
- `2. Development Viper V2 Pro/` and `3. Development Glorious Model O Wireless/` — working scratch copies used while porting a new mouse; not flight code.

## Build / IDE setup

All spoof variants ship with the same Arduino IDE helpers at the spoof root:

| File | Purpose |
|---|---|
| `Arduino1.8.18 Modifications.zip` | Patched Arduino core needed for the composite HID / 16-bit mouse descriptors |
| `Setup Full IDE.bat` | One-shot IDE setup |
| `DEV_CopyAVRCore.bat` | Backs up the stock AVR core before patching |
| `DEV_ReplaceAVRCore.bat` | Drops the patched core into the IDE install |
| `boards.txt` (SOLAKAKA only) | Custom board entry for the SOLAKAKA spoof |

Open the per-device `MouseInTheMiddle.ino` in the patched Arduino 1.8.18, select the Leonardo (or the custom board entry for the SOLAKAKA spoof), and flash.

## Source file roles (per device folder)

| File | Role |
|---|---|
| `MouseInTheMiddle.ino` | Arduino entry point (`setup` / `loop`) |
| `MouseInTheMiddle.h` | Global object wiring (USB host, interceptors, spoofer) |
| `Config.h` / `Config.cpp` | Compile-time + runtime configuration knobs and `FIRMWARE_VERSION` |
| `USBMouseHIDReportInterceptor.*` | Parses HID reports coming from the upstream mouse (device-specific) |
| `SerialMouseHIDReportInterceptor.*` | Parses spoofed HID reports + settings frames arriving over serial from the PC |
| `MouseEventSpoofer.*` | Decides what mouse event (if any) to emit to the host PC each loop |
| `FirmwareSettings.*` | Applies setting-id/value pairs received over serial (see protocol doc) |
| `MathAndConversions.h` | Device-specific X/Y byte unpacking — the part that varies most between mice |
| `CompositeHID.*` (SOLAKAKA only) | Custom USB descriptor for the composite mouse+keyboard device |
| `Mouse16Bit.*` (Synaptics only) | 16-bit mouse descriptor wrapper |

## Python host

[`Python Code/src/`](./Python%20Code/src/) contains the PC-side counterpart. Entry points:

- `test_movement.py` — sends spoofed movement frames
- `test_read_incoming_data_loop.py` — prints whatever the firmware logs back

Run via `pipenv` using the `Pipfile` at `Python Code/`.
