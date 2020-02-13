# from .detector import Detector, DetectorError, free_shared_memory
from .eiger import Eiger
from .ctb import Ctb
from .dacs import DetectorDacs, Dac
from .detector import Detector
from .jungfrau import Jungfrau
from .mythen3 import Mythen3
# from .jungfrau_ctb import JungfrauCTB
# from _slsdet import DetectorApi

import _slsdet

defs = _slsdet.slsDetectorDefs
runStatus = _slsdet.slsDetectorDefs.runStatus
speedLevel = _slsdet.slsDetectorDefs.speedLevel
timingMode = _slsdet.slsDetectorDefs.timingMode
dacIndex = _slsdet.slsDetectorDefs.dacIndex
detectorType = _slsdet.slsDetectorDefs.detectorType
detectorSettings = _slsdet.slsDetectorDefs.detectorSettings
readoutMode = _slsdet.slsDetectorDefs.readoutMode

IpAddr = _slsdet.IpAddr
MacAddr = _slsdet.MacAddr
