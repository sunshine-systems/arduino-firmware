/*
  HC-06 Bluetooth Module Renamer Sketch for Teensy 4.1
  (Uses Arduino Serial Monitor for Feedback - Fix for '=' in name)

  Changes:
  - Sends AT+NAME directly followed by the name string (no '=')
*/

#include <Arduino.h>

// Define the Hardware Serial port connected to the HC-06
#define BT_Serial Serial1

// *** CRITICAL: Set this to the CURRENT baud rate of your HC-06 module! ***
// The default is usually 9600.
#define CURRENT_HC06_BAUD 115200

// Define the new desired name
const char* NEW_BT_NAME = "Sunshine Serial BT";

// Helper function to send a command and print the response to Serial Monitor
// Returns true if "OK" was found in the response
bool sendATCommand(const char* command, unsigned long wait_ms, bool add_crlf = false) {
  while (BT_Serial.available()) { BT_Serial.read(); } // Clear buffer

  Serial.print("Sending Command -> ");
  Serial.print(command);
  if (add_crlf) Serial.print(" [CRLF]");
  Serial.println();

  BT_Serial.print(command);
  if (add_crlf) { BT_Serial.print("\r\n"); }

  delay(wait_ms);

  Serial.print("Received Response <- ");
  String responseText = "";
  String responseRaw = "";
  bool receivedData = false;
  unsigned long startTime = millis();
  while (millis() - startTime < 1500) {
     if (BT_Serial.available()) {
       receivedData = true;
       char c = BT_Serial.read();
       if (isprint(c)) { responseText += c; }
       responseRaw += "0x";
       if (c < 16) responseRaw += "0";
       responseRaw += String(c, HEX);
       responseRaw += " ";
       startTime = millis();
     }
     if (!BT_Serial.available()) delay(5);
  }

  if (receivedData) {
    Serial.print("\n    Raw Bytes: "); Serial.println(responseRaw);
    Serial.print("    Textual:   "); Serial.println(responseText);
    if (responseText.indexOf("OK") != -1) {
      Serial.println(">>> Command Acknowledged (OK received)"); Serial.println("---------------------------------"); return true;
    } else {
      Serial.println(">>> Warning: 'OK' not detected in textual response."); Serial.println("---------------------------------"); return false;
    }
  } else {
    Serial.println("[No Response within 1.5 seconds]"); Serial.println(">>> Check wiring, HC06 power, baud rate. Module might ignore this command."); Serial.println("---------------------------------"); return false;
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000);

  Serial.println("\n--- HC-06 Renamer Initializing (Fix for '=') ---");
  Serial.println("Using Serial Monitor for feedback.");
  Serial.print("Attempting communication with HC-06 on Serial1 at "); Serial.print(CURRENT_HC06_BAUD); Serial.println(" bps.");
  Serial.println("Ensure HC-06 is flashing (not connected via Bluetooth)."); Serial.println("---------------------------------");

  BT_Serial.begin(CURRENT_HC06_BAUD);
  delay(1000);

  Serial.println("Step 1: Testing basic communication...");
  bool basicCommOk = sendATCommand("AT", 500);

  if (basicCommOk) {
    Serial.println("\nStep 2: Attempting to set new name (without '=')...");

    // *** MODIFICATION HERE: Remove the '=' from the command string ***
    String renameCommandStr = "AT+NAME"; // Just the command part
    renameCommandStr += NEW_BT_NAME;     // Append the desired name directly

    // Send AT+NAME<ActualName>, wait 1500ms, no CR/LF
    sendATCommand(renameCommandStr.c_str(), 1500, false);

  } else {
      Serial.println("\nStep 2: Skipped renaming - Basic 'AT' command failed.");
  }

  Serial.println("\n--- Renaming Attempt Complete ---");
  Serial.println("Check the responses above. If 'OK' was received after AT+NAME:");
  Serial.println(" 1. Power cycle the Teensy/HC-06 (unplug/replug USB).");
  Serial.println(" 2. Scan for Bluetooth devices - look for 'Sunshine Serial BT'.");
  Serial.println(" 3. Re-upload your main operational sketch to the Teensy.");
  Serial.println("If no 'OK', the module likely didn't accept the name change.");
  Serial.println("---------------------------------");
}

void loop() {
  delay(500);
  BT_Serial.println("Testing");
}
