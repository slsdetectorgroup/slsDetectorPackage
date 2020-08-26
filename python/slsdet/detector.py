from _slsdet import CppDetectorApi
from _slsdet import slsDetectorDefs
from _slsdet import IpAddr, MacAddr

runStatus = slsDetectorDefs.runStatus
timingMode = slsDetectorDefs.timingMode
speedLevel = slsDetectorDefs.speedLevel
dacIndex = slsDetectorDefs.dacIndex
detectorType = slsDetectorDefs.detectorType

from .utils import element_if_equal, all_equal, get_set_bits, list_to_bitmask
from .utils import Geometry, to_geo, element, reduce_time, is_iterable
from . import utils as ut
from .registers import Register, Adc_register
import datetime as dt

from functools import wraps
from collections import namedtuple
import socket
import numpy as np

def freeze(cls):
    cls._frozen = False

    def frozensetattr(self, key, value):
        if self._frozen and not key in dir(self):
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
        return "{}(id = {})".format(self.__class__.__name__, self.getShmId())

    def free(self):
        """Free detector shared memory"""
        self.freeSharedMemory()

    @property
    def config(self):
        """Load configuration file.

        Note
        -----
        Frees shared memory before loading configuration file. 
        Set up once.
        
        :getter: Not implemented
        :setter: Loads config file

        Examples
        -----------
        >>> d.config = "/path/to/config/file.config"

        """
        return NotImplementedError("config is set only")

    @config.setter
    def config(self, fname):
        fname = ut.make_string_path(fname)
        self.loadConfig(fname)

    @property
    def parameters(self):
        """Sets detector measurement parameters to those contained in fname. Set up per measurement.
        
        Note 
        -----
        Equivalent to config, but does not free shared memory. 


        :getter: Not implemented
        :setter: loads parameters file

        Example
        ---------

        >>> d.parameters = 'path/to/file.par'
        
        """
        return NotImplementedError("parameters is set only")

    @parameters.setter
    def parameters(self, fname):
        fname = ut.make_string_path(fname)
        self.loadParameters(fname)

    @property
    def hostname(self):
        """Frees shared memory and sets hostname (or IP address) of all modules concatenated by + """
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
        # TODO! handle hex print
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
        """
        Dynamic range or number of bits per pixel/channel.

        Note
        -----
        [Eiger] Options: 4, 8, 16, 32. If set to 32, also sets clkdivider to 2 (quarter speed), else to 0 (full speed)\n
        [Mythen3] Options: 8, 16, 32 \n
        [Jungfrau][Gotthard][Ctb][Moench][Mythen3][Gotthard2] 16
        """
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
        """Number of frames per acquisition. In trigger mode, number of frames per trigger.

        Notes
        -----
        Cannot be set in modular level. ????
        In scan mode, number of frames is set to number of steps.
        [Gotthard2] Burst mode has a maximum of 2720 frames.
        """
        return element_if_equal(self.getNumberOfFrames())

    @frames.setter
    def frames(self, n_frames):
        self.setNumberOfFrames(n_frames)

    @property
    def triggers(self):
        return element_if_equal(self.getNumberOfTriggers())

    @triggers.setter
    def triggers(self, n_triggers):
        self.setNumberOfTriggers(n_triggers)

    @property
    def exptime(self):
        """
        Exposure time, accepts either a value in seconds or datetime.timedelta

        Note
        -----
        [Mythen3] sets exposure time to all gate signals in auto and trigger mode (internal gating). To specify gateIndex, use getExptime or setExptime.
        
        :getter: always returns in seconds. To get in datetime.delta, use getExptime

        Examples
        -----------
        >>> d.exptime = 1.05
        >>> d.exptime = datetime.timedelta(minutes = 3, seconds = 1.23)
        >>> d.exptime
        181.23
        >>> d.getExptime()
        [datetime.timedelta(seconds=181, microseconds=230000)]
        """
        if self.type == detectorType.MYTHEN3:
            res = self.getExptimeForAllGates()
        else:
            res = self.getExptime()
        return reduce_time(res)

    @exptime.setter
    def exptime(self, t):
        if self.type == detectorType.MYTHEN3 and is_iterable(t):
            for i, v in enumerate(t):
                if isinstance(v, int):
                    v = float(v)
                self.setExptime(i, v)
        else:
            if isinstance(t, int):
                t = float(t)
            self.setExptime(t)




    @property
    def period(self):
        """
        Period between frames, accepts either a value in seconds or datetime.timedelta

        Note
        -----
        :getter: always returns in seconds. To get in datetime.delta, use getPeriod

        Examples
        -----------
        >>> d.period = 1.05
        >>> d.period = datetime.timedelta(minutes = 3, seconds = 1.23)
        >>> d.period
        181.23
        >>> d.getPeriod()
        [datetime.timedelta(seconds=181, microseconds=230000)]
        """
        res = self.getPeriod()
        return reduce_time(res)

    @period.setter
    def period(self, t):
        self.setPeriod(t)

    @property
    @element
    def delay(self):
        """
        [Gotthard][Jungfrau][CTB][Moench][Mythen3][Gotthard2] Delay after trigger, accepts either a value in seconds or datetime.timedelta

        Note
        -----
        :getter: always returns in seconds. To get in datetime.delta, use getDelayAfterTrigger

        Examples
        -----------
        >>> d.delay = 1.05
        >>> d.delay = datetime.timedelta(minutes = 3, seconds = 1.23)
        >>> d.delay
        181.23
        >>> d.getDelayAfterTrigger()
        [datetime.timedelta(seconds=181, microseconds=230000)]
        """
        return ut.reduce_time(self.getDelayAfterTrigger())

    @delay.setter
    def delay(self, t):
        self.setDelayAfterTrigger(t)

    @property
    @element
    def delayl(self):
        """
        [Gotthard][Jungfrau][CTB][Moench][Mythen3][Gotthard2] Delay left after trigger during acquisition, accepts either a value in seconds or datetime.timedelta

        Note
        -----
        [Gotthard2] only in continuous mdoe.
        :getter: always returns in seconds. To get in datetime.delta, use getDelayAfterTriggerLeft

        Examples
        -----------
        >>> d.delay
        181.23
        >>> d.getDelayAfterTrigger()
        [datetime.timedelta(seconds=181, microseconds=230000)]
        """
        return ut.reduce_time(self.getDelayAfterTriggerLeft())

    def start(self):
        """Start detector"""
        self.startDetector()

    def rx_start(self):
        """Start receiver"""
        self.startReceiver()

    def rx_stop(self):
        """Stop receiver"""
        self.stopReceiver()

    def stop(self):
        """Stop detector"""
        self.stopDetector()

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

    # TODO! add txdelay

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

    # FILE

    @property
    @element
    def numinterfaces(self):
        """[Jungfrau][Gotthard2] Number of udp interfaces to stream data from detector. Default is 1.
        Note
        -----
        Also enables second interface in receiver for listening (Writes a file per interface if writing enabled). \n
        Also restarts client and receiver zmq sockets if zmq streaming enabled. \n
        [Gotthard2] second interface enabled to send veto information via 10Gbps for debugging. By default, if veto enabled, it is sent via 2.5 gbps interface.
        """
        return self.getNumberofUDPInterfaces()

    @numinterfaces.setter
    def numinterfaces(self, value):
        self.setNumberofUDPInterfaces(value)

    @property
    def fformat(self):
        """ File format of data file in receiver.
        
            Note
            -----
            For HDF5, package must be compiled with HDF5 flags. Default is binary. 

            Example
            --------
            d.fformat = slsdet.fileFormat.BINARY

            """
        return element_if_equal(self.getFileFormat())

    @fformat.setter
    def fformat(self, format):
        self.setFileFormat(format)

    @property
    def findex(self):
        """File or Acquisition index in receiver."""
        return element_if_equal(self.getAcquisitionIndex())

    @findex.setter
    def findex(self, index):
        self.setAcquisitionIndex(index)

    @property
    def fname(self):
        """File name prefix for output data file in receiver. Default is run. 
        
        Note
        -----
        File name: [file name prefix]_d[detector index]_f[sub file index]_[acquisition/file index].[raw/h5].

        Examples
        --------
        d.fname = 'run'
        eg. file name: run_d0_f0_5.raw
        """
        return element_if_equal(self.getFileNamePrefix())

    @fname.setter
    def fname(self, file_name):
        self.setFileNamePrefix(file_name)

    @property
    def fpath(self):
        """Directory where output data files are written in receiver.

        Note
        ----
        If path does not exist, it will try to create it.
        
        Examples
        --------
        d.fpath = '/tmp/run_20201705'
        """
        return element_if_equal(self.getFilePath())

    @fpath.setter
    def fpath(self, path):
        path = ut.make_string_path(path)
        self.setFilePath(path)

    @property
    def fwrite(self):
        """Enable or disable receiver file write. Default is enabled. """
        return element_if_equal(self.getFileWrite())

    @fwrite.setter
    def fwrite(self, value):
        self.setFileWrite(value)

    @property
    def foverwrite(self):
        """Enable or disable receiver file overwriting. Default is enabled. """
        return element_if_equal(self.getFileOverWrite())

    @foverwrite.setter
    def foverwrite(self, value):
        self.setFileOverWrite(value)

    @property
    def fmaster(self):
        """Enable or disable receiver master file. Default is enabled."""
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
        if ip == "auto":
            ip = socket.gethostbyname(self.rx_hostname)
        self.setDestinationUDPIP(IpAddr(ip))

    @property
    def udp_dstip2(self):
        return element_if_equal(self.getDestinationUDPIP2())

    @udp_dstip2.setter
    def udp_dstip2(self, ip):
        if ip == "auto":
            ip = socket.gethostbyname(self.rx_hostname)
        self.setDestinationUDPIP2(IpAddr(ip))

    @property
    def udp_dstmac(self):
        return element_if_equal(self.getDestinationUDPMAC())

    @udp_dstmac.setter
    def udp_dstmac(self, mac):
        self.setDestinationUDPMAC(MacAddr(mac))

    @property
    def udp_dstmac2(self):
        return element_if_equal(self.getDestinationUDPMAC2())

    @udp_dstmac2.setter
    def udp_dstmac2(self, mac):
        self.setDestinationUDPMAC2(MacAddr(mac))

    @property
    def udp_srcip(self):
        return element_if_equal(self.getSourceUDPIP())

    @udp_srcip.setter
    def udp_srcip(self, ip):
        self.setSourceUDPIP(IpAddr(ip))

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
        self.setSourceUDPMAC(MacAddr(mac))

    @property
    def src_udpip2(self):
        return element_if_equal(self.getSourceUDPIP())

    @src_udpip2.setter
    def src_udpip2(self, ip):
        self.setSourceUDPIP(IpAddr(ip))

    @property
    def src_udpip(self):
        return element_if_equal(self.getSourceUDPIP())

    @src_udpip.setter
    def src_udpip(self, ip):
        self.setSourceUDPIP(IpAddr(ip))

    @property
    def src_udpmac2(self):
        return element_if_equal(self.getSourceUDPMAC2())

    @src_udpmac2.setter
    def src_udpmac2(self, mac):
        self.setSourceUDPMAC2(MacAddr(mac))

    @property
    def highvoltage(self):
        """High voltage to the sensor in Voltage.

        Note
        -----
        [Gotthard] 0, 90, 110, 120, 150, 180, 200 \n
        [Eiger][Mythen3][Gotthard2] 0 - 200 \n
        [Jungfrau][Ctb][Moench] 0, 60 - 200
        """
        return element_if_equal(self.getHighVoltage())

    @highvoltage.setter
    def highvoltage(self, v):
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
        return NotImplementedError("trimbits are set only")

    @trimbits.setter
    def trimbits(self, fname):
        fname = ut.make_string_path(fname)
        self.loadTrimbits(fname)

    @property
    def lock(self):
        """Lock detector to one client IP, 1 locks, 0 unlocks. Default is unlocked."""
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
        """Get Client IP Address that last communicated with the detector."""
        return element_if_equal(self.getLastClientIP())

    @property
    def reg(self):
        return self._register

    @property
    def daclist(self):
        """Gets the list of enums for every dac for this detector."""
        return self.getDacList()

    @property
    def dacvalues(self):
        """Gets the dac values for every dac for this detector."""
        return {
            dac.name.lower(): np.array(self.getDAC(dac, False))
            for dac in self.getDacList()
        }

    @property
    def timinglist(self):
        return self.getTimingModeList()

    @property
    def templist(self):
        return self.getTemperatureList()

    @property
    def settingslist(self):
        return self.getSettingsList()

    @property
    def adcreg(self):
        """[Jungfrau][Ctb][Moench][Gotthard] Writes to an adc register 

        Note
        -----
        Advanced user Function!

        :getter: Not implemented     
        """
        return self._adc_register

    @property
    def led(self):
        """[Ctb] Switches on/off all LEDs. Default is enabled. """
        return element_if_equal(self.getLEDEnable())

    @led.setter
    def led(self, value):
        self.setLEDEnable(value)

    @property
    def ratecorr(self):
        """ 
        [Eiger] Custom dead time correction constant in ns. 0 will unset rate correction.

        Note
        -----
        To set default rate correction, use setDefaultRateCorrection

        Examples
        -----------
        >>> d.ratecorr = 1.05
        >>> d.period = datetime.timedelta(minutes = 3, seconds = 1.23)
        >>> d.period
        181.23
        >>> d.getPeriod()
        [datetime.timedelta(seconds=181, microseconds=230000)]
        """
        return reduce_time(self.getRateCorrection())

    @ratecorr.setter
    def ratecorr(self, tau):
        self.setRateCorrection(tau)

    @property
    def speed(self):
        return element_if_equal(self.getSpeed())

    @speed.setter
    def speed(self, value):
        self.setSpeed(value)

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
        return element_if_equal(self.getDAC(dacIndex.VTHRESHOLD, False))

    @vthreshold.setter
    def vthreshold(self, value):
        self.setDAC(dacIndex.VTHRESHOLD, value, False)

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
    def subexptime(self):
        res = self.getSubExptime()
        return reduce_time(res)

    @subexptime.setter
    def subexptime(self, t):
        self.setSubExptime(t)

    @property
    def subdeadtime(self):
        res = self.getSubDeadTime()
        reduce_time(res)

    @subdeadtime.setter
    def subdeadtime(self, t):
        self.setSubDeadTime(t)

    @property
    def partialreset(self):
        """[Eiger] Sets up detector to do partial or complete reset at start of acquisition. 0 complete reset, 1 partial reset. Default is complete reset.
        Note
        -----
        Advanced Function!
        """
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
        """[Eiger] Enable or disable show overflow flag in 32 bit mode. Default is disabled. """
        return element_if_equal(self.getOverFlowMode())

    @overflow.setter
    def overflow(self, value):
        self.setOverFlowMode(value)

    @property
    def flowcontrol10g(self):
        """[Eiger][Jungfrau] Enable or disable 10GbE Flow Control."""
        return element_if_equal(self.getTenGigaFlowControl())

    @flowcontrol10g.setter
    def flowcontrol10g(self, enable):
        self.setTenGigaFlowControl(enable)

    @property
    def interruptsubframe(self):
        """[Eiger] Enable last subframe interrupt at required exposure time. Disabling will wait for last sub frame to finish exposing. Default is disabled."""
        return element_if_equal(self.getInterruptSubframe())

    @interruptsubframe.setter
    def interruptsubframe(self, value):
        self.setInterruptSubframe(value)

    @property
    def gappixels(self):
        """[Eiger][Jungfrau] Include Gap pixels in client data call back in Detecor api. Will not be in detector streaming, receiver file or streaming. Default is disabled. """
        return element_if_equal(self.getRxAddGapPixels())

    @gappixels.setter
    def gappixels(self, value):
        self.setRxAddGapPixels(value)

    @property
    def measuredperiod(self):
        """
        [Eiger] Measured frame period between last frame and previous one. 
        
        Note
        -----
        Can be measured with minimum 2 frames in an acquisition. 
        :setter: Not implemented
        """
        res = self.getMeasuredPeriod()
        return element_if_equal([it.total_seconds() for it in res])

    @property
    def measuredsubperiod(self):
        """
        [Eiger] Measured sub frame period between last sub frame and previous one. 
        Note
        -----
        :setter: Not implemented
        """
        res = self.getMeasuredSubFramePeriod()
        return element_if_equal([it.total_seconds() for it in res])

    """
    Jungfrau specific
    """

    @property
    @element
    def auto_comp_disable(self):
        """[Jungfrau] Enable or disable auto comparator disable mode. 

        Note
        -----
        By default, the on-chip gain switching is active during the entire exposure. This mode disables the on-chip gain switching comparator automatically after 93.75% of exposure time (only for longer than 100us).\n
        Default is 0 or this mode disabled (comparator enabled throughout). 1 enables mode. 0 disables mode. 
        """
        return self.getAutoCompDisable()

    @auto_comp_disable.setter
    def auto_comp_disable(self, value):
        self.setAutoCompDisable(value)

    @property
    @element
    def storagecells(self):
        return self.getNumberOfAdditionalStorageCells()

    @storagecells.setter
    def storagecells(self, n_cells):
        self.setNumberOfAdditionalStorageCells(n_cells)

    @property
    @element
    def storagecell_start(self):
        return self.getStorageCellStart()

    @storagecell_start.setter
    def storagecell_start(self, value):
        self.setStorageCellStart(value)

    @property
    @element
    def storagecell_delay(self):
        return ut.reduce_time(self.getStorageCellDelay())

    @storagecell_delay.setter
    def storagecell_delay(self, t):
        self.setStorageCellDelay(t)

    @property
    @element
    def temp_threshold(self):
        return self.getThresholdTemperature()

    @temp_threshold.setter
    def temp_threshold(self, value):
        self.setThresholdTemperature(value)

    @property
    @element
    def temp_event(self):
        return self.getTemperatureEvent()

    @temp_event.setter
    def temp_event(self, value):
        if value != 0:
            raise ValueError("Value needs to be 0 for reset. Setting not allowed")
        self.resetTemperatureEvent()

    @property
    @element
    def temp_control(self):
        return self.getTemperatureControl()

    @temp_control.setter
    def temp_control(self, value):
        self.setTemperatureControl(value)

    @property
    @element
    def selinterface(self):
        return self.getSelectedUDPInterface()

    @selinterface.setter
    def selinterface(self, i):
        self.selectUDPInterface(i)

    """
    Gotthard2
    """

    @property
    @element
    def veto(self):
        """
        [Gotthard2] Enable or disable veto data streaming from detector. Default is 0.
        """
        return self.getVeto()

    @veto.setter
    def veto(self, value):
        self.setVeto(value)

    """
    Mythen3 specific
    """

    @property
    def gatedelay(self):
        """
        [Mythen3] Gate Delay of all gate signals in auto and trigger mode (internal gating), accepts either a value in seconds or datetime.timedelta

        Note
        -----
        To specify gateIndex, use getGateDelay or setGateDelay.
        
        :getter: always returns in seconds. To get in datetime.delta, use getGateDelayForAllGates or getGateDelay(gateIndex)

        Examples
        -----------
        >>> d.gatedelay = 1.05
        >>> d.gatedelay = datetime.timedelta(minutes = 3, seconds = 1.23)
        >>> d.gatedelay
        181.23
        >>> d.setGateDelay(1, datetime.timedelta(seconds = 2))
        >>> d.gatedelay
        >>> [1.0, 2.0, 1.0]
        >>> d.getExptimeForAllGates()
        >>> [[datetime.timedelta(seconds=181, microseconds=230000), datetime.timedelta(seconds=181, microseconds=230000), datetime.timedelta(seconds=181, microseconds=230000)]]
        """
        return reduce_time(self.getGateDelayForAllGates())

    @gatedelay.setter
    def gatedelay(self, value):
        if is_iterable(value):
            for i, v in enumerate(value):
                if isinstance(v, int):
                    v = float(v)
                self.setGateDelay(i, v)
        else:
            if isinstance(value, int):
                value = float(value)
            self.setGateDelay(-1, value)

    @property
    def counters(self):
        """
        [Mythen3] List of counter indices enabled. 
        
        Note
        -----
        Each element in list can be 0 - 2 and must be non repetitive.

        Examples
        -----------
        >>> d.counters = [0, 1]

        """
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
    def runclk(self):
        return element_if_equal(self.getRUNClock())

    @runclk.setter
    def runclk(self, freq):
        self.setRUNClock(freq)

    @property
    def romode(self):
        return element_if_equal(self.getReadoutMode())

    @romode.setter
    def romode(self, mode):
        self.setReadoutMode(mode)

    @property
    def asamples(self):
        """[Ctb][Moench] Number of analog samples expected. """
        return element_if_equal(self.getNumberOfAnalogSamples())

    @asamples.setter
    def asamples(self, N):
        self.setNumberOfAnalogSamples(N)

    @property
    def dsamples(self):
        """[CTB] Number of digital samples expected. """
        return element_if_equal(self.getNumberOfDigitalSamples())

    @dsamples.setter
    def dsamples(self, N):
        self.setNumberOfDigitalSamples(N)

    @property
    def dbitphase(self):
        """[Ctb][Jungfrau] Phase shift of clock to latch digital bits. Absolute phase shift.

        Note
        -----
        [Ctb]Changing dbitclk also resets dbitphase and sets to previous values.
        """
        return element_if_equal(self.getDBITPhase())

    @dbitphase.setter
    def dbitphase(self, value):
        self.setDBITPhase(value)

    @property
    def dbitclk(self):
        """[Ctb] Clock for latching the digital bits in MHz."""
        return element_if_equal(self.getDBITClock())

    @dbitclk.setter
    def dbitclk(self, value):
        self.setDBITClock(value)

    @property
    def dbitpipeline(self):
        """[Ctb] Pipeline of the clock for latching digital bits. """
        return element_if_equal(self.getDBITPipeline())

    @dbitpipeline.setter
    def dbitpipeline(self, value):
        self.setDBITPipeline(value)

    @property
    def maxdbitphaseshift(self):
        """[CTB][Jungfrau] Absolute maximum Phase shift of of the clock to latch digital bits.
        Note
        -----
        :setter: Not Implemented
        """
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
        """[Jungfrau][CTB][Moench] Absolute maximum Phase shift of ADC clock.
        Note
        -----
        :setter: Not Implemented
        """
        return element_if_equal(self.getMaxADCPhaseShift())

    @property
    def adcphase(self):
        """[Gotthard][Jungfrau][CTB][Moench] Sets phase shift of ADC clock. 

        Note
        -----
        [Jungfrau] Absolute phase shift. Changing Speed also resets adcphase to recommended defaults.\n
        [Ctb][Moench] Absolute phase shift. Changing adcclk also resets adcphase and sets it to previous values.\n
        [Gotthard] Relative phase shift.

        :getter: Not implemented for Gotthard
        """
        return element_if_equal(self.getADCPhase())

    @adcphase.setter
    def adcphase(self, value):
        self.setADCPhase(value)

    @property
    def adcpipeline(self):
        """[Ctb][Moench] Sets pipeline for ADC clock. """
        return element_if_equal(self.getADCPipeline())

    @adcpipeline.setter
    def adcpipeline(self, value):
        self.setADCPipeline(value)

    @property
    def adcclk(self):
        """[Ctb][Moench] Sets ADC clock frequency in MHz. """
        return element_if_equal(self.getADCClock())

    @adcclk.setter
    def adcclk(self, value):
        self.setADCClock(value)

    @property
    def syncclk(self):
        return element_if_equal(self.getSYNCClock())

    @property
    def pattern(self):
        """[Mythen3][Moench][Ctb] Loads ASCII pattern file directly to server (instead of executing line by line).

        Examples
        ---------
        >>> d.pattern = '/tmp/pat.txt'
        """
        # TODO! Clean fix
        print("Set only")
        return 0

    @pattern.setter
    def pattern(self, fname):
        fname = ut.make_string_path(fname)
        self.setPattern(fname)

    # patioctrl
    @property
    def patioctrl(self):
        """[Ctb][Moench] 64 bit mask defining input (0) and output (1) signals.
        
        Examples
        --------
        >>> d.patioctrl = 0x8f0effff6dbffdbf
        >>> hex(d.patioctrl)
        '0x8f0effff6dbffdbf'
        """
        return element_if_equal(self.getPatternIOControl())

    @patioctrl.setter
    def patioctrl(self, mask):
        self.setPatternIOControl(mask)

    @property
    def patlimits(self):
        """[Ctb][Moench][Mythen3] Limits (start and stop address) of complete pattern.
        
        Examples
        ---------
        >>> d.patlimits = [0x0, 0x18c]
        >>> d.patlimits
        [0, 396]
        >>> [hex(l) for l in d.patlimits]
        ['0x0', '0x18c']
        """
        return element_if_equal(self.getPatternLoopAddresses(-1))

    @patlimits.setter
    def patlimits(self, lim):
        self.setPatternLoopAddresses(-1, lim[0], lim[1])

    @property
    @element
    def patsetbit(self):
        """[Ctb][Moench][Mythen3] Selects the bits that will have a pattern mask applied to the selected patmask for every pattern.
        
        Examples
        --------
        >>> d.patsetbit = 0x8f0effff6dbffdbf
        >>> hex(d.patsetbit)
        '0x8f0effff6dbffdbf' 
        """
        return self.getPatternBitMask()

    @patsetbit.setter
    def patsetbit(self, mask):
        self.setPatternBitMask(mask)

    @property
    def patmask(self):
        """[Ctb][Moench][Mythen3] Sets the mask applied to every pattern to the selected bits. 
        
        Examples
        --------
        >>> d.patmask = 0x8f0effff6dbffdbf
        >>> hex(d.patmask)
        '0x8f0effff6dbffdbf' 
        """
        return element_if_equal(self.getPatternMask())

    @patmask.setter
    def patmask(self, mask):
        self.setPatternMask(mask)



    @property
    def patwait0(self):
        """[Ctb][Moench][Mythen3] Wait 0 address.
                
        Examples
        --------
        >>> d.patwait0 = 0xaa
        >>> d.patwait0
        170
        >>> hex(d.patwait0)
        '0xaa'
        """
        return element_if_equal(self.getPatternWaitAddr(0))

    @patwait0.setter
    def patwait0(self, addr):
        self.setPatternWaitAddr(0, addr)

    @property
    def patwait1(self):
        """[Ctb][Moench][Mythen3] Wait 1 address.
                
        Examples
        --------
        >>> d.patwait1 = 0xaa
        >>> d.patwait1
        170
        >>> hex(d.patwait1)
        '0xaa'
        """
        return element_if_equal(self.getPatternWaitAddr(1))

    @patwait1.setter
    def patwait1(self, addr):
        self.setPatternWaitAddr(1, addr)

    @property
    def patwait2(self):
        """[Ctb][Moench][Mythen3] Wait 2 address.
                
        Examples
        --------
        >>> d.patwait2 = 0xaa
        >>> d.patwait2
        170
        >>> hex(d.patwait2)
        '0xaa'
        """
        return element_if_equal(self.getPatternWaitAddr(2))

    @patwait2.setter
    def patwait2(self, addr):
        self.setPatternWaitAddr(2, addr)

    @property
    def patwaittime0(self):
        """[Ctb][Moench][Mythen3] Wait 0 time in clock cycles."""
        return element_if_equal(self.getPatternWaitTime(0))

    @patwaittime0.setter
    def patwaittime0(self, nclk):
        self.setPatternWaitTime(0, nclk)

    @property
    def patwaittime1(self):
        """[Ctb][Moench][Mythen3] Wait 1 time in clock cycles."""
        return element_if_equal(self.getPatternWaitTime(1))

    @patwaittime1.setter
    def patwaittime1(self, nclk):
        self.setPatternWaitTime(1, nclk)

    @property
    def patwaittime2(self):
        """[Ctb][Moench][Mythen3] Wait 2 time in clock cycles."""
        return element_if_equal(self.getPatternWaitTime(2))

    @patwaittime2.setter
    def patwaittime2(self, nclk):
        self.setPatternWaitTime(2, nclk)

    @property
    def patloop0(self):
        """[Ctb][Moench][Mythen3] Limits (start and stop address) of loop 0.
        
        Examples
        ---------
        >>> d.patloop0 = [0x0, 0x18c]
        >>> d.patloop0
        [0, 396]
        >>> [hex(l) for l in d.patloop0]
        ['0x0', '0x18c']
        """
        return element_if_equal(self.getPatternLoopAddresses(0))

    @patloop0.setter
    def patloop0(self, addr):
        self.setPatternLoopAddresses(0, addr[0], addr[1])

    @property
    def patloop1(self):
        """[Ctb][Moench][Mythen3] Limits (start and stop address) of loop 1.
        
        Examples
        ---------
        >>> d.patloop1 = [0x0, 0x18c]
        >>> d.patloop1
        [0, 396]
        >>> [hex(l) for l in d.patloop1]
        ['0x0', '0x18c']
        
        """
        return element_if_equal(self.getPatternLoopAddresses(1))

    @patloop1.setter
    def patloop1(self, addr):
        self.setPatternLoopAddresses(1, addr[0], addr[1])

    @property
    def patloop2(self):
        """[Ctb][Moench][Mythen3] Limits (start and stop address) of loop 2.
        
        Examples
        ---------
        >>> d.patloop2 = [0x0, 0x18c]
        >>> d.patloop2
        [0, 396]
        >>> [hex(l) for l in d.patloop2]
        ['0x0', '0x18c']
        
        """
        return element_if_equal(self.getPatternLoopAddresses(2))

    @patloop2.setter
    def patloop2(self, addr):
        self.setPatternLoopAddresses(2, addr[0], addr[1])

    @property
    def patnloop0(self):
        """[Ctb][Moench][Mythen3] Number of cycles of loop 0."""
        return element_if_equal(self.getPatternLoopCycles(0))

    @patnloop0.setter
    def patnloop0(self, n):
        self.setPatternLoopCycles(0, n)

    @property
    def patnloop1(self):
        """[Ctb][Moench][Mythen3] Number of cycles of loop 1."""
        return element_if_equal(self.getPatternLoopCycles(1))

    @patnloop1.setter
    def patnloop1(self, n):
        self.setPatternLoopCycles(1, n)

    @property
    def patnloop2(self):
        """[Ctb][Moench][Mythen3] Number of cycles of loop 2."""
        return element_if_equal(self.getPatternLoopCycles(2))

    @patnloop2.setter
    def patnloop2(self, n):
        self.setPatternLoopCycles(2, n)

    @property
    @element
    def v_a(self):
        return self.getDAC(dacIndex.V_POWER_A, True)

    @v_a.setter
    def v_a(self, value):
        self.setDAC(dacIndex.V_POWER_A, value, True)

    @property
    @element
    def v_b(self):
        return self.getDAC(dacIndex.V_POWER_B, True)

    @v_b.setter
    def v_b(self, value):
        self.setDAC(dacIndex.V_POWER_B, value, True)

    @property
    @element
    def v_c(self):
        return self.getDAC(dacIndex.V_POWER_C, True)

    @v_c.setter
    def v_c(self, value):
        self.setDAC(dacIndex.V_POWER_C, value, True)

    @property
    @element
    def v_d(self):
        return self.getDAC(dacIndex.V_POWER_D, True)

    @v_d.setter
    def v_d(self, value):
        self.setDAC(dacIndex.V_POWER_D, value, True)

    @property
    @element
    def v_io(self):
        return self.getDAC(dacIndex.V_POWER_IO, True)

    @v_io.setter
    def v_io(self, value):
        self.setDAC(dacIndex.V_POWER_IO, value, True)

    @property
    @element
    def v_limit(self):
        return self.getDAC(dacIndex.V_LIMIT, True)

    @v_limit.setter
    def v_limit(self, value):
        self.setDAC(dacIndex.V_LIMIT, value, True)

    @property
    @element
    def im_a(self):
        """[Ctb] Measured current of power supply a in mA.
        
        Notes
        -----
        :setter: Not implemented
        """
        return self.getMeasuredCurrent(dacIndex.I_POWER_A)

    @property
    @element
    def im_b(self):
        """[Ctb] Measured current of power supply b in mA.
        
        Notes
        -----
        :setter: Not implemented
        """
        return self.getMeasuredCurrent(dacIndex.I_POWER_B)

    @property
    @element
    def im_c(self):
        """[Ctb] Measured current of power supply c in mA.
                
        Notes
        -----
        :setter: Not implemented
        """
        return self.getMeasuredCurrent(dacIndex.I_POWER_C)

    @property
    @element
    def im_d(self):
        """[Ctb] Measured current of power supply d in mA.
                
        Notes
        -----
        :setter: Not implemented
        """
        return self.getMeasuredCurrent(dacIndex.I_POWER_D)

    @property
    @element
    def im_io(self):
        """[Ctb] Measured current of power supply io in mA.
                
        Notes
        -----
        :setter: Not implemented
        """
        return self.getMeasuredCurrent(dacIndex.I_POWER_IO)
