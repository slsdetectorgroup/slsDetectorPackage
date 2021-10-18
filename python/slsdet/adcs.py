# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
from functools import partial
class Adc:
    def __init__(self, name, enum, detector):
        self.name = name
        self.enum = enum
        self._detector = detector
        self.get_nmod = self._detector.size
        # Bind functions to get and set the dac
        self.get = partial(self._detector.getAdc, self.enum)

        
    def __getitem__(self, key):
        if key == slice(None, None, None):
            return self.get()
        elif isinstance(key, Iterable):
            return self.get(list(key))
        else:
            return self.get([key])[0] #No list for single value


    # def __getitem__(self, key):
    #     """
    #     Get dacs either by slice, key or list
    #     """
    #     if key == slice(None, None, None):
    #         return [self.get(i) / 1000 for i in range(self.get_nmod())]
    #     elif isinstance(key, Iterable):
    #         return [self.get(k) / 1000 for k in key]
    #     else:
    #         return self.get(key) / 1000

    def __repr__(self):
        """String representation for a single adc in all modules"""
        degree_sign = u'\N{DEGREE SIGN}'
        r_str = ['{:14s}: '.format(self.name)]
        r_str += ['{:6.2f}{:s}C, '.format(self.get(i)/1000, degree_sign) for i in range(self.get_nmod())]
        return ''.join(r_str).strip(', ')



class DetectorAdcs:
    """
    Interface to the ADCs on the readout board
    """
    def __iter__(self):
        for attr, value in self.__dict__.items():
            yield value

    def __repr__(self):
        return '\n'.join([str(t) for t in self])