# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
from ._slsdet import CppDetectorApi
from ._slsdet import slsDetectorDefs
from ._slsdet import IpAddr, MacAddr

runStatus = slsDetectorDefs.runStatus
timingMode = slsDetectorDefs.timingMode
speedLevel = slsDetectorDefs.speedLevel
dacIndex = slsDetectorDefs.dacIndex
detectorType = slsDetectorDefs.detectorType
streamingInterface = slsDetectorDefs.streamingInterface

defs = slsDetectorDefs

from .utils import element_if_equal, all_equal, get_set_bits, list_to_bitmask
from .utils import Geometry, to_geo, element, reduce_time, is_iterable, hostname_list
from ._slsdet import xy
from .gaincaps import Mythen3GainCapsWrapper
from . import utils as ut
from .proxy import JsonProxy, SlowAdcProxy, ClkDivProxy, MaxPhaseProxy, ClkFreqProxy, PatLoopProxy, PatNLoopProxy, PatWaitProxy, PatWaitTimeProxy 
from .registers import Register, Adc_register
import datetime as dt

from functools import wraps
from collections import namedtuple
import socket
import numpy as np

def freeze(cls):
    """
    Decorator to prevent assignments to not existing properties. 
    Protects for example form typos when setting exptime etc.
    """
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
        """Number of modules in shared memory."""
        return self.size()

    @property
    def nmod(self):
        """Number of modules in shared memory."""
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

        #create a new object to replace the old, allow us to
        #do a new initialization of dacs etc.
        new_object = self.__class__(self.getShmId())
        self.__dict__.update(new_object.__dict__)

    @property
    def parameters(self):
        """Sets detector measurement parameters to those contained in fname. 
        Set up per measurement.
        
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

        Note
        -----
        The row and column values in the udp/zmq header are affected by the order in this command and the detsize command. The modules are stacked row by row until they reach the y-axis limit set by detsize (if specified). Then, stacking continues in the next column and so on. This only affects row and column in udp/zmq header.
        
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
        args = hostname_list(hostnames)
        self.setHostname(args)


    @property
    @element
    def port(self):
        """
        Port number of the control server on detector for detector-client tcp interface. 

        Note
        ----
        Default is 1952. Normally unchanged. \n
        Set different ports for virtual servers on same pc.
        """
        return self.getControlPort()

    @port.setter
    def port(self, value):
        ut.validate_port(value)
        ut.set_using_dict(self.setControlPort, value)

    @property
    @element
    def stopport(self):
        """Port number of the stop server on detector for detector-client tcp interface. 

        Note
        ----
        Default is 1953. Normally unchanged.
        """
        return self.getStopPort()

    @stopport.setter
    def stopport(self, args):
        ut.validate_port(args)
        ut.set_using_dict(self.setStopPort, args)


    @property
    @element
    def firmwareversion(self):
        """
        Fimware version of detector in format [0xYYMMDD] or an increasing 2 digit number for Eiger.
        
        Example
        -------
        >>> d.firmwareversion
        '0x200910'
        """
        return ut.lhex(self.getFirmwareVersion())

    @property
    @element
    def detectorserverversion(self):
        """
        On-board detector server software version in format [0xYYMMDD]
        
        Example
        -------
        >>> d.detectorserverversion
        '7.0.0'
        """
        return self.getDetectorServerVersion()

    @property
    @element
    def hardwareversion(self):
        """
        Hardware version of detector. \n
        [Eiger] Hardware version of front FPGA on detector.
        """
        return self.getHardwareVersion()

    @property
    @element
    def kernelversion(self):
        """
        Kernel version on the detector including time and date
        
        Example
        -------
        >>> d.kernelversion
        '#37 PREEMPT Thu Oct 13 14:51:04 CEST 2016'
        """
        return self.getKernelVersion()

    @property
    def clientversion(self):
        """Client software version in format [YYMMDD]
        
        Example
        -------
        >>> d.clientversion
        '7.0.1'
        """
        return self.getClientVersion()

    @property
    @element
    def rx_version(self):
        """Receiver version """
        return self.getReceiverVersion()

    @property
    @element
    def serialnumber(self):
        """Jungfrau][Gotthard][Mythen3][Gotthard2][CTB][Moench] Serial number of detector """
        return ut.lhex(self.getSerialNumber())

    @property
    @element
    def rx_threads(self):
        """
        Get kernel thread ids from the receiver in order of [parent, tcp, listener 0, processor 0, streamer 0, listener 1, processor 1, streamer 1, arping]. 
        
        Note
        -----
        If no streamer yet or there is no second interface, it gives 0 in its place. 

        :setter: Not Implemented
        """
        return self.getRxThreadIds()

    @property
    @element
    def rx_arping(self):
        """Starts a thread in slsReceiver to arping the interface it is listening every minute. Useful in 10G mode. """
        return self.getRxArping()

    @rx_arping.setter
    def rx_arping(self, value):
        ut.set_using_dict(self.setRxArping, value)


    @property
    @element
    def dr(self):
        """
        Dynamic range or number of bits per pixel/channel.

        Note
        -----
        [Eiger] Options: 4, 8, 12, 16, 32. If set to 32, also sets clkdivider to 2 (quarter speed), else to 0 (full speed)\n
        [Mythen3] Options: 8, 16, 32 \n
        [Jungfrau][Moench][Gotthard][Ctb][Mythen3][Gotthard2][Xilinx Ctb] 16
        """
        return self.getDynamicRange()

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
    @element
    def module_size(self):
        return [to_geo(item) for item in self.getModuleSize()]
        

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
        Detector settings. 
        Enum: detectorSettings

        Note
        -----
        
        [Eiger] Use threshold command to load settings
        [Jungfrau] GAIN0, HIGHGAIN0 \n
        [Gotthard] DYNAMICGAIN, HIGHGAIN, LOWGAIN, MEDIUMGAIN, VERYHIGHGAIN \n
        [Gotthard2] DYNAMICGAIN, FIXGAIN1, FIXGAIN2 \n
        [Eiger] settings loaded from file found in settingspath
        [Moench] G1_HIGHGAIN, G1_LOWGAIN, G2_HIGHCAP_HIGHGAIN, G2_HIGHCAP_LOWGAIN, G2_LOWCAP_HIGHGAIN, G2_LOWCAP_LOWGAIN, G4_HIGHGAIN, G4_LOWGAIN
        """
        return element_if_equal(self.getSettings())

    @settings.setter
    def settings(self, value):
        self.setSettings(value)

    @property
    @element
    def frames(self):
        """Number of frames per acquisition. In trigger mode, number of frames per trigger.

        Note
        -----
        Cannot be set in modular level. \n
        In scan mode, number of frames is set to number of steps. \n
        [Gotthard2] Burst mode has a maximum of 2720 frames.
        """
        return self.getNumberOfFrames()

    @frames.setter
    def frames(self, n_frames):
        self.setNumberOfFrames(n_frames)

    @property
    @element
    def framesl(self):
        """
        [Gotthard][Jungfrau][Moench][Mythen3][Gotthard2][CTB][Xilinx CTB] Number of frames left in acquisition.\n

        Note
        ----
        [Gotthard2] only in continuous auto mode.

        :setter: Not Implemented
        """
        return self.getNumberOfFramesLeft()

    @property
    @element
    def framecounter(self):
        """
        [Jungfrau][Moench][Mythen3][Gotthard2][CTB][Xilinx Ctb] Number of frames from start run control.

        Note
        -----
        [Gotthard2] only in continuous mode.

        :setter: Not Implemented
        """
        return self.getNumberOfFramesFromStart()


    @property
    @element
    def scan(self):
        """
        Pass in a scanParameters object 
        see python/examples/use_scan.py

        """
        return self.getScan()

    @scan.setter
    def scan(self, s):
        ut.set_using_dict(self.setScan, s)

    @property
    @element
    def powerchip(self):
        """
        [Jungfrau][Moench][Mythen3][Gotthard2][Xilinx Ctb] Power the chip. 

        Note
        ----
        [Jungfrau][Moench] Default is disabled. Get will return power status. Can be off if temperature event occured (temperature over temp_threshold with temp_control enabled. Will configure chip (only chip v1.1).\n
        [Mythen3][Gotthard2] Default is 1. If module not connected or wrong module, powerchip will fail.
        [Xilinx Ctb] Default is 0. Also configures the chip if powered on.
        """
        return self.getPowerChip()

    @powerchip.setter
    def powerchip(self, value):
        ut.set_using_dict(self.setPowerChip, value)

    def configtransceiver(self):
        """
        [Xilinx Ctb] Waits for transceiver to be aligned. 
        
        Note
        ----
        Chip had to be configured (powered on) before this.
        """
        self.configureTransceiver()

    @property
    @element
    def triggers(self):
        """Number of triggers per aquire. Set timing mode to use triggers."""
        return self.getNumberOfTriggers()

    @triggers.setter
    def triggers(self, n_triggers):
        self.setNumberOfTriggers(n_triggers)

    def resetdacs(self, use_hardware_values):
        self.resetToDefaultDacs(use_hardware_values)

    def trigger(self):
        self.sendSoftwareTrigger()

    def blockingtrigger(self):
        self.sendSoftwareTrigger(True)

    @property
    @element
    def gaincaps(self):
        """
        [Mythen3] Gain caps. 
        Enum: M3_GainCaps
        
        Note
        ----
        Options: M3_GainCaps, M3_C15sh, M3_C30sh, M3_C50sh, M3_C225ACsh, M3_C15pre

        Example
        -------
        >>> d.gaincaps
        C15pre, C30sh
        >>> d.gaincaps = M3_GainCaps.M3_C30sh
        >>> d.gaincaps
        C30sh
        >>> d.gaincaps = M3_GainCaps.M3_C30sh | M3_GainCaps.M3_C15sh
        >>> d.gaincaps
        C15sh, C30sh
        """
        res = [Mythen3GainCapsWrapper(it) for it in self.getGainCaps()]
        return res

    @gaincaps.setter
    def gaincaps(self, caps):
        #convert to int if called with Wrapper
        if isinstance(caps, Mythen3GainCapsWrapper):
            self.setGainCaps(caps.value)
        elif isinstance(caps, dict):
            corr = {}
            for key, value in caps.items():
                if isinstance(value, Mythen3GainCapsWrapper):
                    corr[key] = value.value
                else:
                    corr[key] = value
            ut.set_using_dict(self.setGainCaps, corr)
        else:
            self.setGainCaps(caps)


    @property
    def exptime(self):
        """
        Exposure time, accepts either a value in seconds or datetime.timedelta

        Note
        -----
        [Mythen3] sets exposure time to all gate signals in auto and trigger mode (internal gating). To specify gateIndex, use getExptime or setExptime.
        
        :getter: always returns in seconds. To get in DurationWrapper, use getExptime

        Example
        -----------
        >>> # setting directly in seconds
        >>> d.exptime = 1.05
        >>>
        >>> # setting directly in seconds
        >>> d.exptime = 5e-07
        >>> 
        >>> # using timedelta (up to microseconds precision)
        >>> from datatime import timedelta
        >>> d.exptime = timedelta(seconds = 1, microseconds = 3)
        >>> 
        >>> # using DurationWrapper to set in seconds
        >>> from slsdet import DurationWrapper
        >>> d.exptime = DurationWrapper(1.2)
        >>> 
        >>> # using DurationWrapper to set in ns
        >>> t = DurationWrapper()
        >>> t.set_count(500)
        >>> d.exptime = t
        >>>
        >>> # to get in seconds
        >>> d.exptime
        181.23
        >>> 
        >>> d.getExptime()
        [sls::DurationWrapper(total_seconds: 1e-08 count: 10)]
        """
        if self.type == detectorType.MYTHEN3:
            res = self.getExptimeForAllGates()
        else:
            res = self.getExptime()
        return reduce_time(res)

    @exptime.setter
    def exptime(self, t):
        if self.type == detectorType.MYTHEN3 and is_iterable(t) and not isinstance(t,dict):
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

        :getter: always returns in seconds. To get in DurationWrapper, use getPeriod

        Example
        -----------
        >>> # setting directly in seconds
        >>> d.period = 1.05
        >>>
        >>> # setting directly in seconds
        >>> d.period = 5e-07
        >>> 
        >>> # using timedelta (up to microseconds precision)
        >>> from datatime import timedelta
        >>> d.period = timedelta(seconds = 1, microseconds = 3)
        >>> 
        >>> # using DurationWrapper to set in seconds
        >>> from slsdet import DurationWrapper
        >>> d.period = DurationWrapper(1.2)
        >>> 
        >>> # using DurationWrapper to set in ns
        >>> t = DurationWrapper()
        >>> t.set_count(500)
        >>> d.period = t
        >>>
        >>> # to get in seconds
        >>> d.period
        181.23
        >>> 
        >>> d.getExptime()
        sls::DurationWrapper(total_seconds: 181.23 count: 181230000000)
        """
        res = self.getPeriod()
        return reduce_time(res)

    @period.setter
    def period(self, t):
        ut.set_time_using_dict(self.setPeriod, t)


    @property
    @element
    def periodl(self):
        """
        [Gotthard][Jungfrau][Moench][CTB][Mythen3][Gotthard2][Xilinx Ctb] Period left for current frame.

        Note
        -----
        [Gotthard2] only in continuous mode.

        :getter: always returns in seconds. To get in DurationWrapper, use getPeriodLeft
        :setter: Not Implemented

        Example
        -----------
        >>> d.periodl
        181.23
        >>> d.getPeriodLeft()
        [sls::DurationWrapper(total_seconds: 181.23 count: 181230000000)]
        """
        return self.getPeriodLeft()

    @property
    @element
    def delay(self):
        """
        [Gotthard][Jungfrau][Moench][CTB][Mythen3][Gotthard2][Xilinx Ctb] Delay after trigger, accepts either a value in seconds, DurationWrapper or datetime.timedelta

        :getter: always returns in seconds. To get in DurationWrapper, use getDelayAfterTrigger

        Example
        -----------
        >>> # setting directly in seconds
        >>> d.delay = 1.05
        >>>
        >>> # setting directly in seconds
        >>> d.delay = 5e-07
        >>> 
        >>> # using timedelta (up to microseconds precision)
        >>> from datatime import timedelta
        >>> d.delay = timedelta(seconds = 1, microseconds = 3)
        >>> 
        >>> # using DurationWrapper to set in seconds
        >>> from slsdet import DurationWrapper
        >>> d.delay = DurationWrapper(1.2)
        >>> 
        >>> # using DurationWrapper to set in ns
        >>> t = DurationWrapper()
        >>> t.set_count(500)
        >>> d.delay = t
        >>>
        >>> # to get in seconds
        >>> d.delay
        181.23
        >>> 
        >>> d.getDelayAfterTrigger()
        sls::DurationWrapper(total_seconds: 181.23 count: 181230000000)
        """
        return ut.reduce_time(self.getDelayAfterTrigger())

    @delay.setter
    def delay(self, t):
        ut.set_time_using_dict(self.setDelayAfterTrigger, t)

    @property
    @element
    def delayl(self):
        """
        [Gotthard][Jungfrau][Moench][CTB][Mythen3][Gotthard2][Xilinx Ctb] Delay left after trigger during acquisition, accepts either a value in seconds, datetime.timedelta or DurationWrapper

        Note
        -----
        [Gotthard2] only in continuous mdoe.

        :getter: always returns in seconds. To get in DurationWrapper, use getDelayAfterTriggerLeft
        :setter: Not Implemented

        Example
        -----------
        >>> d.delayl
        181.23
        >>> d.getDelayAfterTriggerLeft()
        [sls::DurationWrapper(total_seconds: 181.23 count: 181230000000)]
        """
        return ut.reduce_time(self.getDelayAfterTriggerLeft())

    def start(self):
        """Start detector acquisition. Status changes to RUNNING or WAITING and automatically returns to idle at the end of acquisition."""
        self.startDetector()

    def clearbusy(self):
        """If acquisition aborted during acquire command, use this to clear acquiring flag in shared memory before starting next acquisition"""
        self.clearAcquiringFlag()

    def rx_start(self):
        """Starts receiver listener for detector data packets and create a data file (if file write enabled)."""
        self.startReceiver()

    def rx_stop(self):
        """Stops receiver listener for detector data packets and closes current data file (if file write enabled)."""
        self.stopReceiver()

    def stop(self):
        """Abort detector acquisition. Status changes to IDLE or STOPPED. Goes to stop server. """
        self.stopDetector()

    # Time
    @property
    @element
    def rx_framescaught(self):
        """Number of frames caught by each port in receiver."""
        return self.getFramesCaught()

    @property
    @element
    def nextframenumber(self):
        """[Eiger][Jungfrau][Moench][CTB][Xilinx CTB][Gotthard2] Next frame number. Stopping acquisition might result in different frame numbers for different modules. So, after stopping, next frame number (max + 1) is set for all the modules afterwards."""
        return self.getNextFrameNumber()

    @nextframenumber.setter
    def nextframenumber(self, value):
        ut.set_using_dict(self.setNextFrameNumber, value)

    @property
    @element
    def txdelay(self):
        """
        [Eiger][Jungfrau][Moench][Mythen3] Set transmission delay for all modules in the detector using the step size provided.
        
        Note
        ----
        Sets up the following for every module:\n
        \t\t[Eiger] txdelay_left to (2 * mod_index * n_delay), \n
        \t\t[Eiger] txdelay_right to ((2 * mod_index + 1) * n_delay) and \n
        \t\t[Eiger] txdelay_frame to (2 *num_modules * n_delay)  \n
        \t\t[Jungfrau][Moench][Mythen3] txdelay_frame to (num_modules * n_delay)\n\n
        Please refer txdelay_left, txdelay_right and txdelay_frame for details.
        """
        return self.getTransmissionDelay()

    @txdelay.setter
    def txdelay(self, args):
        ut.set_using_dict(self.setTransmissionDelay, args)

    @property
    @element
    def txdelay_frame(self):
        """
        [Eiger][Jungfrau][Moench][Mythen3] Transmission delay of first udp packet being streamed out of the module.\n

        Note
        ----
        [Jungfrau][Moench] [0-31] Each value represents 1 ms. \n 
        [Eiger] Additional delay to txdelay_left and txdelay_right. Each value represents 10ns. Typical value is 50000. \n
        [Mythen3] [0-16777215] Each value represents 8 ns (125 MHz clock), max is 134 ms.
        """
        return self.getTransmissionDelayFrame()

    @txdelay_frame.setter
    def txdelay_frame(self, args):
        ut.set_using_dict(self.setTransmissionDelayFrame, args)

    @property
    @element
    def txdelay_left(self):
        """[Eiger] Transmission delay of first packet in an image being streamed out of the module's left UDP port. 

        Note
        -----
        Each value represents 10ns. Typical value is 50000.
        """
        return self.getTransmissionDelayLeft()

    @txdelay_left.setter
    def txdelay_left(self, args):
        ut.set_using_dict(self.setTransmissionDelayLeft, args)

    @property
    @element
    def txdelay_right(self):
        """
        [Eiger] Transmission delay of first packet in an image being streamed out of the module's right UDP port. 

        Note
        ----
        Each value represents 10ns. Typical value is 50000.
        """
        return self.getTransmissionDelayRight()

    @txdelay_right.setter
    def txdelay_right(self, args):
        ut.set_using_dict(self.setTransmissionDelayRight, args)

    @property
    @element
    def use_receiver(self):
        return self.getUseReceiverFlag()

    @property
    @element
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
        return self.getRxHostname()

    @rx_hostname.setter
    def rx_hostname(self, hostname):
        args = hostname_list(hostname)
        self.setRxHostname(args)

    @property
    @element
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
        return self.getRxPort()

    @rx_tcpport.setter
    def rx_tcpport(self, port):
        ut.validate_port(port)
        ut.set_using_dict(self.setRxPort, port)

    @property
    @element
    def rx_fifodepth(self):
        """Sets the number of frames in the receiver fifo depth (buffer between listener and writer threads)."""
        return self.getRxFifoDepth()

    @rx_fifodepth.setter
    def rx_fifodepth(self, frames):
        ut.set_using_dict(self.setRxFifoDepth, frames)

    @property
    @element
    def rx_silent(self):
        """When enabled, switches off receiver text output during acquisition. """
        return self.getRxSilentMode()

    @rx_silent.setter
    def rx_silent(self, value):
        ut.set_using_dict(self.setRxSilentMode, value)

    @property
    @element
    def rx_discardpolicy(self):
        """
        Frame discard policy of receiver. 
        Enum: frameDiscardPolicy
        
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
        return self.getRxFrameDiscardPolicy()

    @rx_discardpolicy.setter
    def rx_discardpolicy(self, policy):
        ut.set_using_dict(self.setRxFrameDiscardPolicy, policy)

    @property
    @element
    def rx_padding(self):
        """Partial frames padding enable in the receiver. 
        
        Note
        ------
        Default: enabled \n
        Disabling is fastest.
        """
        return self.getPartialFramesPadding()

    @rx_padding.setter
    def rx_padding(self, policy):
        ut.set_using_dict(self.setPartialFramesPadding, policy)

    @property
    @element
    def rx_lock(self):
        """Lock the receiver to a specific IP"""
        return self.getRxLock()

    @rx_lock.setter
    def rx_lock(self, value):
        ut.set_using_dict(self.setRxLock, value)

    @property
    @element
    def rx_lastclient(self):
        """Client IP Address that last communicated with the receiver."""
        return self.getRxLastClientIP()

    # FILE

    @property
    @element
    def numinterfaces(self):
        """[Jungfrau][Moench][Gotthard2] Number of udp interfaces to stream data from detector. Default is 1.
        
        Note
        -----
        Also enables second interface in receiver for listening (Writes a file per interface if writing enabled). \n
        Also restarts client and receiver zmq sockets if zmq streaming enabled. \n
        [Gotthard2] second interface enabled to send veto information via 10Gbps for debugging. By default, if veto enabled, it is sent via 2.5 gbps interface.
        """
        return self.getNumberofUDPInterfaces()

    @numinterfaces.setter
    def numinterfaces(self, value):
        ut.set_using_dict(self.setNumberofUDPInterfaces, value)

    @property
    @element
    def fformat(self):
        """ File format of data file in receiver. 
        Enum: fileFormat
    
        Note
        -----
        Options: BINARY, HDF5
        Default: BINARY
        For HDF5, package must be compiled with HDF5 flags. Default is binary. 

        Example
        --------
        d.fformat = fileFormat.BINARY

        """
        return self.getFileFormat()

    @fformat.setter
    def fformat(self, format):
        ut.set_using_dict(self.setFileFormat, format)

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
    @element
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
        return self.getFileNamePrefix()

    @fname.setter
    def fname(self, file_name):
        ut.set_using_dict(self.setFileNamePrefix, file_name)

    @property
    @element
    def fpath(self):
        """Directory where output data files are written in receiver. Default is "/".
        
        Note
        ----
        If path does not exist and fwrite enabled, it will try to create it at start of acquisition.
        
        Example
        --------
        d.fpath = '/tmp/run_20201705'
        """
        return ut.lpath(self.getFilePath())

    @fpath.setter
    def fpath(self, path):
        path = ut.make_string_path(path)
        ut.set_using_dict(self.setFilePath, path)

    @property
    @element
    def fwrite(self):
        """Enable or disable receiver file write. Default is disabled. """
        return self.getFileWrite()

    @fwrite.setter
    def fwrite(self, value):
        ut.set_using_dict(self.setFileWrite, value)

    @property
    @element
    def foverwrite(self):
        """Enable or disable receiver file overwriting. Default is enabled. """
        return self.getFileOverWrite()

    @foverwrite.setter
    def foverwrite(self, value):
        ut.set_using_dict(self.setFileOverWrite, value)

    @property
    def fmaster(self):
        """Enable or disable receiver master file. Default is enabled."""
        return self.getMasterFileWrite()

    @fmaster.setter
    def fmaster(self, enable):
        self.setMasterFileWrite(enable)

    @property
    @element
    def rx_framesperfile(self):
        """Sets the number of frames per file in receiver in an acquisition. 
        
        Note
        -----
        Default: depends on detector type. \n
        0 is infinite or all frames in single file.
        """
        return self.getFramesPerFile()

    @rx_framesperfile.setter
    def rx_framesperfile(self, n_frames):
        ut.set_using_dict(self.setFramesPerFile, n_frames)

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
    @element
    def rx_zmqfreq(self):
        """Frequency of frames streamed out from receiver via zmq.
        
        Note
        -----
        Default: 1, Means every frame is streamed out. \n
        If 2, every second frame is streamed out. \n
        If 0, streaming timer is the timeout, after which current frame is sent out. (default timeout is 200 ms). Usually used for gui purposes.
        """
        return self.getRxZmqFrequency()

    @rx_zmqfreq.setter
    def rx_zmqfreq(self, nth_frame):
        ut.set_using_dict(self.setRxZmqFrequency, nth_frame)

    @property
    @element
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
        return self.getRxZmqPort()

    @rx_zmqport.setter
    def rx_zmqport(self, port):
        if isinstance(port, int):
            ut.validate_port(port)
            self.setRxZmqPort(port, -1)
        elif isinstance(port, dict):
            ut.validate_port(port)
            ut.set_using_dict(self.setRxZmqPort, port)
        elif is_iterable(port):
            for i, p in enumerate(port):
                ut.validate_port(p)
                self.setRxZmqPort(p, i)
        else:
            raise ValueError("Unknown argument type")

    @property
    @element
    def zmqport(self):
        """
        Port number to listen to zmq data streamed out from receiver or intermediate process.
        
        Note
        -----
        Also restarts client zmq streaming if enabled. \n
        Default connects to receiver zmq streaming out port (30001). \n
        Must be different for every detector (and udp port). \n
        Multi command will automatically increment for individual modules, use setClientZmqPort. 
        
        Example
        --------
        >>> d.zmqport
        [30001, 30003]
        >>> d.zmqport = 30002
        >>> d.zmqport = [30002, 30004] #Set ports for the two first detectors
        """
        return self.getClientZmqPort()

    @zmqport.setter
    def zmqport(self, port):
        if isinstance(port, int):
            ut.validate_port(port)
            self.setClientZmqPort(port, -1)
        elif isinstance(port, dict):
            ut.validate_port(port)
            ut.set_using_dict(self.setClientZmqPort, port)
        elif is_iterable(port):
            for i, p in enumerate(port):
                ut.validate_port(p)
                self.setClientZmqPort(p, i)
        else:
            raise ValueError("Unknown argument type")

    @property
    @element
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
        return self.getClientZmqIp()

    @zmqip.setter
    def zmqip(self, ip):
        ip = ut.make_ip(ip) #Convert from int or string to IpAddr
        ut.set_using_dict(self.setClientZmqIp, ip)


    @property
    def zmqhwm(self):
        """
        Client's zmq receive high water mark. Default is the zmq library's default (1000), can also be set here using -1. 
        This is a high number and can be set to 2 for gui purposes. 
        One must also set the receiver's send high water mark to similar value. Final effect is sum of them.
	    Setting it via command line is useful only before zmq enabled (before opening gui).
        """
        return self.getClientZmqHwm()

    @zmqhwm.setter
    def zmqhwm(self, n_frames):
        self.setClientZmqHwm(n_frames)

    @property
    def rx_zmqhwm(self):
        """
        Receiver's zmq send high water mark. Default is the zmq library's default (1000). This is a high number and can be set to 2 for gui purposes. One must also set the client's receive high water mark to similar value. Final effect is sum of them. Also restarts receiver zmq streaming if enabled. Can set to -1 to set default value.
        """
        return self.getRxZmqHwm()

    @rx_zmqhwm.setter
    def rx_zmqhwm(self, n_frames):
        self.setRxZmqHwm(n_frames)

    @property
    @element
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
        return self.getDestinationUDPIP()

    @udp_dstip.setter
    def udp_dstip(self, ip):
        if ip == "auto":
            ip = socket.gethostbyname(self.rx_hostname)
        ip = ut.make_ip(ip)
        ut.set_using_dict(self.setDestinationUDPIP, ip)


    @property
    @element
    def udp_dstip2(self):
        """
        [Jungfrau][Moench][Gotthard2] Ip address of the receiver (destination) udp interface 2.
        
        Note
        ----
        [Jungfrau][Moench] bottom half \n
        [Gotthard2] veto debugging \n
        If 'auto' used, then ip is set to ip of rx_hostname. \n
        To set IPs for individual modules, use setDestinationUDPIP2. 
        
        Example
        ------
        >>> d.udp_dstip2 = '10.1.1.185'
        >>> d.udp_dstip2
        10.1.1.185
        """
        return self.getDestinationUDPIP2()

    @udp_dstip2.setter
    def udp_dstip2(self, ip):
        if ip == "auto":
            ip = socket.gethostbyname(self.rx_hostname)
        ip = ut.make_ip(ip)
        ut.set_using_dict(self.setDestinationUDPIP2, ip)

    @property
    @element
    def udp_dstmac(self):
        """
        Mac address of the receiver (destination) udp interface. 
        
        Note
        ----
        Not mandatory to set as udp_dstip retrieves it from slsReceiver process but must be set if you use a custom receiver (not slsReceiver). \n
        To set MACs for individual modules, use setDestinationUDPMAC. 
        Use router mac if router between detector and receiver.
        
        Example
        -------
        >>> d.udp_dstmac = '00:1b:31:01:8a:de'
        d.udp_dstmac
        00:1b:31:01:8a:de
        """
        return self.getDestinationUDPMAC()

    @udp_dstmac.setter
    def udp_dstmac(self, mac):
        mac = ut.make_mac(mac)
        ut.set_using_dict(self.setDestinationUDPMAC, mac)

    @property
    @element
    def udp_dstmac2(self):
        """
        [Jungfrau][Moench][Gotthard2] Mac address of the receiver (destination) udp interface 2.
        
        Note
        ----
        Not mandatory to set as udp_dstip2 retrieves it from slsReceiver process but must be set if you use a custom receiver (not slsReceiver).  \n
        To set MACs for individual modules, use setDestinationUDPMAC2. \n
        [Jungfrau][Moench] bottom half \n
        [Gotthard2] veto debugging \n
        Use router mac if router between detector and receiver.
        
        Example
        ------
        >>> d.udp_dstmac2 = '00:1b:31:01:8a:de'
        d.udp_dstmac2
        00:1b:31:01:8a:de
        """
        return self.getDestinationUDPMAC2()

    @udp_dstmac2.setter
    def udp_dstmac2(self, mac):
        mac = ut.make_mac(mac)
        ut.set_using_dict(self.setDestinationUDPMAC2, mac)

    @property
    @element
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
        return self.getSourceUDPMAC()

    @udp_srcmac.setter
    def udp_srcmac(self, mac):
        mac = ut.make_mac(mac)
        ut.set_using_dict(self.setSourceUDPMAC, mac)

    @property
    @element
    def udp_srcmac2(self):
        """
        [Jungfrau][Moench][Gotthard2] Mac address of the receiver (source) udp interface 2. 
        
        Note
        ----
        [Jungfrau][Moench] bottom half \n
        [Gotthard2] veto debugging \n
        To set MACs for individual modules, use setSourceUDPMAC2. 
        
        Example
        -------
        >>> d.udp_srcmac2 = '00:1b:31:01:8a:de'
        d.udp_srcmac2
        00:1b:31:01:8a:de
        """
        return self.getSourceUDPMAC2()

    @udp_srcmac2.setter
    def udp_srcmac2(self, mac):
        mac = ut.make_mac(mac)
        ut.set_using_dict(self.setSourceUDPMAC2, mac)

    @property
    @element
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
        return self.getSourceUDPIP()

    @udp_srcip.setter
    def udp_srcip(self, ip):
        if ip == "auto":
            if self.type == detectorType.GOTTHARD:
                raise NotImplementedError('Auto for udp_srcip cannot be used for GotthardI')
            ip = socket.gethostbyname(self.hostname[0])        
        ip = ut.make_ip(ip)
        ut.set_using_dict(self.setSourceUDPIP, ip)

    @property
    @element
    def udp_srcip2(self):
        """
        [Jungfrau][Moench][Gotthard2] Ip address of the detector (source) udp interface 2. 
        
        Note
        -----
        [Jungfrau][Moench] bottom half \n
        [Gotthard2] veto debugging \n
        Must be same subnet as destination udp ip2.\n
        To set IPs for individual modules, use setSourceUDPIP2. 
        
        Example
        -------
        >>> d.udp_srcip2 = '192.168.1.127'
        >>> d.udp_srcip2
        192.168.1.127
        """
        return self.getSourceUDPIP2()

    @udp_srcip2.setter
    def udp_srcip2(self, ip):
        if ip == "auto":
            ip = socket.gethostbyname(self.hostname)      
        ip = ut.make_ip(ip)
        ut.set_using_dict(self.setSourceUDPIP2, ip)

    @property
    @element
    def udp_dstport(self):
        """
        Port number of the receiver (destination) udp interface. 
        
        Note
        ----
        Default is 50001. \n
        Ports for each module is calculated (incremented by 1 if no 2nd interface) \n
        To set ports for individual modules, use setDestinationUDPPort.
        """
        return self.getDestinationUDPPort()

    @udp_dstport.setter
    def udp_dstport(self, port):
        ut.validate_port(port)
        ut.set_using_dict(self.setDestinationUDPPort, port)

    @property
    @element
    def udp_dstport2(self):
        """
        Port number of the receiver (destination) udp interface. 
        
        Note
        ----
        Default is 50002. \n
        [Eiger] right half \n
        [Jungfrau][Moench] bottom half \n
        [Gotthard2] veto debugging \n
        Ports for each module is calculated (incremented by 2) \n
        To set ports for individual modules, use setDestinationUDPPort2.
        """
        return self.getDestinationUDPPort2()

    @udp_dstport2.setter
    def udp_dstport2(self, port):
        ut.validate_port(port)
        ut.set_using_dict(self.setDestinationUDPPort2, port)

    @property
    @element
    def highvoltage(self):
        """High voltage to the sensor in Voltage.

        Note
        -----
        [Gotthard] 0, 90, 110, 120, 150, 180, 200 \n
        [Eiger][Mythen3][Gotthard2] 0 - 200 \n
        [Jungfrau][Moench][Ctb] 0, 60 - 200
        """
        return self.getHighVoltage()

    @highvoltage.setter
    def highvoltage(self, v):
        ut.set_using_dict(self.setHighVoltage, v)

    @property
    def user(self):
        """
        Retrieve user details from shared memory (hostname, type, PID, User, Date)
        """
        return self.getUserDetails()

    @property
    @element
    def settingspath(self):
        """[Eiger] Directory where settings files are loaded from/to."""
        return ut.make_path(self.getSettingsPath())

    @settingspath.setter
    def settingspath(self, path):
        path = ut.make_string_path(path)
        ut.set_using_dict(self.setSettingsPath, path)

    @property
    @element
    def status(self):
        """Gets detector status. 
        Enum: runStatus
        
        Note
        -----
        Options: IDLE, ERROR, WAITING, RUN_FINISHED, TRANSMITTING, RUNNING, STOPPED \n
        Goes to stop server.
        >>> d.status
        runStatus.IDLE
        """
        return self.getDetectorStatus()

    @property
    @element
    def rx_status(self):
        """Gets receiver listener status. 
        Enum: runStatus
        
        Note
        -----
        Options: IDLE, TRANSMITTING, RUNNING
        >>> d.rx_status
        runStatus.IDLE
        """
        return self.getReceiverStatus()

    @property
    @element
    def rx_udpsocksize(self):
        """UDP socket buffer size in receiver. Tune rmem_default and rmem_max accordingly. Max size: INT_MAX/2."""
        return self.getRxUDPSocketBufferSize()

    @rx_udpsocksize.setter
    def rx_udpsocksize(self, buffer_size):
        ut.set_using_dict(self.setRxUDPSocketBufferSize, buffer_size)

    @property
    @element
    def rx_realudpsocksize(self):
        """Gets actual udp socket buffer size. Double the size of rx_udpsocksize due to kernel bookkeeping."""
        return self.getRxRealUDPSocketBufferSize()

    @property
    def trimbits(self):
        """
        [Eiger][Mythen3] Loads/Saves custom trimbit file to detector. 
        
        Note
        -----
        If no extension specified, serial number of each module is attached.

        :setter: Loads the trimbit file to detector
        :getter: Saves the trimbits from the detector to file. Not implemented with 'trimbits'. Use saveTrimbits().

        Example
        -------
        >>> d.trimbits = '/path_to_file/noise'
        - 14:53:27.931 INFO: Settings file loaded: /path_to_file/noise.sn000
        """
        raise NotImplementedError('trimbits is set only. Use saveTrimbits()')

    @trimbits.setter
    def trimbits(self, fname):
        fname = ut.make_string_path(fname)
        ut.set_using_dict(self.loadTrimbits, fname)

    @property
    @element
    def trimval(self):
        """
        [Eiger][Mythen3] Set all trimbits to this value. Returns -1 if all trimbits are different values.
        """
        return self.getAllTrimbits()

    @trimval.setter
    def trimval(self, value):
        ut.set_using_dict(self.setAllTrimbits, value)

    @property
    @element
    def fliprows(self):
        """
        [Eiger] flips rows paramater sent to slsreceiver to stream as json parameter to flip rows in gui. \n
        [Jungfrau] flips rows in the detector itself. For bottom module and number of interfaces must be set to 2. slsReceiver and slsDetectorGui does not handle.
        """
        return self.getFlipRows()

    @fliprows.setter
    def fliprows(self, value):
        ut.set_using_dict(self.setFlipRows, value)


    @property
    @element
    def master(self):
        """
        [Eiger][Gotthard2][Jungfrau][Moench] Sets (half) module to master and other(s) to slaves.\n
        [Gotthard][Gotthard2][Mythen3][Eiger][Jungfrau][Moench] Gets if the current (half) module is master.
        """
        return self.getMaster()

    @master.setter
    def master(self, value):
        ut.set_using_dict(self.setMaster, value)

    @property
    @element
    def sync(self):
        """
        [Jungfrau][Moench] Enables or disables synchronization between modules.

        Note
        ----
        Sync mode requires at least one master configured. Also requires flatband cabling between master and slave with termination board.

        """
        return self.getSynchronization()

    @sync.setter
    def sync(self, value):
        ut.set_using_dict(self.setSynchronization, value)

    @property
    @element
    def badchannels(self):
        """
        [fname|none|0]\n\t[Gotthard2][Mythen3] Sets the bad channels (from file of bad channel numbers) to be masked out. None or 0 unsets all the badchannels.\n
        [Mythen3] Also does trimming
        """
        return self.getBadChannels()

    @badchannels.setter
    def badchannels(self, value):
        ut.set_using_dict(self.setBadChannels, value)

    @property
    @element
    def row(self):
        """
        Set Detector row (udp header) to value. Gui uses it to rearrange for complete image.
        """
        return self.getRow()

    @row.setter
    def row(self, value):
        ut.set_using_dict(self.setRow, value)

    @property
    @element
    def column(self):
        """
        Set Detector column (udp header) to value. Gui uses it to rearrange for complete image.
        """
        return self.getColumn()

    @column.setter
    def column(self, value):
        ut.set_using_dict(self.setColumn, value)

    @property
    @element
    def lock(self):
        """Lock detector to one client IP, 1 locks, 0 unlocks. Default is unlocked."""
        return self.getDetectorLock()

    @lock.setter
    def lock(self, value):
        ut.set_using_dict(self.setDetectorLock, value)

    @property
    @element
    def rx_lock(self):
        """Lock receiver to one client IP, 1 locks, 0 unlocks. Default is unlocked."""
        return self.getRxLock()

    @rx_lock.setter
    def rx_lock(self, value):
        ut.set_using_dict(self.setRxLock, value)

    @property
    @element
    def scanerrmsg(self):
        """Gets Scan error message if scan ended in error for non blocking acquisitions."""
        return self.getScanErrorMessage()

    @property
    @element
    def rx_zmqstartfnum(self):
        """
        The starting frame index to stream out. 
        
        Note
        ----
        0 by default, which streams the first frame in an acquisition, and then depending on the rx zmq frequency/ timer.
        """
        return self.getRxZmqStartingFrame()

    @rx_zmqstartfnum.setter
    def rx_zmqstartfnum(self, value):
        ut.set_using_dict(self.setRxZmqStartingFrame, value)

    @property
    @element
    def lastclient(self):
        """Get Client IP Address that last communicated with the detector."""
        return self.getLastClientIP()

    @property
    def reg(self):
        """
        Reads/writes to a 32 bit register.

        Note
        -----
        Advanced user Function! \n
        Goes to stop server. Hence, can be called while calling blocking acquire(). \n
        [Eiger] Address is +0x100 for only left, +0x200 for only right.
        """
        return self._register

    @property
    def slowadc(self):
        """
        [Ctb] Slow ADC channel in uV of all channels or specific ones from 0-7.
        
        Example
        -------
        >>> d.slowadc
        0: 0 uV
        1: 0 uV
        2: 0 uV
        3: 0 uV
        4: 0 uV
        5: 0 uV
        6: 0 uV
        7: 0 uV
        >>> d.slowadc[3]
        0
        """
        return SlowAdcProxy(self)

    @property
    def daclist(self):
        """
        List of enums/names for every dac for this detector

        :setter: Only implemented for Chiptestboard
        
        """
        return self.getDacNames()

    @daclist.setter
    def daclist(self, value):
        self.setDacNames(value)

    @property
    def adclist(self):
        """
        [Chiptestboard] List of names for every adc for this board. 32 adcs
        """
        return self.getAdcNames()

    @adclist.setter
    def adclist(self, value):
        self.setAdcNames(value)

    @property
    def signallist(self):
        """
        [Chiptestboard] List of names for every io signal for this board. 64 signals        
        """
        return self.getSignalNames()

    @signallist.setter
    def signallist(self, value):
        self.setSignalNames(value)

    @property
    def powerlist(self):
        """
        [Chiptestboard] List of names for every power for this board. 5 power supply
        
        """
        return self.getPowerNames()

    @powerlist.setter
    def powerlist(self, value):
        self.setPowerNames(value)

    @property
    def slowadclist(self):
        """
        [Chiptestboard] List of names for every slowadc for this board. 8 slowadc
        
        """
        return self.getSlowADCNames()

    @slowadclist.setter
    def slowadclist(self, value):
        self.setSlowADCNames(value)
        
    @property
    def dacvalues(self):
        """Gets the dac values for every dac for this detector."""
        return {
            dac.name.lower(): element_if_equal(np.array(self.getDAC(dac, False)))
            for dac in self.getDacList()
        }

    @property
    def powervalues(self):
        """[Chiptestboard] Gets the power values for every power for this detector."""
        return {
            power.name.lower(): element_if_equal(np.array(self.getPower(power)))
            for power in self.getPowerList()
        }

    @property
    def slowadcvalues(self):
        """[Chiptestboard] Gets the slow adc values for every slow adc for this detector."""
        return {
            slowadc.name.lower(): element_if_equal(np.array(self.getSlowADC(slowadc)))
            for slowadc in self.getSlowADCList()
        }

    @property
    def timinglist(self):
        """Gets the list of timing modes (timingMode) for this detector."""
        return self.getTimingModeList()

    @property
    def readoutspeedlist(self):
        """List of readout speed levels implemented for this detector."""
        return self.getReadoutSpeedList()

    @property
    def templist(self):
        """List of temperature enums (dacIndex) implemented for this detector."""
        return self.getTemperatureList()

    @property
    def tempvalues(self):
        """Gets the temp values for every temp for this detector."""
        return {
            t.name.lower(): element_if_equal(np.array(self.getTemperature(t)))
            for t in self.getTemperatureList()
        }

    @property
    def settingslist(self):
        """List of settings implemented for this detector."""
        return self.getSettingsList()

    @property
    def adcreg(self):
        """[Jungfrau][Moench][Ctb][Gotthard] Writes to an adc register 

        Note
        -----
        Advanced user Function!

        :getter: Not implemented     
        """
        return self._adc_register

    @property
    @element
    def adcinvert(self):
        """[Ctb][Jungfrau][Moench] ADC Inversion Mask.
        
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
        """
        [Gotthard][Jungfrau][Moench][Mythen3][Gotthard2][CTB][Xilinx CTB] Number of triggers left in acquisition.\n
        
        Note
        ----
        Only when external trigger used.

        :setter: Not Implemented
        """
        return self.getNumberOfTriggersLeft()

    @property
    @element
    def frametime(self):
        """[Jungfrau][Moench][Mythen3][Gotthard2][CTB][Xilinx Ctb] Timestamp at a frame start.
        
        Note
        ----
        [Gotthard2] not in burst and auto mode.
        """
        return self.getMeasurementTime()

    @property
    @element
    def led(self):
        """[Ctb] Switches on/off all LEDs. Default is enabled. """
        return self.getLEDEnable()

    @led.setter
    def led(self, value):
        ut.set_using_dict(self.setLEDEnable, value)

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
        type = "Unknown"
        firmware = "Unknown"
        detectorserver = "Unknown"
        kernel = "Unknown"
        hardware = "Unknown"    
        receiverversion = "Unknown"
        eiger = False
        firmware_febl = "Unknown"
        firmware_febr = "Unknown"
        firmware_beb = "Unknown"
        receiver_in_shm = False

        release = self.packageversion
        client = self.clientversion

        if self.nmod != 0:
            # shared memory has detectors
            type = self.type
            eiger = (self.type == detectorType.EIGER)
            receiver_in_shm = self.use_receiver
            if receiver_in_shm:
                # cannot connect to receiver
                try:
                    receiverversion = self.rx_version
                except Exception as e:
                    pass
            # cannot connect to Detector
            try:
                firmware = self.firmwareversion
                detectorserver = self.detectorserverversion
                kernel = self.kernelversion
                hardware = self.hardwareversion
                if eiger:
                    firmware_beb = self.firmwareversion
                    firmware_febl = self.getFrontEndFirmwareVersion(slsDetectorDefs.fpgaPosition.FRONT_LEFT)
                    firmware_febr = self.getFrontEndFirmwareVersion(slsDetectorDefs.fpgaPosition.FRONT_RIGHT)
            except Exception as e:
                pass

        version_list = {'type': {type},
                'package': {release}, 
                'client': {client}}
        if eiger:
            version_list ['firmware (Beb)'] = {firmware_beb}
            version_list ['firmware(Febl)'] = {firmware_febl}
            version_list ['firmware (Febr)'] = {firmware_febr}
        else:
            version_list ['firmware'] = {firmware}
        version_list ['detectorserver'] = {detectorserver}
        version_list ['kernel'] = kernel
        version_list ['hardware'] = hardware
        if receiver_in_shm:
            version_list ['receiver'] = {receiverversion}

        return version_list

    @property
    def virtual(self):
        """
        Setup with n virtual servers running on localhost starting with control port p
        
        Note
        ----
        Every virtual server will have a stop port (control port + 1)
        
        Example
        ---------
        >>> d.virtual = n, p
        """
        raise NotImplementedError('Virtual is set only')

    @virtual.setter
    def virtual(self, args):
        n_detectors, starting_port = args
        ut.validate_port(starting_port)
        self.setVirtualDetectorServers(n_detectors, starting_port)

    

    @property
    def packageversion(self):
        """Package version (git branch)."""
        return self.getPackageVersion()

    @property
    def ratecorr(self):
        """ 
        [Eiger] Custom dead time correction constant in ns. 0 will unset rate correction.

        Note
        -----
        To set default rate correction from trimbit file, use setDefaultRateCorrection

        Known Issue:

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
    @element
    def readoutspeed(self):
        """
        [Eiger][Jungfrau|Gotthard2] Readout speed of chip. 
        Enum: speedLevel
        
        Note
        -----
        [Jungfrau][Moench][Mythen3] FULL_SPEED, HALF_SPEED (Default), QUARTER_SPEED
        [Eiger] FULL_SPEED (Default), HALF_SPEED, QUARTER_SPEED
        [Moench] FULL_SPEED (Default), HALF_SPEED, QUARTER_SPEED
        [Gottthard2] G2_108MHZ (Default), G2_144MHZ
        [Jungfrau] FULL_SPEED option only available from v2.0 boards and is recommended to set number of interfaces to 2.  \n
        Also overwrites adcphase to recommended default.
        """
        return element_if_equal(self.getReadoutSpeed())

    @readoutspeed.setter
    def readoutspeed(self, value):
        ut.set_using_dict(self.setReadoutSpeed, value)

    @property
    def rx_jsonpara(self):
        """
        Set the receiver additional json parameter. 
        
        Note
        ----
        Use only if to be processed by an intermediate user process listening to receiver zmq packets, such as Moench \n
        If not found, the pair is appended. Empty value deletes parameter. Max 20 characters for each key/value.\n
        On setting the value is automatically, it is converted to a string. 
        
        Example
        -----------
        >>> d.rx_jsonpara['emin']
        '4500'
        >>> d.rx_jsonpara['emin'] = 5000
        >>> d.rx_jsonpara
        emax: 30
        emin: 5000
        """
        return JsonProxy(self)


    @property
    @element
    def rx_jsonaddheader(self):
        """
        Additional json header to be streamed out from receiver via zmq. 
        
        Note
        -----
        Default is empty. Max 20 characters for each key/value\n 
        Use only if to be processed by an intermediate user process listening to receiver zmq packets, such as Moench \n 
        Empty value deletes header.
        
        Example
        -------
        >>> d.rx_jsonaddheader
        {}
        >>> d.rx_jsonaddheader = {"key1": "value1", "key2":"value2"}
        >>> d.rx_jsonaddheader
        {'emax': '30', 'emin': '50'}
        """
        return self.getAdditionalJsonHeader()

    @rx_jsonaddheader.setter
    def rx_jsonaddheader(self, args):
        ut.set_using_dict(self.setAdditionalJsonHeader, args)

    @property
    @element
    def threshold(self):
        """[Eiger][Mythen3] Threshold in eV
        
        Note
        ----
        To change settings as well or set threshold without trimbits, use setThresholdEnergy.

        :setter: It loads trim files from settingspath.\n [Mythen3] An energy of -1 will pick up values from detector.
        """
        if self.type == detectorType.MYTHEN3:
            return self.getAllThresholdEnergy()
        return self.getThresholdEnergy()

    @threshold.setter
    def threshold(self, eV):
        ut.set_using_dict(self.setThresholdEnergy, eV)

    @property
    @element
    def timing(self):
        """
        Set Timing Mode of detector. 
        Enum: timingMode
        
        Note
        -----
        Default: AUTO_TIMING \n
        [Jungfrau][Moench][Gotthard][Ctb][Gotthard2][Xilinx Ctb] AUTO_TIMING, TRIGGER_EXPOSURE \n
        [Mythen3] AUTO_TIMING, TRIGGER_EXPOSURE, GATED, TRIGGER_GATED \n
        [Eiger] AUTO_TIMING, TRIGGER_EXPOSURE, GATED, BURST_TRIGGER
        """
        return self.getTimingMode()

    @timing.setter
    def timing(self, mode):
        ut.set_using_dict(self.setTimingMode, mode)

    @property
    @element
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
        return self.getTrimEnergies()

    @trimen.setter
    def trimen(self, energies):
        ut.set_using_dict(self.setTrimEnergies, energies)

    @property
    @element
    def vthreshold(self):
        """
        [Eiger][Mythen3] Detector threshold voltage for single photon counters in dac units.
        
        Note
        ----
        [Eiger] Sets vcmp_ll, vcmp_lr, vcmp_rl, vcmp_rr and vcp to the same value. \n
        [Mythen3] Sets vth1, vth2 and vth3 to the same value for enabled counters.
        """
        return self.getDAC(dacIndex.VTHRESHOLD)

    @vthreshold.setter
    def vthreshold(self, value):
        if isinstance(value, dict):
            args = ({k:(dacIndex.VTHRESHOLD,v) for k,v in value.items()},)
        else:
            args = (dacIndex.VTHRESHOLD, value)
        ut.set_using_dict(self.setDAC, *args)


    @property
    @element
    def type(self):
        """ Returns detector type. 
        Enum: detectorType
        [EIGER, JUNGFRAU, GOTTHARD, MOENCH, MYTHEN3, GOTTHARD2, CHIPTESTBOARD]

        :setter: Not implemented
        """
        return self.getDetectorType()

    @property
    @element
    def rx_frameindex(self):
        """Current frame index received for each port in receiver during acquisition."""
        return self.getRxCurrentFrameIndex()

    @property
    @element
    def rx_missingpackets(self):
        """Gets the number of missing packets for each port in receiver. Negative number denotes extra packets. """
        return self.getNumMissingPackets()

    """

    <<<Eiger>>>

    """

    @property
    def datastream(self):
        """
        datastream [left|right] [0, 1]
	    [Eiger] Enables or disables data streaming from left or/and right side of detector for 10GbE mode. 1 (enabled) by default.
        """
        result = {}
        for port in [defs.LEFT, defs.RIGHT]:
            result[port] = element_if_equal(self.getDataStream(port))
        return result

    @datastream.setter
    def datastream(self, value):
        ut.set_using_dict(self.setDataStream, *value)

    @property
    @element
    def quad(self):
        """[Eiger] Sets detector size to a quad. 0 (disabled) is default. (Specific hardware required). """
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

        :getter: always returns in seconds. To get in DurationWrapper, use getSubExptime

        Example
        -----------
        >>> # setting directly in seconds
        >>> d.subexptime = 1.230203
        >>>
        >>> # setting directly in seconds
        >>> d.subexptime = 5e-07
        >>> 
        >>> # using timedelta (up to microseconds precision)
        >>> from datatime import timedelta
        >>> d.subexptime = timedelta(seconds = 1.23, microseconds = 203)
        >>> 
        >>> # using DurationWrapper to set in seconds
        >>> from slsdet import DurationWrapper
        >>> d.subexptime = DurationWrapper(1.2)
        >>> 
        >>> # using DurationWrapper to set in ns
        >>> t = DurationWrapper()
        >>> t.set_count(500)
        >>> d.subexptime = t
        >>>
        >>> # to get in seconds
        >>> d.subexptime
        181.23
        >>> 
        >>> d.getSubExptime()
        sls::DurationWrapper(total_seconds: 181.23 count: 181230000000)
        """
        res = self.getSubExptime()
        return reduce_time(res)

    @subexptime.setter
    def subexptime(self, t):
        ut.set_time_using_dict(self.setSubExptime, t)

    @property
    @element
    def readnrows(self):
        """
        [Eiger] Number of rows to read out per half module starting from the centre.
        [Jungfrau][Moench] Number of rows to read per module starting from the centre.
        
        Note
        ----
        [Eiger] Options: 1 - 256. 256 is default. \n
        [Eiger]The permissible values depend on dynamic range and 10Gbe enabled.\n\n
        [Jungfrau][Moench] Options: 8 - 512 (multiples of 8)
        """
        return self.getReadNRows()

    @readnrows.setter
    def readnrows(self, value):
        ut.set_using_dict(self.setReadNRows, value)


    @property
    def subdeadtime(self):
        """
        [Eiger] Dead time of EIGER subframes in 32 bit mode, accepts either a value in seconds, datetime.timedelta or DurationWrapper
        
        Note
        ----
        Subperiod = subexptime + subdeadtime.

        :getter: always returns in seconds. To get in DurationWrapper, use getSubDeadTime

        Example
        -----------
        >>> # setting directly in seconds
        >>> d.subdeadtime = 1.230203
        >>>
        >>> # setting directly in seconds
        >>> d.subdeadtime = 5e-07
        >>> 
        >>> # using timedelta (up to microseconds precision)
        >>> from datatime import timedelta
        >>> d.subdeadtime = timedelta(seconds = 1.23, microseconds = 203)
        >>> 
        >>> # using DurationWrapper to set in seconds
        >>> from slsdet import DurationWrapper
        >>> d.subdeadtime = DurationWrapper(1.2)
        >>> 
        >>> # using DurationWrapper to set in ns
        >>> t = DurationWrapper()
        >>> t.set_count(500)
        >>> d.subdeadtime = t
        >>>
        >>> # to get in seconds
        >>> d.subdeadtime
        181.23
        >>> 
        >>> d.getSubDeadTime()
        sls::DurationWrapper(total_seconds: 181.23 count: 181230000000)
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
        [Eiger][Mythen3][Gotthard2][Moench] Enable or disable the parallel readout mode of detector. 
        
        Note
        ----
        [Mythen3] If exposure time is too short, acquisition will return with an ERROR and take fewer frames than expected. 
        [Mythen3][Eiger][Moench] Default: Non parallel
        [Gotthard2] Default: parallel. Non parallel mode works only in continuous mode.
        """
        return self.getParallelMode()

    @parallel.setter
    def parallel(self, value):
        ut.set_using_dict(self.setParallelMode, value)

    @property
    @element
    def partialreset(self):
        """[Eiger] Sets up detector to do partial or complete reset at start of acquisition. 0 complete reset, 1 partial reset. Default is complete reset.
        
        Note
        -----
        Advanced Function!
        """
        return self.getPartialReset()

    @partialreset.setter
    def partialreset(self, value):
        ut.set_using_dict(self.setPartialReset, value)

    @property
    @element
    def tengiga(self):
        """[Eiger][Ctb][Mythen3] 10GbE Enable."""
        return self.getTenGiga()

    @tengiga.setter
    def tengiga(self, value):
        ut.set_using_dict(self.setTenGiga, value)

    @property
    @element
    def overflow(self):
        """[Eiger] Enable or disable show overflow flag in 32 bit mode. Default is disabled. """
        return self.getOverFlowMode()

    @overflow.setter
    def overflow(self, value):
        ut.set_using_dict(self.setOverFlowMode, value)

    @property
    @element
    def flowcontrol10g(self):
        """[Eiger][Jungfrau][Moench] Enable or disable 10GbE Flow Control."""
        return self.getTenGigaFlowControl()

    @flowcontrol10g.setter
    def flowcontrol10g(self, enable):
        ut.set_using_dict(self.setTenGigaFlowControl, enable)

    @property
    @element
    def interruptsubframe(self):
        """[Eiger] Enable last subframe interrupt at required exposure time. Disabling will wait for last sub frame to finish exposing. Default is disabled."""
        return self.getInterruptSubframe()

    @interruptsubframe.setter
    def interruptsubframe(self, value):
        ut.set_using_dict(self.setInterruptSubframe, value)

    @property
    @element
    def gappixels(self):
        """[Eiger][Jungfrau][Moench] Include Gap pixels in client data call back in Detecor api. Will not be in detector streaming, receiver file or streaming. Default is disabled. """
        return self.getRxAddGapPixels()

    @gappixels.setter
    def gappixels(self, value):
        ut.set_using_dict(self.setRxAddGapPixels, value)

    @property
    def measuredperiod(self):
        """
        [Eiger] Measured frame period between last frame and previous one. 
        
        Note
        -----
        Can be measured with minimum 2 frames in an acquisition. 

        :setter: Not implemented
        """
        return ut.reduce_time(self.getMeasuredPeriod())
       

    @property
    def measuredsubperiod(self):
        """
        [Eiger] Measured sub frame period between last sub frame and previous one. 
        
        :setter: Not implemented
        """
        return ut.reduce_time(self.getMeasuredSubFramePeriod())

    @property
    @element
    def top(self):
        """[Eiger] Sets half module to top (1), else bottom.
        
        Note
        -----
        Advanced Function!
        """
        return self.getTop()

    @top.setter
    def top(self, value):
        ut.set_using_dict(self.setTop, value)

    """
    ------------------<<<Jungfrau specific>>>-------------------------
    """

    @property
    @element
    def chipversion(self):
        """
        [Jungfrau] Chip version of module. Can be 1.0 or 1.1.

        Example
        -------
        >>> d.chipversion
        '1.0'
        """
        return self.getChipVersion()


    @property
    @element
    def autocompdisable(self):
        """[Jungfrau] Enable or disable auto comparator disable mode. 

        Note
        -----
        By default, the on-chip gain switching is active during the entire exposure. This mode disables the on-chip gain switching comparator automatically and the duration is set using compdisabletime.\n
        Default is 0 or this mode disabled (comparator enabled throughout). 1 enables mode. 0 disables mode. 
        """
        return self.getAutoComparatorDisable()

    @autocompdisable.setter
    def autocompdisable(self, value):
        ut.set_using_dict(self.setAutoComparatorDisable, value)

    @property
    @element
    def compdisabletime(self):
        """[Jungfrau] Time before end of exposure when comparator is disabled. 

        :getter: always returns in seconds. To get in DurationWrapper, use getComparatorDisableTime

        Example
        -----------
        >>> # setting directly in seconds
        >>> d.compdisabletime = 1.05
        >>>
        >>> # setting directly in seconds
        >>> d.compdisabletime = 5e-07
        >>> 
        >>> # using timedelta (up to microseconds precision)
        >>> from datatime import timedelta
        >>> d.compdisabletime = timedelta(seconds = 1, microseconds = 3)
        >>> 
        >>> # using DurationWrapper to set in seconds
        >>> from slsdet import DurationWrapper
        >>> d.compdisabletime = DurationWrapper(1.2)
        >>> 
        >>> # using DurationWrapper to set in ns
        >>> t = DurationWrapper()
        >>> t.set_count(500)
        >>> d.compdisabletime = t
        >>>
        >>> # to get in seconds
        >>> d.compdisabletime
        181.23
        >>> 
        >>> d.getComparatorDisableTime()
        sls::DurationWrapper(total_seconds: 181.23 count: 181230000000)
        """
        return ut.reduce_time(self.getComparatorDisableTime())

    @compdisabletime.setter
    def compdisabletime(self, value):
       ut.set_time_using_dict(self.setComparatorDisableTime, value)


    @property
    @element
    def runtime(self):
        """[Jungfrau][Moench][Mythen3][Gotthard2][CTB][Xilinx Ctb] Time from detector start up.
        
        Note
        -----
        [Gotthard2] not in burst and auto mode.
        """
        return self.getActualTime()

    @property
    @element
    def extrastoragecells(self):
        """
        [Jungfrau] Number of additional storage cells. 

        Note
        ----
        Only for chip v1.0. For advanced users only. \n
        Options: 0 - 15. Default is 0.
        The #images = #frames x #triggers x (#extrastoragecells + 1)
        """
        return self.getNumberOfAdditionalStorageCells()

    @extrastoragecells.setter
    def extrastoragecells(self, n_cells):
        ut.set_using_dict(self.setNumberOfAdditionalStorageCells, n_cells)

    @property
    @element
    def storagecell_start(self):
        """
        [Jungfrau] Storage cell that stores the first acquisition of the series. 
        
        Note
        ----
        For advanced users only.
        Options 0-max. max is 15 (default) for chipv1.0 and 3 (default) for chipv1.1. \n
        """
        return self.getStorageCellStart()

    @storagecell_start.setter
    def storagecell_start(self, value):
        ut.set_using_dict(self.setStorageCellStart, value)

    @property
    def storagecell_delay(self):
        """
        [Jungfrau] Additional time delay between 2 consecutive exposures in burst mode, accepts either a value in seconds, datetime.timedelta or DurationWrapper
        
        Note
        -----
        Only applicable for chipv1.0. For advanced users only \n
        Value: 0-1638375 ns (resolution of 25ns)

        :getter: always returns in seconds. To get in DurationWrapper, use getStorageCellDelay

        Example
        -----------
        >>> # setting directly in seconds
        >>> d.storagecell_delay = 1.05
        >>>
        >>> # setting directly in seconds
        >>> d.storagecell_delay = 5e-07
        >>> 
        >>> # using timedelta (up to microseconds precision)
        >>> from datatime import timedelta
        >>> d.storagecell_delay = timedelta(seconds = 1, microseconds = 3)
        >>> 
        >>> # using DurationWrapper to set in seconds
        >>> from slsdet import DurationWrapper
        >>> d.storagecell_delay = DurationWrapper(1.2)
        >>> 
        >>> # using DurationWrapper to set in ns
        >>> t = DurationWrapper()
        >>> t.set_count(500)
        >>> d.storagecell_delay = t
        >>>
        >>> # to get in seconds
        >>> d.storagecell_delay
        181.23
        >>> 
        >>> d.getStorageCellDelay()
        sls::DurationWrapper(total_seconds: 181.23 count: 181230000000)
        """
        return ut.reduce_time(self.getStorageCellDelay())

    @storagecell_delay.setter
    def storagecell_delay(self, t):
        ut.set_time_using_dict(self.setStorageCellDelay, t)

    @property
    @element
    def temp_threshold(self):
        """
        [Jungfrau][Moench] Threshold temperature in degrees. 
        
        Note
        -----
        If temperature crosses threshold temperature and temperature control is enabled, power to chip will be switched off and temperature event occurs. \n
        To power on chip again, temperature has to be less than threshold temperature and temperature event has to be cleared/reset.
        """
        return self.getThresholdTemperature()

    @temp_threshold.setter
    def temp_threshold(self, value):
        ut.set_using_dict(self.setThresholdTemperature, value)

    @property
    @element
    def temp_event(self):
        """
        [Jungfrau][Moench] 1, if a temperature event occured. \n
        
        Note
        ----
        If temperature crosses threshold temperature and temperature control is enabled, power to chip will be switched off and temperature event occurs. \n
        To power on chip again, temperature has to be less than threshold temperature and temperature event has to be cleared/reset.
        
        :setter: To clear the event, set it to 0.
        """
        return self.getTemperatureEvent()

    @temp_event.setter
    def temp_event(self, value):
        modules = []
        if isinstance(value, dict):
            if any(value.values()):
                raise ValueError("Value needs to be 0 for reset. Setting not allowed")
            modules = list(value.keys())
        else:
            if value != 0:
                raise ValueError("Value needs to be 0 for reset. Setting not allowed")
            
        self.resetTemperatureEvent(modules)

    @property
    @element
    def temp_control(self):
        """
        [Jungfrau][Moench] Temperature control enable. 
        
        Note
        -----
        Default is 0 (disabled). \n
        If temperature crosses threshold temperature and temperature control is enabled, power to chip will be switched off and temperature event occurs. \n
        To power on chip again, temperature has to be less than threshold temperature and temperature event has to be cleared/reset.
        """
        return self.getTemperatureControl()

    @temp_control.setter
    def temp_control(self, value):
        ut.set_using_dict(self.setTemperatureControl, value)

    @property
    @element
    def selinterface(self):
        """[Jungfrau][Moench] The udp interface to stream data from detector. 
        
        Note
        -----
        Effective only when number of interfaces is 1. Default: 0 (outer). Inner is 1.
        """
        return self.getSelectedUDPInterface()

    @selinterface.setter
    def selinterface(self, i):
        ut.set_using_dict(self.selectUDPInterface, i)

    @property
    def gainmodelist(self):
        """List of gainmode implemented for this detector."""
        return self.getGainModeList()

    @property
    def gainmode(self):
        """
        [Jungfrau] Detector gain mode. 
        Enum: gainMode
        
        Note
        -----
        [Jungfrau] DYNAMIC, FORCE_SWITCH_G1, FORCE_SWITCH_G2, FIX_G1, FIX_G2, FIX_G0 \n
        CAUTION: Do not use FIX_G0 without caution, you can damage the detector!!!
        """
        return element_if_equal(self.getGainMode())

    @gainmode.setter
    def gainmode(self, value):
        self.setGainMode(value)

    @property
    @element
    def currentsource(self):
        """
        [Gotthard2][Jungfrau] Pass in a currentSrcParameters object
        see python/examples/use_currentsource.py

        """
        return self.getCurrentSource()

    @currentsource.setter
    def currentsource(self, cs):
        ut.set_using_dict(self.setCurrentSource, cs)

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
    def burstsl(self):
        """
        [Gotthard2] Number of bursts left in acquisition.\n
        
        Note
        ----
        Only in burst auto mode.
        
        :setter: Not Implemented
        """
        return self.getNumberOfBurstsLeft()

    @property
    @element
    def filterresistor(self):
        """
        [Gotthard2][Jungfrau] Set filter resistor. Increasing values for increasing "
        "resistance.
        
        Note
        ----
        Advanced user command.
        [Gotthard2] Default is 0. Options: 0-3.
        [Jungfrau] Default is 1. Options: 0-1.
        """
        return self.getFilterResistor()

    @filterresistor.setter
    def filterresistor(self, value):
        ut.set_using_dict(self.setFilterResistor, value)

    @property
    @element
    def filtercells(self):
        """
        [Jungfrau] Set filter capacitor. 
        
        Note
        ----
        [Jungfrau] Options: 0-12. Default: 0. Advanced user command. Only for chipv1.1.
        """
        return self.getNumberOfFilterCells()

    @filtercells.setter
    def filtercells(self, value):
        ut.set_using_dict(self.setNumberOfFilterCells, value)

    @property
    @element
    def pedestalmode(self):
        """
        [Jungfrau] Enables or disables pedestal mode. Pass in a pedestalParameters object 
        see python/examples/use_pedestalmode.py
        
        Note
        ----
        The number of frames or triggers is overwritten by #pedestal_frames x  pedestal_loops x 2. \n
        In auto timing mode or in trigger mode with #frames > 1, #frames is overwritten and #triggers = 1, else #triggers is overwritten and #frames = 1. \n
        One cannot set #frames, #triggers or timing mode in pedestal mode (exception thrown).\n
        Disabling pedestal mode will set back the normal mode values of #frames and #triggers."
        """
        return self.getPedestalMode()

    @pedestalmode.setter
    def pedestalmode(self, value):
        ut.set_using_dict(self.setPedestalMode, value)

    @property
    @element
    def timing_info_decoder(self):
        """[Jungfrau] [Jungfrau] Advanced Command and only for SWISSFEL and SHINE. Sets the bunch id or timing info decoder. Default is SWISSFEL. Only allowed for pcbv2.0.
        Enum: timingInfoDecoder
        """
        return self.getTimingInfoDecoder()

    @timing_info_decoder.setter
    def timing_info_decoder(self, value):
        ut.set_using_dict(self.setTimingInfoDecoder, value)
        
    @property
    @element
    def collectionmode(self):
        """[Jungfrau] Sets collection mode to HOLE or ELECTRON. Default is HOLE.
        Enum: collectionMode
        """
        return self.getCollectionMode()

    @collectionmode.setter
    def collectionmode(self, value):
        ut.set_using_dict(self.setCollectionMode, value)

    @property
    def maxclkphaseshift(self):
        """
        [Gotthard2][Mythen3] Absolute maximum Phase shift of clocks.\n
        [Gotthard2] Clock index range: 0-5\n
        [Mythen3] Clock index range: 0
               
        :setter: Not Implemented
        
        Example
        -------
        >>> d.maxclkphaseshift
        0: 80
        1: 80
        2: 160
        3: 80
        4: 80
        """
        return MaxPhaseProxy(self)

    @property
    @element
    def timingsource(self):
        """
        [Gotthard2] Timing source. 
        Enum: timingSourceType
        
        Note
        -----
        Options: TIMING_INTERNAL, TIMING_EXTERNAL \n
        Internal is crystaland external is system timing. Default is internal.
        """
        return self.getTimingSource()

    @timingsource.setter
    def timingsource(self, args):
        ut.set_using_dict(self.setTimingSource, args)


    @property
    @element
    def veto(self):
        """
        [Gotthard2] Enable or disable veto data from chip. 
        
        Note
        ----
        Default is 0.
        """
        return self.getVeto()

    @veto.setter
    def veto(self, value):
        ut.set_using_dict(self.setVeto, value)

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
        """[Gotthard2] Burst mode of detector. 
        Enum: burstMode
        
        Note
        ----
        BURST_INTERNAL (default), BURST_EXTERNAL, CONTINUOUS_INTERNAL, CONTINUOUS_EXTERNAL
        Also changes clkdiv 2, 3, 4
        """
        return self.getBurstMode()

    @burstmode.setter
    def burstmode(self, value):
        ut.set_using_dict(self.setBurstMode, value)

    @property
    def burstperiod(self):
        """
        [Gotthard2] Period between 2 bursts. Only in burst mode and auto timing mode.
        
        :getter: always returns in seconds. To get in DurationWrapper, use getBurstPeriod
        :setter: Not Implemented

        Example
        -----------
        >>> # setting directly in seconds
        >>> d.burstperiod = 1.05
        >>>
        >>> # setting directly in seconds
        >>> d.burstperiod = 5e-07
        >>> 
        >>> # using timedelta (up to microseconds precision)
        >>> from datatime import timedelta
        >>> d.burstperiod = timedelta(seconds = 1, microseconds = 3)
        >>> 
        >>> # using DurationWrapper to set in seconds
        >>> from slsdet import DurationWrapper
        >>> d.burstperiod = DurationWrapper(1.2)
        >>> 
        >>> # using DurationWrapper to set in ns
        >>> t = DurationWrapper()
        >>> t.set_count(500)
        >>> d.burstperiod = t
        >>>
        >>> # to get in seconds
        >>> d.burstperiod
        181.23
        >>> 
        >>> d.getBurstPeriod()
        sls::DurationWrapper(total_seconds: 181.23 count: 181230000000)
        """
        return ut.reduce_time(self.getBurstPeriod())

    @burstperiod.setter
    def burstperiod(self, value):
        ut.set_time_using_dict(self.setBurstPeriod, value)

    @property
    def vetofile(self):
        """
        [Gotthard2] Set veto reference for each 128 channels for specific chip. \n
        The file should have 128 rows of gain index and 12 bit value in dec.

        Example
        ---------

        d.vetofile = -1, '/path/to/file.txt' #set for all chips
        d.vetofile = 3, '/path/to/file.txt' # set for chip 3

        """
        raise NotImplementedError('vetofile is set only')

    @vetofile.setter
    def vetofile(self, args):
        if not isinstance(args, tuple):
            args = (args,)
        ut.set_using_dict(self.setVetoFile, *args)

    @property 
    def vetophoton(self):
        """
        [Gotthard2] Set veto reference for 128 channels for chip ichip according to reference file 
        and #photons and energy in keV.
        
        Note
        ----
        Arguments: (chip_index, n_photons, photon_energy, fname)
        
        :getter: Not Implemented
        
        Example
        -------
        >>> d.vetophoton = (2, 24, 2560, '/tmp/bla.txt')
        """
        raise NotImplementedError('vetophoton is set only')

    @vetophoton.setter
    def vetophoton(self, args):
        if not isinstance(args, tuple):
            args = (args,)
        ut.set_using_dict(self.setVetoPhoton, *args)

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
        if not isinstance(args, tuple):
            args = (args,)
        ut.set_using_dict(self.setVetoReference, *args)


    @property
    @element
    def vetostream(self):
        """[Gotthard2] Enabling/ disabling veto interface
        
        Note
        ----
        Default: both off
        Options: NONE, LOW_LATENCY_LINK, 10GBE (debugging)
        Debugging interface also enables second interface in receiver (separate file), which also restarts zmq streaming if enabled.
        """
        return self.getVetoStream()

    @vetostream.setter
    def vetostream(self, args):
        if not isinstance(args, tuple):
            args = (args,)
        ut.set_using_dict(self.setVetoStream, *args)

    @property
    def vetoalg(self):
        """[Gotthard2] Algorithm used for veto. 
        Enum: vetoAlgorithm, streamingInterface
        
        Note
        ----
        Options:
        (vetoAlgorithm): ALG_HITS (default), ALG_RAW
        (streamingInterface): ETHERNET_10GB, LOW_LATENCY_LINK
        
        Example
        ----------
        >>> d.vetoalg = defs.ALG_HITS, defs.ETHERNET_10GB
        """
        result = {}
        interface = [streamingInterface.LOW_LATENCY_LINK, streamingInterface.ETHERNET_10GB]
        for eth in interface:
            result[eth] = element_if_equal(self.getVetoAlgorithm(eth))
        return result


    @vetoalg.setter
    def vetoalg(self, args):
        if not isinstance(args, tuple):
            args = (args,)
        ut.set_using_dict(self.setVetoAlgorithm, *args)

    """
    Mythen3 specific
    """

    @property
    def gatedelay(self):
        """
        [Mythen3] Gate Delay of all gate signals in auto and trigger mode (internal gating), accepts either a value in seconds, datetime.timedelta or DurationWrapper

        Note
        -----
        To specify gateIndex, use getGateDelay or setGateDelay.
        
        :getter: always returns in seconds. To get in DurationWrapper, use getGateDelayForAllGates or getGateDelay(gateIndex)

        Example
        -----------
        >>> # setting directly in seconds
        >>> d.gatedelay = 1.05
        >>>
        >>> # setting directly in seconds
        >>> d.gatedelay = 5e-07
        >>> 
        >>> # using timedelta (up to microseconds precision)
        >>> from datatime import timedelta
        >>> d.gatedelay = timedelta(seconds = 1, microseconds = 3)
        >>> 
        >>> # using DurationWrapper to set in seconds
        >>> from slsdet import DurationWrapper
        >>> d.gatedelay = DurationWrapper(1.2)
        >>> 
        >>> # using DurationWrapper to set in ns
        >>> t = DurationWrapper()
        >>> t.set_count(500)
        >>> d.gatedelay = t
        >>>
        >>> # to get in seconds
        >>> d.gatedelay
        181.23
        >>> 
        >>> d.getExptimeForAllGates()
        sls::DurationWrapper(total_seconds: 181.23 count: 181230000000)
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
        Enabling counters sets vth dacs to remembered values and disabling sets them to disabled values.

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
        values = ut.make_bitmask(values)
        ut.set_using_dict(self.setCounterMask, values)

    """
    <<<CTB>>>
    """

    @property
    @element
    def adcenable(self):
        """[Ctb] ADC Enable Mask for 1Gb. Enable for each 32 ADC channel."""
        return self.getADCEnableMask()

    @adcenable.setter
    def adcenable(self, value):
        ut.set_using_dict(self.setADCEnableMask, value)

    @property
    @element
    def adcenable10g(self):
        """[Ctb] ADC Enable Mask for 10Gb mode for each 32 ADC channel. 

        Note
        -----
        If any of a consecutive 4 bits are enabled, the complete 4 bits are enabled."""
        return self.getTenGigaADCEnableMask()

    @adcenable10g.setter
    def adcenable10g(self, value):
        ut.set_using_dict(self.setTenGigaADCEnableMask, value)

    @property
    @element
    def transceiverenable(self):
        """[CTB][Xilinx CTB] Transceiver Enable Mask. Enable for each 4 transceiver channel."""
        return self.getTransceiverEnableMask()

    @transceiverenable.setter
    def transceiverenable(self, value):
        ut.set_using_dict(self.setTransceiverEnableMask, value)

    #TODO: remove this command or throw if it doesnt match with digital and transceiver
    @property
    @element
    def samples(self):
        """
        [CTB] Number of samples (only analog) expected. \n
        """
        return self.getNumberOfAnalogSamples()

    @samples.setter
    def samples(self, nsamples):
        ut.set_using_dict(self.setNumberOfAnalogSamples, nsamples)

    @property
    @element
    def runclk(self):
        """[Ctb] Run clock in MHz."""
        return self.getRUNClock()

    @runclk.setter
    def runclk(self, freq):
        ut.set_using_dict(self.setRUNClock, freq)

    @property
    @element
    def romode(self):
        """
        [CTB] Readout mode of detector. 
        Enum: readoutMode
        
        Note
        ------
        [CTB] Options: ANALOG_ONLY, DIGITAL_ONLY, ANALOG_AND_DIGITAL, TRANSCEIVER_ONLY, DIGITAL_AND_TRANSCEIVER
        [CTB] Default: ANALOG_ONLY
        [Xilinx CTB] Options: TRANSCEIVER_ONLY
        [Xilinx CTB] Default: TRANSCEIVER_ONLY

        Example
        --------
        >>> d.romode = readoutMode.ANALOG_ONLY
        >>> d.romode
        readoutMode.ANALOG_ONLY
        """
        return self.getReadoutMode()

    @romode.setter
    def romode(self, mode):
        ut.set_using_dict(self.setReadoutMode, mode)

    @property
    @element
    def asamples(self):
        """[Ctb] Number of analog samples expected. """
        return element_if_equal(self.getNumberOfAnalogSamples())

    @asamples.setter
    def asamples(self, N):
        ut.set_using_dict(self.setNumberOfAnalogSamples, N)

    @property
    @element
    def dsamples(self):
        """[CTB] Number of digital samples expected. """
        return self.getNumberOfDigitalSamples()

    @dsamples.setter
    def dsamples(self, N):
        ut.set_using_dict(self.setNumberOfDigitalSamples, N)

    @property
    @element
    def tsamples(self):
        """[CTB][Xilinx CTB] Number of transceiver samples expected. """
        return self.getNumberOfTransceiverSamples()

    @tsamples.setter
    def tsamples(self, N):
        ut.set_using_dict(self.setNumberOfTransceiverSamples, N)

    @property
    @element
    def dbitphase(self):
        """[Ctb][Jungfrau] Phase shift of clock to latch digital bits. Absolute phase shift.

        Note
        -----
        [Ctb]Changing dbitclk also resets dbitphase and sets to previous values.
        """
        return self.getDBITPhase()

    @dbitphase.setter
    def dbitphase(self, value):
        ut.set_using_dict(self.setDBITPhase, value)

    @property
    @element
    def dbitclk(self):
        """[Ctb] Clock for latching the digital bits in MHz."""
        return self.getDBITClock()

    @dbitclk.setter
    def dbitclk(self, value):
        ut.set_using_dict(self.setDBITClock, value)

    @property
    @element
    def adcvpp(self):
        """[Ctb][Moench] Vpp of ADC. [0 -> 1V | 1 -> 1.14V | 2 -> 1.33V | 3 -> 1.6V | 4 -> 2V] \n
            Advanced User function!"""
        return self.getADCVpp(False)

    @adcvpp.setter
    def adcvpp(self, value):
        ut.set_using_dict(self.setADCVpp, value, False)

    @property
    @element
    def dbitpipeline(self):
        """[Ctb][Gotthard2] Pipeline of the clock for latching digital bits. 
        
        Note
        ----
        [CTB] Options: 0 - 255
        [Gotthard2] Options: 0 - 7
        """
        return self.getDBITPipeline()

    @dbitpipeline.setter
    def dbitpipeline(self, value):
        ut.set_using_dict(self.setDBITPipeline, value)

    @property
    @element
    def maxdbitphaseshift(self):
        """[CTB][Jungfrau] Absolute maximum Phase shift of of the clock to latch digital bits.
        
        :setter: Not Implemented
        """
        return self.getMaxDBITPhaseShift()

    @property
    @element
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
        return self.getRxDbitList()

    @rx_dbitlist.setter
    def rx_dbitlist(self, value):
        ut.set_using_dict(self.setRxDbitList, value)

    @property
    @element
    def rx_dbitoffset(self):
        """[Ctb] Offset in bytes in digital data to skip in receiver."""
        return self.getRxDbitOffset()

    @rx_dbitoffset.setter
    def rx_dbitoffset(self, value):
        ut.set_using_dict(self.setRxDbitOffset, value)

    @property
    @element
    def maxadcphaseshift(self):
        """[Jungfrau][Moench][CTB] Absolute maximum Phase shift of ADC clock.
        
        :setter: Not Implemented
        """
        return self.getMaxADCPhaseShift()

    @property
    @element
    def adcphase(self):
        """[Gotthard][Jungfrau][Moench][CTB] Sets phase shift of ADC clock. 

        Note
        -----
        [Jungfrau][Moench] Absolute phase shift. Changing Speed also resets adcphase to recommended defaults.\n
        [Ctb] Absolute phase shift. Changing adcclk also resets adcphase and sets it to previous values.\n
        [Gotthard] Relative phase shift.

        :getter: Not implemented for Gotthard
        """
        return self.getADCPhase()

    @adcphase.setter
    def adcphase(self, value):
        ut.set_using_dict(self.setADCPhase, value)

    @property
    @element
    def adcpipeline(self):
        """[Ctb] Sets pipeline for ADC clock. """
        return self.getADCPipeline()

    @adcpipeline.setter
    def adcpipeline(self, value):
        ut.set_using_dict(self.setADCPipeline, value)

    @property
    @element
    def adcclk(self):
        """[Ctb] Sets ADC clock frequency in MHz. """
        return self.getADCClock()

    @adcclk.setter
    def adcclk(self, value):
        ut.set_using_dict(self.setADCClock, value)

    @property
    @element
    def syncclk(self):
        """
        [Ctb] Sync clock in MHz.
        
        :setter: Not implemented
        """
        return self.getSYNCClock()

    @property
    def pattern(self):
        """[Mythen3][Ctb][Xilinx Ctb] Loads ASCII pattern file directly to server (instead of executing line by line).
               
        :getter: Not Implemented
        
        Example
        ---------
        >>> d.pattern = '/tmp/pat.txt'
        """
        raise NotImplementedError("Pattern is set only")

    @pattern.setter
    def pattern(self, fname):
        fname = ut.make_string_path(fname)
        ut.set_using_dict(self.setPattern, fname)

    @property
    def patfname(self):
        """
        [Ctb][Mythen3][Xilinx Ctb] Gets the pattern file name including path of the last pattern uploaded. Returns an empty if nothing was uploaded or via a server default
        file
        """
        return self.getPatterFileName()

    @property
    @element
    def patioctrl(self):
        """[Ctb] 64 bit mask defining input (0) and output (1) signals.
        
        Example
        --------
        >>> d.patioctrl = 0x8f0effff6dbffdbf
        >>> hex(d.patioctrl)
        '0x8f0effff6dbffdbf'
        """
        return self.getPatternIOControl()

    @patioctrl.setter
    def patioctrl(self, mask):
        ut.set_using_dict(self.setPatternIOControl, mask)

    @property
    @element
    def patlimits(self):
        """[Ctb][Mythen3][Xilinx Ctb] Limits (start and stop address) of complete pattern.
        
        Example
        ---------
        >>> d.patlimits = [0x0, 0x18c]
        >>> d.patlimits
        [0, 396]
        >>> [hex(l) for l in d.patlimits]
        ['0x0', '0x18c']
        """
        return self.getPatternLoopAddresses(-1)

    @patlimits.setter
    def patlimits(self, args):
        args = ut.merge_args(-1, args)
        ut.set_using_dict(self.setPatternLoopAddresses, *args)

    @property
    @element
    def patsetbit(self):
        """[Ctb][Mythen3][Xilinx Ctb] Sets the mask applied to every pattern to the selected bits. 
        
        Example
        --------
        >>> d.patsetbit = 0x8f0effff6dbffdbf
        >>> hex(d.patsetbit)
        '0x8f0effff6dbffdbf' 
        """
        return self.getPatternBitMask()

    @patsetbit.setter
    def patsetbit(self, mask):
        ut.set_using_dict(self.setPatternBitMask, mask)

    @property
    @element
    def patmask(self):
        """[Ctb][Mythen3][Xilinx Ctb] Selects the bits that will have a pattern mask applied to the selected patmask for every pattern.
        
        Example
        --------
        >>> d.patmask = 0x8f0effff6dbffdbf
        >>> hex(d.patmask)
        '0x8f0effff6dbffdbf' 
        """
        return self.getPatternMask()

    @patmask.setter
    def patmask(self, mask):
        ut.set_using_dict(self.setPatternMask, mask)

    @property
    # @element
    def patwait(self):
        """
        [Ctb][Mythen3][Xilinx Ctb] Wait address of loop level provided.
        
        Example
        -------
        >>> d.patwait[0] = 5
        >>> d.patwait[0]
        5
        >>> d.patwait
        0: 5
        1: 20
        2: 30
        """
        return PatWaitProxy(self)

    @property
    @element
    def patwait0(self):
        """[Ctb][Mythen3][Xilinx Ctb] Wait 0 address.
                
        Example
        --------
        >>> d.patwait0 = 0xaa
        >>> d.patwait0
        170
        >>> hex(d.patwait0)
        '0xaa'
        """
        return self.getPatternWaitAddr(0)

    @patwait0.setter
    def patwait0(self, addr):
        addr = ut.merge_args(0, addr)
        ut.set_using_dict(self.setPatternWaitAddr, *addr)

    @property
    @element
    def patwait1(self):
        """[Ctb][Mythen3][Xilinx Ctb] Wait 1 address.
                
        Example
        --------
        >>> d.patwait1 = 0xaa
        >>> d.patwait1
        170
        >>> hex(d.patwait1)
        '0xaa'
        """
        return self.getPatternWaitAddr(1)

    @patwait1.setter
    def patwait1(self, addr):
        addr = ut.merge_args(1, addr)
        ut.set_using_dict(self.setPatternWaitAddr, *addr)

    @property
    @element
    def patwait2(self):
        """[Ctb][Mythen3][Xilinx Ctb] Wait 2 address.
                
        Example
        --------
        >>> d.patwait2 = 0xaa
        >>> d.patwait2
        170
        >>> hex(d.patwait2)
        '0xaa'
        """
        return self.getPatternWaitAddr(2)

    @patwait2.setter
    def patwait2(self, addr):
        addr = ut.merge_args(2, addr)
        ut.set_using_dict(self.setPatternWaitAddr, *addr)

    @property
    def patwaittime(self):
        """
        [Ctb][Mythen3][Xilinx Ctb] Wait time in clock cycles of loop level provided.
        
        Example
        -------
        >>> d.patwaittime[0] = 5
        >>> d.patwaittime[0]
        5
        >>> d.patwaittime
        0: 5
        1: 20
        2: 30
        """
        return PatWaitTimeProxy(self)

    @property
    @element
    def patwaittime0(self):
        """[Ctb][Mythen3][Xilinx Ctb] Wait 0 time in clock cycles."""
        return self.getPatternWaitTime(0)

    @patwaittime0.setter
    def patwaittime0(self, nclk):
        nclk = ut.merge_args(0, nclk)
        ut.set_using_dict(self.setPatternWaitTime, *nclk)

    @property
    @element
    def patwaittime1(self):
        """[Ctb][Mythen3][Xilinx Ctb] Wait 1 time in clock cycles."""
        return self.getPatternWaitTime(1)

    @patwaittime1.setter
    def patwaittime1(self, nclk):
        nclk = ut.merge_args(1, nclk)
        ut.set_using_dict(self.setPatternWaitTime, *nclk)

    @property
    @element
    def patwaittime2(self):
        """[Ctb][Mythen3][Xilinx Ctb] Wait 2 time in clock cycles."""
        return self.getPatternWaitTime(2)

    @patwaittime2.setter
    def patwaittime2(self, nclk):
        nclk = ut.merge_args(2, nclk)
        ut.set_using_dict(self.setPatternWaitTime, *nclk)


    @property
    def patloop(self):
        """
        [Ctb][Mythen3][Xilinx Ctb] Limits (start and stop address) of the loop provided.
        
        Example
        -------
        >>> d.patloop[0] = [5, 20]
        >>> d.patloop[0]
        [5, 20]
        >>> d.patloop
        0: [5, 20]
        1: [20, 4]
        2: [30, 5]
        """
        return PatLoopProxy(self)

    @property
    @element
    def patloop0(self):
        """[Ctb][Mythen3][Xilinx Ctb] Limits (start and stop address) of loop 0.
        
        Example
        ---------
        >>> d.patloop0 = [0x0, 0x18c]
        >>> d.patloop0
        [0, 396]
        >>> [hex(l) for l in d.patloop0]
        ['0x0', '0x18c']
        """
        return self.getPatternLoopAddresses(0)

    @patloop0.setter
    def patloop0(self, addr):
        addr = ut.merge_args(0, addr)
        ut.set_using_dict(self.setPatternLoopAddresses, *addr)

    @property
    @element
    def patloop1(self):
        """[Ctb][Mythen3][Xilinx Ctb] Limits (start and stop address) of loop 1.
        
        Example
        ---------
        >>> d.patloop1 = [0x0, 0x18c]
        >>> d.patloop1
        [0, 396]
        >>> [hex(l) for l in d.patloop1]
        ['0x0', '0x18c']
        
        """
        return self.getPatternLoopAddresses(1)

    @patloop1.setter
    def patloop1(self, addr):
        addr = ut.merge_args(1, addr)
        ut.set_using_dict(self.setPatternLoopAddresses, *addr)

    @property
    @element
    def patloop2(self):
        """[Ctb][Mythen3][Xilinx Ctb] Limits (start and stop address) of loop 2.
        
        Example
        ---------
        >>> d.patloop2 = [0x0, 0x18c]
        >>> d.patloop2
        [0, 396]
        >>> [hex(l) for l in d.patloop2]
        ['0x0', '0x18c']
        
        """
        return self.getPatternLoopAddresses(2)

    @patloop2.setter
    def patloop2(self, addr):
        addr = ut.merge_args(2, addr)
        ut.set_using_dict(self.setPatternLoopAddresses, *addr)


    @property
    def patnloop(self):
        """
        [Ctb][Mythen3][Xilinx Ctb] Number of cycles of the loop provided.
        
        Example
        -------
        >>> d.patnloop[0] = 5
        >>> d.patnloop[0]
        5
        >>> d.patnloop
        0: 5
        1: 20
        2: 30
        """
        return PatNLoopProxy(self)

    @property
    @element
    def patnloop0(self):
        """[Ctb][Mythen3][Xilinx Ctb] Number of cycles of loop 0."""
        return self.getPatternLoopCycles(0)

    @patnloop0.setter
    def patnloop0(self, n):
        n = ut.merge_args(0, n)
        ut.set_using_dict(self.setPatternLoopCycles, *n)

    @property
    @element
    def patnloop1(self):
        """[Ctb][Mythen3][Xilinx Ctb] Number of cycles of loop 1."""
        return self.getPatternLoopCycles(1)

    @patnloop1.setter
    def patnloop1(self, n):
        n = ut.merge_args(1, n)
        ut.set_using_dict(self.setPatternLoopCycles, *n)

    @property
    @element
    def patnloop2(self):
        """[Ctb][Mythen3][Xilinx Ctb] Number of cycles of loop 2."""
        return self.getPatternLoopCycles(2)

    @patnloop2.setter
    def patnloop2(self, n):
        n = ut.merge_args(2, n)
        ut.set_using_dict(self.setPatternLoopCycles, *n)

    @property
    @element
    def v_a(self):
        """[Ctb][Xilinx Ctb] Power supply a in mV."""
        return self.getPower(dacIndex.V_POWER_A)

    @v_a.setter
    def v_a(self, value):
        value = ut.merge_args(dacIndex.V_POWER_A, value)
        ut.set_using_dict(self.setPower, *value)

    @property
    @element
    def v_b(self):
        """[Ctb][Xilinx Ctb] Power supply b in mV."""
        return self.getPower(dacIndex.V_POWER_B)

    @v_b.setter
    def v_b(self, value):
        value = ut.merge_args(dacIndex.V_POWER_B, value)
        ut.set_using_dict(self.setPower, *value)

    @property
    @element
    def v_c(self):
        """[Ctb][Xilinx Ctb] Power supply c in mV."""
        return self.getPower(dacIndex.V_POWER_C)

    @v_c.setter
    def v_c(self, value):
        value = ut.merge_args(dacIndex.V_POWER_C, value)
        ut.set_using_dict(self.setPower, *value)

    @property
    @element
    def v_d(self):
        """[Ctb][Xilinx Ctb] Power supply d in mV."""
        return self.getPower(dacIndex.V_POWER_D)

    @v_d.setter
    def v_d(self, value):
        value = ut.merge_args(dacIndex.V_POWER_D, value)
        ut.set_using_dict(self.setPower, *value)

    @property
    @element
    def v_io(self):
        """[Ctb][Xilinx Ctb] Power supply io in mV. Minimum 1200 mV. 
        
        Note
        ----
        Must be the first power regulator to be set after fpga reset (on-board detector server start up).
        """
        return self.getPower(dacIndex.V_POWER_IO)

    @v_io.setter
    def v_io(self, value):
        value = ut.merge_args(dacIndex.V_POWER_IO, value)
        ut.set_using_dict(self.setPower, *value)

    @property
    @element
    def v_limit(self):
        """[Ctb][Xilinx Ctb] Soft limit for power supplies (ctb only) and DACS in mV."""
        return self.getPower(dacIndex.V_LIMIT)

    @v_limit.setter
    def v_limit(self, value):
        value = ut.merge_args(dacIndex.V_LIMIT, value)
        ut.set_using_dict(self.setPower, *value)


    @property
    @element
    def im_a(self):
        """[Ctb] Measured current of power supply a in mA.
              
        :setter: Not implemented
        """
        return self.getMeasuredCurrent(dacIndex.I_POWER_A)

    @property
    @element
    def im_b(self):
        """[Ctb] Measured current of power supply b in mA.
        
        :setter: Not implemented
        """
        return self.getMeasuredCurrent(dacIndex.I_POWER_B)

    @property
    @element
    def im_c(self):
        """[Ctb] Measured current of power supply c in mA.
                
        :setter: Not implemented
        """
        return self.getMeasuredCurrent(dacIndex.I_POWER_C)

    @property
    @element
    def im_d(self):
        """[Ctb] Measured current of power supply d in mA.
                
        :setter: Not implemented
        """
        return self.getMeasuredCurrent(dacIndex.I_POWER_D)

    @property
    @element
    def im_io(self):
        """[Ctb] Measured current of power supply io in mA.
                
        :setter: Not implemented
        """
        return self.getMeasuredCurrent(dacIndex.I_POWER_IO)

    @property
    def clkphase(self):
        """
        [Gotthard2][Mythen3] Phase shift of all clocks.\n
        [Gotthard2] Clock index range: 0-5\n
        [Mythen3] Clock index range: 0
        
        Example
        -------
        >>> d.clkphase[0] = 20
        >>> d.clkphase
        0: 20
        1: 10
        2: 20
        3: 10
        4: 10
        5: 5
        """
        return ClkPhaseProxy(self)

    @property
    def clkdiv(self):
        """
        [Gotthard2][Mythen3] Clock Divider of all clocks. Must be greater than 1.\n
        [Gotthard2] Clock index range: 0-5\n
        [Mythen3] Clock index range: 0
        
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
        
        :getter: always returns in seconds. To get in DurationWrapper, use getExptimeLeft
        :setter: Not Implemented
        
        Example
        -----------
        >>> d.exptimel
        181.23
        >>> d.getExptimeLeft()
        [sls::DurationWrapper(total_seconds: 181.23 count: 181230000000)]
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
        [Gotthard2][Mythen3] Frequency of clock in Hz.\n
        [Gotthard2] Clock index range: 0-5\n
        [Mythen3] Clock index range: 0
        
        
        :setter: Not implemented. Use clkdiv to set frequency

        Example
        -------
        >>> d.clkfreq[0]
        50000000
        """
        return ClkFreqProxy(self)


    def readout(self):
        """
        [Mythen3] Starts detector readout. Status changes to TRANSMITTING and automatically returns to idle at the end of readout.
        """
        self.startDetectorReadout()
    
    @property
    @element
    def polarity(self):
        """[Mythen3] Set positive or negative polarity. 
        Enum: polarity
        """
        return self.getPolarity()

    @polarity.setter
    def polarity(self, value):
        ut.set_using_dict(self.setPolarity, value)

    @property
    @element
    def interpolation(self):
        """[Mythen3] Enable or disable interpolation.  interpolation mode enables all counters and disables vth3. Disabling sets back counter mask and vth3. """
        return self.getInterpolation()

    @interpolation.setter
    def interpolation(self, value):
        ut.set_using_dict(self.setInterpolation, value)

    @property
    @element
    def pumpprobe(self):
        """[Mythen3] Enable or disable pump probe mode. Pump probe mode only enables vth2. Disabling sets back to previous value """
        return self.getPumpProbe()

    @pumpprobe.setter
    def pumpprobe(self, value):
        ut.set_using_dict(self.setPumpProbe, value)

    @property
    @element
    def apulse(self):
        """[Mythen3] Enable or disable analog pulsing. """
        return self.getAnalogPulsing()

    @apulse.setter
    def apulse(self, value):
        ut.set_using_dict(self.setAnalogPulsing, value)

    @property
    @element
    def dpulse(self):
        """[Mythen3] Enable or disable digital pulsing. """
        return self.getDigitalPulsing()

    @dpulse.setter
    def dpulse(self, value):
        ut.set_using_dict(self.setDigitalPulsing, value)


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