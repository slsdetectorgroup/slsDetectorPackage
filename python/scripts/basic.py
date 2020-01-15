import os
import sys
import numpy as np
sys.path.append(os.path.join(os.getcwd(), 'bin'))
# from sls_detector import Eiger, Jungfrau, Detector, defs

from sls_detector import Detector, Eiger, Jungfrau, DetectorDacs, Dac, Ctb
from sls_detector import dacIndex


d = Detector()
# e = Eiger()
# c = Ctb()
j = Jungfrau()

# def tracefunc(frame, event, arg, indent=[0]):
#       if event == "call":
#           indent[0] += 2
#           print("-" * indent[0] + "> call function", frame.f_code.co_name)
#       elif event == "return":
#           print("<" + "-" * indent[0], "exit function", frame.f_code.co_name)
#           indent[0] -= 2
#       return tracefunc

# import sys
# sys.setprofile(tracefunc)
# print('------------------------------------------------------')
# j.dacs.vb_comp[:] = 1500
# print('------------------------------------------------------')

# # sys.setprofile(None)