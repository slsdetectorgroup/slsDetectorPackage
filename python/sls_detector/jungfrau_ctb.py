from functools import partial
from collections.abc import Iterable
from collections import namedtuple
import socket

from .detector import Detector
from .utils import element_if_equal
from .adcs import DetectorAdcs, Adc
from .dacs import DetectorDacs
from .detector_property import DetectorProperty
from .decorators import error_handling
from .registers import Register, Adc_register

class JungfrauCTBDacs(DetectorDacs):
    _dacs = [('dac0',  0, 4000,    1400),
             ('dac1',  0, 4000,    1200),
             ('dac2',  0, 4000,    900),
             ('dac3',  0, 4000,    1050),
             ('dac4',  0, 4000,     1400),
             ('dac5',  0, 4000,    655),
             ('dac6',  0, 4000,    2000),
             ('dac7',  0, 4000,     1400),
             ('dac8',  0, 4000,    850),
             ('dac9',  0, 4000,    2000),
             ('dac10', 0, 4000,    2294),
             ('dac11', 0, 4000,    983),
             ('dac12', 0, 4000,    1475),
             ('dac13', 0, 4000,    1200),
             ('dac14', 0, 4000,    1600),
             ('dac15', 0, 4000,    1455),
             ('dac16', 0, 4000,       0),
             ('dac17', 0, 4000,    1000),
            ]
    _dacnames = [_d[0] for _d in _dacs]



class JungfrauCTB(Detector):
    def __init__(self, id = 0):
        super().__init__(id)
        self._dacs = JungfrauCTBDacs(self)
        self._register = Register(self)
        self._adc_register = Adc_register(self)

    @property
    def v_a(self):
        return self._api.getDac_mV('v_a', -1)

    @v_a.setter
    def v_a(self, value):
        self._api.setDac_mV('v_a', -1, value)

    @property
    def v_b(self):
        return self._api.getDac_mV('v_b', -1)

    @v_b.setter
    def v_b(self, value):
        self._api.setDac_mV('v_b', -1, value)


    @property
    def v_c(self):
        return self._api.getDac_mV('v_c', -1)

    @v_c.setter
    def v_c(self, value):
        self._api.setDac_mV('v_c', -1, value)

    @property
    def v_d(self):
        return self._api.getDac_mV('v_d', -1)

    @v_d.setter
    def v_d(self, value):
        self._api.setDac_mV('v_d', -1, value)
        
    @property
    def v_io(self):
        return self._api.getDac_mV('v_io', -1)

    @v_io.setter
    def v_io(self, value):
        self._api.setDac_mV('v_io', -1, value)        

    @property
    def v_limit(self):
        return self._api.getDac_mV('v_limit', -1)

    @v_limit.setter
    def v_limit(self, value):
        self._api.setDac_mV('v_limit', -1, value)
        
    @property
    def adc_register(self):
        return self._adc_register

    # @property
    # def register(self):
    #     return self._register

    def adcOFF(self):
        """Switch off the ADC"""
        self.adc_register[0x8] = 1



    @property
    def dacs(self):
        """

        An instance of DetectorDacs used for accessing the dacs of a single
        or multi detector.

        Examples
        ---------

        ::

            #JungfrauCTB


        """
        return self._dacs

    @property
    def dbitpipeline(self):
        return self._api.getDbitPipeline()

    @dbitpipeline.setter
    def dbitpipeline(self, value):
        self._api.setDbitPipeline(value)


    @property
    def dbitphase(self):
        return self._api.getDbitPhase()

    @dbitphase.setter
    def dbitphase(self, value):
        self._api.setDbitPhase(value)

    @property
    def dbitclock(self):
        return self._api.getDbitClock()

    @dbitclock.setter
    def dbitclock(self, value):
        self._api.setDbitClock(value)

    @property
    def samples(self):
        return self._api.getJCTBSamples()

    @samples.setter
    def samples(self, value):
        self._api.setJCTBSamples(value)

    @property
    def readout_clock(self):
        """
        Speed of the readout clock relative to the full speed


        Examples
        ---------

        ::




        """
        return  self._api.getReadoutClockSpeed()


    @readout_clock.setter
    @error_handling
    def readout_clock(self, value):
        self._api.setReadoutClockSpeed(value)
