import os
import sys
import numpy as np
sys.path.append(os.path.join(os.getcwd(), 'bin'))

from slsdet import Detector, Mythen3, Eiger, Jungfrau, DetectorDacs, Dac, Ctb
from slsdet import dacIndex, readoutMode
from slsdet.lookup import view, find

d = Detector()
e = Eiger()
c = Ctb()
# j = Jungfrau()
# m = Mythen3()

