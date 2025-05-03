/*
 * HC-06 Bluetooth Module - Name & Baud Rate Configuration Attempt
 *
 * Target: Teensy 4.1
 * Purpose: Configures the HC-06 module's Bluetooth name and
 *          serial baud rate using AT commands.
 * Flow:
 *   1. Initializes communication at STARTUP_BAUD.
 *   2. Sends 'AT' test command.
 *   3. If OK, sends 'AT+NAME<NewName>' command.
 *   4. Sends 'AT+BAUDx' command for TARGET_BAUD.
 *   5. Reconfigures Teensy's serial port to TARGET_BAUD.
 *   6. Loop sends "Hello World" at TARGET_BAUD for verification.
 * Feedback: Uses the Arduino USB Serial Monitor for setup progress.
 * Verification: Requires user to connect a Bluetooth Serial Terminal configured
 *               to the TARGET baud rate to observe the "Hello World" messages.
 *
 * IMPORTANT: Configuration changes (Name, Baud) are persistent on the HC-06.
 *            This sketch should ideally be run once. Operational sketches
 *            must use the TARGET baud rate.
 *
 * Wiring:
 *   - Teensy Pin 1 (TX1) -> HC-06 RXD
 *   - Teensy Pin 0 (RX1) -> HC-06 TXD
 *   - Teensy VIN (5V)    -> HC-06 VCC
 *   - Teensy GND         -> HC-06 GND
 */

#include <Arduino.h>

//=============================================================================
// Baud Rate Definitions
//=============================================================================
#define BAUD_1200    1200
#define BAUD_2400    2400
#define BAUD_4800    4800
#define BAUD_9600    9600
#define BAUD_19200   19200
#define BAUD_38400   38400
#define BAUD_57600   57600
#define BAUD_115200  115200

//=============================================================================
// Configuration - MODIFY THESE SETTINGS
//=============================================================================

// Define the hardware serial port connected to the HC-06 module
#define BT_Serial Serial1

// Define the desired Bluetooth device name
const char* NEW_BT_NAME = "Sunshine Serial BT";

// Select the baud rate the HC-06 is assumed to be using initially.
// (Factory default is almost always BAUD_9600)
const unsigned long HC06_STARTUP_BAUD = BAUD_9600;

// Select the NEW target baud rate you want to configure the HC-06 to use.
const unsigned long HC06_TARGET_BAUD = BAUD_19200; // Example: Use 38400

// Baud rate for the USB Serial Monitor connection
const unsigned long MONITOR_BAUD = 115200;

//=============================================================================
// Global Variables & Constants
//=============================================================================
// Increased timeout for better response capture
const unsigned long AT_RESPONSE_TIMEOUT_MS = 2000;
// Increased delay between commands for more stable operation
const unsigned long AT_COMMAND_DELAY_MS = 1500;
// Message interval in loop
const unsigned long MESSAGE_INTERVAL_MS = 1000;
// Track last message time
unsigned long lastMessageTime = 0;

//=============================================================================
// Helper Function: getBaudCodeChar
//-----------------------------------------------------------------------------
// Returns the single character code ('1'-'8') for the AT+BAUD command.
// Returns '?' if the baud rate is not standard/supported by this command.
//=============================================================================
char getBaudCodeChar(unsigned long targetBaud) {
    switch (targetBaud) {
        case BAUD_1200:   return '1';
        case BAUD_2400:   return '2';
        case BAUD_4800:   return '3';
        case BAUD_9600:   return '4';
        case BAUD_19200:  return '5';
        case BAUD_38400:  return '6';
        case BAUD_57600:  return '7';
        case BAUD_115200: return '8';
        default:          return '?'; // Indicate an unsupported/invalid rate
    }
}

