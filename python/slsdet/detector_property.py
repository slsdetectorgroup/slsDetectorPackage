# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
from collections.abc import Iterable
import numpy as np

class DetectorProperty:
    """
    Base class for a detector property that should be accessed by name and index
    TODO! Calls are not in parallel and exposes object that can be passes around
    """
    def __init__(self, get_func, set_func, nmod_func, name):
        self.get = get_func
        self.set = set_func
        self.get_nmod = nmod_func
        self.__name__ = name

    def __getitem__(self, key):
        if key == slice(None, None, None):
            return self.get()
        elif isinstance(key, Iterable):
            return self.get(list(key))
        else:
            return self.get([key])[0] #No list for single value

    def __setitem__(self, key, value):
        #operate on all values
        if key == slice(None, None, None):
            if isinstance(value, (np.integer, int)):
                self.set(value, [])
            elif isinstance(value, Iterable):
                for i in range(self.get_nmod()):
                    self.set(value[i], [i])
            else:
                raise ValueError('Value should be int or np.integer not', type(value))
        
        #Iterate over some
        elif isinstance(key, Iterable):
            if isinstance(value, Iterable):
                for k,v in zip(key, value):
                    self.set(v, [k])

            elif isinstance(value, int):
                self.set(value, list(key))

        #Set single value
        elif isinstance(key, int):
            self.set(value, [key])

    def __repr__(self):
        s = ', '.join(str(v) for v in self[:])
        return '{}: [{}]'.format(self.__name__, s)