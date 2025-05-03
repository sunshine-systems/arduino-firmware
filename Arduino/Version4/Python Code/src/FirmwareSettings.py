
class FirmwareSettings:
    def __init__(self):
        """ Create default Settings Object
            DO NOT TOUCH
        """
        self.defaultSettings = {
            "FIRMWARE_VERSION": {"id": 0, "value": 0}, # this will be discarded if we try to change it. it will just return the on-device firmware version
            "logPerformanceMetrics": {"id": 1, "value": False},
            "mouseLockoutEnable": {"id": 2, "value": False},
            "mouseLockoutDurationMilliseconds": {"id": 3, "value": 0},
            "enableSpinning": {"id": 4, "value": 0}, # 0=Disabled, 1=Enabled
            "spinAmountPerRotation": {"id": 5, "value": 0},
            "spinNumberOfRotations": {"id": 6, "value": 0},
            "spinDelayBetweenRotationsMilliseconds": {"id": 7, "value": 0},
            "spinLockoutMouse": {"id": 8, "value": False},
            "spinBeforeAfterMouseEvent": {"id": 9, "value": 2} # 0=Before, 1=After, 2=Both
        }

        self.mySettings = {
            "FIRMWARE_VERSION": {"id": 0, "value": 0}, # this will be discarded if we try to change it. it will just return the on-device firmware version
            "logPerformanceMetrics": {"id": 1, "value": False},
            "mouseLockoutEnable": {"id": 2, "value": True},
            "mouseLockoutDurationMilliseconds": {"id": 3, "value": 16},
            "enableSpinning": {"id": 4, "value": 1}, # 0=Disabled, 1=Enabled
            "spinAmountPerRotation": {"id": 5, "value": 5357},
            "spinNumberOfRotations": {"id": 6, "value": 6},
            "spinDelayBetweenRotationsMilliseconds": {"id": 7, "value": 16},
            "spinLockoutMouseUntilCompletion": {"id": 8, "value": True},
            "spinBeforeAfterMouseEvent": {"id": 9, "value": 2} # 0=Before, 1=After, 2=Both
        }