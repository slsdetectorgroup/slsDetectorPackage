# from .detector import Detector, DetectorError, free_shared_memory
from .eiger import Eiger
from .ctb import Ctb
from .dacs import DetectorDacs, Dac
from .detector import Detector
from .jungfrau import Jungfrau
from .mythen3 import Mythen3
from .gotthard2 import Gotthard2
from .gotthard import Gotthard
from .moench import Moench
# from .jungfrau_ctb import JungfrauCTB
# from _slsdet import DetectorApi

import _slsdet

defs = _slsdet.slsDetectorDefs

from .enums import *
# runStatus = _slsdet.slsDetectorDefs.runStatus
# speedLevel = _slsdet.slsDetectorDefs.speedLevel
# detectorType = _slsdet.slsDetectorDefs.detectorType
# frameDiscardPolicy = _slsdet.slsDetectorDefs.frameDiscardPolicy
# fileFormat = _slsdet.slsDetectorDefs.fileFormat
# dimension = _slsdet.slsDetectorDefs.dimension
# externalSignalFlag = _slsdet.slsDetectorDefs.externalSignalFlag
# timingMode = _slsdet.slsDetectorDefs.timingMode
# dacIndex = _slsdet.slsDetectorDefs.dacIndex
# detectorSettings = _slsdet.slsDetectorDefs.detectorSettings
# clockIndex = _slsdet.slsDetectorDefs.clockIndex
# readoutMode = _slsdet.slsDetectorDefs.readoutMode
# masterFlags = _slsdet.slsDetectorDefs.masterFlags
# frameModeType = _slsdet.slsDetectorDefs.frameModeType
# detectorModeType = _slsdet.slsDetectorDefs.detectorModeType
# burstMode = _slsdet.slsDetectorDefs.burstMode
# timingSourceType = _slsdet.slsDetectorDefs.timingSourceType


IpAddr = _slsdet.IpAddr
MacAddr = _slsdet.MacAddr
