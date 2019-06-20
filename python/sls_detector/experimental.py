
from _sls_detector import multiDetectorApi
from .utils import element_if_equal, all_equal
import datetime as dt
class ExperimentalDetector(multiDetectorApi):
    def __init__(self):
        super().__init__(0)
        self.online = True

    @property
    def exptime(self):
        res = self.getExptime()
        return element_if_equal([it.total_seconds() for it in res])
    @exptime.setter
    def exptime(self, t):
        self.setExptime(dt.timedelta(seconds = t))

    @property
    def period(self):
        res = self.getPeriod()
        return element_if_equal([it.total_seconds() for it in res])
    @period.setter
    def period(self, t):
        self.setPeriod(dt.timedelta(seconds = t))

    @property
    def subexptime(self):
        res = self.getSubExptime()
        return element_if_equal([it.total_seconds() for it in res])
    @subexptime.setter
    def subexptime(self, t):
        self.setSubExptime(dt.timedelta(seconds = t))