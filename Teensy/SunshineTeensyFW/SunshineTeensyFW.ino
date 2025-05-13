#include <Arduino.h>
#include "USBHost_t36.h"

unsigned long loopCounter = 0;

void setup() {
  // 1. Start Serial Device for logging
  Serial1.begin(115200);
  Serial4.begin(115200);
  // Optional: Wait for serial connection for a moment
  unsigned long serial_timeout_start = millis();
  while (!Serial1 && (millis() - serial_timeout_start < 2000)) { delay(10); }

  Serial1.println("\n\n--- Sunshine Teensy FW Test ||  LED BLINK ");
  Serial1.println("-----------------------------------------------------");

  // 2. Initialize built-in LED for visual feedback
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // Turn LED ON during setup
  Serial1.println("LED Initialized: ON");
  Serial1.flush();

  digitalWrite(LED_BUILTIN, LOW); // Turn LED OFF before loop
}

void loop() {
  loopCounter++;

  // CRITICAL: Call myHost.Task() in your loop.
  // This function processes USB events and handles ongoing enumeration tasks or driver tasks.
  // Even without custom drivers, it's needed for things like hub port event processing.
  //myHost.Task();

  // Blink the LED to show the loop is running
  if (loopCounter % 10 == 0) { // Blink less aggressively
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
  
  // Short delay to allow USB tasks to run and keep loop responsive
  delay(100);
}