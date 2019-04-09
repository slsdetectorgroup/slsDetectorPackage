#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Dec  6 11:51:18 2017

@author: l_frojdh
"""

import socket
from collections.abc import Iterable
from collections import namedtuple
from functools import partial

from .adcs import Adc, DetectorAdcs
from .dacs import DetectorDacs
from .decorators import error_handling
from .detector import Detector
from .detector_property import DetectorProperty
from .utils import element_if_equal
from sls_detector.errors import DetectorValueError, DetectorError

class EigerVcmp:
    """
    Convenience class to be able to loop over vcmp for Eiger
    
    
    .. todo::
        
        Support single assignment and perhaps unify with Dac class
    
    """
    
    def __init__(self, detector):
        _names = ['vcmp_ll',
                  'vcmp_lr',
                  'vcmp_rl',
                  'vcmp_rr']
        self.set = []
        self.get = []
        for i in range(detector.n_modules):
            if i % 2 == 0:
                name = _names
            else:
                name = _names[::-1]
            for n in name:
                self.set.append(partial(detector._api.setDac, n, i))
                self.get.append(partial(detector._api.getDac, n, i))
    
    def __getitem__(self, key):
        if key == slice(None, None, None):
            return [_d() for _d in self.get]
        return self.get[key]()
    
    def __setitem__(self, i, value):
        self.set[i](value)

    def __repr__(self):
        return 'vcmp: '+ str(self[:])


class EigerDacs(DetectorDacs):
    _dacs = [('vsvp',    0, 4000,    0),
             ('vtr',     0, 4000, 2500),
             ('vrf',     0, 4000, 3300),
             ('vrs',     0, 4000, 1400),
             ('vsvn',    0, 4000, 4000),
             ('vtgstv',  0, 4000, 2556),
             ('vcmp_ll', 0, 4000, 1500),
             ('vcmp_lr', 0, 4000, 1500),
             ('vcall',   0, 4000, 4000),
             ('vcmp_rl', 0, 4000, 1500),
             ('rxb_rb',  0, 4000, 1100),
             ('rxb_lb',  0, 4000, 1100),
             ('vcmp_rr', 0, 4000, 1500),
             ('vcp',     0, 4000,  200),
             ('vcn',     0, 4000, 2000),
             ('vis',     0, 4000, 1550),
             ('iodelay', 0, 4000,  660)]
    _dacnames = [_d[0] for _d in _dacs]


# noinspection PyProtectedMember
class DetectorDelays:
    _delaynames = ['frame', 'left', 'right']

    def __init__(self, detector):
        # We need to at least initially know which detector we are connected to
        self._detector = detector
        
        setattr(self, '_frame', DetectorProperty(detector._api.getDelayFrame,
                                                 detector._api.setDelayFrame,
                                                 detector._api.getNumberOfDetectors,
                                                 'frame'))

        setattr(self, '_left', DetectorProperty(detector._api.getDelayLeft,
                                                detector._api.setDelayLeft,
                                                detector._api.getNumberOfDetectors,
                                                'left'))

        setattr(self, '_right', DetectorProperty(detector._api.getDelayRight,
                                                 detector._api.setDelayRight,
                                                 detector._api.getNumberOfDetectors,
                                                 'right'))
        # Index to support iteration
        self._current = 0

    def __getattr__(self, name):
        return self.__getattribute__('_' + name)

    def __setattr__(self, name, value):
        if name in self._delaynames:
            return self.__getattribute__('_' + name).__setitem__(slice(None, None, None), value)
        else:
            super().__setattr__(name, value)

    def __next__(self):
        if self._current >= len(self._delaynames):
            self._current = 0
            raise StopIteration
        else:
            self._current += 1
            return self.__getattr__(self._delaynames[self._current-1])

    def __iter__(self):
        return self

    def __repr__(self):
        hn = self._detector.hostname
        r_str = ['Transmission delay [ns]\n'
                 '{:11s}{:>8s}{:>8s}{:>8s}'.format('', 'left', 'right', 'frame')]
        for i in range(self._detector.n_modules):
            r_str.append('{:2d}:{:8s}{:>8d}{:>8d}{:>8d}'.format(i, hn[i], self.left[i], self.right[i], self.frame[i]))
        return '\n'.join(r_str)


class Eiger(Detector):
    """
    Subclassing Detector to set up correct dacs and detector specific 
    functions. 
    """
    _detector_dynamic_range = [4, 8, 16, 32]


    _settings = ['standard', 'highgain', 'lowgain', 'veryhighgain', 'verylowgain']
    """available settings for Eiger, note almost always standard"""

    def __init__(self, id=0):
        super().__init__(id)

        self._active = DetectorProperty(self._api.getActive,
                                        self._api.setActive,
                                        self._api.getNumberOfDetectors,
                                        'active')

        self._vcmp = EigerVcmp(self)
        self._dacs = EigerDacs(self)
        self._trimbit_limits = namedtuple('trimbit_limits', ['min', 'max'])(0, 63)
        self._delay = DetectorDelays(self)
        
        # Eiger specific adcs
        self._temp = DetectorAdcs()
        self._temp.fpga = Adc('temp_fpga', self)
        self._temp.fpgaext = Adc('temp_fpgaext', self)
        self._temp.t10ge = Adc('temp_10ge', self)
        self._temp.dcdc = Adc('temp_dcdc', self)
        self._temp.sodl = Adc('temp_sodl', self)
        self._temp.sodr = Adc('temp_sodr', self)
        self._temp.fpgafl = Adc('temp_fpgafl', self)
        self._temp.fpgafr = Adc('temp_fpgafr', self)

    @property
    def active(self):
        """
        Is the detector active? Can be used to enable or disable a detector
        module
        
        Examples
        ----------
        
        ::
            
            d.active
            >> active: [True, True]
            
            d.active[1] = False
            >> active: [True, False]
        """
        return self._active
    
    @active.setter
    def active(self, value):
        self._active[:] = value
    
    @property
    def measured_period(self):
        return self._api.getMeasuredPeriod()

    @property
    def measured_subperiod(self):
        return self._api.getMeasuredSubPeriod()

    @property
    def add_gappixels(self):
       """Enable or disable the (virual) pixels between ASICs

       Examples
       ----------

       ::

           d.add_gappixels = True

           d.add_gappixels
           >> True

       """
       return self._api.getGapPixels()

    @add_gappixels.setter
    def add_gappixels(self, value):
       self._api.setGapPixels(value)

    @property
    def dacs(self):
        """

        An instance of DetectorDacs used for accessing the dacs of a single
        or multi detector.

        Examples
        ---------

        ::

            d = Eiger()

            #Set all vrf to 1500
            d.dacs.vrf = 1500

            #Check vrf
            d.dacs.vrf
            >> vrf       :  1500,  1500

            #Set a single vtr
            d.dacs.vtr[0] = 1800

            #Set vrf with multiple values
            d.dacs.vrf = [3500,3700]
            d.dacs.vrf
            >> vrf       :  3500,  3700

            #read into a variable
            var = d.dacs.vrf[:]

            #set multiple with multiple values, mostly used for large systems
            d.dacs.vcall[0,1] = [3500,3600]
            d.dacs.vcall
            >> vcall     :  3500,  3600

            d.dacs
            >>
            ========== DACS =========
            vsvp      :     0,     0
            vtr       :  4000,  4000
            vrf       :  1900,  1900
            vrs       :  1400,  1400
            vsvn      :  4000,  4000
            vtgstv    :  2556,  2556
            vcmp_ll   :  1500,  1500
            vcmp_lr   :  1500,  1500
            vcall     :  4000,  4000
            vcmp_rl   :  1500,  1500
            rxb_rb    :  1100,  1100
            rxb_lb    :  1100,  1100
            vcmp_rr   :  1500,  1500
            vcp       :  1500,  1500
            vcn       :  2000,  2000
            vis       :  1550,  1550
            iodelay   :   660,   660

        """
        return self._dacs

    @property
    def tx_delay(self):
        """
        Transmission delay of the modules to allow running the detector
        in a network not supporting the full speed of the detector.


        ::
            
            d.tx_delay
            >>
            Transmission delay [ns]
                           left   right   frame
             0:beb048         0   15000       0
             1:beb049       100  190000     100
             
             d.tx_delay.left = [2000,5000]
        """
        return self._delay

    def default_settings(self):
        """
        reset the detector to some type of standard settings
        mostly used when testing
        """
        self.n_frames = 1
        self.exposure_time = 1
        self.period = 0
        self.n_cycles = 1
        self.n_measurements = 1
        self.dynamic_range = 16

    @property
    def eiger_matrix_reset(self):
        """
        Matrix reset bit for Eiger.

        :py:obj:`True` : Normal operation, the matrix is reset before each acq.
        :py:obj:`False` : Matrix reset disabled. Used to not reset before
        reading out analog test pulses.
        """
        return self._api.getCounterBit()

    @eiger_matrix_reset.setter
    def eiger_matrix_reset(self, value):
        self._api.setCounterBit(value)

    @property
    def flowcontrol_10g(self):
        """
        :py:obj:`True` - Flow control enabled :py:obj:`False` flow control disabled.
        Sets for all moduels, if for some reason access to a single module is needed
        this can be done trough the C++ API.

        """
        fc = self._api.getNetworkParameter('flow_control_10g')
        return element_if_equal([bool(int(e)) for e in fc])

    @flowcontrol_10g.setter
    def flowcontrol_10g(self, value):
        if value is True:
            v = '1'
        else:
            v = '0'
        self._api.setNetworkParameter('flow_control_10g', v, -1)

    def pulse_all_pixels(self, n):
        """
        Pulse each pixel of the chip **n** times using the analog test pulses.
        The pulse height is set using d.dacs.vcall with 4000 being 0 and 0 being
        the highest pulse.
        
        ::
            
            #Pulse all pixels ten times
            d.pulse_all_pixels(10)
            
            #Avoid resetting before acq
            d.eiger_matrix_reset = False
            
            d.acq() #take frame
            
            #Restore normal behaviour
            d.eiger_matrix_reset = True
        
        
        """
        self._api.pulseAllPixels(n)
        

    def pulse_diagonal(self, n):
        """
        Pulse pixels in super colums in a diagonal fashion. Used for calibration
        of vcall. Saves time compared to pulsing all pixels.
        """
        self._api.pulseDiagonal(n)


    def pulse_chip(self, n):
        """
        Advance the counter by toggling enable. Gives 2*n+2 int the counter
        
        """
        n = int(n)
        if n >= -1:
            self._api.pulseChip(n)
        else:
            raise ValueError('n must be equal or larger than -1')

    @property
    def vcmp(self):
        """
        Convenience function to get and set the individual vcmp of chips
        Used mainly in the calibration code.

        Examples
        ---------

        ::

            #Reading
            d.vcmp[:]
            >> [500, 500, 500, 500, 500, 500, 500, 500]

            #Setting
            d.vcmp = [500, 500, 500, 500, 500, 500, 500, 500]


        """

        return self._vcmp
    
    @vcmp.setter
    def vcmp(self, values):
        if len(values) == len(self._vcmp.set):
            for i, v in enumerate(values):
                self._vcmp.set[i](v)
        else:
            raise ValueError('vcmp only compatible with setting all')

    @property
    def rx_udpport(self):
        """
        UDP port for the receiver. Each module has two ports referred to
        as rx_udpport and rx_udpport2 in the command line interface
        here they are grouped for each detector
        
        ::
            
            [0:rx_udpport, 0:rx_udpport2, 1:rx_udpport ...]
        
        Examples
        -----------
        
        ::
        
            d.rx_udpport
            >> [50010, 50011, 50004, 50005]
            
            d.rx_udpport = [50010, 50011, 50012, 50013]
        
        """
        p0 = self._api.getReceiverUDPPort()
        p1 = self._api.getReceiverUDPPort2()
        return [int(val) for pair in zip(p0, p1) for val in pair]
    
    @rx_udpport.setter
    def rx_udpport(self, ports):
        """Requires iterating over elements two and two for setting ports"""
        a = iter(ports)
        for i, p in enumerate(zip(a, a)):
            self._api.setReceiverUDPPort(p[0], i)
            self._api.setReceiverUDPPort2(p[1], i)

    @property
    def rx_zmqport(self):
        """
        Return the receiver zmq ports. Note that Eiger has two ports per receiver!

        ::

            detector.rx_zmqport
            >> [30001, 30002, 30003, 30004]


        """
        _s = self._api.getReceiverStreamingPort()
        if _s == '':
            return []
        else:
            return [int(_p) + i for _p in _s for i in range(2)]

    @rx_zmqport.setter
    def rx_zmqport(self, port):
        if isinstance(port, Iterable):
            for i, p in enumerate(port):
                self._api.setReceiverStreamingPort(p, i)
        else:
            self._api.setReceiverStreamingPort(port, -1)


    @property
    def sub_exposure_time(self):
        """
        Sub frame exposure time in *seconds* for Eiger in 32bit autosumming mode

        ::

            d.sub_exposure_time
            >> 0.0023

            d.sub_exposure_time = 0.002

        """
        return self._api.getSubExposureTime() / 1e9

    
    @sub_exposure_time.setter
    def sub_exposure_time(self, t):
        #TODO! checking here or in the detector?
        ns_time = int(t * 1e9)
        if ns_time > 0:
            self._api.setSubExposureTime(ns_time)
        else:
            raise DetectorValueError('Sub exposure time must be larger than 0')

    @property
    def sub_deadtime(self):
        """
        Deadtime between subexposures. Used to mimize noise by delaying the start of the next
        subexposure.
        """
        return self._api.getSubExposureDeadTime() / 1e9

    
    @sub_deadtime.setter
    def sub_deadtime(self, t):
        ns_time = int(t * 1e9)
        if ns_time >= 0:
            self._api.setSubExposureDeadTime(ns_time)
        else:
            raise ValueError('Sub deadtime time must be larger or equal to 0')

    @property
    def temp(self):
        """
        An instance of DetectorAdcs used to read the temperature
        of different components

        Examples
        -----------

        ::

            detector.temp
            >>
            temp_fpga     :  36.90°C,  45.60°C
            temp_fpgaext  :  31.50°C,  32.50°C
            temp_10ge     :   0.00°C,   0.00°C
            temp_dcdc     :  36.00°C,  36.00°C
            temp_sodl     :  33.00°C,  34.50°C
            temp_sodr     :  33.50°C,  34.00°C
            temp_fpgafl   :  33.81°C,  30.93°C
            temp_fpgafr   :  27.88°C,  29.15°C

            a = detector.temp.fpga[:]
            a
            >> [36.568, 45.542]


        """
        return self._temp

    @property
    def tengiga(self):
        """Enable 10Gbit/s data output
        
        Examples
        ----------
        
        ::
            
            d.tengiga
            >> False
            
            d.tengiga = True
            
        """
        return self._api.getTenGigabitEthernet()
    
    @tengiga.setter
    def tengiga(self, value):
        self._api.setTenGigabitEthernet(value)

    def set_delays(self, delta):
        self.tx_delay.left = [delta*(i*2) for i in range(self.n_modules)]
        self.tx_delay.right = [delta*(i*2+1) for i in range(self.n_modules)]


    def setup500k(self, hostnames):
        """
        Setup the Eiger detector to run on the local machine
        """
        
        self.hostname = hostnames
        self.file_write = False
        self.image_size = (512, 1024)
        self.rx_tcpport = [1954, 1955]
        self.rx_udpport = [50010, 50011, 50004, 50005]
        self.rx_hostname = socket.gethostname().split('.')[0]
        self.rx_datastream = False
        self.file_write = False
        self.online = True
        self.receiver_online = True
