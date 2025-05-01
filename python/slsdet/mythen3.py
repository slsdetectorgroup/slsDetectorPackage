# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
This file contains the specialization for the Mythen3 detector
"""


from .detector import Detector, freeze

# from .adcs import Adc, DetectorAdcs
from .dacs import DetectorDacs
from . import _slsdet
dacIndex = _slsdet.slsDetectorDefs.dacIndex
gc_enums = _slsdet.slsDetectorDefs.M3_GainCaps
from .detector_property import DetectorProperty


# @freeze
class Mythen3Dacs(DetectorDacs):
    """
    Jungfrau specific DACs
    """
    _dacs = [('vcassh',     dacIndex.VCASSH,     0, 4000,  1220),
             ('vth2',       dacIndex.VTH2,       0, 4000,  2800),
             ('vrshaper',   dacIndex.VRSHAPER,   0, 4000,  1280),
             ('vrshaper_n', dacIndex.VRSHAPER_N, 0, 4000,  2800),
             ('vipre_out',  dacIndex.VIPRE_OUT,  0, 4000,  1220),
             ('vth3',       dacIndex.VTH3,       0, 4000,  2800),
             ('vth1',       dacIndex.VTH1,       0, 4000,  2800),
             ('vicin',      dacIndex.VICIN,      0, 4000,  1708),
             ('vcas',       dacIndex.VCAS,       0, 4000,  1800),
             ('vrpreamp',   dacIndex.VRPREAMP,   0, 4000,  1100),
             ('vcal_n',     dacIndex.VCAL_N,     0, 4000,  1100),
             ('vipre',      dacIndex.VIPRE,      0, 4000,  2624),
             ('vishaper',   dacIndex.VISHAPER,   0, 4000,  1708),
             ('vcal_p',     dacIndex.VCAL_P,     0, 4000,  1712),
             ('vtrim',      dacIndex.VTRIM,      0, 4000,  2800),
             ('vdcsh',      dacIndex.VDCSH,      0, 4000,   800),
            ]
    _dacnames = [_d[0] for _d in _dacs]

#vthreshold??


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
    

