# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
from .detector import Detector, freeze
from .utils import element_if_equal
from .dacs import DetectorDacs, NamedDacs
from .voltages import DetectorVoltages, NamedVoltages
from .slowadcs import DetectorSlowAdcs, NamedSlowAdcs
import _slsdet
dacIndex = _slsdet.slsDetectorDefs.dacIndex
from .detector_property import DetectorProperty


from .utils import element
@freeze
class Ctb(Detector):
    def __init__(self, id = 0):
        super().__init__(id)
        self._frozen = False 
        self._dacs = NamedDacs(self)
        self._voltages = NamedVoltages(self)
        self._slowadcs = NamedSlowAdcs(self)
    
    @property
    def dacs(self):
        return self._dacs

    @property
    def voltages(self):
        return self._voltages

    @property
    def slowadcs(self):
        return self._slowadcs