from RuntimeSettings import RuntimeSettings
from ArduinoInterface import ArduinoInterface
from FirmwareSettings import FirmwareSettings

class FirmwareInterface:
    def __init__(self, arduinoInterface: ArduinoInterface, runtimeSettings: RuntimeSettings):
        self.firmwareSettings = FirmwareSettings()
        self.arduinoInterface = arduinoInterface
        self.runtimeSettings = runtimeSettings
        self.outgoing_data_fifo_queue = []

    def start(self):
        if not self.arduinoInterface.connected:
            self.arduinoInterface.connect()

    """ Read / Write Handlers """
    def handle_incoming_data(self):
        data = self.arduinoInterface.read_line_from_buffer()
        if data is not None:
           self.process_incoming_data(data)

    def handle_outgoing_data(self):
        if len(self.outgoing_data_fifo_queue) > 0:
            data = self.outgoing_data_fifo_queue.pop(0)
            self.arduinoInterface.write_line_to_buffer(data)
            print(f"Wrote out data to arduino: {self.hexToString(data)}")

    """ Data Creators """

    def add_data_to_write_queue(self, data):
        self.outgoing_data_fifo_queue.append(data)
    
    def create_spoofed_hid_report(self, mouse_buttons, desired_x, desired_y, enableLockout=True):
        data = bytearray(8)  # Initialize with all zeroes

        data[0] = mouse_buttons

        if desired_x >= 127:
            data[1] = 0x7F  # Set xRanged to 0x7F for positive overflow
        elif desired_x <= -128:
            data[1] = 0x80  # Set xRanged to 0x80 for negative overflow
        else:
            data[1] = desired_x & 0xFF  # No overflow, direct value

        if desired_y >= 127:
            data[2] = 0x7F  # Set yRanged to 0x7F for positive overflow
        elif desired_y <= -128:
            data[2] = 0x80  # Set yRanged to 0x80 for negative overflow
        else:
            data[2] = desired_y & 0xFF  # No overflow, direct value

        if (enableLockout or 
            (self.runtimeSettings.enableMouseMovementLockoutOnLMBorMovement and 
            self.runtimeSettings.valorantAimOnTapfire and 
            (desired_x != 0 or desired_y != 0))):
            data[3] = 1
        elif (enableLockout or 
            (self.runtimeSettings.enableMouseMovementLockoutOnLMBorMovement and 
            (desired_x != 0 or desired_y != 0))):
            data[3] = 1
        else:
            data[3] = 0x00

        data[4] = desired_x & 0xFF  # Low byte of desired_x
        data[5] = (desired_x >> 8) & 0xFF  # High byte of desired_x

        data[6] = desired_y & 0xFF  # Low byte of desired_y
        data[7] = (desired_y >> 8) & 0xFF  # High byte of desired_y

        length_prefixed_data = bytearray([len(data)]) + data  # Prefix length

        return length_prefixed_data

    def create_settings_report(self, name, setting_id, value):
        # Start with an 8-byte array filled with zeros
        data = bytearray(8)

        # Assume the setting ID is one byte and the value is two bytes (little endian)
        # Place the setting ID in the first byte
        data[0] = setting_id

        # Convert the setting value to bytes and place them in the next two bytes
        if isinstance(value, bool):
            value = 1 if value else 0  # Convert boolean to 1 or 0 for transmission
        value_bytes = value.to_bytes(2, byteorder='little', signed=True)
        data[1:3] = value_bytes  # Fill bytes 1 and 2 with the value

        # The number of bytes to process is 3 (ID + two bytes of the value)
        length_of_data_to_process = 3

        # Final data to send includes the length prefix and the 8-byte data array
        length_prefixed_data = bytearray([length_of_data_to_process]) + data

        # Return the 9-byte total packet, which includes the length prefix and the 8-byte data array
        return length_prefixed_data

    def firmware_version_request(self):
        
        return

    """ Process Incoming Data Handlers """

    def process_mrpt_message(self, message):
        stripped_message = message[len("M:"):].strip()  # Remove "MRPT:" and leading/trailing whitespace
        message_list = stripped_message.split()  # Split the stripped message into a list
        mouse_button = int(message_list[0], 16)  # update mouse_button object
        return mouse_button

    def process_incoming_data(self, data):
        dataFirst2Characters = data[:2]

        if dataFirst2Characters == "M:":
            print(f"Device Mouse Report: {data[3:]}")  # Removes 'M: ' from the message
            # TODO: Replace with userActivations.handle whatever
        elif dataFirst2Characters == "V:":
            print(f"Device Firmware Version: {data[3:]}")
        elif dataFirst2Characters == "E:":
            print(f"Device Error: {data[3:]}")  # Removes 'E: ' from the message
        elif dataFirst2Characters == "I:":
            print(f"Device Information: {data[3:]}")  # Removes 'I: ' from the message
        elif dataFirst2Characters == "S:":
            print(f"Device Setup Process: {data[3:]}")  # Removes 'S:' from the message
        else:
            print(f"Device Message: {data}")  # Default case for other messages
    
    """ Handlers for Settings """
    def syncDeviceWithDefaultSettings(self):
        """ Runs every time the program starts, this sets all device settings to default """
        for name, details in self.firmwareSettings.defaultSettings.items():
            setting_id = details["id"]
            value = details["value"]
            hid_data = self.create_settings_report(name, setting_id, value)
            self.add_data_to_write_queue(hid_data)

    def syncDeviceWithFileLoadedSettings(self):
        # TODO: Implement this later 
        """ Runs after default settings are applied, this reads data from a file and applies changed settings"""
        pass

    def updateSettingInRealTime(self):
        # TODO: Implement this Later
        """ Runs everytime a setting in the gui is changed so the device settings get updated in real time and the program is in sync with them """
        pass

    """ Utility method """
    def hexToString(self, data):
        hex_data = ' '.join(f'{byte:02X}' for byte in data)
        return hex_data