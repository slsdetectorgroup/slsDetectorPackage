# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
from .detector_property import DetectorProperty
from functools import partial
import numpy as np
from . import _slsdet
from .detector import freeze
dacIndex = _slsdet.slsDetectorDefs.dacIndex
class Power(DetectorProperty):
    """
    This class represents a power on the Chip Test Board. One instance handles all
    powers with the same name for a multi detector instance. (TODO: Not needed for CTB)

    .. note ::

        This class is used to build up DetectorPowers and is in general
        not directly accessible to the user.


    """
    def __init__(self, name, enum, default, detector):

        super().__init__(partial(detector.getPower, enum),
                         lambda x, y : detector.setPower(enum, x, y),
                         detector.size,
                         name)

        self.default = default


    def __repr__(self):
        """String representation for a single power in all modules"""
        powerstr = ''.join([f'{item:5d}' for item in self.get()])
        return f'{self.__name__:15s}:{powerstr}'

class NamedPowers:
    """
    New implementation of the detector powers. 
    """
    _frozen = False
    _direct_access = ['_detector', '_current', '_powernames']
    def __init__(self, detector):
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
            k = dacIndex(i + int(dacIndex.V_POWER_A))
            setattr(self, name, Power(name, k, 0, detector))

        self._frozen = True

    # def __getattr__(self, name):
    #     return self.__getattribute__('_' + name)

    def __setattr__(self, name, value):
        if not self._frozen:
            #durning init we need to be able to set up the class
            super().__setattr__(name, value)
        else:
            #Later we restrict us to manipulate powers and a few fields
            if name in self._direct_access:
                super().__setattr__(name, value)
            elif name in self._powernames:
                return self.__getattribute__(name).__setitem__(slice(None, None), value)
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
    def get_asarray(self):
        """
        Read the powers into a numpy array with dimensions [npowers, nmodules]
        """
        power_array = np.zeros((len(self._powernames), len(self._detector)))
        for i, _d in enumerate(self):
            power_array[i,:] = _d[:]
        return power_array

    def to_array(self):
        return self.get_asarray()       

    def set_from_array(self, power_array):
        """
        Set the power from an numpy array with power values. [npowers, nmodules]
        """
        power_array = power_array.astype(np.int)
        for i, _d in enumerate(self):
            _d[:] = power_array[i]

    def from_array(self, power_array):
        self.set_from_array(power_array)

class DetectorPowers:
    _powers = []
    _powernames = [_d[0] for _d in _powers]
    _allowed_attr = ['_detector', '_current']
    _frozen = False

    def __init__(self, detector):
        # We need to at least initially know which detector we are connected to
        self._detector = detector

        # Index to support iteration
        self._current = 0

        # Name the attributes? 
        for _d in self._powers:
            setattr(self, '_'+_d[0], Power(*_d, detector))

        self._frozen = True

    def __getattr__(self, name):
        return self.__getattribute__('_' + name)

    @property
    def powernames(self):
        return [_d[0] for _d in _powers]
        
    def __setattr__(self, name, value):
        if name in self._powernames:
            return self.__getattribute__('_' + name).__setitem__(slice(None, None), value)
        else:
            if self._frozen == True and name not in self._allowed_attr:
                raise AttributeError(f'Power not found: {name}')
            super().__setattr__(name, value)


    def __next__(self):
        if self._current >= len(self._powers):
            self._current = 0
            raise StopIteration
        else:
            self._current += 1
            return self.__getattr__(self._powernames[self._current-1])

    def __iter__(self):
        return self

    def __repr__(self):
        r_str = ['========== POWERS =========']
        r_str += [repr(power) for power in self]
        return '\n'.join(r_str)

    def get_asarray(self):
        """
        Read the powers into a numpy array with dimensions [npowers, nmodules]
        """
        power_array = np.zeros((len(self._powers), len(self._detector)))
        for i, _d in enumerate(self):
            power_array[i,:] = _d[:]
        return power_array

    def to_array(self):
        return self.get_asarray()

    def set_from_array(self, power_array):
        """
        Set the powers from an numpy array with power values. [npowers, nmodules]
        """
        power_array = power_array.astype(np.int)
        for i, _d in enumerate(self):
            _d[:] = power_array[i]

    def from_array(self, power_array):
        self.set_from_array(power_array)

    def set_default(self):
        """
        Set all powers to their default values
        """
        for _d in self:
            _d[:] = _d.default

