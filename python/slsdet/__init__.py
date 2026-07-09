# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
from importlib.metadata import PackageNotFoundError, version
from pathlib import Path

from .eiger import Eiger
from .ctb import Ctb
from .dacs import NamedDacs, DetectorDacs, Dac
from .powers import NamedPowers, Power
from .detector import Detector
from .jungfrau import Jungfrau
from .mythen3 import Mythen3
from .gotthard2 import Gotthard2
from .moench import Moench
from .pattern import Pattern, patternParameters
from .gaincaps import Mythen3GainCapsWrapper
from .pattern_generator import PatternGenerator

from . import _slsdet
from ._slsdet import freeSharedMemory, getUserDetails

xy = _slsdet.xy
defs = _slsdet.slsDetectorDefs

# Make enums and #defines available at top level
from .enums import *
from .defines import *


IpAddr = _slsdet.IpAddr
MacAddr = _slsdet.MacAddr
RegisterAddress = _slsdet.RegisterAddress
BitAddress = _slsdet.BitAddress
RegisterValue = _slsdet.RegisterValue
scanParameters = _slsdet.scanParameters
currentSrcParameters = _slsdet.currentSrcParameters
DurationWrapper = _slsdet.DurationWrapper
pedestalParameters = _slsdet.pedestalParameters
Hz = _slsdet.Hz
kHz = _slsdet.kHz
MHz = _slsdet.MHz

try:
    __version__ = version("slsdet")
except PackageNotFoundError:
    __version__ = Path(__file__).parent.joinpath("VERSION").read_text().strip()
