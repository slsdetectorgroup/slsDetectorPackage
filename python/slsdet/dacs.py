# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
from .detector_property import DetectorProperty
from functools import partial
import numpy as np
from . import _slsdet
from .detector import freeze
dacIndex = _slsdet.slsDetectorDefs.dacIndex
class Dac(DetectorProperty):
    """
    This class represents a dac on the detector. One instance handles all
    dacs with the same name for a multi detector instance.

    .. note ::

        This class is used to build up DetectorDacs and is in general
        not directly accessed to the user.


    """
    def __init__(self, name, enum, low, high, default, detector):

        super().__init__(partial(detector.getDAC, enum, False),
                         lambda x, y : detector.setDAC(enum, x, False, y),
                         detector.size,
                         name)

        self.min_value = low
        self.max_value = high
        self.default = default



    def __repr__(self):
        """String representation for a single dac in all modules"""
        dacstr = ''.join([f'{item:5d}' for item in self.get()])
        return f'{self.__name__:15s}:{dacstr}'

class NamedDacs:
    """
    New implementation of the detector dacs. Used at the moment for 
    Ctb but should replace the old one for all detectors
    """
    _frozen = False
    _direct_access = ['_detector', '_current', '_dacnames']
    def __init__(self, detector):
        self._detector = detector
        self._current = 0

        #only get the dacnames if we have modules attached
        if detector.size() == 0:
            self._dacnames  = [f"dac{i}" for i in range(18)]
        else:
            self._dacnames = [n.replace(" ", "") for n in detector.getDacNames()]

        # Populate the dacs
        for i,name in enumerate(self._dacnames):
            #name, enum, low, high, default, detector
            setattr(self, name, Dac(name, dacIndex(i), 0, 4000, 1000, detector))

        self._frozen = True

    # def __getattr__(self, name):
    #     return self.__getattribute__('_' + name)

    def __setattr__(self, name, value):
        if not self._frozen:
            #durning init we need to be able to set up the class
            super().__setattr__(name, value)
        else:
            #Later we restrict us to manipulate dacs and a few fields
            if name in self._direct_access:
                super().__setattr__(name, value)
            elif name in self._dacnames:
                return self.__getattribute__(name).__setitem__(slice(None, None, None), value)
            else:
                raise AttributeError(f'Dac not found: {name}')

    def __next__(self):
        if self._current >= len(self._dacnames):
            self._current = 0
            raise StopIteration
        else:
            self._current += 1
            return self.__getattribute__(self._dacnames[self._current-1])
            # return self.__getattr__(self._dacnames[self._current-1])

    def __iter__(self):
        return self

    def __repr__(self):
        r_str = ['========== DACS =========']
        r_str += [repr(dac) for dac in self]
        return '\n'.join(r_str)
    def get_asarray(self):
        """
        Read the dacs into a numpy array with dimensions [ndacs, nmodules]
        """
        dac_array = np.zeros((len(self._dacnames), len(self._detector)))
        for i, _d in enumerate(self):
            dac_array[i,:] = _d[:]
        return dac_array

    def to_array(self):
        return self.get_asarray()       

    def set_from_array(self, dac_array):
        """
        Set the dacs from an numpy array with dac values. [ndacs, nmodules]
        """
        dac_array = dac_array.astype(np.int)
        for i, _d in enumerate(self):
            _d[:] = dac_array[i]

    def from_array(self, dac_array):
        self.set_from_array(dac_array)

class DetectorDacs:
    _dacs = []
    _dacnames = [_d[0] for _d in _dacs]
    _allowed_attr = ['_detector', '_current']
    _frozen = False

    def __init__(self, detector):
        # We need to at least initially know which detector we are connected to
        self._detector = detector

        # Index to support iteration
        self._current = 0

        # Name the attributes? 
        for _d in self._dacs:
            setattr(self, '_'+_d[0], Dac(*_d, detector))

        self._frozen = True

    def __getattr__(self, name):
        return self.__getattribute__('_' + name)

    @property
    def dacnames(self):
        return [_d[0] for _d in _dacs]
        
    def __setattr__(self, name, value):
        if name in self._dacnames:
            return self.__getattribute__('_' + name).__setitem__(slice(None, None, None), value)
        else:
            if self._frozen == True and name not in self._allowed_attr:
                raise AttributeError(f'Dac not found: {name}')
            super().__setattr__(name, value)


    def __next__(self):
        if self._current >= len(self._dacs):
            self._current = 0
            raise StopIteration
        else:
            self._current += 1
            return self.__getattr__(self._dacnames[self._current-1])

    def __iter__(self):
        return self

    def __repr__(self):
        r_str = ['========== DACS =========']
        r_str += [repr(dac) for dac in self]
        return '\n'.join(r_str)

    def get_asarray(self):
        """
        Read the dacs into a numpy array with dimensions [ndacs, nmodules]
        """
        dac_array = np.zeros((len(self._dacs), len(self._detector)))
        for i, _d in enumerate(self):
            dac_array[i,:] = _d[:]
        return dac_array

    def to_array(self):
        return self.get_asarray()

    def set_from_array(self, dac_array):
        """
        Set the dacs from an numpy array with dac values. [ndacs, nmodules]
        """
        dac_array = dac_array.astype(np.int)
        for i, _d in enumerate(self):
            _d[:] = dac_array[i]

    def from_array(self, dac_array):
        self.set_from_array(dac_array)

    def set_default(self):
        """
        Set all dacs to their default values
        """
        for _d in self:
            _d[:] = _d.default

