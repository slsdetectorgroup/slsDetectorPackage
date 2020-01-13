# from .detector import Detector, DetectorError, free_shared_memory
from .eiger import Eiger
from .ctb import Ctb
from .dacs import DetectorDacs, Dac
from .detector import Detector
from .jungfrau import Jungfrau
# from .jungfrau_ctb import JungfrauCTB
# from _sls_detector import DetectorApi

import _sls_detector

defs = _sls_detector.slsDetectorDefs
runStatus = _sls_detector.slsDetectorDefs.runStatus
speedLevel = _sls_detector.slsDetectorDefs.speedLevel
timingMode = _sls_detector.slsDetectorDefs.timingMode
dacIndex = _sls_detector.slsDetectorDefs.dacIndex
detectorType = _sls_detector.slsDetectorDefs.detectorType
detectorSettings = _sls_detector.slsDetectorDefs.detectorSettings

IpAddr = _sls_detector.IpAddr
MacAddr = _sls_detector.MacAddr
