/*
 * HC-06 Smart Configuration Tool
 * Sunshine####_v#.# Naming Format (Shortened)
 *
 * Purpose: Automatically detect the current baud rate of an HC-06 module
 *          and configure it with a shorter name format, PIN code, and baud rate.
 *
 * Features:
 *   - Auto-detects current baud rate by trying standard rates
 *   - Sets device name to "Sunshine####_v#.#" format with random 4-digit number (0000-9999)
 *   - Uses proper 4-digit format with leading zeros for small numbers
 *   - Tries multiple command formats for HC-06 modules with different firmware
 *   - Version number supports decimal places (e.g., v1.0, v1.1, etc.)
 *   - Sets PIN code to "0000"
 *   - Sets baud rate to specified value (default 19200)
 */

#include <Arduino.h>

//=============================================================================
// CONFIGURATION SETTINGS - MODIFY THESE AS NEEDED
//=============================================================================

// Define the hardware serial port connected to the HC-06 module
#define BT_Serial Serial1

// Customize the device name format - SHORTENED to stay under 20 chars
const char* NAME_PREFIX = "Sunshine";  // Shortened from "SunshineSystems"
const char* VERSION_PREFIX = "_v";     // Prefix before version number
const char* VERSION_NUMBER = "1.0";    // Include decimal point (e.g., "1.0")

// Define the PIN code to set
const char* NEW_PIN_CODE = "0000";

// Define your preferred TARGET baud rate
const unsigned long PREFERRED_BAUD = 115200;

// Baud rate for the USB Serial Monitor connection
const unsigned long MONITOR_BAUD = 115200;

// List of standard baud rates to try in detection mode (in order of likelihood)
const unsigned long BAUD_RATES[] = {
  9600,    // Most common default
  38400,   // Another common setting
  19200,
  57600,
  115200,
  4800,
  2400,
  1200
};
const int NUM_BAUD_RATES = sizeof(BAUD_RATES) / sizeof(BAUD_RATES[0]);

//=============================================================================
// Global Variables & Constants
//=============================================================================
unsigned long AT_RESPONSE_TIMEOUT_MS = 1500;  // Made variable so it can be temporarily changed
const unsigned long AT_COMMAND_DELAY_MS = 1200;
const unsigned long MESSAGE_INTERVAL_MS = 1000;

unsigned long lastMessageTime = 0;
unsigned long currentBaudRate = 0;
bool baudRateDetected = false;
bool changeCompleted = false;
bool nameChangeFailed = false;

char deviceName[50];       // Buffer to hold the generated device name

//=============================================================================
// Helper Function: getBaudCodeChar
//-----------------------------------------------------------------------------
// Returns the character code for AT+BAUD command
//=============================================================================
char getBaudCodeChar(unsigned long baudRate) {
  switch (baudRate) {
    case 1200:    return '1';
    case 2400:    return '2';
    case 4800:    return '3';
    case 9600:    return '4';
    case 19200:   return '5';
    case 38400:   return '6';
    case 57600:   return '7';
    case 115200:  return '8';
    default:      return '?';
  }
}

//=============================================================================
// Helper Function: initialize randomness
//=============================================================================
void initializeRandomness() {
  Serial.println(F("Initializing random number generator..."));
  
  // Create a more entropic seed
  unsigned long seed = 0;
  
  // Use pin floating behavior for entropy
  for (int i = 0; i < 8; i++) {
    // Read from different analog pins
    int pin = i % 6;  // Pins 0-5
    
    // Discard first reading and wait
    analogRead(pin);
    delayMicroseconds(i * 73 + 53);  // Variable delays
    
    // Read multiple times from same pin
    for (int j = 0; j < 4; j++) {
      unsigned long reading = analogRead(pin);
      seed ^= (reading << ((i+j) % 15));  // Shift and XOR
      delayMicroseconds(j * 31 + 37);     // More variable delays
    }
  }
  
  // Mix in timing information
  unsigned long time_start = micros();
  for (volatile int i = 0; i < 243; i++) {
    // Empty loop with intentionally strange limit to create variable timing
  }
  unsigned long time_end = micros();
  seed ^= (time_end - time_start);
  seed ^= (seed << 7);
  seed ^= (seed >> 5);
  
  // Set the seed
  randomSeed(seed);
  
  // Burn a few values to get away from initial pattern
  for (int i = 0; i < 15; i++) {
    random(10000);
  }
  
  Serial.println(F("Random number generator initialized"));
}

