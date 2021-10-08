from slsdet import Detector, currentSrcParameters

s = currentSrcParameters()
s.enable_ = 1
s.fix_= 1
s.normal_ = 1
s.select_ = 10


d = Detector()
d.currentsource = s