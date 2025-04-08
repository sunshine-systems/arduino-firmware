#include "MouseInTheMiddle.h"
#include "Config.h"

// Global variables
DeviceInfo* connectedDevice = nullptr; // Pointer to store information about the connected USB device
bool hasReadConnectedDevice = false; // Flag to check if device detection has occurred

void setup() {
  FT232RL.begin(BAUD); // Initialize serial communication
  FT232RL.setTimeout(1);
  FT232RL.println("S: Arduino Leonardo Started.");

  // Initialize USB Host Shield
  if (Usb.Init() == -1) {
    FT232RL.println("E: Host Shield did not start.");
  } else {
    FT232RL.println("S: USB Host Shield Started.");
  }

  delay(200);

  // Set the report parser for the USB Host Shield
  if (!Hid.SetReportParser(0, &usbInterceptor)) {
    ErrorMessage<uint8_t >(PSTR("SetReportParser"), 1);
  } else {
    FT232RL.println("S: ReportParser set successfully.");
  }

  delay(500);
}

void loop() {
  Usb.Task(); // Regularly process USB tasks

  // Detect connected USB device if not already done
  if (!hasReadConnectedDevice) {
    hasReadConnectedDevice = detectDevice();
  } else {
    // Regular operations once the device is detected
    serialInterceptor.sniffForSpoofableDataOverSerial();
    mouseEventSpoofer.spoofEvent();
  }
}

// Function to detect the connected USB device
bool detectDevice() {
  static unsigned long lastAttempt = 0; // Track the last attempt time
  const unsigned long detectionInterval = 5000; // Detection attempt interval (5 seconds)
  // Check if the interval has passed to prevent frequent detection attempts
  if (millis() - lastAttempt < detectionInterval) {
    return false;
  }

  FT232RL.println("S: Attempting to connect to USB Device.");

  lastAttempt = millis(); // Update the last attempt time

  uint8_t buf[12]; // Buffer to hold the device descriptor
  
  // Attempt to get the device descriptor
  if (Usb.getDevDescr(1, 0, 12, buf) != 0) {
    FT232RL.println("E: Failed getting device information, re-plugin the device.");
    return false;
  }

  char pidStr[5]; // Buffer for formatted PID
  formatPID(buf, pidStr);

  // Match the formatted PID with known devices
  return matchDevice(pidStr);
}

// Function to format the PID into a string
void formatPID(uint8_t* buf, char* pidStr) {
  // Extract PID from the device descriptor
  uint16_t pid = (buf[11] << 8) | buf[10];
  // Convert PID to a hexadecimal string
  sprintf(pidStr, "%04X", pid);
}

// Function to match the PID with known devices
bool matchDevice(const char* pidStr) {
  // Loop through the list of supported devices
  for (int i = 0; i < sizeof(supportedDevices) / sizeof(DeviceInfo); i++) {
    // Check if the PID matches any supported device
    if (strcmp(supportedDevices[i].pid, pidStr) == 0) {
      connectedDevice = &supportedDevices[i]; // Set the connected device
      FT232RL.print("S: Connected to device: ");
      FT232RL.println(connectedDevice->name);
      return true;
    }
  }
  FT232RL.println("E: Device not recognized."); // Log warning if no device is matched
  return false;
}
