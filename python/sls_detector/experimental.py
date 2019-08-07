from _sls_detector import multiDetectorApi
from _sls_detector import slsDetectorDefs

runStatus = slsDetectorDefs.runStatus
from .utils import element_if_equal, all_equal
import datetime as dt

from functools import wraps


def freeze(cls):
    cls.__frozen = False

    def frozensetattr(self, key, value):
        if self.__frozen and not hasattr(self, key):
            raise AttributeError(
                "Class {} is frozen. Cannot set {} = {}".format(
                    cls.__name__, key, value
                )
            )
        else:
            object.__setattr__(self, key, value)

    def init_decorator(func):
        @wraps(func)
        def wrapper(self, *args, **kwargs):
            func(self, *args, **kwargs)
            self.__frozen = True

        return wrapper

    cls.__setattr__ = frozensetattr
    cls.__init__ = init_decorator(cls.__init__)
    return cls


@freeze
class ExperimentalDetector(multiDetectorApi):
    """
    This class is the base for detector specific 
    interfaces. Most functions exists in two versions
    like the getExptime() function that uses the 
    C++ API directly and the simplified exptime property. 
    """
    def __init__(self, multi_id = 0):
        """
        multi_id refers to the shared memory id of the 
        slsDetectorPackage. Default value is 0. 
        """
        super().__init__(multi_id)

    # Acq
    @property
    def rx_status(self):
        """
        Read the status of the receiver
        """
        return element_if_equal(self.getReceiverStatus())

    @rx_status.setter
    def rx_status(self, status_str):
        if status_str == "start":
            self.startReceiver()
        elif status_str == "stop":
            self.stopReceiver()
        else:
            raise NotImplementedError("Unknown argument to rx_status")

    @property
    def busy(self):
        """
        Checks if the detector is acquiring. Can also be set but should only be used if the acquire fails and
        leaves the detector with busy == True

        .. note ::

            Only works when the measurement is launched using acquire, not with status start!

        Returns
        --------
        bool
            :py:obj:`True` if the detector is acquiring otherwise :py:obj:`False`

        Examples
        ----------

        ::

            d.busy
            >> True

            #If the detector is stuck reset by:
            d.busy = False


        """
        return self.getAcquiringFlag()
    
    @busy.setter
    def busy(self, value):
        self.setAcquiringFlag(value)

    # Configuration
    @property
    def startingfnum(self):
        return element_if_equal(self.getStartingFrameNumber())

    @startingfnum.setter
    def startingfnum(self, value):
        self.setStartingFrameNumber(value)

    @property
    def config(self):
        return NotImplementedError("config is set only")

    @config.setter
    def config(self, fname):
        self.setConfig(fname)

    # File
    @property
    def fname(self):
        return element_if_equal(self.getFileName())

    @fname.setter
    def fname(self, file_name):
        self.setFileName(file_name)

    @property
    def fpath(self):
        return element_if_equal(self.getFilePath())
    
    @fpath.setter
    def fpath(self, path):
        self.setFilePath(path)

    @property
    def fwrite(self):
        return element_if_equal(self.getFileWrite())

    @fwrite.setter
    def fwrite(self, value):
        self.setFileWrite(value)

    @property
    def foverwrite(self):
        return element_if_equal(self.getFileOverWrite())

    @foverwrite.setter
    def foverwrite(self, value):
        self.setFileOverWrite(value)

    # Time
    @property
    def exptime(self):
        res = self.getExptime()
        return element_if_equal([it.total_seconds() for it in res])

    @exptime.setter
    def exptime(self, t):
        self.setExptime(dt.timedelta(seconds=t))

    @property
    def subexptime(self):
        res = self.getSubExptime()
        return element_if_equal([it.total_seconds() for it in res])

    @subexptime.setter
    def subexptime(self, t):
        self.setSubExptime(dt.timedelta(seconds=t))

    @property
    def period(self):
        res = self.getPeriod()
        return element_if_equal([it.total_seconds() for it in res])

    @period.setter
    def period(self, t):
        self.setPeriod(dt.timedelta(seconds=t))