//=============================================================================
// Helper Function: generateRandomNumber
//-----------------------------------------------------------------------------
// Generates a random 4-digit number (0000-9999) with better distribution
//=============================================================================
int generateRandomNumber() {
  // Multiple entropy sources
  unsigned long a = micros();           // Current microseconds
  unsigned long b = millis();           // Current milliseconds
  unsigned long c = analogRead(0) << 7; // Analog reading from pin 0
  unsigned long d = analogRead(1) << 5; // Analog reading from pin 1
  
  // Mix entropy sources with different bit operations to reduce predictability
  unsigned long entropy = 0;
  entropy ^= a;                 // XOR with microseconds
  entropy += b;                 // Add milliseconds
  entropy ^= c;                 // XOR with shifted analog reading 0
  entropy += (d * 13);          // Add (analog reading 1 × 13)
  entropy ^= (entropy >> 11);   // XOR with right shifted version of itself
  entropy += (entropy << 7);    // Add left shifted version of itself
  
  // Add more variability
  delay(3);                     // Small delay to change microsecond timing
  entropy ^= micros();          // XOR with new microsecond reading
  
  // Using a hybrid approach for final number generation:
  
  // Method 1: Direct modulo with offset - range 0-9999
  unsigned int num1 = (entropy % 10000);
  
  // Method 2: Using individual digits
  unsigned int num2 = 
                     ((entropy & 0x0F) % 10) +              // 1s place
                     (((entropy >> 4) & 0x0F) % 10) * 10 +  // 10s place
                     (((entropy >> 8) & 0x0F) % 10) * 100 + // 100s place
                     (((entropy >> 12) & 0x0F) % 10) * 1000;// 1000s place
  if (num2 > 9999) num2 = num2 % 10000;
  
  // Method 3: Hardware-based approach
  unsigned int num3 = 0;
  for (int i = 0; i < 4; i++) {  // Collect 4 digits
    int pin = i % 4;              // Use analog pins 0-3
    analogRead(pin);              // Discard first reading
    delay(1);                     // Small delay
    int val = analogRead(pin) % 10; // Get a digit (0-9)
    num3 = num3 * 10 + val;       // Add to number
  }
  if (num3 > 9999) num3 = num3 % 10000;
  
  // Method 4: Time-based with bit manipulation
  unsigned int num4 = (micros() ^ (micros() >> 10) ^ (entropy)) % 10000;
  
  // Choose between methods based on entropy
  unsigned int selector = entropy & 0x03;  // 0-3
  
  // Add a final round of mixing to break patterns
  unsigned int result;
  switch (selector) {
    case 0: result = num1; break;
    case 1: result = num2; break;
    case 2: result = num3; break;
    case 3: result = num4; break;
    default: result = num1;  // Fallback
  }
  
  // Add extra randomness by mixing with the current time in a way that
  // depends on the specific pattern of the result itself
  unsigned long final_mix = micros() ^ (result * 17);
  result = (result + (final_mix % 997)) % 10000;
  
  // Print diagnostic information to help troubleshoot randomness
  Serial.print(F("Random methods: "));
  Serial.print(num1); Serial.print(F(", "));
  Serial.print(num2); Serial.print(F(", "));
  Serial.print(num3); Serial.print(F(", "));
  Serial.print(num4); Serial.print(F(" → "));
  Serial.println(result);
  
  return result;
}

//=============================================================================
// Helper Function: generateDeviceName
//-----------------------------------------------------------------------------
// Generates the device name in format "Sunshine####_v#.#"
//=============================================================================
void generateDeviceName() {
  // Generate a random number (0000-9999)
  int randomNumber = generateRandomNumber();
  
  // Format the name with the prefix, random number with leading zeros, and version
  sprintf(deviceName, "%s%04d%s%s", 
          NAME_PREFIX,       // e.g., "Sunshine"
          randomNumber,      // e.g., "0123" (with leading zeros)
          VERSION_PREFIX,    // e.g., "_v"
          VERSION_NUMBER);   // e.g., "1.0"
  
  Serial.print(F("Generated device name: "));
  Serial.println(deviceName);
  Serial.print(F("Name length: "));
  Serial.print(strlen(deviceName));
  Serial.println(F(" characters (should be under 20)"));
}

