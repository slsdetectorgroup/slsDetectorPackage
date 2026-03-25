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


class NamedPowers:
    """
    List implementation of the all the power supplies with its names. 
    """
    _direct_access = ['_detector', '_current']

    def __init__(self, detector):
        self._frozen = False
        self._detector = detector
        self._current = 0
        self._frozen = True


    @property
    def _powernames(self):
        if self._detector.size() == 0:
            raise RuntimeError("No modules added")
        # always get the latest list
        if hasattr(self._detector, 'powerlist'):
            return [n.replace(" ", "") for n in self._detector.powerlist]
        else:
            raise RuntimeError("Detector does not have powerlist attribute")

    def __getattr__(self, name):
        if name in self._powernames:
            idx = self._powernames.index(name)
            return Power(name, powerIndex(idx), 0, self._detector)
        raise AttributeError(f'Power not found: {name}')

    def __setattr__(self, name, value):
        if name in ("_detector", "_current", "_frozen"):
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
            return getattr(self, self._powernames[self._current-1])
            # return self.__getattr__(self._powernames[self._current-1])

    def __iter__(self):
        self._current = 0
        return self

    def __repr__(self):
        r_str = ['========== POWERS =========']
        r_str += [repr(power) for power in self]
        return '\n'.join(r_str)
