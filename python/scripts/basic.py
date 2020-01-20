import os
import sys
import numpy as np
sys.path.append(os.path.join(os.getcwd(), 'bin'))

from sls_detector import Detector, Mythen3, Eiger, Jungfrau, DetectorDacs, Dac, Ctb
from sls_detector import dacIndex, readoutMode


d = Detector()
# e = Eiger()
# c = Ctb()
# j = Jungfrau()
m = Mythen3()

