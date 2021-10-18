# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
from functools import partial
from collections.abc import Iterable
import numpy as np
class Temperature:
    degree_sign = u"\N{DEGREE SIGN}"

    def __init__(self, name, enum, detector):
        self.name = name
        self.enum = enum
        self._detector = detector
        self.get_nmod = self._detector.size
        # Bind functions to get and set the dac
        self.get = partial(self._detector.getTemperature, self.enum)

    def __getitem__(self, key):
        if key == slice(None, None, None):
            return self.get()
        elif isinstance(key, Iterable):
            return self.get(list(key))
        else:
            return self.get([key])[0]  # No list for single value

    def __repr__(self):
        """String representation for a single temperature in all modules"""
        
        tempstr = ''.join([f'{item:5d}{self.degree_sign}C' for item in self.get()])
        return f'{self.name:15s}:{tempstr}'

class DetectorTemperature:
    """
    Interface to temperatures on the readout board
    """

    def __iter__(self):
        for attr, value in self.__dict__.items():
            yield value

    def __repr__(self):
        """String representation of all temps all mods"""
        r_str = '\n'.join([repr(temp) for temp in self])
        return r_str

    def to_dict(self):
        """Get temperatures as a dictionary with numpy arrays"""
        return {attr:np.array(value.get()) for attr, value in self.__dict__.items()}

    def to_array(self):
        """Get all temperatures as a numpy array"""
        t = self.to_dict()
        return np.vstack([value for key, value in t.items()])
