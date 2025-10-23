# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
This file contains the specialization for the Moench detector
"""


from .detector import Detector, freeze
from .dacs import DetectorDacs
from . import _slsdet
dacIndex = _slsdet.slsDetectorDefs.dacIndex
from .detector_property import DetectorProperty

# @freeze
class MoenchDacs(DetectorDacs):
    """
    Jungfrau specific DACs
    """
    _dacs = [('vbp_colbuf',     dacIndex.VBP_COLBUF,    0, 4000,  1300),
             ('vipre',          dacIndex.VIPRE,         0, 4000,  1000),
             ('vin_cm,',        dacIndex.VIN_CM,        0, 4000,  1400),
             ('vb_sda',         dacIndex.VB_SDA,        0, 4000,   680),
             ('vcasc_sfp',      dacIndex.VCASC_SFP,     0, 4000,  1428),
             ('vout_cm',        dacIndex.VOUT_CM,       0, 4000,  1200),
             ('vipre_cds',      dacIndex.VIPRE_CDS,     0, 4000,   800),
             ('ibias_sfp',      dacIndex.IBIAS_SFP,     0, 4000,   900),
            ]
    _dacnames = [_d[0] for _d in _dacs]

#vthreshold??


@freeze
class Moench(Detector):
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
        self._dacs = MoenchDacs(self)
    
    @property
    def dacs(self):
        return self._dacs