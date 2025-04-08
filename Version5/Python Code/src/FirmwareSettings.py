
class FirmwareSettings:
    def __init__(self):
        """ Create default Settings Object
            DO NOT TOUCH
        """
        self.defaultSettings = {
            "FIRMWARE_VERSION": {"id": 0, "value": 5}, # this will be discarded if we try to change it. it will just return the on-device firmware version
            "logPerformanceMetrics": {"id": 1, "value": False},
            "enableSensReduction": {"id": 2, "value": True},
            "sensReductionDurationMilliseconds": {"id": 3, "value": 12000},
            "sensReductionAmmountX": {"id": 4, "value": 60}, # 100 = full movement (no reduction) 0 = full lockout (full reduction)
            "sensReductionAmmountY": {"id": 5, "value": 60}, # 100 = full movement (no reduction) 0 = full lockout (full reduction)
            "enableSpinning": {"id": 6, "value": False},
            "spinAmountPerRotation": {"id": 7, "value": 120},
            "spinNumberOfRotations": {"id": 8, "value": 8},
            "spinDelayBetweenRotationsMilliseconds": {"id": 9, "value": 9},
            "spinLockoutMouse": {"id": 10, "value": False},
            "spinBeforeAfterMouseEvent": {"id": 11, "value": 2}, # 0=Before, 1=After, 2=Both
            "disablePassthroughForMMB": {"id": 12, "value": 1}, # 0 = False, 1 = True, 2 = Only if MMB has been pressed within duration, 3 = Only pass through if double tapped (and or held) within duration
            "disablePassthroughForRMB": {"id": 13, "value": 3},
            "disablePassthroughForLMB": {"id": 14, "value": 0},
            "disablePassthroughForMB4": {"id": 15, "value": 2},
            "disablePassthroughForMB5": {"id": 16, "value": 2},
        }

        self.mySettings = {
            "FIRMWARE_VERSION": {"id": 0, "value": 5}, # this will be discarded if we try to change it. it will just return the on-device firmware version
            "logPerformanceMetrics": {"id": 1, "value": False},
            "mouseLockoutEnable": {"id": 2, "value": True},
            "mouseLockoutDurationMilliseconds": {"id": 3, "value": 8},
            "enableSpinning": {"id": 4, "value": 0}, # 0=Disabled, 1=Enabled
            "spinAmountPerRotation": {"id": 5, "value": 5357},
            "spinNumberOfRotations": {"id": 6, "value": 6},
            "spinDelayBetweenRotationsMilliseconds": {"id": 7, "value": 16},
            "spinLockoutMouseUntilCompletion": {"id": 8, "value": True},
            "spinBeforeAfterMouseEvent": {"id": 9, "value": 2}, # 0=Before, 1=After, 2=Both
            "disablePassthroughForMMB": {"id": 10, "value": 1}, # 0 = False, 1 = True, 2 = Only if MMB has been pressed within duration
            "disablePassthroughForRMB": {"id": 11, "value": 0},
            "disablePassthroughForLMB": {"id": 12, "value": 0},
            "disablePassthroughForMB4": {"id": 13, "value": 1},
            "disablePassthroughForMB5": {"id": 14, "value": 1},
        }