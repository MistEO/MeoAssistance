from enum import Enum


class AlgorithmType(Enum):
    # Enum for algorithm types
    JustReturn = "JustReturn"
    MatchTemplate = "MatchTemplate"
    OcrDetect = "OcrDetect"
    Hash = "Hash"

    def __eq__(self, other):
        return self.value == other


class ActionType(Enum):
    # Enum for actions
    ClickSelf = "ClickSelf"
    ClickRect = "ClickRect"
    DoNothing = "DoNothing"
    Stop = "Stop"
    Swipe = "Swipe"

    def __eq__(self, other):
        return self.value == other


class MethodType(Enum):
    # Enum for method types
    Ccoeff = "Ccoeff"
    CcoeffHSV = "CcoeffHSV"
    RGBCount = "RGBCount"
    HSVCount = "HSVCount"

    def __eq__(self, other):
        return self.value == other


class TaskStatus(Enum):
    # Enum for task derived types
    Raw = "Raw"
    Original = "Original"
    Initialized = "Initialized"
    Interpreted = "Interpreted"
