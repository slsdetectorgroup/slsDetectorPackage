

from slsdet import Detector, patternParameters

d = Detector()
pat = patternParameters()

#Access to memers of the structure using numpy arrays
pat.patlimits = 0x0, 0xa

d.setPattern(pat)

#Load pattern from file
pat.load("/some/dir/some.pat")
