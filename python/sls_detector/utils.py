"""
Utility functions that are useful for testing and troubleshooting
but not directly used in controlling the detector
"""
from collections import namedtuple
import _sls_detector #C++ lib
import functools

Geometry = namedtuple('Geometry', ['x', 'y'])

def to_geo(value):
    if isinstance(value, _sls_detector.xy):
        return Geometry(x = value.x, y = value.y)
    else:
        raise ValueError("Can only convert sls_detector.xy")

def all_equal(mylist):
    """If all elements are equal return true otherwise false"""
    return all(x == mylist[0] for x in mylist)


def element_if_equal(mylist):
    """If all elements are equal return only one element"""
    if all_equal(mylist):
        if len(mylist) == 0:
            return None
        else:
            return mylist[0]
    else:
        return mylist

def element(func):
    @functools.wraps(func)
    def wrapper(self, *args, **kwargs):
        return element_if_equal(func(self, *args, **kwargs))
    return wrapper

def eiger_register_to_time(register):
    """
    Decode register value and return time in s. Values are stored in
    a 32bit register with bits 2->0 containing the exponent and bits
    31->3 containing the significand (int value)

    """
    clocks = register >> 3
    exponent = register & 0b111
    return clocks*10**exponent / 100e6
