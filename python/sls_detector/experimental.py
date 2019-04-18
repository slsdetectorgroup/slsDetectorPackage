
from _sls_detector import multiDetectorApi

class ExperimentalDetector(multiDetectorApi):
    def __init__(self):
        super().__init__(0)
        self.online = True

    