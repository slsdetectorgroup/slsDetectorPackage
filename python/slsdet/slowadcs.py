# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
from .detector_property import DetectorProperty
from functools import partial
import numpy as np
import _slsdet
from .detector import freeze
dacIndex = _slsdet.slsDetectorDefs.dacIndex
class SlowAdc(DetectorProperty):
    """
    This class represents a slowadc on the Chip Test Board. One instance handles all
    slowadcs with the same name for a multi detector instance. (TODO: Not needed for CTB)

    .. note ::

        This class is used to build up DetectorSlowAdcs and is in general
        not directly accessible to the user.


    """
    def __init__(self, name, enum, default, detector):

        super().__init__(partial(detector.getSlowADC, enum),
                         lambda x, y : detector.setVoltage(enum, x, y),
                         detector.size,
                         name)
        #FIXME ??????????????

        self.default = default


    def __repr__(self):
        """String representation for a single slowadc in all modules"""
        slowadcstr = ''.join([f'{item:5d}' for item in self.get()])
        return f'{self.__name__:15s}:{slowadcstr}'

class NamedSlowAdcs:
    """
    New implementation of the detector slowadcs. 
    """
    _frozen = False
    _direct_access = ['_detector', '_current', '_slowadcnames']
    def __init__(self, detector):
        self._detector = detector
        self._current = 0

        #only get the slowadcnames if we have modules attached
        if detector.size() == 0:
            self._slowadcnames  = [f"slowadc{i}" for i in range(8)]
        else:
            self._slowadcnames = [n.replace(" ", "") for n in detector.getSlowADCNames()]

        # Populate the slowadcs
        for i,name in enumerate(self._slowadcnames):
            #name, enum, low, high, default, detector
            k = dacIndex(i + int(dacIndex.SLOW_ADC0))
            setattr(self, name, SlowAdc(name, k, 0, detector))

        self._frozen = True

    # def __getattr__(self, name):
    #     return self.__getattribute__('_' + name)

    def __setattr__(self, name, value):
        if not self._frozen:
            #durning init we need to be able to set up the class
            super().__setattr__(name, value)
        else:
            #Later we restrict us to manipulate slowadcs and a few fields
            if name in self._direct_access:
                super().__setattr__(name, value)
            elif name in self._slowadcnames:
                return self.__getattribute__(name).__setitem__(slice(None, None), value)
            else:
                raise AttributeError(f'SlowAdc not found: {name}')

    def __next__(self):
        if self._current >= len(self._slowadcnames):
            self._current = 0
            raise StopIteration
        else:
            self._current += 1
            return self.__getattribute__(self._slowadcnames[self._current-1])
            # return self.__getattr__(self._slowadcnames[self._current-1])

    def __iter__(self):
        return self

    def __repr__(self):
        r_str = ['========== SLOW ADCS =========']
        r_str += [repr(slowadc) for slowadc in self]
        return '\n'.join(r_str)
    def get_asarray(self):
        """
        Read the slowadcs into a numpy array with dimensions [nslowadcs, nmodules]
        """
        slowadc_array = np.zeros((len(self._slowadcnames), len(self._detector)))
        for i, _d in enumerate(self):
            slowadc_array[i,:] = _d[:]
        return slowadc_array

    def to_array(self):
        return self.get_asarray()       

    def set_from_array(self, slowadc_array):
        """
        Set the slowadc from an numpy array with slowadc values. [nslowadcs, nmodules]
        """
        slowadc_array = slowadc_array.astype(np.int)
        for i, _d in enumerate(self):
            _d[:] = slowadc_array[i]

    def from_array(self, slowadc_array):
        self.set_from_array(slowadc_array)

class DetectorSlowAdcs:
    _slowadcs = []
    _slowadcnames = [_d[0] for _d in _slowadcs]
    _allowed_attr = ['_detector', '_current']
    _frozen = False

    def __init__(self, detector):
        # We need to at least initially know which detector we are connected to
        self._detector = detector

        # Index to support iteration
        self._current = 0

        # Name the attributes? 
        for _d in self._slowadcs:
            setattr(self, '_'+_d[0], SlowAdc(*_d, detector))

        self._frozen = True

    def __getattr__(self, name):
        return self.__getattribute__('_' + name)

    @property
    def slowadcnames(self):
        return [_d[0] for _d in _slowadcs]
        
    def __setattr__(self, name, value):
        if name in self._slowadcnames:
            return self.__getattribute__('_' + name).__setitem__(slice(None, None), value)
        else:
            if self._frozen == True and name not in self._allowed_attr:
                raise AttributeError(f'SlowAdc not found: {name}')
            super().__setattr__(name, value)


    def __next__(self):
        if self._current >= len(self._slowadcs):
            self._current = 0
            raise StopIteration
        else:
            self._current += 1
            return self.__getattr__(self._slowadcnames[self._current-1])

    def __iter__(self):
        return self

    def __repr__(self):
        r_str = ['========== SLOW ADCS =========']
        r_str += [repr(slowadc) for slowadc in self]
        return '\n'.join(r_str)

    def get_asarray(self):
        """
        Read the slowadcs into a numpy array with dimensions [nslowadcs, nmodules]
        """
        slowadc_array = np.zeros((len(self._slowadcs), len(self._detector)))
        for i, _d in enumerate(self):
            slowadc_array[i,:] = _d[:]
        return slowadc_array

    def to_array(self):
        return self.get_asarray()

    def set_from_array(self, slowadc_array):
        """
        Set the slowadcs from an numpy array with slowadc values. [nslowadcs, nmodules]
        """
        slowadc_array = slowadc_array.astype(np.int)
        for i, _d in enumerate(self):
            _d[:] = slowadc_array[i]

    def from_array(self, slowadc_array):
        self.set_from_array(slowadc_array)

    def set_default(self):
        """
        Set all slowadcs to their default values
        """
        for _d in self:
            _d[:] = _d.default

