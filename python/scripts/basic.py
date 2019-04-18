import os
import sys
import numpy as np
sys.path.append(os.path.join(os.getcwd(), 'bin'))
from sls_detector import Eiger, Detector
from sls_detector import ExperimentalDetector

from _sls_detector.io import read_my302_file

d = Detector()
e = ExperimentalDetector()

