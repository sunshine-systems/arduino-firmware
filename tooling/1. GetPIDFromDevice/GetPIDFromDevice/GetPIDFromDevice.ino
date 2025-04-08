#include <usbhub.h>

USB Usb;

void setup() {
  Serial1.begin(115200);
  if (Usb.Init() == -1) {
    Serial1.println("USB host did not start");
    while(1); // halt
  }
  delay(200);
}

void loop() {
  Usb.Task();
  if ( Usb.getUsbTaskState() == USB_STATE_RUNNING ) {
    uint8_t buf[12]; // buffer to hold device descriptor
    Usb.getDevDescr(1, 0, 12, buf); // get first 12 bytes of device descriptor

    // Product ID is bytes 10 and 11 of the device descriptor
    uint16_t pid = (buf[11] << 8) | buf[10];

    // Buffer to hold the formatted Product ID
    char pidStr[5]; // 4 characters for the hex PID + 1 for the null terminator

    // Format the PID as a 4-character hexadecimal string with leading zeros
    sprintf(pidStr, "%04X", pid);

    Serial1.print("Product ID: ");
    Serial1.println(pidStr);
  }
}
