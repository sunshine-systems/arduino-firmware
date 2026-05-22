# Stream Cheats Firmware

This repository contains the firmware and host-side tooling for the Stream Cheats "Mouse in the Middle" project — an Arduino Leonardo (and now Teensy 4.1) that sits between a USB mouse and the host PC, intercepts HID reports, and can modify, suppress, or inject mouse events under control of a PC-side Python program over a serial link.

## Repository layout

```
.
├── Arduino/
│   ├── streamcheats firmware/   Latest (V5-schema) firmware + Python host code
│   │   ├── Arduino Code/        Per-device firmware sketches, grouped by spoof variant
│   │   └── Python Code/         Host-side serial interface
│   └── tooling/                 Standalone Arduino sketches for device discovery / HID capture
├── Teensy/
│   └── Tests/                   Experimental Teensy 4.1 sketches (mouse, USB host, FT232H, HC-06, latency)
├── MouseInTheMiddle.md          Serial logging prefix conventions (S / E / I / M / V)
└── README.md                    (this file)
```

## Where to start

- **Building / flashing firmware:** [`Arduino/streamcheats firmware/README.md`](./Arduino/streamcheats%20firmware/README.md)
- **Wire protocol between PC and firmware:** [`Arduino/streamcheats firmware/Arduino Code/FIRMWARE_PROTOCOL.md`](./Arduino/streamcheats%20firmware/Arduino%20Code/FIRMWARE_PROTOCOL.md)
- **Adding support for a new mouse:** [`Arduino/streamcheats firmware/Arduino Code/simplified-firmware-guide.md`](./Arduino/streamcheats%20firmware/Arduino%20Code/simplified-firmware-guide.md)
- **Tools for discovering a new device (PID, HID descriptor):** [`Arduino/tooling/README.md`](./Arduino/tooling/README.md)
- **Teensy experiments:** [`Teensy/README.md`](./Teensy/README.md)

## Firmware version

The on-device `FIRMWARE_VERSION` (currently `5.0`) refers to the **serial wire schema** between the firmware and the Python host — not the per-device HID parsing code. All sketches under `streamcheats firmware/Arduino Code/` speak the same V5 schema regardless of which mouse or spoof variant they target.

The current work on the `update-firmware-to-match-latest-teensy` branch is aligning this firmware with the newer Teensy 4.1 implementation under `Teensy/`.

## History

Earlier schema versions (V1–V4) were removed during cleanup; they are available via git history before commit `a75e5b7`.
