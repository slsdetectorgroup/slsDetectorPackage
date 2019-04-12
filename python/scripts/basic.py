import os
import sys
import numpy as np
sys.path.append(os.path.join(os.getcwd(), 'bin'))
from sls_detector import Eiger
from sls_detector import ExperimentalDetector

from _sls_detector.io import read_ctb_file

d = Eiger()
e = ExperimentalDetector()

# ncols = 192
# start = 600
# end = 1800
# nrows = end-start
# data = np.zeros((nrows, ncols))

# for i in range(nrows):
#     data[i, :] = read_ctb_file(f'/home/l_frojdh/mythendata/MoKbZr_30kV60mA_1s_200V_thr{start+i}_1.raw', 8, 24)