// serialcommands.h
#ifndef SERIALCOMMANDS_H
#define SERIALCOMMANDS_H

class SerialCommands {
private:
  using CommandCallback = void (*)(uint8_t *buf, uint8_t size);

  CommandCallback callback; // Callback function to process the received data

public:
  SerialCommands(CommandCallback _callback) {
    callback = _callback;
  }

void processInput() {
  static uint8_t commandBuffer[7]; // Buffer to store the command
  static uint8_t bufferIndex = 0;

  while (Serial1.available()) {
    uint8_t incomingByte = Serial1.read();

    if (incomingByte == '\n') {
      if (bufferIndex == sizeof(commandBuffer)) {
        // Print the received data
        Serial1.print("MSG: Arduino Received -> ");
        for (uint8_t i = 0; i < bufferIndex; i++) {
          Serial1.print(' ');
          Serial1.print(commandBuffer[i], HEX);
        }
        Serial1.println();
        // Use callback here to send the buffer of data
        uint8_t newBuffer[8];
        newBuffer[0] = commandBuffer[0];
        newBuffer[1] = commandBuffer[1];
        newBuffer[2] = commandBuffer[2];
        newBuffer[3] = 0; // insert 0 for scroll wheel. required for hid report
        newBuffer[4] = commandBuffer[3];
        newBuffer[5] = commandBuffer[4];
        newBuffer[6] = commandBuffer[5];
        newBuffer[7] = commandBuffer[6];

        Serial1.print("MSG: Arduino Converted Data into New Buf -> ");
        for (uint8_t i = 0; i < 8; i++) {
          Serial1.print(' ');
          Serial1.print(newBuffer[i], HEX);
        }
        Serial1.println();

        callback(newBuffer, 8);
      } else {
        // Print debug log for incomplete data
        Serial1.print("MSG: Incomplete data received. Received ");
        Serial1.print(bufferIndex);
        Serial1.println(" bytes.");
      }

      // Reset the buffer index
      bufferIndex = 0;
    } else {
      // Store the incoming byte in the buffer
      if (bufferIndex < sizeof(commandBuffer)) {
        commandBuffer[bufferIndex] = incomingByte;
        bufferIndex++;
      }
    }
  }
}


};
#endif // SERIALCOMMANDS_H
