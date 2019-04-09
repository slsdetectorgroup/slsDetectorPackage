import os
import sys
sys.path.append(os.path.join(os.getcwd(), 'bin'))
from sls_detector import Eiger
from sls_detector import ExperimentalDetector

d = Eiger()
e = ExperimentalDetector()