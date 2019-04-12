from .detector_property import DetectorProperty
from functools import partial
import numpy as np

class Dac(DetectorProperty):
    """
    This class represents a dac on the detector. One instance handles all
    dacs with the same name for a multi detector instance.

    .. note ::

        This class is used to build up DetectorDacs and is in general
        not directly accessed to the user.


    """
    def __init__(self, name, low, high, default, detector):

        super().__init__(partial(detector._api.getDac, name),
                         partial(detector._api.setDac, name),
                         detector._api.getNumberOfDetectors,
                         name)

        self.min_value = low
        self.max_value = high
        self.default = default



    def __repr__(self):
        """String representation for a single dac in all modules"""
        r_str = ['{:10s}: '.format(self.__name__)]
        r_str += ['{:5d}, '.format(self.get(i)) for i in range(self.get_nmod())]
        return ''.join(r_str).strip(', ')


class DetectorDacs:
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

    def __init__(self, detector):
        # We need to at least initially know which detector we are connected to
        self._detector = detector

        # Index to support iteration
        self._current = 0

        # Populate the dacs
        for _d in self._dacs:
            setattr(self, '_'+_d[0], Dac(*_d, detector))

    def __getattr__(self, name):
        return self.__getattribute__('_' + name)


    def __setattr__(self, name, value):
        if name in self._dacnames:
            return self.__getattribute__('_' + name).__setitem__(slice(None, None, None), value)
        else:
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
        dac_array = np.zeros((len(self._dacs), self._detector.n_modules))
        for i, _d in enumerate(self):
            dac_array[i,:] = _d[:]
        return dac_array

    def set_from_array(self, dac_array):
        """
        Set the dacs from an numpy array with dac values. [ndacs, nmodules]
        """
        dac_array = dac_array.astype(np.int)
        for i, _d in enumerate(self):
            _d[:] = dac_array[i]

    def set_default(self):
        """
        Set all dacs to their default values
        """
        for _d in self:
            _d[:] = _d.default

    def update_nmod(self):
        """
        Update the cached value of nmod, needs to be run after adding or
        removing detectors
        """
        for _d in self:
            _d._n_modules = self._detector.n_modules

