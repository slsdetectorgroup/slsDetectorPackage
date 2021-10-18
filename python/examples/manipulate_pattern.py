# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package


from slsdet import Detector, patternParameters

d = Detector()
pat = patternParameters()

#Access to members of the structure using numpy arrays
pat.patlimits = 0x0, 0xa

d.setPattern(pat)

#Load pattern from file
pat.load("/some/dir/some.pat")