//=============================================================================
// Helper Function: sendATCommand
//-----------------------------------------------------------------------------
// Sends AT command, returns true if "OK" received
//=============================================================================
bool sendATCommand(const char* command, String* response_out = NULL) {
  // Clear incoming buffer
  while (BT_Serial.available()) BT_Serial.read();

  Serial.print(F("Sending: ")); Serial.println(command);
  
  // Send command to HC-06
  BT_Serial.write(command);
  BT_Serial.flush();
  
  // Allow time for the module to process and respond
  delay(AT_COMMAND_DELAY_MS);
  
  // Check for response
  String response = "";
  unsigned long startTime = millis();
  bool receivedResponse = false;
  
  // Wait for complete response with timeout
  while (millis() - startTime < AT_RESPONSE_TIMEOUT_MS) {
    if (BT_Serial.available()) {
      receivedResponse = true;
      char c = BT_Serial.read();
      if (isprint(c)) {
        response += c;
      } else if (c == '\r') {
        response += "\\r";
      } else if (c == '\n') {
        response += "\\n";
      } else {
        response += '?';
      }
      // Reset the timeout when we receive data
      startTime = millis();
    }
    
    // Small delay if no data
    if (!BT_Serial.available()) delay(5);
  }
  
  if (receivedResponse) {
    Serial.print(F("Response: ")); Serial.println(response);
    
    // If caller wants the response text, copy it
    if (response_out != NULL) {
      *response_out = response;
    }
    
    return (response.indexOf("OK") != -1);
  } else {
    Serial.println(F("No response"));
    
    // Try one more time with a slight variation
    // Some HC-06 variants need a CRLF after commands
    Serial.println(F("Retrying with CRLF..."));
    BT_Serial.print(command);
    BT_Serial.print("\r\n");
    BT_Serial.flush();
    
    delay(AT_COMMAND_DELAY_MS);
    
    response = "";
    startTime = millis();
    receivedResponse = false;
    
    while (millis() - startTime < AT_RESPONSE_TIMEOUT_MS) {
      if (BT_Serial.available()) {
        receivedResponse = true;
        char c = BT_Serial.read();
        if (isprint(c)) {
          response += c;
        } else if (c == '\r') {
          response += "\\r";
        } else if (c == '\n') {
          response += "\\n";
        } else {
          response += '?';
        }
        startTime = millis();
      }
      
      if (!BT_Serial.available()) delay(5);
    }
    
    if (receivedResponse) {
      Serial.print(F("Retry response: ")); Serial.println(response);
      
      if (response_out != NULL) {
        *response_out = response;
      }
      
      return (response.indexOf("OK") != -1);
    }
    
    return false;
  }
}

//=============================================================================
// Helper Function: changeDeviceName
//-----------------------------------------------------------------------------
// Changes the HC-06 device name
//=============================================================================
bool changeDeviceName(const char* newName) {
  // HC-06 modules have name length limitations
  if (strlen(newName) > 20) {
    Serial.println(F("WARNING: Name is too long for HC-06 modules (max 20 chars)"));
    Serial.println(F("Please shorten the NAME_PREFIX or VERSION_NUMBER"));
    return false;
  }
  
  // ATTEMPT 1: Standard command without equals sign (most common format)
  String nameCommand = "AT+NAME";
  nameCommand += newName;
  
  Serial.print(F("Setting device name to '")); Serial.print(newName); Serial.println(F("'..."));
  bool success = sendATCommand(nameCommand.c_str());
  
  // ATTEMPT 2: Try with equals sign format if first attempt failed
  if (!success) {
    Serial.println(F("First attempt failed, trying with equals sign format..."));
    nameCommand = "AT+NAME=";
    nameCommand += newName;
    success = sendATCommand(nameCommand.c_str());
  }
  
  // ATTEMPT 3: Try with space format if second attempt failed
  if (!success) {
    Serial.println(F("Second attempt failed, trying with space format..."));
    nameCommand = "AT+NAME ";
    nameCommand += newName;
    success = sendATCommand(nameCommand.c_str());
  }
  
  if (success) {
    Serial.println(F("Name change successful!"));
    return true;
  } else {
    // Increased timeout may help for some modules
    Serial.println(F("All standard methods failed. Trying with extended timeout..."));
    
    // Temporarily increase the timeout for this operation
    unsigned long savedTimeout = AT_RESPONSE_TIMEOUT_MS;
    AT_RESPONSE_TIMEOUT_MS = 3000;  // Use a longer timeout
    
    success = sendATCommand(nameCommand.c_str());
    
    // Restore the original timeout
    AT_RESPONSE_TIMEOUT_MS = savedTimeout;
    
    if (success) {
      Serial.println(F("Name change successful with extended timeout!"));
      return true;
    } else {
      Serial.println(F("Failed to change device name."));
      Serial.println(F("This may be a module limitation or firmware quirk."));
      Serial.println(F("The module may still work fine with its default name."));
      return false;
    }
  }
}

