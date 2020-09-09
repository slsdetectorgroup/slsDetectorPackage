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

import _slsdet

defs = _slsdet.slsDetectorDefs

from .enums import *


IpAddr = _slsdet.IpAddr
MacAddr = _slsdet.MacAddr
