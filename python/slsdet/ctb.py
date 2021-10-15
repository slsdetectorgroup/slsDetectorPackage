# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
from .detector import Detector
from .utils import element_if_equal
from .dacs import DetectorDacs
import _slsdet
dacIndex = _slsdet.slsDetectorDefs.dacIndex
from .detector_property import DetectorProperty

class CtbDacs(DetectorDacs):
    """
    Ctb dacs
    """
    _dacs = [('dac0',  dacIndex(0), 0, 4000,    1400),
             ('dac1',  dacIndex(1), 0, 4000,    1200),
             ('dac2',  dacIndex(2), 0, 4000,    900),
             ('dac3',  dacIndex(3), 0, 4000,    1050),
             ('dac4',  dacIndex(4), 0, 4000,     1400),
             ('dac5',  dacIndex(5), 0, 4000,    655),
             ('dac6',  dacIndex(6), 0, 4000,    2000),
             ('dac7',  dacIndex(7), 0, 4000,     1400),
             ('dac8',  dacIndex(8), 0, 4000,    850),
             ('dac9',  dacIndex(9), 0, 4000,    2000),
             ('dac10', dacIndex(10), 0, 4000,    2294),
             ('dac11', dacIndex(11), 0, 4000,    983),
             ('dac12', dacIndex(12), 0, 4000,    1475),
             ('dac13', dacIndex(13), 0, 4000,    1200),
             ('dac14', dacIndex(14), 0, 4000,    1600),
             ('dac15', dacIndex(15), 0, 4000,    1455),
             ('dac16', dacIndex(16), 0, 4000,       0),
             ('dac17', dacIndex(17), 0, 4000,    1000),
            ]
    _dacnames = [_d[0] for _d in _dacs]

from .utils import element
class Ctb(Detector):
    def __init__(self, id = 0):
        super().__init__(id)
        self._frozen = False 
        self._dacs = CtbDacs(self)
    
    @property
    def dacs(self):
        return self._dacs