//=============================================================================
// Helper Function: testBaudRate
//-----------------------------------------------------------------------------
// Tests communication at a specific baud rate
//=============================================================================
bool testBaudRate(unsigned long baudRate) {
  // Close and reopen the serial port at the test baud rate
  BT_Serial.end();
  delay(100);
  
  Serial.print(F("Testing baud rate: ")); Serial.print(baudRate); Serial.print("... ");
  BT_Serial.begin(baudRate);
  delay(300); // Allow serial to stabilize
  
  // Try AT command at this baud rate
  bool success = sendATCommand("AT");
  
  if (success) {
    Serial.println(F("SUCCESS! HC-06 responded."));
    currentBaudRate = baudRate;
    return true;
  } else {
    Serial.println(F("Failed."));
    return false;
  }
}

//=============================================================================
// Helper Function: changeBaudRate
//-----------------------------------------------------------------------------
// Changes the HC-06 baud rate
//=============================================================================
bool changeBaudRate(unsigned long newBaudRate) {
  char baudCode = getBaudCodeChar(newBaudRate);
  
  if (baudCode == '?') {
    Serial.print(F("ERROR: Invalid baud rate for HC-06: ")); 
    Serial.println(newBaudRate);
    return false;
  }
  
  String baudCommand = "AT+BAUD";
  baudCommand += baudCode;
  
  Serial.print(F("Changing baud rate to ")); Serial.print(newBaudRate); Serial.println(F("..."));
  bool success = sendATCommand(baudCommand.c_str());
  
  if (success) {
    Serial.println(F("Baud rate change command accepted!"));
    return true;
  } else {
    Serial.println(F("Failed to change baud rate."));
    return false;
  }
}

//=============================================================================
// Helper Function: changePINCode
//-----------------------------------------------------------------------------
// Changes the HC-06 PIN code
//=============================================================================
bool changePINCode(const char* newPIN) {
  String pinCommand = "AT+PIN";
  pinCommand += newPIN;
  
  Serial.print(F("Setting PIN code to '")); Serial.print(newPIN); Serial.println(F("'..."));
  bool success = sendATCommand(pinCommand.c_str());
  
  if (success) {
    Serial.println(F("PIN code change successful!"));
    return true;
  } else {
    Serial.println(F("Failed to change PIN code."));
    return false;
  }
}