//=============================================================================
// Helper Function: sendATCommandSimple (Prints results, returns simple bool success)
//=============================================================================
bool sendATCommandSimple(const char* command, bool add_crlf = false) {
    Serial.print(F("  Sending -> ")); Serial.print(command);
    if (add_crlf) Serial.print(F(" [CRLF]")); Serial.println();
    
    // Clear any existing data in buffer
    while (BT_Serial.available() > 0) { BT_Serial.read(); }

    // Send the command to the HC-06
    BT_Serial.print(command);
    if (add_crlf) BT_Serial.print(F("\r\n"));
    BT_Serial.flush(); // Ensure all data is sent

    delay(AT_COMMAND_DELAY_MS); // Allow processing time for HC-06

    // Read response
    Serial.print(F("  Response <- "));
    String responseText = "";
    bool receivedData = false;
    unsigned long startTime = millis();
    
    // Wait for and capture response with timeout
    while (millis() - startTime < AT_RESPONSE_TIMEOUT_MS) {
        if (BT_Serial.available() > 0) {
            receivedData = true;
            char c = BT_Serial.read();
            // Capture printable chars, CR, and LF for display
            if (isprint(c)) {
                responseText += c;
            } else if (c == '\r') {
                responseText += "\\r"; // Show CR in output
            } else if (c == '\n') {
                responseText += "\\n"; // Show LF in output
            } else {
                responseText += '?'; // Indicate non-printable byte
            }
            startTime = millis(); // Reset timeout while receiving
        }
        yield(); // Allow other tasks to run
        if (!BT_Serial.available()) delay(5); // Small pause if buffer empty
    }

    // Process and display the response
    bool success = false;
    if (receivedData) {
        Serial.println(responseText); // Print whatever came back
        
        // Simple check for "OK" in any part of the response
        if (responseText.indexOf("OK") != -1) {
            Serial.println(F("    (OK detected)"));
            success = true;
        } else {
            Serial.println(F("    (OK *not* detected)"));
        }
    } else {
        Serial.println(F("[No Response]"));
        // Try resending once if no response
        delay(500);
        BT_Serial.print(command);
        if (add_crlf) BT_Serial.print(F("\r\n"));
        BT_Serial.flush();
        
        delay(AT_COMMAND_DELAY_MS);
        
        // Check for delayed response
        receivedData = false;
        startTime = millis();
        while (millis() - startTime < AT_RESPONSE_TIMEOUT_MS) {
            if (BT_Serial.available() > 0) {
                receivedData = true;
                char c = BT_Serial.read();
                // Similar processing as above
                if (isprint(c)) {
                    responseText += c;
                } else if (c == '\r') {
                    responseText += "\\r";
                } else if (c == '\n') {
                    responseText += "\\n";
                } else {
                    responseText += '?';
                }
            }
            yield();
            if (!BT_Serial.available()) delay(5);
        }
        
        if (receivedData) {
            Serial.print(F("  Retry Response <- "));
            Serial.println(responseText);
            if (responseText.indexOf("OK") != -1) {
                Serial.println(F("    (OK detected on retry)"));
                success = true;
            }
        } else {
            Serial.println(F("    (No response on retry)"));
        }
    }
    
    Serial.println(F("---------------------------------"));
    return success;
}

