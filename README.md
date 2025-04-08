# Stream Cheats Arduino Project 🚀

Welcome to the Stream Cheats Arduino Project! This guide will help you add a new device to the project. Follow these steps to ensure a smooth integration.

## Step 1: Get the Product ID (PID) 🧐
- Run the `GetPIDFromDevice` project on your Arduino.
- This will read and display the Product ID (PID) of the connected device.
- Note down the PID as it's essential for the next steps.

## Step 2: Clone and Analyze the Device Folder 📁
- Clone one of the existing `<MouseName>` folders as a starting point.
- Start capturing and logging the HID (Human Interface Device) reports of your device.
- Create a README in this folder detailing information about the device, including its behavior and any special features.

## Step 3: Update the MouseInTheMiddle Project 🖱️
- Edit the `Config.h` file in the `MouseInTheMiddle` project.
- Add the necessary configurations for your new device, using the PID obtained in Step 1.
- Ensure the configurations match the specifics of your device to enable correct functionality.

## Contributing 🤝
Feel free to contribute to this project by submitting pull requests or suggesting improvements. Every contribution helps make this project better!

Thank you for being a part of the Stream Cheats community! 🌟