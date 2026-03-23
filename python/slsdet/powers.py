# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
from functools import partial
import numpy as np
from . import _slsdet
from .detector import freeze
powerIndex = _slsdet.slsDetectorDefs.powerIndex
class Power:
    """
    This class represents a power supply on the Chip Test Board. 

    .. note ::

        This class is used to build up NamedPowers and is in general
        not directly accessible to the user.


    """
    _direct_access = ['_detector']

    def __init__(self, name, enum, default, detector):
        self._frozen = False
        self.__name__ = name
        self.enum = enum
        self.default = default
        self.detector = detector
        self._frozen = True

    @property
    def dac(self):
        " Returns the dac value for this power supply in mV."
        return self.detector.getPowerDAC(self.enum)
    
    @dac.setter
    def dac(self, value):
        " Set the dac value for this power supply in mV."
        self.detector.setPowerDAC(self.enum, value)

    @property
    def enable(self):
        " Returns whether this power supply is enabled."
        return self.detector.isPowerEnabled(self.enum)
    
    @enable.setter
    def enable(self, value):
        " Set whether this power supply is enabled."
        self.detector.setPowerEnabled([self.enum], value)

    # prevent unknown attributes
    def __setattr__(self, name, value):
        if not getattr(self, "_frozen", False):
            super().__setattr__(name, value)
        elif name in ("dac", "enable", "_frozen", "enum", "default", "detector", "__name__"):
            super().__setattr__(name, value)
        else:
            raise AttributeError(f"Cannot set attribute '{name}' on Power. Allowed attributes are 'dac' and 'enable'.")
        
    def __repr__(self):
        "String representation for a single power supply"
        return f'{self.__name__:15s}: {str(self.enable):5s}, {self.dac:5d} mV'


class DetectorPowers:
    """
    List implementation of the all the power supplies with its names. 
    """
    _direct_access = ['_detector', '_current', '_powernames']

    def __init__(self, detector):
        self._frozen = False

        self._detector = detector
        self._current = 0

        #only get the powernames if we have modules attached
        if detector.size() == 0:
            self._powernames  = ["VA", "VB", "VC", "VD", "VIO"]
        else:
            self._powernames = [n.replace(" ", "") for n in detector.getPowerNames()]

        # Populate the powers
        for i,name in enumerate(self._powernames):
            #name, enum, low, high, default, detector
            setattr(self, name, Power(name, powerIndex(i), 0, detector))

        self._frozen = True

    def __setattr__(self, name, value):
        if not getattr(self, "_frozen", False):
            #durning init we need to be able to set up the class
            super().__setattr__(name, value)
        else:
            #Later we restrict us to manipulate powers and a few fields
            if name in self._direct_access or name == '_frozen':
                super().__setattr__(name, value)
            elif name in self._powernames:
                raise AttributeError(
                    f"Cannot assign directly to power '{name}'. "
                    f"Use '{name}.dac = value' or '{name}.enable = value'"
                )
            else:
                raise AttributeError(f'Power not found: {name}')

    def __next__(self):
        if self._current >= len(self._powernames):
            self._current = 0
            raise StopIteration
        else:
            self._current += 1
            return self.__getattribute__(self._powernames[self._current-1])
            # return self.__getattr__(self._powernames[self._current-1])

    def __iter__(self):
        return self

    def __repr__(self):
        r_str = ['========== POWERS =========']
        r_str += [repr(power) for power in self]
        return '\n'.join(r_str)
