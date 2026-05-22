# Teensy 4.1 Experiments

Research sketches exploring whether a Teensy 4.1 can replace the Arduino Leonardo + USB Host Shield combo used by the [main firmware](../Arduino/streamcheats%20firmware/). The Teensy has native USB host support (USB1) and a separate USB device peripheral (USB0), so a single board can both sniff a mouse and present itself as one — without the SPI bottleneck of the Leonardo + Host Shield.

The work on the `update-firmware-to-match-latest-teensy` branch is aimed at bringing the Arduino firmware's behaviour in line with what these Teensy sketches do.

## Test sketches

| Folder | Purpose |
|---|---|
| `teensy_mouse_move_test/` | Smoke test — Teensy enumerates as a mouse and emits alternating ±50 px moves at the maximum the USB host will negotiate. Used to validate the toolchain and confirm polling rate behaviour. |
| `usb_parser/` | Parses incoming HID reports from a USB mouse plugged into the Teensy's USB host port. The Teensy-side counterpart to `USBMouseHIDReportInterceptor` on the Leonardo. |
| `ft232h_communication/` | Loopback / throughput test for talking to the PC via an FT232H (the planned replacement for the Leonardo's onboard FTDI). |
| `hc-06_configuration_fixer/` | Utility to reconfigure HC-06 Bluetooth modules — used during the Bluetooth-vs-FT232H latency comparison. |
| `Latency Loopback/` | End-to-end PC↔Teensy round-trip latency test. `python/main.py` sends 10-byte packets, `teensy/teensy.ino` echoes them back. See [`test_results.md`](./Tests/Latency%20Loopback/test_results.md) — TL;DR: FT232H at 115200 averages ~1.5 ms RTT vs. ~44 ms for HC-06 at the same baud. |
| `Library Mods/` | Patched copies of PJRC's `USBHost_t36`. Two attempts: |
| | • `USBHost_t36 - 1 - force FS worked kinda` — forced Full-Speed enumeration of an upstream HS device. Partial success. |
| | • `USBHost_t36 - 2 - HS Working` — current working High-Speed version. Drop this over the stock `USBHost_t36` install when working on Teensy USB host code. |

## Hardware notes

- All sketches assume **Teensy 4.1** (Tools → Board → Teensy 4.1 in Teensyduino).
- Sketches that present as a USB mouse require **USB Type: Mouse** (or a composite that includes Mouse).
- Sketches that read from a USB mouse use the Teensy 4.1 USB host port (the dedicated 5-pin header, *not* the micro-USB).
