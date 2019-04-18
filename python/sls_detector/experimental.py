
from _sls_detector import multiDetectorApi

class ExperimentalDetector(multiDetectorApi):
    def __init__(self):
        super().__init__(0)
        self.online = True

    @property
    def online(self):
        return self._setOnline() == 1

    @online.setter
    def online(self, value):
        self._setOnline(value)

    @property
    def rx_udpip(self):
        return self._getReceiverUDPIP(-1)

    @rx_udpip.setter
    def rx_udpip(self, ip):
        self._setReceiverUDPIP(ip, -1)
    