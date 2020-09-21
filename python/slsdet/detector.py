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
from _slsdet import xy
from . import utils as ut
from .proxy import JsonProxy, SlowAdcProxy, ClkDivProxy, MaxPhaseProxy, ClkFreqProxy
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

    @property
    def nmod(self):
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

        Example
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
    def parameters(self, value):
        if isinstance(value, str):
            value = ut.make_string_path(value)
        self.loadParameters(value)

    @property
    def hostname(self):
        """Frees shared memory and sets hostname (or IP address) of all modules concatenated by + 
        Virtual servers can already use the port in hostname separated by ':' and ports incremented by 2 to accomodate the stop server as well.
        Example
        -------
        >>> d.hostname = 'beb031+beb032+'
        >>> d.hostname = 'localhost:1912+localhost:1914+'
        >>> d.hostname
        ['localhost']
        """
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
    @element
    def port(self):
        return self.getControlPort()

    @port.setter
    def port(self, value):
        ut.set_using_dict(self.setControlPort, value)

    @property
    @element
    def stopport(self):
        return self.getStopPort()

    @stopport.setter
    def stopport(self, args):
        ut.set_using_dict(self.setStopPort, args)


    @property
    def firmwareversion(self):
        """
        Fimware version of detector in format [0xYYMMDD] or an increasing 2 digit number for Eiger.
        Example
        -------
        >>> hex(d.firmwareversion)
        '0x200910'
        """
        return element_if_equal(self.getFirmwareVersion())

    @property
    def detectorserverversion(self):
        """
        On-board detector server software version in format [0xYYMMDD]
        Example
        -------
        >>> hex(d.detectorserverversion)
        '0x200910'
        """
        # TODO! handle hex print
        return element_if_equal(self.getDetectorServerVersion())

    @property
    def clientversion(self):
        """Client software version in format [YYMMDD]
        Example
        -------
        >>> hex(d.clientversion)
        '0x200810'
        """
        return self.getClientVersion()

    @property
    def rx_version(self):
        """Receiver version in format [0xYYMMDD]."""
        return element_if_equal(self.getReceiverVersion())

    @property
    @element
    def rx_threads(self):
        return self.getRxThreadIds()

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
    def drlist(self):
        """List of possible dynamic ranges for this detector"""
        return self.getDynamicRangeList()

    @property
    def module_geometry(self):
        return to_geo(self.getModuleGeometry())

    @property
    def module_size(self):
        ms = [to_geo(item) for item in self.getModuleSize()]
        return element_if_equal(ms)

    @property
    def detsize(self):
        """
        Sets the detector size in both dimensions (number of channels). 
        Note
        -----
        This value is used to calculate row and column positions for each module and included into udp data packet header. \n 
        By default, it adds modules in y dimension for 2d detectors and in x dimension for 1d detectors.
        Example
        -------
        >>> d.detsize
        Geometry(x=3840, y=1)
        >>> d.detsize = [1024, 512]
        Geometry(x=1024, y = 512)
        """
        return to_geo(self.getDetectorSize())

    @detsize.setter
    def detsize(self, size):
        if isinstance(size, xy):
            self.setDetectorSize(size)
        else:
            self.setDetectorSize(xy(*size))

    @property
    def settings(self):
        """
        Detector settings. Enum: detectorSettings
        Note
        -----
        
        [Eiger] Use threshold command to load settings
        [Jungfrau] DYNAMICGAIN, DYNAMICHG0, FIXGAIN1, FIXGAIN2, FORCESWITCHG1, FORCESWITCHG2 \n
        [Gotthard] DYNAMICGAIN, HIGHGAIN, LOWGAIN, MEDIUMGAIN, VERYHIGHGAIN \n
        [Gotthard2] DYNAMICGAIN, FIXGAIN1, FIXGAIN2 \n
        [Moench] G1_HIGHGAIN, G1_LOWGAIN, G2_HIGHCAP_HIGHGAIN, G2_HIGHCAP_LOWGAIN, G2_LOWCAP_HIGHGAIN, G2_LOWCAP_LOWGAIN, G4_HIGHGAIN, G4_LOWGAIN \n
        [Eiger] settings loaded from file found in settingspath
        """
        return element_if_equal(self.getSettings())

    @settings.setter
    def settings(self, value):
        self.setSettings(value)

    @property
    def frames(self):
        """Number of frames per acquisition. In trigger mode, number of frames per trigger.

        Note
        -----
        Cannot be set in modular level. \n
        In scan mode, number of frames is set to number of steps. \n
        [Gotthard2] Burst mode has a maximum of 2720 frames.
        """
        return element_if_equal(self.getNumberOfFrames())

    @frames.setter
    def frames(self, n_frames):
        self.setNumberOfFrames(n_frames)

    @property
    @element
    def framesl(self):
        """
        [Gotthard][Jungfrau][Mythen3][Gotthard2][CTB][Moench] Number of frames left in acquisition.\n
        [Gotthard2] only in continuous mode.
        """
        return self.getNumberOfFramesLeft()

    @property
    @element
    def nframes(self):
        return self.getNumberOfFramesFromStart()

    @property
    @element
    def powerchip(self):
        return self.getPowerChip()

    @powerchip.setter
    def powerchip(self, value):
        ut.set_using_dict(self.setPowerChip, value)

    @property
    def triggers(self):
        """Number of triggers per aquire. Set timing mode to use triggers."""
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

        Example
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
            ut.set_time_using_dict(self.setExptime, t)




    @property
    def period(self):
        """
        Period between frames, accepts either a value in seconds or datetime.timedelta

        Note
        -----
        :getter: always returns in seconds. To get in datetime.delta, use getPeriod

        Example
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
        ut.set_time_using_dict(self.setPeriod, t)


    @property
    @element
    def periodl(self):
        return self.getPeriodLeft()

    @property
    @element
    def delay(self):
        """
        [Gotthard][Jungfrau][CTB][Moench][Mythen3][Gotthard2] Delay after trigger, accepts either a value in seconds or datetime.timedelta

        Note
        -----
        :getter: always returns in seconds. To get in datetime.delta, use getDelayAfterTrigger

        Example
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

        Example
        -----------
        >>> d.delayl
        181.23
        >>> d.getDelayAfterTriggerLeft()
        [datetime.timedelta(seconds=181, microseconds=230000)]
        """
        return ut.reduce_time(self.getDelayAfterTriggerLeft())

    def start(self):
        """Start detector acquisition. Status changes to RUNNING or WAITING and automatically returns to idle at the end of acquisition."""
        self.startDetector()

    def rx_start(self):
        """Starts receiver listener for detector data packets and create a data file (if file write enabled)."""
        self.startReceiver()

    def rx_stop(self):
        """Stops receiver listener for detector data packets and closes current data file (if file write enabled)."""
        self.stopReceiver()

    def stop(self):
        """Abort detector acquisition. Status changes to IDLE or STOPPED"""
        self.stopDetector()

    # Time
    @property
    def rx_framescaught(self):
        """Number of frames caught by receiver."""
        return element_if_equal(self.getFramesCaught())

    @property
    def startingfnum(self):
        """[Eiger][Jungfrau] Starting frame number for next acquisition. Stopping acquiistion might result in different frame numbers for different modules. """
        return element_if_equal(self.getStartingFrameNumber())

    @startingfnum.setter
    def startingfnum(self, value):
        self.setStartingFrameNumber(value)

    # TODO! add txdelay

    @property
    @element
    def txndelay_frame(self):
        return self.getTransmissionDelayFrame()

    @txndelay_frame.setter
    def txndelay_frame(self, args):
        ut.set_using_dict(self.setTransmissionDelayFrame, args)

    @property
    @element
    def txndelay_left(self):
        return self.getTransmissionDelayLeft()

    @txndelay_left.setter
    def txndelay_left(self, args):
        ut.set_using_dict(self.setTransmissionDelayLeft, args)

    @property
    @element
    def txndelay_right(self):
        return self.getTransmissionDelayRight()

    @txndelay_right.setter
    def txndelay_right(self, args):
        ut.set_using_dict(self.setTransmissionDelayRight, args)

    @property
    def use_receiver(self):
        return element_if_equal(self.getUseReceiverFlag())

    @property
    def rx_hostname(self):
        """ Sets receiver hostname or IP address. Used for TCP control communication between client and receiver to configure receiver. Also updates receiver with detector parameters.
        Note
        -----
        Also resets any prior receiver property (not on detector). \n
        Can concatenate receiver hostnames for every module. \n
        If port included, then its the receiver tcp port for every receiver hostname.
        Example
        --------
        >>> d.rx_hostname
        'mpc1922'
        >>> d.rx_hostname = 'mpc1922'
        >>> d.rx_hostname = 'mpc1922:2000'
        >>> d.rx_hostname = 'mpc1922:2000+mpc1922:2002'
        >>> d.rx_hostname
        'mpc1922'
        >>> d.rx_tcpport
        [2000, 2002]
        """
        return element_if_equal(self.getRxHostname())

    @rx_hostname.setter
    def rx_hostname(self, hostname):
        self.setRxHostname(hostname)

    @property
    def rx_tcpport(self):
        """
        TCP port for client-receiver communication. 
        Note
        -----
        Default is 1954. \n
        Must be different if multiple receivers on same pc. \n
        Must be first command to set a receiver parameter to be able to communicate. \n
        Multi command will automatically increment port for individual modules, which must be set via setRxPort.
        Example
        -------
        >>> d.rx_tcpport
        2010
        >>> d.rx_tcpport
        [2000, 2002]
        """
        return element_if_equal(self.getRxPort())

    @rx_tcpport.setter
    def rx_tcpport(self, port):
        self.setRxPort(port)

    @property
    def rx_fifodepth(self):
        """Sets the number of frames in the receiver fifo depth (buffer between listener and writer threads)."""
        return element_if_equal(self.getRxFifoDepth())

    @rx_fifodepth.setter
    def rx_fifodepth(self, frames):
        self.setRxFifoDepth(frames)

    @property
    def rx_silent(self):
        """When enabled, switches off receiver text output during acquisition. """
        return element_if_equal(self.getRxSilentMode())

    @rx_silent.setter
    def rx_silent(self, value):
        self.setRxSilentMode(value)

    @property
    def rx_discardpolicy(self):
        """
        Frame discard policy of receiver. Enum: frameDiscardPolicy
        Note
        -----
        Options: NO_DISCARD, DISCARD_EMPTY_FRAMES, DISCARD_PARTIAL_FRAMES \n
        Default: NO_DISCARD \n
        DISCARD_PARTIAL_FRAMES is the fastest.

        Example
        --------
        >>> d.rx_discardpolicy = frameDiscardPolicy.NO_DISCARD
        >>> d.rx_discardpolicy
        frameDiscardPolicy.NO_DISCARD
        """
        return element_if_equal(self.getRxFrameDiscardPolicy())

    @rx_discardpolicy.setter
    def rx_discardpolicy(self, policy):
        self.setRxFrameDiscardPolicy()

    @property
    def rx_padding(self):
        """Partial frames padding enable in the receiver. 
        Note
        ------
        Default: enabled \n
        Disabling is fastest.
        """
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
        """Client IP Address that last communicated with the receiver."""
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
        """ File format of data file in receiver. Enum: fileFormat
        
            Note
            -----
            Options: BINARY, HDF5
            Default: BINARY
            For HDF5, package must be compiled with HDF5 flags. Default is binary. 

            Example
            --------
            d.fformat = fileFormat.BINARY

            """
        return element_if_equal(self.getFileFormat())

    @fformat.setter
    def fformat(self, format):
        self.setFileFormat(format)

    @property
    @element
    def findex(self):
        """File or Acquisition index in receiver.
        Note
        ----
        File name: [file name prefix]_d[detector index]_f[sub file index]_[acquisition/file index].[raw/h5].
        """
        return self.getAcquisitionIndex()

    @findex.setter
    def findex(self, index):
        ut.set_using_dict(self.setAcquisitionIndex, index)

    @property
    def fname(self):
        """File name prefix for output data file in receiver. Default is run. 
        
        Note
        -----
        File name: [file name prefix]_d[detector index]_f[sub file index]_[acquisition/file index].[raw/h5].

        Example
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
        """Directory where output data files are written in receiver. Default is "/".
        Note
        ----
        If path does not exist, it will try to create it.
        
        Example
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
        """Sets the number of frames per file in receiver. 
        
        Note
        -----
        Default: depends on detector type. \n
        0 is infinite or all frames in single file.
        """
        return element_if_equal(self.getFramesPerFile())

    @rx_framesperfile.setter
    def rx_framesperfile(self, n_frames):
        self.setFramesPerFile(n_frames)

    # ZMQ Streaming Parameters (Receiver<->Client)

    @property
    @element
    def rx_zmqstream(self):
        """
        Enable/ disable data streaming from receiver via zmq (eg. to GUI or to another process for further processing). \n
        This creates/ destroys zmq streamer threads in receiver. \n
        Switching to Gui automatically enables data streaming in receiver. \n
        Switching back to command line acquire will require disabling data streaming in receiver for fast applications.
        """
        return self.getRxZmqDataStream()

    @rx_zmqstream.setter
    def rx_zmqstream(self, enable):
        ut.set_using_dict(self.setRxZmqDataStream, enable)

    @property
    def rx_zmqfreq(self):
        """Frequency of frames streamed out from receiver via zmq.
        Note
        -----
        Default: 1, Means every frame is streamed out. \n
        If 2, every second frame is streamed out. \n
        If 0, streaming timer is the timeout, after which current frame is sent out. (default timeout is 200 ms). Usually used for gui purposes.
        """
        return element_if_equal(self.getRxZmqFrequency())

    @rx_zmqfreq.setter
    def rx_zmqfreq(self, nth_frame):
        self.setRxZmqFrequency(nth_frame)

    @property
    def rx_zmqport(self):
        """
        Zmq port for data to be streamed out of the receiver. 
        Note
        -----
        Also restarts receiver zmq streaming if enabled. \n
        Default is 30001. \n
        Must be different for every detector (and udp port). \n
        Multi command will automatically increment for individual modules, use setRxZmqPort.

        Example
        --------
        >>> d.rx_zmqport
        [30001, 30002, 30003, 300004]
        >>> d.rx_zmqport = 30001
        >>> d.rx_zmqport = [30001, 30005] #Set ports for the two first detectors

        """
        return element_if_equal(self.getRxZmqPort())

    @rx_zmqport.setter
    def rx_zmqport(self, port):
        if isinstance(port, int):
            self.setRxZmqPort(port, -1)
        elif is_iterable(port):
            for i, p in enumerate(port):
                self.setRxZmqPort(p, i)
        else:
            raise ValueError("Unknown argument type")

    @property
    def zmqport(self):
        """
        Port number to listen to zmq data streamed out from receiver or intermediate process.
        Note
        -----
        Also restarts client zmq streaming if enabled. \n
        Default connects to receiver zmq streaming out port (30001). \n
        Must be different for every detector (and udp port). \n
        Multi command will automatically increment for individual modules, use setClientZmqPort. \n

        Example
        --------
        >>> d.zmqport
        [30001, 30003]
        >>> d.zmqport = 30002
        >>> d.zmqport = [30002, 30004] #Set ports for the two first detectors
        """
        return element_if_equal(self.getClientZmqPort())

    @zmqport.setter
    def zmqport(self, port):
        if isinstance(port, int):
            self.setClientZmqPort(port, -1)
        elif is_iterable(port):
            for i, p in enumerate(port):
                self.setClientZmqPort(p, i)
        else:
            raise ValueError("Unknown argument type")

    @property
    def rx_zmqip(self):
        """
        Zmq Ip Address from which data is to be streamed out of the receiver. 
        Note
        -----
        Also restarts receiver zmq streaming if enabled. \n
        Default is from rx_hostname. \n
        Modified only when using an intermediate process after receiver.

        Example
        -------
        >>> d.rx_zmqip
        192.168.0.101
        >>> d.rx_zmqip = '192.168.0.101'
        """
        return element_if_equal(self.getRxZmqIP())

    @rx_zmqip.setter
    def rx_zmqip(self, ip):
        self.setRxZmqIP(IpAddr(ip))

    @property
    def zmqip(self):
        """
        Ip Address to listen to zmq data streamed out from receiver or intermediate process.
        Note
        -----
        Also restarts client zmq streaming if enabled. \n
        Default is from rx_hostname. \n
        Modified only when using an intermediate process after receiver.

        Example
        -------
        >>> d.zmqip
        192.168.0.101
        >>> d.zmqip = '192.168.0.101'
        """
        return element_if_equal(self.getClientZmqIp())

    @zmqip.setter
    def zmqip(self, ip):
        self.setClientZmqIp(IpAddr(ip))

    @property
    def udp_dstip(self):
        """
        Ip address of the receiver (destination) udp interface. 
        Note
        ----
        If 'auto' used, then ip is set to ip of rx_hostname. \n
        To set IPs for individual modules, use setDestinationUDPIP. 
        Example
        ------
        >>> d.udp_dstip = '192.168.1.110'
        >>> d.udp_dstip
        192.168.1.110
        """
        return element_if_equal(self.getDestinationUDPIP())

    @udp_dstip.setter
    def udp_dstip(self, ip):
        if ip == "auto":
            ip = socket.gethostbyname(self.rx_hostname)
        self.setDestinationUDPIP(IpAddr(ip))

    @property
    def udp_dstip2(self):
        """
        [Jungfrau][Gotthard2] Ip address of the receiver (destination) udp interface 2.
        Note
        ----
        [Jungfrau] bottom half \n
        [Gotthard2] veto debugging \n
        If 'auto' used, then ip is set to ip of rx_hostname. \n
        To set IPs for individual modules, use setDestinationUDPIP2. 
        Example
        ------
        >>> d.udp_dstip2 = '10.1.1.185'
        >>> d.udp_dstip2
        10.1.1.185
        """
        return element_if_equal(self.getDestinationUDPIP2())

    @udp_dstip2.setter
    def udp_dstip2(self, ip):
        if ip == "auto":
            ip = socket.gethostbyname(self.rx_hostname)
        self.setDestinationUDPIP2(IpAddr(ip))

    @property
    def udp_dstmac(self):
        """
        Mac address of the receiver (destination) udp interface. 
        Note
        ----
        Not mandatory to set as udp_dstip retrieves it from slsReceiver process but must be set if you use a custom receiver (not slsReceiver). \n
        To set MACs for individual modules, use setDestinationUDPMAC. 
        Example
        -------
        >>> d.udp_dstmac = '00:1b:31:01:8a:de'
        d.udp_dstmac
        00:1b:31:01:8a:de
        """
        return element_if_equal(self.getDestinationUDPMAC())

    @udp_dstmac.setter
    def udp_dstmac(self, mac):
        self.setDestinationUDPMAC(MacAddr(mac))

    @property
    def udp_dstmac2(self):
        """
        [Jungfrau][Gotthard2] Mac address of the receiver (destination) udp interface 2.
        Note
        ----
        Not mandatory to set as udp_dstip2 retrieves it from slsReceiver process but must be set if you use a custom receiver (not slsReceiver).  \n
        To set MACs for individual modules, use setDestinationUDPMAC2. \n
        [Jungfrau] bottom half \n
        [Gotthard2] veto debugging \n
        Example
        ------
        >>> d.udp_dstmac2 = '00:1b:31:01:8a:de'
        d.udp_dstmac2
        00:1b:31:01:8a:de
        """
        return element_if_equal(self.getDestinationUDPMAC2())

    @udp_dstmac2.setter
    def udp_dstmac2(self, mac):
        self.setDestinationUDPMAC2(MacAddr(mac))

    @property
    def udp_srcmac(self):
        """
        Mac address of the receiver (source) udp interface. 
        Note
        ----
        [Eiger] Do not set as detector will replace with its own DHCP Mac (1G) or DHCP Mac + 1 (10G). \n
        To set MACs for individual modules, use setSourceUDPMAC. 
        Example
        -------
        >>> d.udp_srcmac = '00:1b:31:01:8a:de'
        d.udp_srcmac
        00:1b:31:01:8a:de
        """
        return element_if_equal(self.getSourceUDPMAC())

    @udp_srcmac.setter
    def udp_srcmac(self, mac):
        if isinstance(mac, (list, tuple)):
            for i, m in enumerate(mac):
                self.setSourceUDPMAC(MacAddr(m), [i])
        else:
            self.setSourceUDPMAC(MacAddr(mac))

    @property
    def udp_srcmac2(self):
        """
        [Jungfrau][Gotthard2] Mac address of the receiver (source) udp interface 2. 
        Note
        ----
        [Jungfrau] bottom half \n
        [Gotthard2] veto debugging \n
        To set MACs for individual modules, use setSourceUDPMAC2. 
        Example
        -------
        >>> d.udp_srcmac2 = '00:1b:31:01:8a:de'
        d.udp_srcmac2
        00:1b:31:01:8a:de
        """
        return element_if_equal(self.getSourceUDPMAC2())

    @udp_srcmac2.setter
    def udp_srcmac2(self, mac):
        if isinstance(mac, (list, tuple)):
            for i, m in enumerate(mac):
                self.setSourceUDPMAC2(MacAddr(m), [i])
        else:
            self.setSourceUDPMAC2(MacAddr(mac))

    @property
    def udp_srcip(self):
        """
        Ip address of the detector (source) udp interface. 
        Note
        -----
        Must be same subnet as destination udp ip.\n
        [Eiger] Set only for 10G. For 1G, detector will replace with its own DHCP IP address. \n
        To set IPs for individual modules, use setSourceUDPIP. 
        Example
        -------
        >>> d.udp_srcip = '192.168.1.127'
        >>> d.udp_srcip
        192.168.1.127
        """
        return element_if_equal(self.getSourceUDPIP())

    @udp_srcip.setter
    def udp_srcip(self, ip):
        self.setSourceUDPIP(IpAddr(ip))

    @property
    def udp_srcip2(self):
        """
        [Jungfrau][Gotthard2] Ip address of the detector (source) udp interface 2. 
        Note
        -----
        [Jungfrau] bottom half \n
        [Gotthard2] veto debugging \n
        Must be same subnet as destination udp ip2.\n
        To set IPs for individual modules, use setSourceUDPIP2. 
        Example
        -------
        >>> d.udp_srcip2 = '192.168.1.127'
        >>> d.udp_srcip2
        192.168.1.127
        """
        return element_if_equal(self.getSourceUDPIP2())

    @udp_srcip2.setter
    def udp_srcip2(self, ip):
        self.setSourceUDPIP2(IpAddr(ip))

    @property
    def udp_dstport(self):
        """
        Port number of the receiver (destination) udp interface. 
        Note
        ----
        Default is 50001. \n
        Ports for each module is calculated (incremented by 1 if no 2nd interface) \n
        To set ports for individual modules, use setDestinationUDPPort.
        """
        return element_if_equal(self.getDestinationUDPPort())

    @udp_dstport.setter
    def udp_dstport(self, port):
        self.setDestinationUDPPort(port)

    @property
    def udp_dstport2(self):
        """
        Port number of the receiver (destination) udp interface. 
        Note
        ----
        Default is 50002. \n
        [Eiger] right half \n
        [Jungfrau] bottom half \n
        [Gotthard2] veto debugging \n
        Ports for each module is calculated (incremented by 2) \n
        To set ports for individual modules, use setDestinationUDPPort2.
        """
        return element_if_equal(self.getDestinationUDPPort2())

    @udp_dstport2.setter
    def udp_dstport2(self, port):
        self.setDestinationUDPPort2(port)

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
        """
        Retrieve user details from shared memory (hostname, type, PID, User, Date)
        """
        return self.getUserDetails()

    @property
    def settingspath(self):
        """[Eiger] Directory where settings files are loaded from/to."""
        return element_if_equal(self.getSettingsPath())

    @settingspath.setter
    def settingspath(self, path):
        self.setSettingsPath(path)

    @property
    def status(self):
        """Gets detector status. Enum: runStatus
        Note
        -----
        Options: IDLE, ERROR, WAITING, RUN_FINISHED, TRANSMITTING, RUNNING, STOPPED
        >>> d.status
        runStatus.IDLE
        """
        return element_if_equal(self.getDetectorStatus())

    @property
    def rx_status(self):
        """Gets receiver listener status. Enum: runStatus
        Note
        -----
        Options: IDLE, TRANSMITTING, RUNNING
        >>> d.rx_status
        runStatus.IDLE
        """
        return element_if_equal(self.getReceiverStatus())

    @property
    def rx_udpsocksize(self):
        """UDP socket buffer size in receiver. Tune rmem_default and rmem_max accordingly."""
        return element_if_equal(self.getRxUDPSocketBufferSize())

    @rx_udpsocksize.setter
    def rx_udpsocksize(self, buffer_size):
        self.setRxUDPSocketBufferSize(buffer_size)

    @property
    def rx_realudpsocksize(self):
        """Gets actual udp socket buffer size. Double the size of rx_udpsocksize due to kernel bookkeeping."""
        return element_if_equal(self.getRxRealUDPSocketBufferSize())

    @property
    def trimbits(self):
        """
        [Eiger][Mythen3] Loads custom trimbit file to detector. 
        Note
        -----
        If no extension specified, serial number of each module is attached.
        :getter: Not implemented
        Example
        -------
        >>> d.trimbits = '/path_to_file/noise'
        - 14:53:27.931 INFO: Settings file loaded: /path_to_file/noise.sn000
        """
        return NotImplementedError("trimbits are set only")

    @trimbits.setter
    def trimbits(self, fname):
        fname = ut.make_string_path(fname)
        self.loadTrimbits(fname)

    @property
    @element
    def trimval(self):
        """
        [Eiger][Mythen3] Set all trimbits to this value. Returns -1 if all trimbits are different values.
        """
        return self.getAllTrimbits()

    @trimval.setter
    def trimval(self, value):
        self.setAllTrimbits(value)

    @property
    def lock(self):
        """Lock detector to one client IP, 1 locks, 0 unlocks. Default is unlocked."""
        return element_if_equal(self.getDetectorLock())

    @lock.setter
    def lock(self, value):
        self.setDetectorLock(value)

    @property
    def rx_lock(self):
        """Lock receiver to one client IP, 1 locks, 0 unlocks. Default is unlocked."""
        return element_if_equal(self.getRxLock())

    @rx_lock.setter
    def rx_lock(self, value):
        self.setRxLock(value)


    @property
    @element
    def scanerrmsg(self):
        return self.getScanErrorMessage()

    @property
    @element
    def rx_zmqstartfnum(self):
        return self.getRxZmqStartingFrame()

    @rx_zmqstartfnum.setter
    def rx_zmqstartfnum(self, value):
        ut.set_using_dict(self.setRxZmqStartingFrame, value)

    @property
    def lastclient(self):
        """Get Client IP Address that last communicated with the detector."""
        return element_if_equal(self.getLastClientIP())

    @property
    def reg(self):
        """
        Reads/writes to a 32 bit register.

        Note
        -----
        Advanced user Function! \n
        [Eiger] Address is +0x100 for only left, +0x200 for only right.
        """
        return self._register

    @property
    def slowadc(self):
        return SlowAdcProxy(self)

    @property
    def daclist(self):
        """Gets the list of enums for every dac for this detector."""
        return self.getDacList()

    @property
    def dacvalues(self):
        """Gets the dac values for every dac for this detector."""
        return {
            dac.name.lower(): element_if_equal(np.array(self.getDAC(dac, False)))
            for dac in self.getDacList()
        }

    @property
    def timinglist(self):
        return self.getTimingModeList()

    @property
    def templist(self):
        return self.getTemperatureList()

    @property
    def tempvalues(self):
        """Gets the temp values for every temp for this detector."""
        return {
            t.name.lower(): np.array(self.getTemperature(t))
            for t in self.getTemperatureList()
        }

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
    @element
    def adcinvert(self):
        """[Ctb][Moench][Jungfrau] ADC Inversion Mask.
        Note
        -----
        [Jungfrau][Moench] Inversions on top of the default mask.
        """
        return self.getADCInvert()

    @adcinvert.setter
    def adcinvert(self, value):
        ut.set_using_dict(self.setADCInvert, value)

    @property
    @element
    def triggersl(self):
        return self.getNumberOfTriggersLeft()

    @property
    @element
    def timestamp(self):
        return self.getMeasurementTime()

    @property
    def led(self):
        """[Ctb] Switches on/off all LEDs. Default is enabled. """
        return element_if_equal(self.getLEDEnable())

    @led.setter
    def led(self, value):
        self.setLEDEnable(value)

    def acquire(self):
        """
        Run the configured measurement
        Note
        ----
        Blocking command, where control server is blocked and cannot accept other commands until acquisition is done. \n
        - sets acquiring flag
        - starts the receiver listener (if enabled)
        - starts detector acquisition for number of frames set
        - monitors detector status from running to idle
        - stops the receiver listener (if enabled)
        - increments file index if file write enabled
        - resets acquiring flag
        """
        super().acquire()
        print('\n', end = '')


    @property
    def versions(self):
        return {'type': self.type,
                'package': self.packageversion, 
                'client': self.clientversion,
                'firmware': self.firmwareversion,
                'detectorserver': self.detectorserverversion,
                'receiver': self.rx_version}

    @property
    def virtual(self):
        """
        Setup with n virtual servers running on localhost
        starting with port p

        Example
        ---------

        >>> d.virtual = n, p

        """
        raise NotImplementedError('Virtual is set only')

    @virtual.setter
    def virtual(self, args):
        n_detectors, starting_port = args
        self.setVirtualDetectorServers(n_detectors, starting_port)

    

    @property
    def packageversion(self):
        return self.getPackageVersion()

    @property
    def ratecorr(self):
        """ 
        [Eiger] Custom dead time correction constant in ns. 0 will unset rate correction.

        Note
        -----
        To set default rate correction, use setDefaultRateCorrection

        Known Issue
        ------------
        :getter: Always give 0 due to the microseconds precision.
        :setter: Use scientific notation to set custom rate correction, since timedelta resolution is 1 microseconds. \n
        Or use setDefaultRateCorrection to set the default one from trimbit file


        Example
        -----------
        >>> d.ratecorr = 10e-9 
        >>> d.setDefaultRateCorrection()
        >>> d.ratecorr = 0.0
        """
        return reduce_time(self.getRateCorrection())

    @ratecorr.setter
    def ratecorr(self, tau):
        if isinstance(tau, int):
            tau = float(tau)
        self.setRateCorrection(tau)

    @property
    def speed(self):
        """
        [Eiger][Jungfrau] Readout speed of chip. Enum: speedLevel
        Note
        -----
        Options: FULL_SPEED, HALF_SPEED, QUARTER_SPEED \n
        [Jungfrau] FULL_SPEED option only available from v2.0 boards and with setting number of interfaces to 2.  \n
        Also overwrites adcphase to recommended default.
        """
        return element_if_equal(self.getSpeed())

    @speed.setter
    def speed(self, value):
        self.setSpeed(value)

    @property
    def rx_jsonpara(self):
        """
        Get the receiver additional json parameter. In case the parameter is different between
        the modules a list of strings will be returned. On setting the value is automatically
        converted to a string. 

        Example
        -----------

        >>> d.rx_jsonpara['emin']
        '4500'

        >>> d.rx_jsonpara['emin'] = 5000

        """
        return JsonProxy(self)


    @property
    @element
    def rx_jsonaddheader(self):
        return self.getAdditionalJsonHeader()

    @rx_jsonaddheader.setter
    def rx_jsonaddheader(self, args):
        ut.set_using_dict(self.setAdditionalJsonHeader, args)

    @property
    def rx_frameindex(self):
        return self.getRxCurrentFrameIndex()

    @property
    def threshold(self):
        """[Eiger] Threshold in eV
        Note
        ----
        To change settings as well or set threshold without trimbits, use setThresholdEnergy.
        :setter: It loads trim files from settingspath.
        """
        return element_if_equal(self.getThresholdEnergy())

    @threshold.setter
    def threshold(self, eV):
        self.setThresholdEnergy(eV)

    @property
    def timing(self):
        """
        Set Timing Mode of detector. Enum: timingMode
        Note
        -----
        Default: AUTO_TIMING \n
        [Jungfrau][Gotthard][Ctb][Moench][Gotthard2] AUTO_TIMING, TRIGGER_EXPOSURE \n
        [Mythen3] AUTO_TIMING, TRIGGER_EXPOSURE, GATED, TRIGGER_GATED \n
        [Eiger] AUTO_TIMING, TRIGGER_EXPOSURE, GATED, BURST_TRIGGER
        """
        return element_if_equal(self.getTimingMode())

    @timing.setter
    def timing(self, mode):
        self.setTimingMode(mode)

    @property
    def trimen(self):
        """
        [Eiger] List of trim energies, where corresponding default trim files exist in corresponding trim folders.
        Example
        ------
        >>> d.trimen
        []
        >>> d.trimen = [4500, 5400, 6400]
        >>> d.trimen
        [4500, 5400, 6400]
        """
        return element_if_equal(self.getTrimEnergies())

    @trimen.setter
    def trimen(self, energies):
        self.setTrimEnergies(energies)

    @property
    def vthreshold(self):
        """
        [Eiger][Mythen3] Detector threshold voltage for single photon counters in dac units.
        Note
        ----
        [Eiger] Sets vcmp_ll, vcmp_lr, vcmp_rl, vcmp_rr and vcp to the same value. \n
        [Mythen3] Sets vth1, vth2 and vth3 to the same value.
        """
        return element_if_equal(self.getDAC(dacIndex.VTHRESHOLD, False))

    @vthreshold.setter
    def vthreshold(self, value):
        self.setDAC(dacIndex.VTHRESHOLD, value, False)

    @property
    def type(self):
        """ Returns detector type. Enum: detectorType
        Note
        ----
        :setter: Not implemented
        Values: EIGER, JUNGFRAU, GOTTHARD, MOENCH, MYTHEN3, GOTTHARD2, CHIPTESTBOARD
        """
        return element_if_equal(self.getDetectorType())

    @property
    def rx_frameindex(self):
        """Current frame index received in receiver during acquisition."""
        return element_if_equal(self.getRxCurrentFrameIndex())

    @property
    def rx_missingpackets(self):
        """Gets the number of missing packets for each port in receiver."""
        return element_if_equal(self.getNumMissingPackets())

    """

    <<<-----------------------Eiger specific----------------------->>>

    """

    @property
    @element
    def quad(self):
        return self.getQuad()

    @quad.setter
    def quad(self, value):
        self.setQuad(value)

    @property
    def subexptime(self):
        """
        [Eiger] Exposure time of EIGER subframes in 32 bit mode.
        Note
        ----
        Subperiod = subexptime + subdeadtime.
        :getter: always returns in seconds. To get in datetime.delta, use getSubExptime

        Example
        -----------
        >>> d.subexptime = 1.230203
        >>> d.subexptime = datetime.timedelta(seconds = 1.23, microseconds = 203)
        >>> d.subexptime
        1.230203
        >>> d.getSubExptime()
        [datetime.timedelta(seconds = 1, microseconds = 203)]
        """
        res = self.getSubExptime()
        return reduce_time(res)

    @subexptime.setter
    def subexptime(self, t):
        ut.set_time_using_dict(self.setSubExptime, t)

    @property
    @element
    def readnlines(self):
        return self.getPartialReadout()

    @readnlines.setter
    def readnlines(self, value):
        ut.set_using_dict(self.setPartialReadout, value)


    @property
    def subdeadtime(self):
        """
        [Eiger] Dead time of EIGER subframes in 32 bit mode, accepts either a value in seconds or datetime.timedelta
        Note
        ----
        Subperiod = subexptime + subdeadtime.
        :getter: always returns in seconds. To get in datetime.delta, use getSubDeadTime

        Example
        -----------
        >>> d.subdeadtime = 1.230203
        >>> d.subdeadtime = datetime.timedelta(seconds = 1.23, microseconds = 203)
        >>> d.subdeadtime
        1.230203
        >>> d.getSubDeadTime()
        [datetime.timedelta(seconds = 1, microseconds = 203)]
        """
        res = self.getSubDeadTime()
        return reduce_time(res)

    @subdeadtime.setter
    def subdeadtime(self, t):
        ut.set_time_using_dict(self.setSubDeadTime, t)

    @property
    @element
    def parallel(self):
        """
        [Eiger][Mythen3] Enable or disable the parallel readout mode of detector. 
        """
        return self.getParallelMode()

    @parallel.setter
    def parallel(self, value):
        self.setParallelMode(value)

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
        """[Eiger][Ctb][Moench][Mythen3] 10GbE Enable."""
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
    def now(self):
        return self.getActualTime()

    @property
    @element
    def storagecells(self):
        """
        [Jungfrau] Number of additional storage cells. 
        Note
        ----
        For advanced users only. \n
        Options: 0 - 15. Default is 0.
        The #images = #frames x #triggers x (#storagecells + 1)
        """
        return self.getNumberOfAdditionalStorageCells()

    @storagecells.setter
    def storagecells(self, n_cells):
        self.setNumberOfAdditionalStorageCells(n_cells)

    @property
    @element
    def storagecell_start(self):
        """
        [Jungfrau] Storage cell that stores the first acquisition of the series. 
        
        Note
        ----
        For advanced users only.
        Options 0-15. Default is 15. \n
        """
        return self.getStorageCellStart()

    @storagecell_start.setter
    def storagecell_start(self, value):
        self.setStorageCellStart(value)

    @property
    @element
    def storagecell_delay(self):
        """
        [Jungfrau] Additional time delay between 2 consecutive exposures in burst mode, accepts either a value in seconds or datetime.timedelta
        Note
        -----
        For advanced users only \n
        Value: 0-1638375 ns (resolution of 25ns) \n
        :getter: always returns in seconds. To get in datetime.delta, use getStorageCellDelay

        Example
        -----------
        >>> d.storagecell_delay = 0.00056
        >>> d.storagecell_delay = datetime.timedelta(microseconds = 45)
        >>> d.storagecell_delay
        4.5e-05
        >>> d.getStorageCellDelay()
        [datetime.timedelta(microseconds=45)]
        """
        return ut.reduce_time(self.getStorageCellDelay())

    @storagecell_delay.setter
    def storagecell_delay(self, t):
        self.setStorageCellDelay(t)

    @property
    @element
    def temp_threshold(self):
        """
        [Jungfrau] Threshold temperature in degrees. 
        Note
        -----
        If temperature crosses threshold temperature and temperature control is enabled, power to chip will be switched off and temperature event occurs. \n
        To power on chip again, temperature has to be less than threshold temperature and temperature event has to be cleared/reset.
        """
        return self.getThresholdTemperature()

    @temp_threshold.setter
    def temp_threshold(self, value):
        self.setThresholdTemperature(value)

    @property
    @element
    def temp_event(self):
        """
        [Jungfrau] 1, if a temperature event occured. \n
        Note
        ----
        If temperature crosses threshold temperature and temperature control is enabled, power to chip will be switched off and temperature event occurs. \n
        To power on chip again, temperature has to be less than threshold temperature and temperature event has to be cleared/reset.
        :setter: To clear the event, set it to 0.
        """
        return self.getTemperatureEvent()

    @temp_event.setter
    def temp_event(self, value):
        if value != 0:
            raise ValueError("Value needs to be 0 for reset. Setting not allowed")
        self.resetTemperatureEvent()

    @property
    @element
    def temp_control(self):
        """
        [Jungfrau] Temperature control enable. 
        Note
        -----
        Default is 0 (disabled). \n
        If temperature crosses threshold temperature and temperature control is enabled, power to chip will be switched off and temperature event occurs. \n
        To power on chip again, temperature has to be less than threshold temperature and temperature event has to be cleared/reset.
        """
        return self.getTemperatureControl()

    @temp_control.setter
    def temp_control(self, value):
        self.setTemperatureControl(value)

    @property
    @element
    def selinterface(self):
        """[Jungfrau] The udp interface to stream data from detector. 
        Note
        -----
        Effective only when number of interfaces is 1. Default: 0 (outer). Inner is 1.
        """
        return self.getSelectedUDPInterface()

    @selinterface.setter
    def selinterface(self, i):
        self.selectUDPInterface(i)

    """
    ---------------------------<<<Gotthard2 specific>>>---------------------------
    """

    @property
    @element
    def bursts(self):
        """[Gotthard2] Number of bursts per aquire. Only in auto timing mode and burst mode."""
        return self.getNumberOfBursts()

    @bursts.setter
    def bursts(self, value):
        self.setNumberOfBursts(value)

    @property
    @element
    def filter(self):
        """[Gotthard2] Set filter resistor. 
        Note
        ----
        Default is 0. Options: 0-3.
        """
        return self.getFilter()

    @filter.setter
    def filter(self, value):
        ut.set_using_dict(self.setFilter, value)

    @property
    def maxclkphaseshift(self):
        return MaxPhaseProxy(self)

    @property
    @element
    def timingsource(self):
        return self.getTimingSource()

    @timingsource.setter
    def timingsource(self, args):
        ut.set_using_dict(self.setTimingSource, args)


    @property
    @element
    def veto(self):
        """
        [Gotthard2] Enable or disable veto data streaming from detector. 
        Note
        ----
        Default is 0.
        """
        return self.getVeto()

    @veto.setter
    def veto(self, value):
        self.setVeto(value)

    @property
    @element
    def cdsgain(self):
        """[Gotthard2] Enable or disable CDS gain. Default is disabled. """
        return self.getCDSGain()

    @cdsgain.setter
    def cdsgain(self, value):
        ut.set_using_dict(self.setCDSGain, value)


    @property
    @element
    def burstmode(self):
        """[Gotthard2] Burst mode of detector. Enum: burstMode
        Note
        ----
        BURST_INTERNAL (default), BURST_EXTERNAL, CONTINUOUS_INTERNAL, CONTINUOUS_EXTERNAL
        """
        return self.getBurstMode()

    @burstmode.setter
    def burstmode(self, value):
        ut.set_using_dict(self.setBurstMode, value)

    @property
    def burstperiod(self):
        """
        [Gotthard2] Period between 2 bursts. Only in burst mode and auto timing mode.
        Note
        -----
        :getter: always returns in seconds. To get in datetime.delta, use getBurstPeriod

        Example
        -----------
        >>> d.burstperiod = 1.05
        >>> d.burstperiod = datetime.timedelta(minutes = 3, seconds = 1.23)
        >>> d.burstperiod
        181.23
        >>> d.getBurstPeriod()
        [datetime.timedelta(seconds=181, microseconds=230000)]

        """
        return ut.reduce_time(self.getBurstPeriod())

    @burstperiod.setter
    def burstperiod(self, value):
        ut.set_time_using_dict(self.setBurstPeriod, value)

    @property
    def vetofile(self):
        """
        [Gotthard2] Set veto reference for each 128 channels for specific chip. 
        The file should have 128 rows of gain index and 12 bit value in dec

        Example
        ---------

        d.vetofile = '/path/to/file.txt' #set for all chips
        d.vetofile = 3, '/path/to/file.txt' # set for chip 3

        """
        raise NotImplementedError('vetofile is set only')

    @vetofile.setter
    def vetofile(self, args):
        if isinstance(args, str):
            chip_index = -1
            fname = args
        elif isinstance(args, (tuple, list)):
            chip_index, fname = args
        else:
            raise ValueError("unknow argument to vetofile")

        self.setVetoFile(chip_index, fname)

    @property 
    def vetophoton(self):
        raise NotImplementedError('vetofile is set only')

    @vetophoton.setter
    def vetophoton(self, args):
        chip_index, n_photons, photon_energy, fname = args
        self.setVetoPhoton(chip_index, n_photons, photon_energy, fname)

    @property
    @element
    def vetoref(self):
        """
        [Gotthard2] Set veto reference for all 128 channels for all chips.

        Example
        ----------

        >>> d.vetoref = chip, value

        """
        raise NotImplementedError('vetoref is set only')

    @vetoref.setter
    def vetoref(self, args):
        gain_index, value = args
        self.setVetoReference(gain_index, value)


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

        Example
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

        Example
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
    <<<CTB>>>
    """

    @property
    @element
    def adcenable(self):
        """[Ctb][Moench] ADC Enable Mask for 1Gb. Enable for each 32 ADC channel."""
        return self.getADCEnableMask()

    @adcenable.setter
    def adcenable(self, value):
        ut.set_using_dict(self.setADCEnableMask, value)

    @property
    @element
    def adcenable10g(self):
        """[Ctb][Moench] ADC Enable Mask for 10Gb mode for each 32 ADC channel. 
        Note
        -----
        If any of a consecutive 4 bits are enabled, the complete 4 bits are enabled."""
        return self.getTenGigaADCEnableMask()

    @adcenable10g.setter
    def adcenable10g(self, value):
        ut.set_using_dict(self.setTenGigaADCEnableMask, value)


    @property
    @element
    def samples(self):
        return self.getNumberOfAnalogSamples()

    @samples.setter
    def samples(self, nsamples):
        self.setNumberOfAnalogSamples(nsamples)

    @property
    def runclk(self):
        """[Ctb][Moench] Run clock in MHz."""
        return element_if_equal(self.getRUNClock())

    @runclk.setter
    def runclk(self, freq):
        self.setRUNClock(freq)

    @property
    def romode(self):
        """
        [CTB] Readout mode of detector. Enum: readoutMode
        
        Note
        ------
        Options: ANALOG_ONLY, DIGITAL_ONLY, ANALOG_AND_DIGITAL
        Default: ANALOG_ONLY

        Example
        --------
        >>> d.romode = readoutMode.ANALOG_ONLY
        >>> d.romode
        readoutMode.ANALOG_ONLY
        """
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
        """
        [Ctb] List of digital signal bits read out. 
        
        Note
        -----
        Each element in list can be 0 - 63 and must be non repetitive.

        Example
        ---------
        >>> d.rxdbitlist = [0, 1, 61, 9]
        >>> d.rxdbitlist
        [0, 1, 61, 9]
        >>> d.rxdbitlist = []
        >>> d.rxdbitlist
        []
        """
        return element_if_equal(self.getRxDbitList())

    @rx_dbitlist.setter
    def rx_dbitlist(self, value):
        self.setRxDbitList(value)

    @property
    def rx_dbitoffset(self):
        """[Ctb] Offset in bytes in digital data to skip in receiver."""
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
        """
        [Ctb][Moench] Sync clock in MHz.
        Note
        -----
        :setter: Not implemented
        """
        return element_if_equal(self.getSYNCClock())

    @property
    def pattern(self):
        """[Mythen3][Moench][Ctb] Loads ASCII pattern file directly to server (instead of executing line by line).

        Example
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
        
        Example
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
        
        Example
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
        
        Example
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
        
        Example
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
                
        Example
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
                
        Example
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
                
        Example
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
        
        Example
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
        
        Example
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
        
        Example
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
        """[Ctb] Voltage supply a in mV."""
        return self.getDAC(dacIndex.V_POWER_A, True)

    @v_a.setter
    def v_a(self, value):
        self.setDAC(dacIndex.V_POWER_A, value, True)

    @property
    @element
    def v_b(self):
        """[Ctb] Voltage supply a in mV."""
        return self.getDAC(dacIndex.V_POWER_B, True)

    @v_b.setter
    def v_b(self, value):
        self.setDAC(dacIndex.V_POWER_B, value, True)

    @property
    @element
    def v_c(self):
        """[Ctb] Voltage supply a in mV."""
        return self.getDAC(dacIndex.V_POWER_C, True)

    @v_c.setter
    def v_c(self, value):
        self.setDAC(dacIndex.V_POWER_C, value, True)

    @property
    @element
    def v_d(self):
        """[Ctb] Voltage supply a in mV."""
        return self.getDAC(dacIndex.V_POWER_D, True)

    @v_d.setter
    def v_d(self, value):
        self.setDAC(dacIndex.V_POWER_D, value, True)

    @property
    @element
    def v_io(self):
        """[Ctb] Voltage supply a in mV."""
        return self.getDAC(dacIndex.V_POWER_IO, True)

    @v_io.setter
    def v_io(self, value):
        self.setDAC(dacIndex.V_POWER_IO, value, True)

    @property
    @element
    def v_limit(self):
        """[Ctb][Moench] Soft limit for power supplies (ctb only) and DACS in mV."""
        return self.getDAC(dacIndex.V_LIMIT, True)

    @v_limit.setter
    def v_limit(self, value):
        self.setDAC(dacIndex.V_LIMIT, value, True)

    @property
    @element
    def im_a(self):
        """[Ctb] Measured current of power supply a in mA.
        
        Note
        -----
        :setter: Not implemented
        """
        return self.getMeasuredCurrent(dacIndex.I_POWER_A)

    @property
    @element
    def im_b(self):
        """[Ctb] Measured current of power supply b in mA.
        
        Note
        -----
        :setter: Not implemented
        """
        return self.getMeasuredCurrent(dacIndex.I_POWER_B)

    @property
    @element
    def im_c(self):
        """[Ctb] Measured current of power supply c in mA.
                
        Note
        -----
        :setter: Not implemented
        """
        return self.getMeasuredCurrent(dacIndex.I_POWER_C)

    @property
    @element
    def im_d(self):
        """[Ctb] Measured current of power supply d in mA.
                
        Note
        -----
        :setter: Not implemented
        """
        return self.getMeasuredCurrent(dacIndex.I_POWER_D)

    @property
    @element
    def im_io(self):
        """[Ctb] Measured current of power supply io in mA.
                
        Note
        -----
        :setter: Not implemented
        """
        return self.getMeasuredCurrent(dacIndex.I_POWER_IO)


    @property
    def clkdiv(self):
        """
        [Gotthard2][Mythen3] Clock Divider of 5 clocks. Must be greater than 1.
        Example
        -------
        >>> d.clkdiv[0] = 20
        >>> d.clkdiv
        0: 20
        1: 10
        2: 20
        3: 10
        4: 10
        5: 5
        """
        return ClkDivProxy(self)


    """
    ---------------------------<<<Gotthard specific>>>---------------------------
    """

    @property
    def exptimel(self):
        """[Gotthard] Exposure time left for current frame.
        Note
        -----
        :getter: always returns in seconds. To get in datetime.delta, use getExptimeLeft

        Example
        -----------
        >>> d.exptimel
        181.23
        >>> d.getExptimeLeft()
        [datetime.timedelta(seconds=181, microseconds=230000)]
        """
        t = self.getExptimeLeft()
        return reduce_time(t)


    """
    ---------------------------<<<Mythen3 specific>>>---------------------------
    """

    @property
    @element
    def gates(self):
        """[Mythen3] Number of external gates in gating or trigger_gating mode (external gating)."""
        return self.getNumberOfGates()

    @gates.setter
    def gates(self, value):
        ut.set_using_dict(self.setNumberOfGates, value)


    @property
    def clkfreq(self):
        """
        [Gotthard2][Mythen3] Frequency of clock in Hz. 
        Note
        -----
        :setter: Not implemented. Use clkdiv to set frequency
        Example
        -------
        >>> d.clkfreq[0]
        50000000
        """
        return ClkFreqProxy(self)

    """
    ---------------------------<<<Debug>>>---------------------------
    """

    @property
    def initialchecks(self):
        """
        Enable or disable intial compatibility and other checks at detector start up. 
        Note
        ----
        It is enabled by default. Must come before 'hostname' command to take effect. \n
        Can be used to reprogram fpga when current firmware is incompatible. \n
        Advanced user function!
        """
        return self.getInitialChecks()
    
    @initialchecks.setter
    def initialchecks(self, value):
        self.setInitialChecks(value)