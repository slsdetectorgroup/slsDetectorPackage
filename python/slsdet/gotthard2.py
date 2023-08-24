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
class Gotthard2Dacs(DetectorDacs):
    """
    Gotthard2 specific DACs
    """
    _dacs = [('vref_h_adc',     dacIndex.VREF_H_ADC,   0, 4000, 2116),
             ('vb_comp_fe',     dacIndex.VB_COMP_FE,   0, 4000,    0),
             ('vb_comp_adc',    dacIndex.VB_COMP_ADC,  0, 4000,    0),
             ('vcom_cds',      dacIndex.VCOM_CDS,      0, 4000,  705),
             ('vref_rstore',   dacIndex.VREF_RSTORE,   0, 4000,  205),
             ('vb_opa_1st',    dacIndex.VB_OPA_1ST,    0, 4000,    0),
             ('vref_comp_fe',  dacIndex.VREF_COMP_FE,  0, 4000,    0),
             ('vcom_adc1',     dacIndex.VCOM_ADC1,     0, 4000,  705),
             ('vref_prech',    dacIndex.VREF_PRECH,    0, 4000,  900),
             ('vref_l_adc',    dacIndex.VREF_L_ADC,    0, 4000,  700),
             ('vref_cds',      dacIndex.VREF_CDS,      0, 4000,  600),
             ('vb_cs',         dacIndex.VB_CS,         0, 4000, 2799),
             ('vb_opa_fd',     dacIndex.VB_OPA_FD,     0, 4000,    0),
             ('vcom_adc2',      dacIndex.VCOM_ADC2,    0, 4000,  704),
            ]
    _dacnames = [_d[0] for _d in _dacs]




@freeze
class Gotthard2(Detector):
    """
    Subclassing Detector to set up correct dacs and detector specific 
    functions. 
    """
    _detector_dynamic_range = [16]


    _settings = ['standard', 'highgain', 'lowgain', 'veryhighgain', 'verylowgain']
    """available settings for Eiger, note almost always standard"""

    def __init__(self, id=0):
        super().__init__(id)
        self._frozen = False 
        self._dacs = Gotthard2Dacs(self)
    
    @property
    def dacs(self):
        return self._dacs