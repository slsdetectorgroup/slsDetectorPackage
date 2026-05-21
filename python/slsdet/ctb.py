# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
from .detector import Detector, freeze
from .utils import element_if_equal
from .dacs import NamedDacs
from .powers import NamedPowers
from .proxy import SlowAdcProxy
from . import _slsdet
dacIndex = _slsdet.slsDetectorDefs.dacIndex
from .detector_property import DetectorProperty

import numpy as np


from .utils import element
@freeze
class Ctb(Detector):
    def __init__(self, id = 0):
        super().__init__(id)
        self._frozen = False 
        self._dacs = NamedDacs(self)
        self._powers = NamedPowers(self)
    
    @property
    def dacs(self):
        return self._dacs

    @property
    def powers(self):
        """ 
        [Chiptestboard][Xilinx CTB] Power names and values of all power supplies.

        Example
        -----------
        >>> # print all powers with DAC and info if enabled
        >>> d.powers 
        >>> # set DAC or enables for power supply VA 
        >>> d.powers.VA = 1200
        >>> # enable or disable power subbly VA
        >>> d.powers.VA.enable()
        >>> d.powers.VA.disable()
        >>> # get dac value of power supply VA
        >>> d.powers.VA.dac
        >>> # check if power supply VA is enabled
        >>> d.powers.VA.enabled
        >>> # print both enabled and dac value of power supply VA
        >>> d.powers.VA
        """
        return self._powers
    
    @property
    def powerlist(self):
        """
        List of power supply names on the Chip Test Board.

        :setter: List of custom power supply names to set. 
        """
        return self.getPowerNames()

    @powerlist.setter
    def powerlist(self, value):
        self.setPowerNames(value)


    @property
    def adclist(self):
        return self.getAdcNames()

    @adclist.setter
    def adclist(self, value):
        self.setAdcNames(value)

    @property
    def signallist(self):
        return self.getSignalNames()

    @signallist.setter
    def signallist(self, value):
        self.setSignalNames(value)

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
    def slowadclist(self):
        return self.getSlowADCNames()

    @slowadclist.setter
    def slowadclist(self, value):
        self.setSlowADCNames(value)
        
    @property
    def slowadcvalues(self):
        """[Chiptestboard][Xilinx CTB] Gets the slow adc values for every slow adc for this detector."""
        return {
            slowadc.name.lower(): element_if_equal(np.array(self.getSlowADC(slowadc)))
            for slowadc in self.getSlowADCList()
        }
