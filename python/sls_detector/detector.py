#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Python - sls
=============

"""
import os
from collections.abc import Iterable
from collections import namedtuple

from _sls_detector import DetectorApi
from .decorators import error_handling
from .detector_property import DetectorProperty
from .errors import DetectorError, DetectorValueError
from .registers import Register
from .utils import element_if_equal


class Detector:
    """
    Base class used as interface with the slsDetectorSoftware. To control a specific detector use the
    derived classes such as Eiger and Jungfrau. Functions as an interface to the C++ API and provides a 
    more Pythonic interface
    """

    _speed_names = {0: 'Full Speed', 1: 'Half Speed', 2: 'Quarter Speed', 3: 'Super Slow Speed'}
    _speed_int = {'Full Speed': 0, 'Half Speed': 1, 'Quarter Speed': 2, 'Super Slow Speed': 3}
    _settings = []

    def __init__(self, multi_id=0):
        self._api = DetectorApi(multi_id)
        self._register = Register(self)

        self._flippeddatax = DetectorProperty(self._api.getFlippedDataX,
                                              self._api.setFlippedDataX,
                                              self._api.getNumberOfDetectors,
                                              'flippeddatax')
        self._flippeddatay = DetectorProperty(self._api.getFlippedDataY,
                                              self._api.setFlippedDataY,
                                              self._api.getNumberOfDetectors,
                                              'flippeddatay')
        try:
            self.online = True
            self.receiver_online = True
        except DetectorError:
            print('WARNING: Cannot connect to detector')


    def __len__(self):
        return self._api.getNumberOfDetectors()

    def __repr__(self):
        return '{}(id = {})'.format(self.__class__.__name__,
                                    self._api.getMultiDetectorId())


    def acq(self):
        """
        Blocking command to launch the programmed measurement. Number of frames specified by frames, cycles etc.
        """
        self._api.acq()


    @property
    def busy(self):
        """
        Checks the detector is acquiring. Can also be set but should only be used if the acquire fails and
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
        return self._api.getAcquiringFlag()

    @busy.setter
    def busy(self, value):
        self._api.setAcquiringFlag(value)

    def clear_errors(self):
        """Clear the error mask for the detector. Used to reset after checking."""
        self._api.clearErrorMask()

    @property
    def client_version(self):
        """
        :py:obj:`str` The date of commit for the client API version

        Examples
        ----------

        ::

            d.client_version
            >> '20180327'

        """
        v = hex(self._api.getClientVersion())
        return v[2:]

    @property
    def detectornumber(self):
        """
        Get all detector numbers as a list. For Eiger the detector numbers
        correspond to the beb numbers.

        Examples
        ---------

        ::

            #for beb083 and beb098
            detector.detector_number
            >> [83, 98]

        """
        return self._api.getDetectorNumber()

    @property
    def detector_type(self):
        """
        Return either a string or list of strings with the detector type.

        * Eiger
        * Jungfrau
        * etc.

        Examples
        ----------

        ::

            detector.detector_type
            >> 'Eiger'

            detector.detector_type
            >> ['Eiger',  'Jungfrau']

        """
        return element_if_equal(self._api.getDetectorType())

    @property
    def dynamic_range(self):
        """
        :obj:`int`: Dynamic range of the detector.

        +----+-------------+------------------------------+
        | dr |  max counts |    comments                  |
        +====+=============+==============================+
        | 4  |       15    |                              |
        +----+-------------+------------------------------+
        | 8  |      255    |                              |
        +----+-------------+------------------------------+
        |16  |     4095    | 12 bit internally            |
        +----+-------------+------------------------------+
        |32  |  4294967295 | Autosumming of 12 bit frames |
        +----+-------------+------------------------------+

        Raises
        -------
        ValueError
            If the dynamic range is not available in the detector


        """
        return self._api.getDynamicRange()

    @dynamic_range.setter
    def dynamic_range(self, dr):
        if dr in self._detector_dynamic_range:
            self._api.setDynamicRange(dr)
            return
        else:
            raise DetectorValueError('Cannot set dynamic range to: {:d} availble options: '.format(dr),
                                     self._detector_dynamic_range)


    @property
    def exposure_time(self):
        """
        :obj:`double` Exposure time in [s] of a single frame.
        """
        return self._api.getExposureTime() / 1e9

    @exposure_time.setter
    def exposure_time(self, t):
        ns_time = int(t * 1e9)
        if ns_time <= 0:
            raise DetectorValueError('Exposure time must be larger than 0')
        self._api.setExposureTime(ns_time)

    @property
    def file_index(self):
        """
        :obj:`int` Index for frames and file names

        Raises
        -------
        ValueError
            If the user tries to set an index less than zero

        Examples
        ---------

        ::

            detector.file_index
            >> 0

            detector.file_index = 10
            detector.file_index
            >> 10

        """
        return self._api.getFileIndex()

    @file_index.setter
    def file_index(self, i):
        if i < 0:
            raise ValueError('Index needs to be positive')
        self._api.setFileIndex(i)

    @property
    def file_name(self):
        """
        :obj:`str`: Base file name for writing images

        Examples
        ---------

        ::

            detector.file_name
            >> 'run'

            detector.file_name = 'myrun'

            #For a single acquisition the detector now writes
            # myrun_master_0.raw
            # myrun_d0_0.raw
            # myrun_d1_0.raw
            # myrun_d2_0.raw
            # myrun_d3_0.raw

        """
        return self._api.getFileName()

    @file_name.setter
    def file_name(self, fname):
        self._api.setFileName(fname)

    @property
    def file_path(self):
        """
        :obj:`str`: Path where images are written

        Raises
        -------
        FileNotFoundError
            If path does not exists

        Examples
        ---------

        ::

            detector.file_path
            >> '/path/to/files'

            detector.file_path = '/new/path/to/other/files'

        """
        fp = self._api.getFilePath()
        if fp == '':
            return [self._api.getFilePath(i) for i in range(len(self))]
        else:
            return fp

    @file_path.setter
    def file_path(self, path):
        if os.path.exists(path) is True:
            self._api.setFilePath(path)
        else:
            raise FileNotFoundError('File path does not exists')

    @property
    def file_write(self):
        """
        :obj:`bool` If True write files to disk
        """
        return self._api.getFileWrite()

    @file_write.setter
    def file_write(self, fwrite):
        self._api.setFileWrite(fwrite)


    @property
    def file_overwrite(self):
        """
        :obj:`bool` If true overwrite files on disk
        """
        return self._api.getFileOverWrite()

    @file_overwrite.setter
    def file_overwrite(self, value):
        self._api.setFileOverWrite(value)

    @property
    def file_padding(self):
        """
        Pad files in the receiver
        :obj:`bool` If true pads partial frames
        """
        return self._api.getReceiverPartialFramesPadding()

    @file_padding.setter
    def file_padding(self, value):
        self._api.getReceiverPartialFramesPadding(value)

    @property
    def firmware_version(self):
        """
        :py:obj:`int` Firmware version of the detector
        """
        return self._api.getFirmwareVersion()

    @property
    def flags(self):
        """Read and set flags. Accepts both single flag as
        string or list of flags.

        Raises
        --------
        RuntimeError
            If flag not recognized


        Examples
        ----------

        ::

            #Eiger
            detector.flags
            >> ['storeinram', 'parallel']

            detector.flags = 'nonparallel'
            detector.flags
            >> ['storeinram', 'nonparallel']

            detector.flags = ['continous', 'parallel']


        """
        return self._api.getReadoutFlags()

    @flags.setter
    def flags(self, flags):
        if isinstance(flags, str):
            self._api.setReadoutFlag(flags)
        elif isinstance(flags, Iterable):
            for f in flags:
                self._api.setReadoutFlag(f)

    @property
    def frames_caught(self):
        """
        Number of frames caught by the receiver. Can be used to check for
        package loss.
        """
        return self._api.getFramesCaughtByReceiver()


    @property
    def frame_discard_policy(self):
        """
        Decides what the receiver does when packet loss occurs.
        nodiscard - keep all frames 
        discardempty - discard only empty frames
        discardpartial - discard partial and empty frames
        """
        return self._api.getReceiverFrameDiscardPolicy()

    @frame_discard_policy.setter
    def frame_discard_policy(self, policy):
        self._api.setReceiverFramesDiscardPolicy(policy)


    @property
    def api_compatibility(self):
        Compatibility = namedtuple('Compatibility', ['client_detector', 'client_receiver'])
        c = Compatibility(self._api.isClientAndDetectorCompatible(), self._api.isClientAndReceiverCompatible())
        return c

    @property
    def frame_padding(self):
        """
        Padd partial frames in the receiver
        """
        return self._api.getPartialFramesPadding()
    
    @frame_padding.setter
    def frame_padding(self, padding):
        self._api.setPartialFramesPadding(padding)

    def free_shared_memory(self):
        """
        Free the shared memory that contains the detector settings
        and reinitialized with 0 detectors so that you can keep
        using the same object.

        """
        self._api.freeSharedMemory()
        self.__init__(self._api.getMultiDetectorId())

    @property
    def flipped_data_x(self):
        """Flips data on x axis. Set for eiger bottom modules"""
        return self._flippeddatax

    @property
    def flipped_data_y(self):
        """Flips data on y axis."""
        return self._flippeddatax

    @property
    def high_voltage(self):
        """
        High voltage applied to the sensor
        """
        return self._api.getDac('vhighvoltage', -1)

    @high_voltage.setter
    def high_voltage(self, voltage):
        voltage = int(voltage)
        if voltage < 0 or voltage > 200:
            raise DetectorValueError('High voltage {:d}V is out of range.  Should be between 0-200V'.format(voltage))
        self._api.setDac('vhighvoltage', -1, voltage)


    @property
    def hostname(self):
        """
        :obj:`list` of :obj:`str`: hostnames of all connected detectors

        Examples
        ---------

        ::

            detector.hostname
            >> ['beb059', 'beb058']

        """
        _hm = self._api.getHostname()
        if _hm == '':
            return []
        return _hm.strip('+').split('+')


    @hostname.setter
    def hostname(self, hn):
        if isinstance(hn, str):
            self._api.setHostname(hn)
        else:
            name = ''.join([''.join((h, '+')) for h in hn])
            self._api.setHostname(name)

    @property
    def image_size(self):
        """
        :py:obj:`collections.namedtuple` with the image size of the detector
        Also works setting using a normal tuple

        .. note ::

            Follows the normal convention in Python of (rows, cols)

        Examples
        ----------

        ::

            d.image_size = (512, 1024)

            d.image_size
            >> ImageSize(rows=512, cols=1024)

            d.image_size.rows
            >> 512

            d.image_size.cols
            >> 1024

        """
        size = namedtuple('ImageSize', ['rows', 'cols'])
        return size(*self._api.getImageSize())

    @image_size.setter
    def image_size(self, size):
        self._api.setImageSize(*size)

    
    def load_config(self, fname):
        """
        Load detector configuration from a configuration file

        Raises
        --------
        FileNotFoundError
            If the file does not exists

        """
        if os.path.isfile(fname):
            self._api.readConfigurationFile(fname)
        else:
            raise FileNotFoundError('Cannot find configuration file')

    
    def load_parameters(self, fname):
        """
        Setup detector by executing commands in a parameters file


        .. note ::

            If you are relying mainly on the Python API it is probably
            better to track the settings from Python. This function uses
            parameters stored in a text file and the command line commands.

        Raises
        --------
        FileNotFoundError
            If the file does not exists

        """
        if os.path.isfile(fname):
            self._api.readParametersFile(fname)
        else:
            raise FileNotFoundError('Cannot find parameters file')

    
    def load_trimbits(self, fname, idet=-1):
        """
        Load trimbit file or files. Either called with detector number or -1
        to try to load detector specific trimbit files

        Parameters
        -----------
        fname:
            :py:obj:`str` Filename (including path) to the trimbit files

        idet
            :py:obj:`int` Detector to load trimbits to, -1 for all


        ::

            #Assuming 500k consisting of beb049 and beb048
            # 0 is beb049
            # 1 is beb048

            #Load name.sn049 to beb049 and name.sn048 to beb048
            detector.load_trimbits('/path/to/dir/name')

            #Load one file to a specific detector
            detector.load_trimbits('/path/to/dir/name.sn049', 0)

        """
        self._api.loadTrimbitFile(fname, idet)


    @property
    def lock(self):
        """Lock the detector to this client

        ::

            detector.lock = True

        """
        return self._api.getServerLock()

    @lock.setter
    def lock(self, value):
        self._api.setServerLock(value)

    @property
    def lock_receiver(self):
        """Lock the receivers to this client

        ::

            detector.lock_receiver = True

        """

        return self._api.getReceiverLock()

    @lock_receiver.setter
    def lock_receiver(self, value):
        self._api.setReceiverLock(value)

    @property
    def module_geometry(self):
        """
        :obj:`namedtuple` Geometry(horizontal=nx, vertical=ny)
         of the detector modules.

         Examples
         ---------

         ::

             detector.module_geometry
             >> Geometry(horizontal=1, vertical=2)

             detector.module_geometry.vertical
             >> 2

             detector.module_geometry[0]
             >> 1

        """
        _t = self._api.getDetectorGeometry()
        Geometry = namedtuple('Geometry', ['horizontal', 'vertical'])
        return Geometry(horizontal=_t[0], vertical=_t[1])

    @property
    def n_frames(self):
        """
        :obj:`int` Number of frames per acquisition
        """
        return self._api.getNumberOfFrames()

    @n_frames.setter
    def n_frames(self, n):
        if n >= 1:
            self._api.setNumberOfFrames(n)
        else:
            raise DetectorValueError('Invalid value for n_frames: {:d}. Number of'\
                             ' frames should be an integer greater than 0'.format(n))

    @property
    def frames_per_file(self):
        return self._api.getFramesPerFile()

    @frames_per_file.setter
    def frames_per_file(self, n):
        self._api.setFramesPerFile(n)

    @property
    def n_cycles(self):
        """Number of cycles for the measurement (exp*n_frames)*n_cycles"""
        return self._api.getCycles()

    @n_cycles.setter
    def n_cycles(self, n_cycles):
        if n_cycles > 0:
            self._api.setCycles(n_cycles)
        else:
            raise DetectorValueError('Number of cycles must be positive')

    @property
    def n_measurements(self):
        """
        Number of times to repeat the programmed measurement.
        This is the outer most part. Real time operation is not
        guaranteed since this is software controlled.

        Examples
        ----------

        ::

            detector.n_frames = 1
            detector.n_cycles = 1
            detector.n_measurements = 3

            detector.acq() # 1 frame 3 times

            detector.n_frames = 5
            detector.n_cycles = 3
            detector.n_measurements = 2

            detector.acq() # 5x3 frames 2 times total 30 frames

        """
        return self._api.getNumberOfMeasurements()

    @n_measurements.setter
    def n_measurements(self, value):
        if value > 0:
            self._api.setNumberOfMeasurements(value)
        else:
            raise DetectorValueError('Number of measurements must be positive')

    @property
    def n_modules(self):
        """
        :obj:`int` Number of (half)modules in the detector

        Examples
        ---------

        ::

            detector.n_modules
            >> 2

        """
        return self._api.getNumberOfDetectors()

    @property
    def online(self):
        """Online flag for the detector

        Examples
        ----------

        ::

            d.online
            >> False

            d.online = True

        """
        return self._api.getOnline()

    @online.setter
    def online(self, value):
        self._api.setOnline(value)


    @property
    def last_client_ip(self):
        """Returns the ip address of the last client
        that accessed the detector

        Returns
        -------

        :obj:`str` last client ip

        Examples
        ----------

        ::

            detector.last_client_ip
            >> '129.129.202.117'

        """
        return self._api.getLastClientIP()

    @property
    def receiver_last_client_ip(self):
        """Returns the ip of the client last talking to the receiver"""
        return self._api.getReceiverLastClientIP()

    @property
    def receiver_online(self):
        """
        Online flag for the receiver. Is set together with detector.online when creating the detector object

        Examples
        ---------

        ::

            d.receiver_online
            >> True

            d.receiver_online = False

        """
        return self._api.getReceiverOnline()

    @receiver_online.setter
    def receiver_online(self, value):
        self._api.setReceiverOnline(value)

    @property
    def receiver_version(self):
        """
        :py:obj:`str` Receiver version as a string. [yearmonthday]

        Examples
        ----------

        ::

            d.receiver_version
            >> '20180327'

        """
        v = hex(self._api.getReceiverVersion())
        return v[2:]

    #When returning instance error hadling needs to be done in the
    #class that is returned
    @property
    def register(self):
        """Directly manipulate registers on the readout board

        Examples
        ---------

        ::

            d.register[0x5d] = 0xf00

        """
        return self._register

    def reset_frames_caught(self):
        """
        Reset the number of frames caught by the receiver.

        .. note ::

            Automatically done when using d.acq()

        """
        self._api.resetFramesCaught()

    @property
    def period(self):
        """
        :obj:`double` Period between start of frames. Set to 0 for the detector
        to choose the shortest possible
        """
        _t = self._api.getPeriod()
        return _t / 1e9

    @period.setter
    def period(self, t):
        ns_time = int(t * 1e9)
        if ns_time < 0:
            raise ValueError('Period must be 0 or larger')
        self._api.setPeriod(ns_time)

    @property
    def rate_correction(self):
        """
        :obj:`list` of :obj:`double` Rate correction for all modules.
        Set to 0 for **disabled**

        .. todo ::

            Should support individual assignments

        Raises
        -------
        ValueError
            If the passed list is not of the same length as the number of
            detectors

        Examples
        ---------

        ::

            detector.rate_correction
            >> [125.0, 155.0]

            detector.rate_correction = [125, 155]


        """
        return self._api.getRateCorrection()

    @rate_correction.setter
    def rate_correction(self, tau_list):
        if len(tau_list) != self.n_modules:
            raise ValueError('List of tau needs the same length')
        self._api.setRateCorrection(tau_list)


    @property
    def readout_clock(self):
        """
        Speed of the readout clock relative to the full speed

        * Full Speed
        * Half Speed
        * Quarter Speed
        * Super Slow Speed

        Examples
        ---------

        ::

            d.readout_clock
            >> 'Half Speed'

            d.readout_clock = 'Full Speed'


        """
        speed = self._api.getReadoutClockSpeed()
        return self._speed_names[speed]

    @readout_clock.setter
    def readout_clock(self, value):
        speed = self._speed_int[value]
        self._api.setReadoutClockSpeed(speed)

    @property
    def receiver_frame_index(self):
        return self._api.getReceiverCurrentFrameIndex()

    @property
    def rx_datastream(self):
        """
        Zmq datastream from receiver. :py:obj:`True` if enabled and :py:obj:`False`
        otherwise

        ::

            #Enable data streaming from receiver
            detector.rx_datastream = True

            #Check data streaming
            detector.rx_datastream
            >> True

        """
        return self._api.getRxDataStreamStatus()

    @rx_datastream.setter
    def rx_datastream(self, status):
        self._api.setRxDataStreamStatus(status)


    @property
    def rx_hostname(self):
        """
        Receiver hostname
        TODO! setting of individual hostnames, now done with API call
        """
        return self._api.getReceiverHostname()


    @rx_hostname.setter
    def rx_hostname(self, name):
        self._api.setReceiverHostname(name)


    @property
    def rx_udpip(self):
        """
        Receiver UDP ip
        """
        return self._api.getReceiverUDPIP(-1)
        

    @rx_udpip.setter
    def rx_udpip(self, ip):
        if isinstance(ip, list):
            for i, addr in enumerate(ip):
                self._api.setReceiverUDPIP(addr, i)
        else:
            self._api.setReceiverUDPIP(ip, -1)


    @property
    def rx_udpmac(self):
        return self._api.getReceiverUDPMAC(-1)

    @rx_udpmac.setter
    def rx_udpmac(self, mac):
        if isinstance(mac, list):
            for i, m in enumerate(mac):
                self._api.setReceiverUDPMAC(m, i)
        else:
            self._api.setReceiverUDPMAC(mac, -1)

    @property
    def rx_tcpport(self):
        return self._api.getReceiverPort()

    @rx_tcpport.setter
    def rx_tcpport(self, ports):
        if len(ports) != len(self):
            raise ValueError('Number of ports: {} not equal to number of '
                             'detectors: {}'.format(len(ports), len(self)))
        else:
            for i, p in enumerate(ports):
                self._api.setReceiverPort(i, p)

    @property
    def rx_zmqip(self):
        """
        ip where the receiver streams data
        """
        ip = self._api.getNetworkParameter('rx_zmqip')
        return element_if_equal(ip)

    @rx_zmqip.setter
    def rx_zmqip(self, ip):
        self._api.setNetworkParameter('rx_zmqip', ip, -1)


    @property
    def detectormac(self):
        """
        Read detector mac address
        """
        mac = self._api.getNetworkParameter('detectormac')
        return element_if_equal(mac)

    @property
    def detectorip(self):
        """
        Read detector ip address
        """
        return self._api.getDetectorIp(-1)


    # @detectorip.setter
    # def detectorip(self, ip):


    @property
    def client_zmqip(self):
        """
        Ip address where the client listens to zmq stream
        """
        ip = self._api.getNetworkParameter('client_zmqip')
        return element_if_equal(ip)

    @client_zmqip.setter
    
    def client_zmqip(self, ip):
        self._api.setNetworkParameter('client_zmqip', ip, -1)



    @property
    def rx_fifodepth(self):
        """
        Fifo depth of receiver in number of frames
        """
        return self._api.getReceiverFifoDepth()

    @rx_fifodepth.setter
    def rx_fifodepth(self, n_frames):
        self._api.setReceiverFifoDepth(n_frames)


    @property
    def rx_udpsocksize(self):
        """
        UDP buffer size
        """
        buffer_size = [int(s) for s in self._api.getNetworkParameter('rx_udpsocksize')]
        return element_if_equal(buffer_size)       

    @property
    def rx_jsonaddheader(self):
        """
        UDP buffer size
        """
        header = self._api.getNetworkParameter('rx_jsonaddheader')
        return element_if_equal(header)  

    @rx_jsonaddheader.setter
    def rx_jsonaddheader(self, header):
        self._api.setNetworkParameter('rx_jsonaddheader', header, -1)



    @rx_udpsocksize.setter
    def rx_udpsocksize(self, buffer_size):
        self._api.setNetworkParameter('rx_udpsocksize', str(buffer_size), -1)


    @property
    def rx_realudpsocksize(self):
        """
        UDP buffer size
        """
        buffer_size = [int(s) for s in self._api.getNetworkParameter('rx_realudpsocksize')]
        return element_if_equal(buffer_size) 


    @property
    def rx_zmqport(self):
        """
        Return the receiver zmq ports.

        ::

            detector.rx_zmqport
            >> [30001, 30002]

        """
        _s = self._api.getNetworkParameter('rx_zmqport')
        if _s == '':
            return []
        else:
            return [int(_p) for _p in _s]

    @rx_zmqport.setter
    def rx_zmqport(self, port):
        if isinstance(port, Iterable):
            for i, p in enumerate(port):
                self._api.setNetworkParameter('rx_zmqport', str(p), i)
        else:
            self._api.setNetworkParameter('rx_zmqport', str(port), -1)

