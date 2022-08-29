# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
import os
import sys
import numpy as np
# from pathlib import Path
# sys.path.append(os.path.join(os.getcwd(), 'bin'))

from slsdet import Detector, Mythen3, Eiger, Jungfrau, DetectorDacs, Dac, Ctb, Gotthard2, Moench
from slsdet import dacIndex, readoutMode
from slsdet.lookup import view, find
import slsdet

from slsdet import DurationWrapper


d = Detector()
t = DurationWrapper(1.0)

d.setExptime(t)

# d.setExptime(0.3)