import os
import sys
import numpy as np
sys.path.append(os.path.join(os.getcwd(), 'bin'))
# from sls_detector import Eiger, Jungfrau, Detector, defs

from sls_detector import Detector, Eiger, DetectorDacs, Dac
from sls_detector import dacIndex


d = Detector()
e = Eiger()

# from sls_detector.eiger import EigerVcmp
# v = EigerVcmp(d)