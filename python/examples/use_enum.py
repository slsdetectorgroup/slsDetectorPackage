# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
# Most settings are represented as enums that can be 
# explicitly imported

from slsdet import Detector, fileFormat
d = Detector()
d.fformat = fileFormat.BINARY

# Altough not recommended for convenience all enums 
# and some other things can be imported using *

from slsdet import *
d.speed = speedLevel.FULL_SPEED 

# To list the available enums, use dir()

import slsdet.enums
for enum in dir(slsdet.enums):
    # filter out special members
    if not enum.startswith('_'):
        print(enum)
