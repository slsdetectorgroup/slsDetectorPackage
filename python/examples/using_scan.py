# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package

from slsdet import Mythen3, scanParameters, dacIndex

#Configure scan
sp = scanParameters()
sp.enable = 1 
sp.dacInd = dacIndex.VTH1
sp.startOffset = 0
sp.stopOffset = 1000
sp.stepSize = 100
sp.dacSettleTime_ns = int(1e9)


# Send scan to detector
d = Mythen3()
d.setScan(sp)

#or d.scan = sp



