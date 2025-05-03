/*
  Teensy 4.1 Mouse Polling Rate Test (Default Speed)
  - Includes LED blink on startup for visual confirmation.
  - Alternates sending tiny mouse movements left and right.

  Make sure you have set:
  Tools -> Board: Teensy 4.1
  Tools -> USB Type: Mouse (or Keyboard + Mouse + Joystick)
*/

// The usb_mouse library is automatically included by Teensyduino
// when the USB Type includes "Mouse".

// *** GLOBAL VARIABLE DECLARATION (This was missing before) ***
int x_move = 50; // Variable to alternate direction (+1 for right, -1 for left)

// The setup function runs once when the Teensy starts or resets
void setup() {
  // Setup the onboard LED (pin 13) as an output
  pinMode(LED_BUILTIN, OUTPUT);
  // Turn the LED ON immediately to show setup has started
  digitalWrite(LED_BUILTIN, HIGH);

  // Required: Ensure USB Type includes "Mouse" in Arduino IDE Tools menu.

  // Increased delay to make the LED blink visible and allow USB enumeration
  delay(2000); // Wait for 2 seconds

  // Turn the LED OFF after the delay to show setup is complete
  digitalWrite(LED_BUILTIN, LOW);
}

// The loop function runs over and over again forever
void loop() {
  // Send the mouse movement report.
  // Moves 1 pixel horizontally (alternating left/right), 0 vertically, 0 scroll.
  Mouse.move(x_move, 0, 0);

  // Flip the direction for the next movement
  x_move = -x_move;

  // NO DELAY - Run as fast as possible.
  // The actual polling rate will be determined by the USB host
  // and the speed negotiated with the Teensy (Full or High).
}
