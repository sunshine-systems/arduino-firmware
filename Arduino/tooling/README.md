# Arduino Tooling

Standalone Arduino sketches used during the workflow of bringing up a new mouse for the [streamcheats firmware](../streamcheats%20firmware/). These are not part of the production firmware; you flash them temporarily, capture what you need, then reflash the real firmware.

The folders are numbered in roughly the order you use them when adding a new device.

## `0. Auto Mouse Data Collection/FindInterfaces`

Walks the connected USB device, enumerates its interfaces, and identifies which one(s) act as a mouse. Useful when a device exposes multiple HID interfaces (composite mice often expose a separate keyboard / consumer-control interface).

## `1. GetPIDFromDevice/GetPIDFromDevice`

Prints the VID/PID of whatever USB device is currently plugged into the USB Host Shield. Step 1 of the porting flow described in the project's top-level README — you need the PID before you can spoof a new mouse.

## `2. MouseSpecific/<mouse>`

Per-mouse experimental sketches (PulsarX2A, Superlight, ViperV2Pro, ViperV3Hyperspeed). Each is a stripped-down version of the firmware focused on parsing **only** that mouse's HID reports, so you can iterate on `MathAndConversions`-equivalent logic without dragging in the full spoofing pipeline. The accompanying `.md` files (where present) document quirks observed for that mouse.

## `3. GetAllHIDDeviceData/GetAllHIDDeviceData`

Dumps every raw HID report the connected device sends, with no filtering. Use this when you don't yet know which reports a mouse emits or when you suspect non-standard report IDs.

## Typical porting flow

1. `1. GetPIDFromDevice` — grab the VID/PID.
2. `0. Auto Mouse Data Collection/FindInterfaces` — figure out which interface to subscribe to.
3. `3. GetAllHIDDeviceData` — capture raw reports while exercising the mouse (see [`simplified-firmware-guide.md`](../streamcheats%20firmware/Arduino%20Code/simplified-firmware-guide.md) for capture technique).
4. `2. MouseSpecific/<mouse>` — prototype the parser.
5. Copy the working parser into a new device folder under one of the `Spoof - *` directories and ship it.
