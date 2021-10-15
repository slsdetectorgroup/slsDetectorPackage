# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
from slsdet import Detector, currentSrcParameters

s = currentSrcParameters()
s.enable = 1
s.fix= 1
s.normal = 1
s.select = 10


d = Detector()
d.currentsource = s