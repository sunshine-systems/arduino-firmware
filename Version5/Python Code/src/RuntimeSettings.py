class RuntimeSettings:
    def __init__(self):
        # Settings for arduino
        self.arduinoPort = "COM8"
        self.arduinoBaudRate = 115200

        # Your Device Settings
        self.gamingPcResolution = {'height': 2560, 'width': 1440}
        self.cheatPcResolution = {'height': 2560, 'width': 1440}
        self.DPI = 1200
        self.valorantSensitivity = 0.16

        # Settings for logging
        self.enableLogs = True  # False | Enable or disable application-based logging in the terminal
        self.enableFromArduinoLogs = False  # False | Logging of data from the arduino
        self.enableToArduinoLogs = False  # False | Logging of data to the arduino
        self.enableFpsMetrics = True  # True| Logging of FPS processing to terminal every 1 second

        # Screenshot Settings & See what program sees (causes FPS drops)
        self.enableDisplayCvFrames = False  # Causes minor fps drops; but enables basic program visualization (displays frames)
        self.enableDisplayDebugCvFrames = False # Causes fps drops, but enabled debugging of the programs visualization
        self.monitorRefreshRate = 240
        self.enableScreenshotProcessing = True
        self.screenshotSize = 180
        self.screenshotProcessSize = 6
        self.offsetX = 0
        self.offsetY = 0

        """ Games for different types of cheats
            pixelinspector is a debug tool, which prints out the colors seen by the program
            humanbenchmark is for humanbenchmark.com
            all others are self explanitory
        """
        self.gameSelected = "humanbenchmark"  # Valid options are "humanbenchmark", "valorant"
        self.gamesSupported = ["pixelinspector", "humanbenchmark", "valorant"]

        # Valorant Specific Settings
        # https://codepen.io/Jtonna/pen/mdQGYJw colors
        # https://codepen.io/Jtonna/pen/Poxdvvx variance
        self.valorant_agent_highlights_options = ["purple", "yellow", "red"]
        self.valorant_agent_highlights = "purple"  # Valid options are "purple", "yellow", "red"
        self.valorantAimbotFPSMetricLogging = True
        self.valorantAimOnTapfire = True
        self.valorantTrackOnTapDuration = 5.400 # 400 MS of tracking max
        self.valorant_360_on_shoot = False
        self.valorant_FOV = 103
        self.valorant_FOV_vertical = 71
        self.enableMouseMovementLockoutOnLMBorMovement = True
        self.mouseMovementLockoutDurationsMS = 16