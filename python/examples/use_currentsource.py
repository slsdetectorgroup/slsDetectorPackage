from slsdet import Detector, currentSrcParameters

s = currentSrcParameters()
s.enable = 1
s.fix= 1
s.normal = 1
s.select = 10


d = Detector()
d.currentsource = s