/*
 * Teensy 4.1 - Bluetooth Serial Echo
 *
 * Purpose: Listens on a hardware serial port (Serial1) connected to an
 *          HC-06 module and echoes back every byte received.
 *
 * Assumes:
 *   - HC-06 module is connected to Teensy Serial1 (Pins 0 RX, 1 TX).
 *   - HC-06 module has ALREADY been configured to use 115200 baud rate.
 *
 * Operation:
 *   - Receives data via Bluetooth (Serial1).
 *   - Immediately transmits the exact same data back via Bluetooth (Serial1).
 */

#include <Arduino.h>

//=============================================================================
// Configuration
//=============================================================================

// Define the hardware serial port connected to the HC-06 module
#define BT_Serial Serial1

// Define the baud rate the HC-06 module is expected to be using
// (MUST match the rate configured on the HC-06 and used by the Python script)
const unsigned long HC06_BAUD = 115200;

//=============================================================================
// setup() - Runs once on boot/reset
//=============================================================================
void setup() {
  // Initialize the hardware serial port for the HC-06
  BT_Serial.begin(HC06_BAUD);

  // Optional: Short delay for stability
  delay(200);

  // Optional: Indicate readiness (can be commented out)
  // BT_Serial.println("Teensy Echo Ready.");

  // Note: No USB Serial needed unless debugging this sketch itself
  // Serial.begin(115200);
}

//=============================================================================
// loop() - Runs repeatedly after setup()
//=============================================================================
void loop() {
  // Check if data is available from Bluetooth Serial
  if (BT_Serial.available() > 0) {
    // Read one byte
    int receivedByte = BT_Serial.read();

    // Check if read was valid (should always be > 0 if available() > 0)
    if (receivedByte != -1) {
      // Immediately write the byte back out
      BT_Serial.write((uint8_t)receivedByte);
    }
  }
  // The loop runs very fast, constantly checking for incoming bytes.
  // No delay() here ensures minimum latency for echoing.
}
