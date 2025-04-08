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
        self.stop_event = threading.Event()  # Event to signal stopping the thread

    def _firmware_processing_loop(self):
        if self.runtimeSettings.enableLogs:
            print("Firmware processing loop started")
               
        self.arduino.connect()
        self.start_time = time.perf_counter()  # Initialize start time

        while not self.stop_event.is_set() and self.arduino.connected:
            self.firmwareInterface.handle_incoming_data()
            self.firmwareInterface.handle_outgoing_data()

    def start_threads(self):
        self.firmware_thread = threading.Thread(target=self._firmware_processing_loop)
        self.firmware_thread.start()

    def stop_threads(self):
        if self.runtimeSettings.enableLogs:
            print("Stopping threads...")
        self.stop_event.set()  # Signal the thread to stop
        self.firmware_thread.join()  # Wait for the thread to finish

if __name__ == "__main__":
    try:
        """ Initialize all objects"""
        runtimeSettings = RuntimeSettings()
        arduinoInterface = ArduinoInterface(runtimeSettings=runtimeSettings)
        firmwareInterface = FirmwareInterface(runtimeSettings=runtimeSettings, arduinoInterface=arduinoInterface)
        processes = Processes(arduinoInterface=arduinoInterface, firmwareInterface=firmwareInterface, runtimeSettings=runtimeSettings)

        """ Test application of default settings"""
        firmwareInterface.syncDeviceWithDefaultSettings()
        print(firmwareInterface.outgoing_data_fifo_queue)
        
        """ Start Processing """
        processes.start_threads()

    except KeyboardInterrupt:
        print("CTRL+C detected! Stopping the process...")
        processes.stop_threads()
        print("Process terminated cleanly.")
