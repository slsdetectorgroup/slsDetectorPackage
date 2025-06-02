# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
# from .detector import Detector, DetectorError, free_shared_memory
from .eiger import Eiger
from .ctb import Ctb
from .dacs import DetectorDacs, Dac
from .powers import DetectorPowers, Power
from .detector import Detector
from .jungfrau import Jungfrau
from .mythen3 import Mythen3
from .gotthard2 import Gotthard2
from .gotthard import Gotthard
from .moench import Moench
from .pattern import Pattern, patternParameters
from .gaincaps import Mythen3GainCapsWrapper

from . import _slsdet
xy = _slsdet.xy
defs = _slsdet.slsDetectorDefs

#Make enums and #defines available at top level
from .enums import *
from .defines import *


IpAddr = _slsdet.IpAddr
MacAddr = _slsdet.MacAddr
scanParameters = _slsdet.scanParameters
currentSrcParameters = _slsdet.currentSrcParameters
DurationWrapper = _slsdet.DurationWrapper
pedestalParameters = _slsdet.pedestalParameters

import os
def read_version():
    try:
        version_file = os.path.join(os.path.dirname(__file__), 'VERSION')
        with open(version_file, "r") as f:
            return f.read().strip()
    except:
        raise RuntimeError("VERSION file not found in slsdet package from init.py")
    
__version__ = read_version()