//=============================================================================
// setup() - Runs once on boot/reset
//=============================================================================
void setup() {
    // --- Initialize USB Serial for Monitor Output ---
    Serial.begin(MONITOR_BAUD);
    unsigned long serialWaitStart = millis();
    // Wait for Serial but with timeout
    while (!Serial && (millis() - serialWaitStart < 4000)) { yield(); }

    Serial.println(F("\n================================================="));
    Serial.println(F("  HC-06 Name & Baud Rate Configuration Attempt "));
    Serial.println(F("================================================="));
    Serial.print(F("Desired BT Name:            ")); Serial.println(NEW_BT_NAME);
    Serial.print(F("HC-06 STARTUP Baud assumed: ")); Serial.println(HC06_STARTUP_BAUD);
    Serial.print(F("HC-06 TARGET Baud desired:  ")); Serial.println(HC06_TARGET_BAUD);
    Serial.println(F("Ensure HC-06 is powered and flashing (not connected)."));
    Serial.println(F("---------------------------------"));

    // --- Initialize BT_Serial at the STARTUP rate ---
    Serial.print(F("Step 1: Initializing BT_Serial at STARTUP rate ("));
    Serial.print(HC06_STARTUP_BAUD); Serial.println(F(")..."));
    BT_Serial.begin(HC06_STARTUP_BAUD);
    delay(1500); // Allow HC-06 boot and stabilization

    // --- Step 2: Test basic 'AT' communication at STARTUP rate ---
    Serial.println(F("Step 2: Testing basic 'AT' communication..."));
    // Try AT command, repeat if not successful initially
    bool basicCommOk = sendATCommandSimple("AT");
    
    if (!basicCommOk) {
        Serial.println(F("Retrying basic 'AT' command..."));
        delay(1000);
        basicCommOk = sendATCommandSimple("AT");
    }

    // --- Proceed with configuration only if basic communication works ---
    if (basicCommOk) {
        // --- Step 3: Attempt to set Bluetooth Name ---
        Serial.print(F("\nStep 3: Attempting to set BT Name to '"));
        Serial.print(NEW_BT_NAME); Serial.println(F("'..."));
        
        // Construct name command (no spaces between command and parameter)
        String nameCommand = "AT+NAME";
        nameCommand += NEW_BT_NAME;
        sendATCommandSimple(nameCommand.c_str()); // Send name command
        
        // Wait a moment for name change to settle
        delay(1000);

        // --- Step 4: Attempt to set Baud Rate ---
        Serial.print(F("\nStep 4: Attempting to set TARGET baud rate ("));
        Serial.print(HC06_TARGET_BAUD); Serial.println(F(")..."));

        char baudCode = getBaudCodeChar(HC06_TARGET_BAUD); // Get '1'-'8' code

        if (baudCode != '?') {
            String baudCommand = "AT+BAUD";
            baudCommand += baudCode; // Append the code character (e.g., '6')
            
            // Important: Send baud command and wait for response
            bool baudChangeOk = sendATCommandSimple(baudCommand.c_str());
            
            if (baudChangeOk) {
                Serial.println(F("  Baud change command successful. Proceeding to next step."));
            } else {
                Serial.println(F("  Baud change command might not have worked, but continuing anyway."));
            }
            
            // Add delay after baud change to let HC-06 stabilize
            delay(2000);
            
        } else {
            Serial.println(F("  >>> ERROR: TARGET Baud Rate ("));
            Serial.print(HC06_TARGET_BAUD);
            Serial.println(F(") is not supported by standard AT+BAUDx command! <<<"));
            Serial.println(F("  >>> Baud rate change skipped. <<<"));
        }

    } else {
        Serial.println(F("\nSteps 3 & 4 Skipped - Basic 'AT' communication failed."));
        Serial.println(F("Configuration not attempted. Check wiring/power/STARTUP_BAUD."));
        Serial.println(F("Common issues:"));
        Serial.println(F("- HC-06 is already paired (LED should be blinking, not solid)"));
        Serial.println(F("- Incorrect wiring (ensure TX->RX, RX->TX crossover)"));
        Serial.println(F("- Incorrect startup baud rate (try other common values)"));
        Serial.println(F("- HC-06 needs power cycle"));
    }
    Serial.println(F("---------------------------------"));

    // --- Step 5: Re-initialize BT_Serial at the TARGET rate ---
    // This happens regardless of previous steps
    Serial.print(F("Step 5: Re-initializing Teensy's BT_Serial at TARGET rate ("));
    Serial.print(HC06_TARGET_BAUD); Serial.println(F(")..."));
    BT_Serial.end(); // Close the port
    delay(500);      // Ensure port closure completes
    BT_Serial.begin(HC06_TARGET_BAUD); // Reopen at the TARGET rate
    delay(1000);     // Allow serial port to stabilize

    Serial.println(F("\n--- Configuration Attempt in Setup Complete ---"));
    Serial.println(F(">>> Teensy BT_Serial is now operating at the TARGET rate. <<<"));
    Serial.println(F(">>> Connect a Bluetooth Terminal App at TARGET rate ("));
    Serial.print(HC06_TARGET_BAUD); Serial.println(F(" bps) <<<"));
    Serial.println(F(">>> Look for 'Hello World' messages in the terminal to verify. <<<"));
    Serial.println(F("---------------------------------"));
    
    lastMessageTime = millis(); // Initialize message timing
}

//=============================================================================
// loop() - Runs repeatedly after setup()
//=============================================================================
void loop() {
    // This loop assumes the HC-06 is operating at HC06_TARGET_BAUD
    // and communicates using BT_Serial initialized at that rate in setup().
    
    unsigned long currentTime = millis();
    
    // Send a message at regular intervals using non-blocking approach
    if (currentTime - lastMessageTime >= MESSAGE_INTERVAL_MS) {
        // Check if connected (optional - can be determined by LED state on HC-06)
        // Send a message over Bluetooth Serial
        BT_Serial.println("Hello World");
        
        // Print to monitor as well for debugging
        Serial.println("Sent: Hello World");
        
        // Update last message time
        lastMessageTime = currentTime;
    }
    
    // Check for any incoming data from Bluetooth
    while (BT_Serial.available() > 0) {
        char incomingChar = BT_Serial.read();
        // Echo back to Serial Monitor for debugging
        Serial.print("Received: ");
        Serial.println(incomingChar);
    }
    
    // Allow other tasks to run
    yield();
}
