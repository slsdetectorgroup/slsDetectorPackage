from _sls_detector import CppDetectorApi
from _sls_detector import slsDetectorDefs

runStatus = slsDetectorDefs.runStatus
speedLevel = slsDetectorDefs.speedLevel
dacIndex = slsDetectorDefs.dacIndex

from .utils import element_if_equal, all_equal
from .utils import Geometry, to_geo
import datetime as dt

from functools import wraps
from collections import namedtuple

class Register:
    """
    Helper class to read and write to registers using a
    more Pythonic syntax
    """
    def __init__(self, detector):
        self._detector = detector

    def __getitem__(self, key):
        return self._detector.readRegister(key)

    def __setitem__(self, key, value):
        self._detector.writeRegister(key, value)


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
        self._register = Register(self)

    # CONFIGURATION
    def __len__(self):
        return self.size()

    def __repr__(self):
            return '{}(id = {})'.format(self.__class__.__name__,
                                        self.getShmId())


    def free(self):
        self.freeSharedMemory()




    @property
    def config(self):
        return NotImplementedError("config is set only")

    @config.setter
    def config(self, fname):
        self.loadConfig(fname)

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
    def dr(self):
        return element_if_equal(self.getDynamicRange())

    @dr.setter
    def dr(self, dr):
        self.setDynamicRange(dr)    

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

    

    
  
    # Time



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
    def rx_datastream(self):
        return element_if_equal(self.getRxZmqDataStream())

    @rx_datastream.setter
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
    def rx_zmqip(self):
        return element_if_equal(self.getRxZmqIP())

    @rx_zmqip.setter
    def rx_zmqip(self, ip):
        self.setRxZmqIP(ip)

    @property
    def zmqip(self):
        return element_if_equal(self.getClientZmqIp())

    @zmqip.setter
    def zmqip(self, ip):
        self.setClientZmqIp(ip)

    #TODO! Change to dst
    @property
    def rx_udpip(self):
        return element_if_equal(self.getDestinationUDPIP())

    @rx_udpip.setter
    def rx_udpip(self, ip):
        self.getDestinationUDPIP(ip)
    @property
    def rx_udpip2(self):
        return element_if_equal(self.getDestinationUDPIP2())

    @rx_udpip2.setter
    def rx_udpip2(self, ip):
        self.getDestinationUDPIP2(ip)

    @property
    def rx_udpmac(self):
        return element_if_equal(self.getDestinationUDPMAC())

    @rx_udpmac.setter
    def rx_udpmac(self, mac):
        self.getDestinationUDPMAC2(mac)

    @property
    def rx_udpmac2(self):
        return element_if_equal(self.getDestinationUDPMAC2())

    @rx_udpmac2.setter
    def rx_udpmac2(self, mac):
        self.getDestinationUDPMAC2(mac)


    @property
    def detectormac(self):
        return element_if_equal(self.getSourceUDPMAC())

    @detectormac.setter
    def detectormac(self, mac):
        self.setSourceUDPMAC()

    @property
    def detectormac2(self):
        return element_if_equal(self.getSourceUDPMAC2())

    @detectormac2.setter
    def detectormac2(self, mac):
        self.setSourceUDPMAC2()

    @property
    def vhighvoltage(self):
        return element_if_equal(self.getHighVoltage())

    @vhighvoltage.setter
    def vhighvoltage(self, v):
        self.setHighVoltage(v)

    @property
    def user(self):
        return self.getUserDetails()

    @property
    def settingsdir(self):
        return element_if_equal(self.getSettingsDir())

    @settingsdir.setter
    def settingsdir(self, dir):
        self.setSettingsDir(dir)

    @property
    def status(self):
        return element_if_equal(self.getDetectorStatus())

    @property
    def rx_status(self):
        return element_if_equal(self.getReceiverStatus())



    @property
    def rx_udpsocksize(self):
        return element_if_equal(self.getRxUDPSocketBufferSize())

    @rx_udpsocksize.setter
    def rx_udpsocksize(self, buffer_size):
        self.setRxUDPSocketBufferSize(buffer_size)

    @property
    def rx_realudpsocksize(self):
        return element_if_equal(self.getRxRealUDPSocketBufferSize())

    @property
    def trimbits(self):
        return NotImplementedError('trimbits are set only')

    @trimbits.setter
    def trimbits(self, fname):
        self.loadTrimbits(fname)

    @property
    def lock(self):
        return element_if_equal(self.getDetectorLock())

    @lock.setter
    def lock(self, value):
        self.setDetectorLock(value)

    @property
    def rx_lock(self):
        return element_if_equal(self.getRxLock())

    @rx_lock.setter
    def rx_lock(self, value):
        self.setRxLock(value)

    @property
    def lastclient(self):
        return element_if_equal(self.getLastClientIP())

    @property
    def reg(self):
        return self._register

    @property
    def ratecorr(self):
        """ tau in ns """
        return element_if_equal(self.getRateCorrection())

    @ratecorr.setter
    def ratecorr(self, tau):
        self.setRateCorrection(tau)

    @property
    def clkdivider(self):
        res = [int(value) for value in self.getSpeed()]
        return element_if_equal(res)

    @clkdivider.setter
    def clkdivider(self, value):
        self.setSpeed(speedLevel(value))

    @property
    def frameindex(self):
        return self.getRxCurrentFrameIndex()

    @property
    def threshold(self):
        return element_if_equal(self.getThresholdEnergy())

    @threshold.setter
    def threshold(self, eV):
        self.setThresholdEnergy(eV)

    @property
    def timing(self):
        return element_if_equal(self.getTimingMode()) 

    @timing.setter
    def timing(self, mode):
        self.setTimingMode(mode) 

    @property
    def trimen(self):
        return element_if_equal(self.getTrimEnergies())

    @trimen.setter
    def trimen(self, energies):
        self.setTrimEnergies(energies)

    @property
    def vthreshold(self):
        return element_if_equal(self.getDAC(dacIndex.THRESHOLD))
