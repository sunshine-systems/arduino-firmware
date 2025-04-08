import time
import serial

from RuntimeSettings import RuntimeSettings

class ArduinoInterface:
    def __init__(self, runtimeSettings: RuntimeSettings):
        self.runtimeSettings = runtimeSettings
        self.port = runtimeSettings.arduinoPort
        self.baudrate = runtimeSettings.arduinoBaudRate
        self.connected = False
        self.serial = None

    def connect(self):
        while not self.connected:
            try:
                self.serial = serial.Serial(self.port, baudrate=115200, timeout=2)
                self.connected = True
                if self.runtimeSettings.enableLogs:
                    print(f"Serial connection established with FT232RL FTDI!")
            except serial.SerialException:
                if self.runtimeSettings.enableLogs:
                    print(f"Failed to establish serial with FT232RL FTDI connection. Retrying in 1s...")
                time.sleep(1)

    def read_line_from_buffer(self):
        if self.serial.in_waiting:
            return self.serial.readline().decode().strip()
        return None

    def write_line_to_buffer(self, data):
        if self.connected:
            self.serial.write(data)
        else:
            if self.runtimeSettings.enableLogs:
                print(f"Unable to write data, FTDI device is not connected...")

    def get_connected(self):
        return self.connected