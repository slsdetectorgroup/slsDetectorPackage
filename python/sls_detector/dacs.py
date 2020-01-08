from .detector_property import DetectorProperty
from functools import partial
import numpy as np
import _sls_detector
dacIndex = _sls_detector.slsDetectorDefs.dacIndex
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
        return f'{self.__name__:10s}:{dacstr}'

# a = Dac('vrf', dacIndex.VRF, 0, 4000, 2500, d )
class DetectorDacs:
    _dacs = [('vsvp',    dacIndex.SVP,0, 4000,    0),
             ('vtr',     dacIndex.VTR,0, 4000, 2500),
             ('vrf',     dacIndex.VRF,0, 4000, 3300),
             ('vrs',     dacIndex.VRS,0, 4000, 1400),
             ('vsvn',    dacIndex.SVN,0, 4000, 4000),
             ('vtgstv',  dacIndex.VTGSTV,0, 4000, 2556),
             ('vcmp_ll', dacIndex.VCMP_LL,0, 4000, 1500),
             ('vcmp_lr', dacIndex.VCMP_LR,0, 4000, 1500),
             ('vcall',   dacIndex.CAL,0, 4000, 4000),
             ('vcmp_rl', dacIndex.VCMP_RL,0, 4000, 1500),
             ('rxb_rb',  dacIndex.RXB_RB,0, 4000, 1100),
             ('rxb_lb',  dacIndex.RXB_LB,0, 4000, 1100),
             ('vcmp_rr', dacIndex.VCMP_RR,0, 4000, 1500),
             ('vcp',     dacIndex.VCP,0, 4000,  200),
             ('vcn',     dacIndex.VCN,0, 4000, 2000),
             ('vis',     dacIndex.VIS,0, 4000, 1550),
             ('iodelay', dacIndex.IO_DELAY,0, 4000,  660)]
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
        dac_array = np.zeros((len(self._dacs), len(self._detector)))
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

