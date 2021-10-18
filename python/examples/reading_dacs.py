# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
from slsdet import Detector, Eiger, dacIndex


#using the specialized class

e = Eiger()
e.dacs
# >>> e.dacs
# ========== DACS =========
# vsvp      :    0    0
# vtrim     : 2480 2480
# vrpreamp  : 3300 3300
# vrshaper  : 1400 1400
# vsvn      : 4000 4000
# vtgstv    : 2556 2556
# vcmp_ll   : 1000 1000
# vcmp_lr   : 1000 1000
# vcal      :    0    0
# vcmp_rl   : 1000 1000
# rxb_rb    : 1100 1100
# rxb_lb    : 1100 1100
# vcmp_rr   : 1000 1000
# vcp       : 1000 1000
# vcn       : 2000 2000
# vishaper  : 1550 1550
# iodelay   :  650  650

# or using the general class and the list
d = Detector()
for dac in d.daclist:
    r = d.getDAC(dac, False)
    print(f'{dac.name:10s} {r}')
