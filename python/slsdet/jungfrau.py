# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
This file contains the specialization for the Jungfrau detector
"""


from .detector import Detector, freeze

# from .adcs import Adc, DetectorAdcs
from .dacs import DetectorDacs
from . import _slsdet
dacIndex = _slsdet.slsDetectorDefs.dacIndex
from .detector_property import DetectorProperty

# @freeze
class JungfrauDacs(DetectorDacs):
    """
    Jungfrau specific DACs
    """
    _dacs = [('vb_comp',    dacIndex.VB_COMP, 0, 4000,    1220),
             ('vdd_prot',   dacIndex.VDD_PROT, 0, 4000,    3000),
             ('vin_com',    dacIndex.VIN_COM, 0, 4000,    1053),
             ('vref_prech', dacIndex.VREF_PRECH, 0, 4000,    1450),
             ('vb_pixbuf', dacIndex.VB_PIXBUF, 0, 4000,     750),
             ('vb_ds',      dacIndex.VB_DS, 0, 4000,    1000),
             ('vref_ds',    dacIndex.VREF_DS, 0, 4000,     480),
             ('vref_comp',  dacIndex.VREF_COMP, 0, 4000,     420),
            ]
    _dacnames = [_d[0] for _d in _dacs]




@freeze
class Jungfrau(Detector):
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
        self._dacs = JungfrauDacs(self)
    
    @property
    def dacs(self):
        return self._dacs