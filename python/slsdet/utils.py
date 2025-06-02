# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
"""
Utility functions that are useful for testing and troubleshooting
but not directly used in controlling the detector
"""

from collections import namedtuple
from . import _slsdet  #C++ lib
import functools
import datetime as dt
import pathlib
import os
from pathlib import Path

Geometry = namedtuple('Geometry', ['x', 'y'])


def is_iterable(item):
    try:
        iter(item)
    except TypeError:
        return False
    return True


def get_set_bits(mask):
    """
    Return a list of the set bits in a python integer
    """
    return [i for i in range(mask.bit_length()) if (mask >> i) & 1]


def list_to_bitmask(values):
    """
    Convert a list of integers to a bitmask with set bits
    where the list indicates
    """
    mask = int(0)
    values = list(set(values))  #Remove duplicates
    for v in values:
        mask += 1 << v
    return mask

def make_bitmask(args):
    if isinstance(args, (list,tuple)):
        return list_to_bitmask(args)
    elif isinstance(args, dict):
        return {key: list_to_bitmask(value) for key, value in args.items()}
    else:
        raise ValueError("Cannot convert arg to bitmask")


def to_geo(value):
    if isinstance(value, _slsdet.xy):
        return Geometry(x=value.x, y=value.y)
    else:
        raise ValueError("Can only convert slsdet.xy")


def all_equal(mylist):
    """If all elements are equal return true otherwise false"""
    return all(x == mylist[0] for x in mylist)


def element_if_equal(mylist):
    """If all elements are equal return only one element"""
    if not is_iterable(mylist):
        return mylist

    if all_equal(mylist):
        if len(mylist) == 0:
            return None
        else:
            return mylist[0]
    else:
        return mylist


def reduce_time(mylist):
    res = element_if_equal(element_if_equal(mylist))
    if isinstance(res, (dt.timedelta, _slsdet.DurationWrapper)):
        return res.total_seconds()
    elif isinstance(res[0], list):
        return [[item.total_seconds() for item in subl] for subl in res]
    else:
        return [r.total_seconds() for r in res]


def element(func):
    """
    Wrapper to return either list or element
    """
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
    return clocks * 10**exponent / 100e6


def make_timedelta(t):
    if isinstance(t, dt.timedelta):
        return t
    else:
        return dt.timedelta(seconds=t)


def _make_string_path(path):
    """
    Accepts either a pathlib.Path or a string, expands ~ to user and convert
    Path to str
    """
    if isinstance(path, pathlib.Path):
        return path.expanduser().as_posix()
    elif isinstance(path, str):
        return os.path.expanduser(path)
    else:
        raise ValueError("Cannot convert argument to posix path")


def make_string_path(path):
    return _make(path, _make_string_path)


def make_ip(arg):
    return _make(arg, _slsdet.IpAddr)


def make_mac(arg):
    return _make(arg, _slsdet.MacAddr)


def make_path(arg):
    return _make(arg, Path)


def _make(arg, transform):
    """Helper function for make_mac and make_ip special cases for
    dict, list and tuple. Otherwise just calls transform"""
    if isinstance(arg, dict):
        return {key: transform(value) for key, value in arg.items()}
    elif isinstance(arg, list):
        return [transform(a) for a in arg]
    elif isinstance(arg, tuple):
        return tuple(transform(a) for a in arg)
    else:
        return transform(arg)


def set_using_dict(func, *args):

    if len(args) == 1 and isinstance(args[0], dict) and all(
            isinstance(k, int) for k in args[0].keys()):
        for key, value in args[0].items():
            if not isinstance(value, tuple):
                value = (value,)
            try:
                func(*value, [key])
            except TypeError:
                func(*value, key)
    else:
        func(*args)


def set_time_using_dict(func, args):
    if isinstance(args, dict) and all(isinstance(k, int) for k in args.keys()):
        for key, value in args.items():
            if isinstance(value, int):
                value = float(value)
            func(value, [key])
    else:
        if isinstance(args, int):
            args = float(args)
        func(args)


def lhex(iterable):
    return [hex(item) for item in iterable]


def lpath(iterable):
    return [Path(item) for item in iterable]

def add_argument_before(a, args):
    """Add a before the other arguments. Also works with
    dict that holds args to several modules. Always puts the
    args in a dict to be compatible with set_using_dict"""
    if isinstance(args, tuple):
        return (a, *args)
    elif isinstance(args, dict):
        ret = {}
        for key, value in args.items():
            if isinstance(value, tuple):
                ret[key] = (a, *value)
            else:
                ret[key] = (a, value)
        return (ret,)
    return a, args

def add_argument_after(args, a):
    """Add a before the other arguments. Also works with
    dict that holds args to several modules. Always puts the
    args in a dict to be compatible with set_using_dict"""
    if isinstance(args, tuple):
        return (*args, a)
    elif isinstance(args, dict):
        ret = {}
        for key, value in args.items():
            if isinstance(value, tuple):
                ret[key] = (*value, a)
            else:
                ret[key] = (value, a)
        return (ret,)
    return args, a

def pop_dict(args):
    for i,a in enumerate(args):
        if isinstance(a, dict):
            return args.pop(i), i

def tuplify(args):
    if not isinstance(args, tuple):
        return (args, )
    else:
        return args

def merge_args(*args):
    n_dict = sum(isinstance(a, dict) for a in args)
    
    if n_dict == 0: #no dict just make a tuple of arguments
        ret = []
        for a in args:
            if isinstance(a, tuple):
                ret.extend(a)
            else:
                ret.append(a)
        return tuple(ret)

    elif n_dict == 1:
        args = [a for a in args] #these are the args to be added
        values,pos = pop_dict(args)
        ret = {}
        for k, v in values.items():
            v = tuplify(v)
            items = [a for a in args]
            items[pos:pos] = v
            ret[k] = tuple(items)
        return (ret,)

    else:
        raise ValueError("Multiple dictionaries passes cannot merge args")


def hostname_list(args):
    """
    Generates a list from a hostname string
    * Lists are passed through
    * as are tuples (conversion in pybind11 to vector)
    * if + is found it splits the string
    """
    if isinstance(args, (list, tuple)):
        return args
    elif(isinstance(args, str)):
        hosts = args.split('+')
        hosts = [it for it in hosts if len(it)]
        return hosts
    else:
        raise ValueError("hostname needs to be string or list of strings")


def validate_port(value):
    if value <= 0 or value > 65535:
        raise ValueError("port must be in range 1 - 65535")

