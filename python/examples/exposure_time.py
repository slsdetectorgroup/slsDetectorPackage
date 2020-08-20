"""
Example showing how to set and get exposure times
"""

import datetime as dt
from slsdet import Detector



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