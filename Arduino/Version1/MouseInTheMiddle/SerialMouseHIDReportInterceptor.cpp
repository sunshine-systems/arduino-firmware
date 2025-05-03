// SerialMouseHIDReportInterceptor.cpp
#include "SerialMouseHIDReportInterceptor.h"
#include "Config.h"
#include "EdianConversions.h"

SerialMouseHIDReportInterceptor::SerialMouseHIDReportInterceptor() {}

void SerialMouseHIDReportInterceptor::sniffForSpoofableDataOverSerial() {
      static uint8_t commandBuffer[7]; // Buffer to store the command
      static uint8_t bufferIndex = 0;

      while (Serial1.available()) {
        uint8_t incomingByte = Serial1.read();

        if (incomingByte == '\n') {
          if (bufferIndex == sizeof(commandBuffer)) {
            // TODO: Wrap this in a DEBUG flag check so it doesnt always do this
//            Serial1.print("MSG: Arduino Received -> ");
//            for (uint8_t i = 0; i < bufferIndex; i++) {
//              Serial1.print(' ');
//              Serial1.print(commandBuffer[i], HEX);
//            }
//            Serial1.println(" ");

            processAndSetHIDReportData(commandBuffer);

          } else {
            // Print debug log for incomplete data
            Serial1.print("MSG: Incomplete data received. Received -> ");
            Serial1.print(bufferIndex);
            Serial1.println(" ");
          }

          // Reset the buffer index
          bufferIndex = 0;
        }
        else {
          // Store the incoming byte in the buffer
          if (bufferIndex < sizeof(commandBuffer)) {
            commandBuffer[bufferIndex] = incomingByte;
            bufferIndex++;
          }
        }
      }
}

/*
    Byte 0 = Mouse Buttons
    Byte 1 = Mouse X movement within the range of -32767...32767
    Byte 2 = Mouse X movement within the range of -32767...32767
*/
void SerialMouseHIDReportInterceptor::processAndSetHIDReportData(const uint8_t* data)
{
    previousMouseButtons = mouseButtons;
    mouseButtons = data[0];
    xMovement = edianConversions.parseX(data);
    yMovement = edianConversions.parseY(data);
    isDataAvailable = true;  // Set dataReceived to true when new data is set

//    Serial1.print("MSG: mouseButtons: ");
//    Serial1.print(mouseButtons, HEX);
//    Serial1.println(" ");
}

bool SerialMouseHIDReportInterceptor::hasData(){
    return isDataAvailable;
}

// reset mouse movement to prevent ghosting
void SerialMouseHIDReportInterceptor::reset(){
    isDataAvailable = false;
}