//=============================================================================
// setup() - Runs once on boot/reset
//=============================================================================
void setup() {
  // Initialize monitor serial first so we can show output
  Serial.begin(MONITOR_BAUD);
  
  // Wait a bit for Serial to be available, but with timeout
  unsigned long serialWaitStart = millis();
  while (!Serial && (millis() - serialWaitStart < 3000)) {
    delay(100);
  }
  
  // Initialize the random number generator
  initializeRandomness();
  
  Serial.println(F("\n================================================"));
  Serial.println(F("  HC-06 Smart Configuration Tool"));
  Serial.println(F("================================================"));
  
  // Create a truly unique device name
  generateDeviceName();
  
  Serial.println(F("This tool will:"));
  Serial.println(F("1. Try to find the current baud rate of your HC-06"));
  Serial.print(F("2. Set its name to: ")); Serial.println(deviceName);
  Serial.print(F("3. Set the PIN code to: ")); Serial.println(NEW_PIN_CODE);
  Serial.print(F("4. Set the baud rate to: ")); Serial.println(PREFERRED_BAUD);
  Serial.println(F("================================================"));
  Serial.println(F("Beginning baud rate detection sequence..."));
  Serial.println(F("Make sure the HC-06 module is powered and not paired."));
  Serial.println(F("The LED should be blinking, not solid."));
  Serial.println();
  
  // Try each baud rate until successful
  for (int i = 0; i < NUM_BAUD_RATES; i++) {
    if (testBaudRate(BAUD_RATES[i])) {
      baudRateDetected = true;
      Serial.print(F("\nSUCCESS! HC-06 current baud rate detected: "));
      Serial.println(currentBaudRate);
      break;
    }
  }
  
  if (!baudRateDetected) {
    Serial.println(F("\nFAILED! Could not detect HC-06 baud rate."));
    Serial.println(F("Please check:"));
    Serial.println(F("1. Wiring (TX → RX, RX → TX)"));
    Serial.println(F("2. Power to HC-06 (LED should be blinking)"));
    Serial.println(F("3. HC-06 module is not paired (LED should not be solid)"));
    Serial.println(F("4. Try adding additional baud rates to the BAUD_RATES array"));
    return; // Stop execution
  }
  
  // Continue with configuration since we found the baud rate
  Serial.println(F("\n--- Beginning HC-06 Configuration ---"));
  
  // Skip name reading attempt and just try to set the name
  bool nameChangeSuccess = changeDeviceName(deviceName);
  if (!nameChangeSuccess) {
    Serial.println(F("WARNING: Failed to change device name."));
    Serial.println(F("This is not uncommon with some HC-06 variants."));
    Serial.println(F("The module should still function normally."));
    nameChangeFailed = true;
  }
  
  // Wait a moment for name change to settle
  delay(1000);
  
  // Change PIN code
  bool pinChangeSuccess = changePINCode(NEW_PIN_CODE);
  if (!pinChangeSuccess) {
    Serial.println(F("WARNING: Failed to change PIN code. Continuing anyway..."));
  }
  delay(1000);
  
  // Handle baud rate change if needed
  if (currentBaudRate != PREFERRED_BAUD) {
    // Change the baud rate to preferred rate
    if (changeBaudRate(PREFERRED_BAUD)) {
      // Update the current baud rate value
      currentBaudRate = PREFERRED_BAUD;
      
      // Reinitialize serial at the new baud rate
      Serial.println(F("Reinitializing serial connection at new baud rate..."));
      BT_Serial.end();
      delay(500);
      BT_Serial.begin(PREFERRED_BAUD);
      delay(1000);
      
      changeCompleted = true;
    } else {
      Serial.println(F("WARNING: Failed to change baud rate. Will continue with current rate."));
    }
  } else {
    Serial.println(F("Module already at preferred baud rate. No change needed."));
    changeCompleted = true;
  }
  
  // Configuration status
  Serial.println(F("\n--- HC-06 Configuration Complete! ---"));
  Serial.println(F("The HC-06 module has been configured with:"));
  
  if (nameChangeFailed) {
    Serial.println(F("- Name: [UNKNOWN - Name change failed]"));
  } else {
    Serial.print(F("- Name: ")); Serial.println(deviceName);
  }
  
  Serial.print(F("- PIN: ")); Serial.println(NEW_PIN_CODE);
  Serial.print(F("- Baud Rate: ")); Serial.println(currentBaudRate);
  
  if (!changeCompleted) {
    Serial.println(F("\nWARNING: Some configuration steps did not complete successfully."));
  }
  
  Serial.println(F("\nNow sending 'Hello World' messages at the current baud rate."));
  Serial.println(F("You can connect a Bluetooth terminal to verify."));
  
  lastMessageTime = millis();
}

//=============================================================================
// loop() - Runs repeatedly after setup()
//=============================================================================
void loop() {
  // Only continue if we successfully detected the baud rate
  if (baudRateDetected) {
    unsigned long currentTime = millis();
    
    // Send a message at regular intervals
    if (currentTime - lastMessageTime >= MESSAGE_INTERVAL_MS) {
      BT_Serial.println("Hello World");
      Serial.println(F("Sent: Hello World"));
      lastMessageTime = currentTime;
    }
    
    // Check for any incoming messages
    while (BT_Serial.available()) {
      char c = BT_Serial.read();
      Serial.print(F("Received: "));
      if (isprint(c)) {
        Serial.println(c);
      } else {
        Serial.print(F("0x"));
        Serial.println(c, HEX);
      }
    }
  }
  
  // Allow other tasks to run
  yield();
}
