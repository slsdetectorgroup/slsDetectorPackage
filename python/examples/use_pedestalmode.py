# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
from slsdet import Detector, pedestalParameters

p = pedestalParameters()
p.frames = 10
p.loops= 20



d = Detector()
d.pedestalmode = p