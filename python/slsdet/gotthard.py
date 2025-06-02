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

# vref_ds, vcascn_pb, vcascp_pb, vout_cm, vcasc_out, vin_cm, vref_comp, ib_test_c
class GotthardDacs(DetectorDacs):
    _dacs = [('vref_ds',     dacIndex.VREF_DS,    0, 4000,   660),
             ('vcascn_pb',   dacIndex.VCASCN_PB,  0, 4000,   650),
             ('vcascp_pb,',  dacIndex.VCASCP_PB,  0, 4000,  1480),
             ('vout_cm',     dacIndex.VOUT_CM,    0, 4000,  1520),
             ('vcasc_out',   dacIndex.VCASC_OUT,  0, 4000,  1320),
             ('vin_cm',      dacIndex.VIN_CM,     0, 4000,  1350),
             ('vref_comp',   dacIndex.VREF_COMP,  0, 4000,   350),
             ('ib_test_c',   dacIndex.IB_TESTC,   0, 4000,   2001),
            ]
    _dacnames = [_d[0] for _d in _dacs]

#vthreshold??


@freeze
class Gotthard(Detector):
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
        self._dacs = GotthardDacs(self)
    
    @property
    def dacs(self):
        return self._dacs