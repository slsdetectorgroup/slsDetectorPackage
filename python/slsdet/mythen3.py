#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
This file contains the specialization for the Jungfrau detector
"""


from .detector import Detector, freeze

# from .adcs import Adc, DetectorAdcs
from .dacs import DetectorDacs
import _sls_detector
dacIndex = _sls_detector.slsDetectorDefs.dacIndex
from .detector_property import DetectorProperty

# vcassh 1200, 
# vth2 2800, 
# vshaper 1280, 
# vshaperneg 2800, 
# vipre_out 1220, 
# vth3 2800, 
# vth1 2800, 
# vicin 1708, 
# vcas 1800, 
# vpreamp 1100, 
# vpl 1100, 
# vipre 2624, 
# viinsh 1708, 
# vph 1712, 
# vtrim 2800, 
# vdcsh 800


# @freeze
class Mythen3Dacs(DetectorDacs):
    """
    Jungfrau specific DACs
    """
    _dacs = [('vcassh',     dacIndex.CASSH,     0, 4000, 1220),
             ('vth2',       dacIndex.VTH2,      0, 4000, 2800),
             ('vshaper',    dacIndex.SHAPER1,   0, 4000, 1280),
             ('vshaperneg', dacIndex.SHAPER2,   0, 4000, 2800),
             ('vipre_out',  dacIndex.VIPRE_OUT, 0, 4000, 1220),
             ('vth3',       dacIndex.VTH3,      0, 4000, 2800),
             ('vth1',       dacIndex.THRESHOLD,      0, 4000,  2800),
             ('vicin',      dacIndex.VICIN,     0, 4000,  1708),
             ('vcas',       dacIndex.CAS,       0, 4000,  1800),
             ('vpreamp',    dacIndex.PREAMP,    0, 4000,  1100),
             ('vpl',        dacIndex.VPL,       0, 4000,  1100),
             ('vipre',      dacIndex.VIPRE,     0, 4000,  2624),
             ('viinsh',     dacIndex.VIINSH,    0, 4000,  1708),
             ('vph',    dacIndex.CALIBRATION_PULSE,    0, 4000,  1712),
             ('vtrim',    dacIndex.TRIMBIT_SIZE,    0, 4000,  2800),
             ('vdcsh',    dacIndex.VDCSH,    0, 4000,  800),
            ]
    _dacnames = [_d[0] for _d in _dacs]




@freeze
class Mythen3(Detector):
    """
    Subclassing Detector to set up correct dacs and detector specific 
    functions. 
    """
    _detector_dynamic_range = [4, 8, 16, 32]


    _settings = ['standard', 'highgain', 'lowgain', 'veryhighgain', 'verylowgain']
    """available settings for Eiger, note almost always standard"""

    def __init__(self, id=0):
        super().__init__(id)
        self._frozen = False 
        self._dacs = Mythen3Dacs(self)
    
    @property
    def dacs(self):
        return self._dacs