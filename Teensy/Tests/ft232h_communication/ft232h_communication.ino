/*
  Teensy 4.1 to HC-06 Bluetooth BT_Serial Communication Example
  Connects Teensy BT_Serial1 (Pins 0 RX, 1 TX) to an HC-06 module.
  Relays data between the Arduino BT_Serial Monitor (USB) and the Bluetooth device.
*/

// Define which hardware BT_Serial port we are using
#define BT_BT_Serial BT_Serial1 // Using BT_Serial1 (Pins 0 RX, 1 TX)

// Define the baud rate - MUST match the HC-06 module's setting!
// Default for HC-06 is usually 9600.
#define BT_BAUD 9600

void setup() {
  // Start the built-in USB BT_Serial for debugging/communication with BT_Serial Monitor
  BT_Serial.begin(9600); // Or any desired baud rate for the monitor
  // while (!BT_Serial && millis() < 4000) {
  //   // Wait for BT_Serial Monitor to open, with a timeout
  // }
  delay(1000); // Short delay for stability

  BT_Serial.println("Teensy 4.1 HC-06 Bluetooth BT_Serial Example");
  BT_Serial.print("Initializing Bluetooth BT_Serial (BT_Serial1) at ");
  BT_Serial.print(BT_BAUD);
  BT_Serial.println(" bps...");

  // Start the hardware BT_Serial port connected to the HC-06
  BT_BT_Serial.begin(BT_BAUD);

  BT_Serial.println("Setup complete. Power cycle HC-06 if needed.");
  BT_Serial.println("Pair your device (PC/Phone) with HC-06.");
  BT_Serial.println("Connect via Bluetooth BT_Serial / Virtual COM port.");
  BT_Serial.println("Type in BT_Serial Monitor to send over Bluetooth.");
  BT_Serial.println("Data received via Bluetooth will be printed here.");
}

void loop() {
  BT_Serial.println("testing");
  // Check if data has come IN from the HC-06 (via BT_Serial1)
  if (BT_BT_Serial.available() > 0) {
    char receivedChar = BT_BT_Serial.read();
    // Print the received character to the Arduino BT_Serial Monitor (USB)
    BT_Serial.print(receivedChar);
  }

  // Check if data has been sent FROM the Arduino BT_Serial Monitor (USB)
  if (BT_Serial.available() > 0) {
    char sendChar = BT_Serial.read();
    // Send the character OUT to the HC-06 (via BT_Serial1)
    BT_BT_Serial.print(sendChar);
  }
}
