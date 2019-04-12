
from _sls_detector import multiDetectorApi

class ExperimentalDetector(multiDetectorApi):
    def __init__(self):
        super().__init__(0)


    @property
    def rx_udpip(self):
        return self._getReceiverUDPIP(-1)

    @rx_udpip.setter
    def rx_udpip(self, ip):
        self._setReceiverUDPIP(ip, -1)
    