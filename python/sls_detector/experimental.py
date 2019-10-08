from _sls_detector import CppDetectorApi
from _sls_detector import slsDetectorDefs

runStatus = slsDetectorDefs.runStatus
from .utils import element_if_equal, all_equal
from .utils import Geometry, to_geo
import datetime as dt

from functools import wraps
from collections import namedtuple


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
class ExperimentalDetector(CppDetectorApi):
    """
    This class is the base for detector specific 
    interfaces. Most functions exists in two versions
    like the getExptime() function that uses the 
    C++ API directly and the simplified exptime property. 
    """

    def __init__(self, multi_id=0):
        """
        multi_id refers to the shared memory id of the 
        slsDetectorPackage. Default value is 0. 
        """
        super().__init__(multi_id)

    # CONFIGURATION
    def __len__(self):
        return self.size()

    def free(self):
        self.freeSharedMemory()

    @property
    def config(self):
        return NotImplementedError("config is set only")

    @config.setter
    def config(self, fname):
        self.setConfig(fname)

    @property
    def parameters(self):
        return NotImplementedError("parameters is set only")

    @parameters.setter
    def parameters(self, fname):
        self.loadParameters(fname)

    @property
    def hostname(self):
        return self.getHostname()

    @hostname.setter
    def hostname(self, hostnames):
        if isinstance(hostnames, str):
            hostnames = [hostnames]
        if isinstance(hostnames, list):
            self.setHostname(hostnames)
        else:
            raise ValueError("hostname needs to be string or list of strings")

    @property
    def fw_version(self):
        return element_if_equal(self.getFirmwareVersion())

    @property
    def server_version(self):
        #TODO! handle hex print
        return element_if_equal(self.getDetectorServerVersion())

    @property
    def client_version(self):
        return element_if_equal(self.getClientVersion())

    @property
    def rx_version(self):
        return element_if_equal(self.getReceiverVersion())

    @property
    def detector_type(self):
        return element_if_equal(self.getDetectorType())

    @property
    def module_geometry(self):
        return to_geo(self.getModuleGeometry())

    @property
    def module_size(self):
        ms = [to_geo(item) for item in self.getModuleSize()]
        return element_if_equal(ms)

    @property
    def detector_size(self):
        return to_geo(self.getDetectorSize())

    @property
    def settings(self):
        return element_if_equal(self.getSettings())

    @settings.setter
    def settings(self, value):
        self.setSettings(value)

    @property
    def frames(self):
        return element_if_equal(self.getNumberOfFrames())

    @frames.setter
    def frames(self, n_frames):
        self.setNumberOfFrames(n_frames)


    @property
    def exptime(self):
        res = self.getExptime()
        return element_if_equal([it.total_seconds() for it in res])

    @exptime.setter
    def exptime(self, t):
        if isinstance(t, dt.timedelta):
            self.setExptime(t)
        else:
            self.setExptime(dt.timedelta(seconds=t))

    @property
    def subexptime(self):
        res = self.getSubExptime()
        return element_if_equal([it.total_seconds() for it in res])

    @subexptime.setter
    def subexptime(self, t):
        if isinstance(t, dt.timedelta):
            self.setSubExptime(t)
        else:
            self.setSubExptime(dt.timedelta(seconds=t))


    @property
    def period(self):
        res = self.getPeriod()
        return element_if_equal([it.total_seconds() for it in res])

    @period.setter
    def period(self, t):
        if isinstance(t, dt.timedelta):
            self.setPeriod(t)
        else:
            self.setPeriod(dt.timedelta(seconds=t))
    # Acq
    

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
    
  
    # Time
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

    #TODO! Rename to rx_framescaught
    @property
    def framescaught(self):
        return element_if_equal(self.getFramesCaught())
    

    @property
    def startingfnum(self):
        return element_if_equal(self.getStartingFrameNumber())

    @startingfnum.setter
    def startingfnum(self, value):
        self.setStartingFrameNumber(value)

   #TODO! testing switches on automatically?
    @property
    def flowcontrol_10g(self):
        return element_if_equal(self.getTenGigaFlowControl())

    @flowcontrol_10g.setter
    def flowcontrol_10g(self, enable):
        self.setTenGigaFlowControl(enable)

     #TODO! add txdelay

    @property
    def use_receiver(self):
        return element_if_equal(self.getUseReceiverFlag())

    @property
    def rx_hostname(self):
        return element_if_equal(self.getRxHostname())

    @rx_hostname.setter
    def rx_hostname(self, hostname):
        self.setRxHostname(hostname)

    @property
    def rx_tcpport(self):
        return element_if_equal(self.getRxPort())

    @rx_tcpport.setter
    def rx_tcpport(self, port):
        self.setRxPort(port)

    @property
    def rx_fifodepth(self):
        return element_if_equal(self.getRxFifoDepth())

    @rx_fifodepth.setter
    def rx_fifodepth(self, frames):
        self.setRxFifoDepth(frames)

    @property
    def rx_silent(self):
        return element_if_equal(self.getRxSilentMode())

    @rx_silent.setter
    def rx_silent(self, value):
        self.setRxSilentMode(value)

    @property
    def rx_discardpolicy(self):
        return element_if_equal(self.getRxFrameDiscardPolicy())

    @rx_discardpolicy.setter
    def rx_discardpolicy(self, policy):
        self.setRxFrameDiscardPolicy()

    @property
    def rx_padding(self):
        return element_if_equal(self.getPartialFramesPadding())

    @rx_padding.setter
    def rx_padding(self, policy):
        self.setPartialFramesPadding(policy)

    @property
    def rx_lock(self):
        """Lock the receiver to a specific IP"""
        return element_if_equal(self.getRxLock())

    @rx_lock.setter
    def rx_lock(self, value):
        self.setRxLock(value)

    @property
    def rx_lastclient(self):
        return element_if_equal(self.getRxLastClientIP())


    #FILE

    @property
    def fformat(self):
        return element_if_equal(self.getFileFormat())
    
    @fformat.setter
    def fformat(self, format):
        self.setFileFormat(format)

    @property
    def findex(self):
        return element_if_equal(self.getAcquisitionIndex())

    @findex.setter
    def findex(self, index):
        self.setAcquisitionIndex(index)

    @property
    def fname(self):
        return element_if_equal(self.getFileNamePrefix())

    @fname.setter
    def fname(self, file_name):
        self.setFileNamePrefix(file_name)

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

    @property
    def fmaster(self):
        return element_if_equal(self.getMasterFileWrite())

    @fmaster.setter
    def fmaster(self, enable):
        self.setMasterFileWrite(enable)

    @property
    def rx_framesperfile(self):
        return element_if_equal(self.getFramesPerFile())

    @rx_framesperfile.setter
    def rx_framesperfile(self, n_frames):
        self.setFramesPerFile(n_frames)

    # ZMQ Streaming Parameters (Receiver<->Client)

    @property
    def rx_zmqdatastream(self):
        return element_if_equal(self.getRxZmqDataStream())

    @rx_zmqdatastream.setter
    def rx_zmqdatastream(self, enable):
        self.setRxZmqDataStream(enable)

    @property
    def rx_readfreq(self):
        return element_if_equal(self.getRxZmqFrequency())

    @rx_readfreq.setter
    def rx_readfreq(self, nth_frame):
        self.setRxZmqFrequency(nth_frame)

    @property
    def rx_zmqport(self):
        return element_if_equal(self.getRxZmqPort())

    @rx_zmqport.setter
    def rx_zmqport(self, port):
        self.setRxZmqPort(port)

    @property
    def zmqport(self):
        return element_if_equal(self.getClientZmqPort())

    @zmqport.setter
    def zmqport(self, port):
        self.setClientZmqPort(port)

    @property
    def zmqip(self):
        return element_if_equal(self.getClientZmqIp())

    @zmqip.setter
    def zmqip(self, ip):
        self.setClientZmqIp(ip)