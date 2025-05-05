#include <Arduino.h>
#include "config.h" // Include our configuration definitions

unsigned long loopCounter = 0;

void setup() {
  // 1. Initialize the debug serial port (defined in config.h)
  DEBUG_SERIAL_PORT.begin(DEBUG_BAUD_RATE);

  // 2. Small delay to allow Serial to stabilize (optional but sometimes helpful)
  delay(500);

  // 3. Print startup messages
  DEBUG_SERIAL_PORT.println("\n\n--- Teensy Core Modification Baseline Sketch ---");
  DEBUG_SERIAL_PORT.println("----------------------------------------------");
  DEBUG_SERIAL_PORT.printf("Debug Port: Initialized @ %lu baud\n", (unsigned long)DEBUG_BAUD_RATE);

  // 4. Initialize built-in LED for visual feedback
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // Turn LED ON during setup
  DEBUG_SERIAL_PORT.println("LED Initialized: ON");

  DEBUG_SERIAL_PORT.println("Setup complete. Entering loop...");
  DEBUG_SERIAL_PORT.println("----------------------------------------------");
  DEBUG_SERIAL_PORT.flush(); // Ensure messages are sent before loop starts

  digitalWrite(LED_BUILTIN, LOW); // Turn LED OFF before loop
}

void loop() {
  // 1. Indicate loop is running via Serial
  loopCounter++;
  DEBUG_SERIAL_PORT.printf("Loop iteration: %lu\n", loopCounter);

  // 2. Blink the LED
  digitalWrite(LED_BUILTIN, HIGH); // LED ON
  delay(100);                     // Keep LED on briefly
  digitalWrite(LED_BUILTIN, LOW);  // LED OFF

  // 3. Delay before next loop iteration
  delay(900); // Makes the total loop time approx 1 second
}