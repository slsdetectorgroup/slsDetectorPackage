from _sls_detector import CppDetectorApi
from _sls_detector import slsDetectorDefs

runStatus = slsDetectorDefs.runStatus
speedLevel = slsDetectorDefs.speedLevel
dacIndex = slsDetectorDefs.dacIndex

from .utils import element_if_equal, all_equal, get_set_bits, list_to_bitmask
from .utils import Geometry, to_geo
from .registers import Register, Adc_register
import datetime as dt

from functools import wraps
from collections import namedtuple

# class Register:
#     """
#     Helper class to read and write to registers using a
#     more Pythonic syntax
#     """
#     def __init__(self, detector):
#         self._detector = detector

#     def __getitem__(self, key):
#         return self._detector.readRegister(key)

#     def __setitem__(self, key, value):
#         self._detector.writeRegister(key, value)


def freeze(cls):
    cls._frozen = False

    def frozensetattr(self, key, value):
        if self._frozen and not hasattr(self, key):
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
            self._frozen = True

        return wrapper

    cls.__setattr__ = frozensetattr
    cls.__init__ = init_decorator(cls.__init__)
    return cls


@freeze
class Detector(CppDetectorApi):
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
        self._adc_register = Adc_register(self)



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
    def subdeadtime(self):
        res = self.getSubDeadTime()
        return element_if_equal([it.total_seconds() for it in res])

    @subdeadtime.setter
    def subdeadtime(self, t):
        if isinstance(t, dt.timedelta):
            self.setSubDeadTime(t)
        else:
            self.setSubDeadTime(dt.timedelta(seconds=t))


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
    @property
    def rx_framescaught(self):
        return element_if_equal(self.getFramesCaught())
    

    @property
    def startingfnum(self):
        return element_if_equal(self.getStartingFrameNumber())

    @startingfnum.setter
    def startingfnum(self, value):
        self.setStartingFrameNumber(value)



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


    @property
    def udp_dstip(self):
        return element_if_equal(self.getDestinationUDPIP())

    @udp_dstip.setter
    def udp_dstip(self, ip):
        self.getDestinationUDPIP(ip)

    @property
    def udp_dstip2(self):
        return element_if_equal(self.getDestinationUDPIP2())

    @udp_dstip2.setter
    def udp_dstip2(self, ip):
        self.getDestinationUDPIP2(ip)

    @property
    def udp_dstmac(self):
        return element_if_equal(self.getDestinationUDPMAC())

    @udp_dstmac.setter
    def udp_dstmac(self, mac):
        self.getDestinationUDPMAC2(mac)

    @property
    def udp_dstmac2(self):
        return element_if_equal(self.getDestinationUDPMAC2())

    @udp_dstmac2.setter
    def udp_dstmac2(self, mac):
        self.getDestinationUDPMAC2(mac)


    @property
    def udp_srcip(self):
        return element_if_equal(self.getSourceUDPIP())

    @udp_srcip.setter
    def udp_srcip(self, ip):
        self.setSourceUDPIP(ip)

    @property
    def udp_srcip2(self):
        return element_if_equal(self.getSourceUDPIP2())

    @udp_srcip2.setter
    def udp_srcip2(self, ip):
        self.setSourceUDPIP2(ip)

    @property
    def udp_dstport(self):
        return element_if_equal(self.getDestinationUDPPort())

    @udp_dstport.setter
    def udp_dstport(self, port):
        self.setDestinationUDPPort(port)

    @property
    def udp_dstport2(self):
        return element_if_equal(self.getDestinationUDPPort2())

    @udp_dstport2.setter
    def udp_dstport2(self, port):
        self.setDestinationUDPPort2(port)

    @property
    def src_udpmac(self):
        return element_if_equal(self.getSourceUDPMAC())

    @src_udpmac.setter
    def src_udpmac(self, mac):
        self.setSourceUDPMAC(mac)

    @property
    def src_udpip2(self):
        return element_if_equal(self.getSourceUDPIP())

    @src_udpip2.setter
    def src_udpip2(self, ip):
        self.setSourceUDPIP(ip)

    @property
    def src_udpip(self):
        return element_if_equal(self.getSourceUDPIP())

    @src_udpip.setter
    def src_udpip(self, ip):
        self.setSourceUDPIP(ip)


    @property
    def src_udpmac2(self):
        return element_if_equal(self.getSourceUDPMAC2())

    @src_udpmac2.setter
    def src_udpmac2(self, mac):
        self.setSourceUDPMAC2(mac)

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
    def settingspath(self):
        return element_if_equal(self.getSettingsPath())

    @settingspath.setter
    def settingspath(self, path):
        self.setSettingsPath(path)

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
    def adcreg(self):
        return self._adc_register


    @property
    def led(self):
        return element_if_equal(self.getLEDEnable())

    @led.setter
    def led(self, value):
        self.setLEDEnable(value)

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

    

    @property
    def type(self):
        return element_if_equal(self.getDetectorType())

    @property
    def rx_frameindex(self):
        return element_if_equal(self.getRxCurrentFrameIndex())

    @property
    def rx_missingpackets(self):
        return element_if_equal(self.getNumMissingPackets())


    """
    Some Eiger stuff, does this have to be here or can we move it to subclass?
    """
    @property
    def partialreset(self):
        return element_if_equal(self.getPartialReset())

    @partialreset.setter
    def partialreset(self, value):
        self.setPartialReset(value)

    @property
    def tengiga(self):
        return element_if_equal(self.getTenGiga())
    
    @tengiga.setter
    def tengiga(self, value):
        self.setTenGiga(value)

    @property
    def overflow(self):
        return element_if_equal(self.getOverFlowMode())

    @overflow.setter
    def overflow(self, value):
        self.setOverFlowMode(value)

    @property
    def flowcontrol10g(self):
        return element_if_equal(self.getTenGigaFlowControl())

    @flowcontrol10g.setter
    def flowcontrol10g(self, enable):
        self.setTenGigaFlowControl(enable)

    @property
    def interruptsubframe(self):
        return element_if_equal(self.getInterruptSubframe())

    @interruptsubframe.setter
    def interruptsubframe(self, value):
        self.setInterruptSubframe(value)

    @property
    def gappixels(self):
        return element_if_equal(self.getRxAddGapPixels())

    @gappixels.setter
    def gappixels(self, value):
        self.setRxAddGapPixels(value)

    @property
    def measuredperiod(self):
        res = self.getMeasuredPeriod()
        return element_if_equal([it.total_seconds() for it in res])

    @property
    def measuredsubperiod(self):
        res = self.getMeasuredSubFramePeriod()
        return element_if_equal([it.total_seconds() for it in res])


    @property
    def storeinram(self):
        return element_if_equal(self.getStoreInRamMode())

    @storeinram.setter
    def storeinram(self, value):
        self.setStoreInRamMode(value)


    """
    Mythen3 specific
    """

    @property
    def counters(self):
        mask = self.getCounterMask()
        mask = element_if_equal(mask)
        if type(mask) == int:
            return get_set_bits(mask)
        else:
            return [get_set_bits(m) for m in mask]


    @counters.setter
    def counters(self, values):
        self.setCounterMask(list_to_bitmask(values))
        
    """
    CTB stuff 
    """

    @property
    def asamples(self):
        return element_if_equal(self.getNumberOfAnalogSamples())

    @asamples.setter
    def asamples(self, N):
        self.setNumberOfAnalogSamples(N)

    @property
    def dsamples(self):
        return element_if_equal(self.getNumberOfDigitalSamples())

    @dsamples.setter
    def dsamples(self, N):
        self.setNumberOfDigitalSamples(N)

    @property
    def dbitphase(self):
        return element_if_equal(self.getDBITPhase())

    @dbitphase.setter
    def dbitphase(self, value):
        self.setDBITPhase(value)

    @property
    def dbitclk(self):
        return element_if_equal(self.getDBITClock())
    
    @dbitclk.setter
    def dbitclk(self, value):
        self.setDBITClock(value)

    @property
    def dbitpipeline(self):
        return element_if_equal(self.getDBITPipeline())

    @dbitpipeline.setter
    def dbitpipeline(self, value):
        self.setDBITPipeline(value)

    @property
    def maxdbitphaseshift(self):
        return element_if_equal(self.getMaxDBITPhaseShift())

    @property
    def rx_dbitlist(self):
        return element_if_equal(self.getRxDbitList())

    @rx_dbitlist.setter
    def rx_dbitlist(self, value):
        self.setRxDbitList(value)

    @property
    def rx_dbitoffset(self):
        return element_if_equal(self.getRxDbitOffset())

    @rx_dbitoffset.setter
    def rx_dbitoffset(self, value):
        self.setRxDbitOffset(value)

    @property
    def maxadcphaseshift(self):
        return element_if_equal(self.getMaxADCPhaseShift())

    @property
    def maxclkphaseshift(self):
        return element_if_equal(self.getMaxClockPhaseShift())

    @property
    def adcphase(self):
        return element_if_equal()