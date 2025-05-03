import threading
import time
from ArduinoInterface import ArduinoInterface
from FirmwareInterface import FirmwareInterface
from RuntimeSettings import RuntimeSettings

class Processes:
    def __init__(self, arduinoInterface, firmwareInterface, runtimeSettings, timeout=20) -> None:
        self.arduino = arduinoInterface
        self.firmwareInterface = firmwareInterface
        self.runtimeSettings = runtimeSettings
        self.timeout = timeout  # Timeout for the thread to run

        self.firmware_thread = None
        self.start_time = None  # Track start time

    def _firmware_processing_loop(self):
        if self.runtimeSettings.enableLogs:
            print("firmware processing loop started")
               
        self.arduino.connect()
        self.start_time = time.perf_counter()  # Initialize start time
        last_outgoing_time = 0  # Initialize last outgoing data handle time

        while self.arduino.connected and (time.perf_counter() - self.start_time < self.timeout):
            self.firmwareInterface.handle_incoming_data()

            # Check if it's time to handle outgoing data (every half second)
            current_time = time.perf_counter()
            if current_time - last_outgoing_time >= 0.5:
                self.firmwareInterface.handle_outgoing_data()
                last_outgoing_time = current_time  # Update the last handled time

    def start_threads(self):
        self.firmware_thread = threading.Thread(target=self._firmware_processing_loop)
        self.firmware_thread.start()

    def stop_threads(self):
        self.firmware_thread.join()  # Wait for the thread to finish

class TestSuiteOfData:
    def __init__(self, firmwareInterface) -> None:
        self.firmwareInterface = firmwareInterface


    def create_and_set_all_test_data(self):
        firmwareInterface = self.firmwareInterface

        data = firmwareInterface.create_spoofed_hid_report(0, 0, 0)
        firmwareInterface.add_data_to_write_queue(data)

        # # Move up 22px
        # print("up")
        # data = firmwareInterface.create_spoofed_hid_report(0, 0, -11)
        # firmwareInterface.add_data_to_write_queue(data)
        # data = firmwareInterface.create_spoofed_hid_report(0, 0, -22)
        # firmwareInterface.add_data_to_write_queue(data)
        # data = firmwareInterface.create_spoofed_hid_report(0, 0, -33)
        # firmwareInterface.add_data_to_write_queue(data)

        # # Move down 22px
        # print("down")
        # data = firmwareInterface.create_spoofed_hid_report(0, 0, 11)
        # firmwareInterface.add_data_to_write_queue(data)
        # data = firmwareInterface.create_spoofed_hid_report(0, 0, 22)
        # firmwareInterface.add_data_to_write_queue(data)
        # data = firmwareInterface.create_spoofed_hid_report(0, 0, 33)
        # firmwareInterface.add_data_to_write_queue(data)

        # # Move Left 22px
        # print("right")
        # data = firmwareInterface.create_spoofed_hid_report(0, -11, 0)
        # firmwareInterface.add_data_to_write_queue(data)
        # data = firmwareInterface.create_spoofed_hid_report(0, -22, 0)
        # firmwareInterface.add_data_to_write_queue(data)
        # data = firmwareInterface.create_spoofed_hid_report(0, -33, 0)
        # firmwareInterface.add_data_to_write_queue(data)

        # # Move Right 22px
        # print("left")
        # data = firmwareInterface.create_spoofed_hid_report(0, 11, 0)
        # firmwareInterface.add_data_to_write_queue(data)
        # data = firmwareInterface.create_spoofed_hid_report(0, 22, 0)
        # firmwareInterface.add_data_to_write_queue(data)
        # data = firmwareInterface.create_spoofed_hid_report(0, 33, 0)
        # firmwareInterface.add_data_to_write_queue(data)


if __name__ == "__main__":
    """ Initalize all objects"""
    runtimeSettings = RuntimeSettings()
    arduinoInterface = ArduinoInterface(runtimeSettings=runtimeSettings)
    firmwareInterface = FirmwareInterface(runtimeSettings=runtimeSettings, arduinoInterface=arduinoInterface)
    testSuite = TestSuiteOfData(firmwareInterface)
    processes = Processes(arduinoInterface=arduinoInterface, firmwareInterface=firmwareInterface, runtimeSettings=runtimeSettings, timeout=20)

    """ Test application of default settings"""
    firmwareInterface.syncDeviceWithDefaultSettings()

    """ Apply all movement test data """
    testSuite.create_and_set_all_test_data()
    
    """ Start Processing """
    processes.start_threads()