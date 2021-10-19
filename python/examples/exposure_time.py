# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
"""
Example showing how to set and get exposure times
"""

import datetime as dt
from slsdet import Detector
from slsdet.utils import element_if_equal

d = Detector()

# The simplest way is to set the exposure time in 
# seconds by using the exptime property
# This sets the exposure time for all modules
d.exptime = 0.5

# exptime also accepts a python datetime.timedelta
# which can be used to set the time in almost any unit
t = dt.timedelta(milliseconds = 2.3)
d.exptime = t

# or combination of units
t = dt.timedelta(minutes = 3, seconds = 1.23)
d.exptime = t

#exptime however always returns the time in seconds
# >>> d.exptime
# 181.23 

# To get back the exposure time for each module 
# it's possible to use getExptime, this also returns
# the values as datetime.timedelta

# >>> d.getExptime()
# [datetime.timedelta(seconds=181, microseconds=230000), datetime.timedelta(seconds=181, microseconds=230000)]

# In case the values are the same it's possible to use the
# element_if_equal function to reduce the values to a single 
# value

# >>> t = d.getExptime()
# >>> element_if_equal(t)
# datetime.timedelta(seconds=1)