# Add back when versioning is defined
#    @property
#    def software_version(self):
#        return self._api.getSoftwareVersion();


    @property
    def user(self):
        return self._api.getUserDetails()

    @property
    def server_version(self):
        """
        :py:obj:`int` On-board server version of the detector
        """
        return hex(self._api.getServerVersion())

    @property
    def settings(self):
        """
        Detector settings used to control for example calibration or gain
        switching. For EIGER almost always standard standard.

        .. warning ::

            For Eiger setting settings should be followed by setting the threshold
            otherwise reading of the settings will overwrite the set value


        """
        return self._api.getSettings()

    @settings.setter
    def settings(self, s):
        if s in self._settings:
            self._api.setSettings(s)
        else:
            raise DetectorValueError('Settings: {:s}, not defined for {:s}. '
                                     'Valid options are: [{:s}]'.format(s, self.detector_type, ', '.join(self._settings)))


    @property
    def settings_path(self):
        """
        The path where the slsDetectorSoftware looks for settings/trimbit files
        """
        return self._api.getSettingsDir()

    @settings_path.setter
    def settings_path(self, path):
        if os.path.isdir(path):
            self._api.setSettingsDir(path)
        else:
            raise FileNotFoundError('Settings path does not exist')

    @property
    def status(self):
        """
        :py:obj:`str` Status of the detector: idle, running,

        .. todo ::

            Check possible values

        """
        return self._api.getRunStatus()

    def start_detector(self):
        """
        Non blocking command to star acquisition. Needs to be used in combination
        with receiver start.
        """
        self._api.startAcquisition()

    def stop_detector(self):
        """
        Stop acquisition early or if the detector hangs
        """
        self._api.stopAcquisition()


    def start_receiver(self):
        self._api.startReceiver()

    def stop_receiver(self):
        self._api.stopReceiver()

    @property
    def threaded(self):
        """
        Enable parallel execution of commands to the different detector modules

        Examples
        ----------

        ::

            d.threaded
            >> True

            d.threaded = False

        """
        return self._api.getThreadedProcessing()

    @threaded.setter
    def threaded(self, value):
        self._api.setThreadedProcessing(value)

    @property
    def threshold(self):
        """
        Detector threshold in eV
        """
        return self._api.getThresholdEnergy()

    @threshold.setter
    def threshold(self, eV):
        self._api.setThresholdEnergy(eV)

    @property
    def timing_mode(self):
        """
        :py:obj:`str` Timing mode of the detector

        * **auto** Something
        * **trigger** Something else


        """
        return self._api.getTimingMode()

    @timing_mode.setter
    def timing_mode(self, mode):
        self._api.setTimingMode(mode)


    @property
    def trimmed_energies(self):
        """
        EIGER: the energies at which the detector was trimmed. This also sets
        the range for which the calibration of the detector is valid.


        ::

            detector.trimmed_energies = [5400, 6400, 8000]

            detector.trimmed_energies
            >> [5400, 6400, 8000]

        """

        return self._api.getTrimEnergies()

    @trimmed_energies.setter
    def trimmed_energies(self, energy_list):
        self._api.setTrimEnergies(energy_list)

    @property
    def vthreshold(self):
        """
        Threshold in DAC units for the detector. Sets the individual vcmp of
        all chips in the detector.
        """
        return self._api.getDac('vthreshold', -1)

    @vthreshold.setter
    def vthreshold(self, th):
        self._api.setDac('vthreshold', -1, th)

    @property
    def trimbits(self):
        """
        Set or read trimbits of the detector.

        Examples
        ---------

        ::

            #Set all to 32
            d.trimbits = 32

            d.trimbits
            >> 32

            #if undefined or different
            d.trimbits
            >> -1

        """
        return self._api.getAllTrimbits()

    @trimbits.setter
    def trimbits(self, value):
        if self._trimbit_limits.min <= value <= self._trimbit_limits.max:
            self._api.setAllTrimbits(value)
        else:
            raise DetectorValueError('Trimbit setting {:d} is  outside of range:'\
                             '{:d}-{:d}'.format(value, self._trimbit_limits.min, self._trimbit_limits.max))

    @property
    def client_zmqport(self):
        """zmq port of the client"""
        _s = self._api.getNetworkParameter('client_zmqport')
        if _s == '':
            return []
        return [int(_p)+i for _p in _s for i in range(2)]

    
    def _provoke_error(self):
        self._api.setErrorMask(1)


    def config_network(self):
        """
        Configures the detector source and destination MAC addresses, IP addresses
        and UDP ports, and computes the IP header checksum for such parameters
        """
        self._api.configureNetworkParameters()


    #TODO! can we make this one function?
    @property
    def patnloop0(self):
        return self._api.getPatternLoops(0, -1)

    @patnloop0.setter
    def patnloop0(self, n):
        self._api.setPatternLoops(0, -1, -1, n, -1)

    @property
    def patnloop1(self):
        return self._api.getPatternLoops(1, -1)

    @patnloop1.setter
    def patnloop1(self, n):
        self._api.setPatternLoops(1, -1, -1, n, -1)

    @property
    def patnloop2(self):
        return self._api.getPatternLoops(2, -1)

    @patnloop2.setter
    def patnloop2(self, n):
        self._api.setPatternLoops(2, -1, -1, n, -1)

    def setPatternWord(self, addr, word, det_id = -1):
        self._api.setPatternWord(addr, word, det_id)

    def setPatternLoops(self, level, start, stop, n, det_id=-1):
        self._api.setPatternLoops(level, start, stop, n, det_id)


def free_shared_memory(multi_id=0):
    """
    Function to free the shared memory but do not initialize with new
    0 size detector
    """
    api = DetectorApi(multi_id)
    api.freeSharedMemory()
