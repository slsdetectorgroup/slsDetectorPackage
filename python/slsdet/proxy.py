# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
from .utils import element_if_equal
from .enums import dacIndex


def set_proxy_using_dict(func, key, value):
    if isinstance(value, dict) and all(isinstance(k, int) for k in value.keys()):
        for dkey, dvalue in value.items():
            func(key, dvalue, [dkey])
    else:
        func(key, value)

class JsonProxy:
    """
    Proxy class to allow for intuitive setting and getting of rx_jsonpara
    This class is returned by Detectr.rx_jsonpara
    """
    def __init__(self, det):
        self.det = det

    def __getitem__(self, key):
        return element_if_equal(self.det.getAdditionalJsonParameter(key))

    def __setitem__(self, key, value):
        self.det.setAdditionalJsonParameter(key, str(value))

    def __repr__(self):
        r = element_if_equal(self.det.getAdditionalJsonHeader())
        if isinstance(r, list):
            rstr = ''
            for i, list_item in enumerate(r):
                list_item = dict(list_item)
                rstr += ''.join([f'{i}:{key}: {value}\n' for key, value in list_item.items()])

            return rstr.strip('\n')
        else:
            r = dict(r)
            return '\n'.join([f'{key}: {value}' for key, value in r.items()])

    

class SlowAdcProxy:
    """
    Proxy class to allow for more intuitive reading the slow ADCs
    """
    def __init__(self, det):
        self.det = det

    def __getitem__(self, key):
        dac_index = dacIndex(int(dacIndex.SLOW_ADC0)+key)
        return element_if_equal(self.det.getSlowADC(dac_index))

    def __repr__(self):
        rstr = ''
        for i in range(8):
            r = element_if_equal(self.__getitem__(i))
            if isinstance(r, list):
                rstr += ' '.join(f'{item} uV' for item in r)
            else:
                rstr += f'{i}: {r} uV\n'
        
        return rstr.strip('\n')

class ClkDivProxy:
    """
    Proxy class to allow for more intuitive reading clockdivider
    """
    def __init__(self, det):
        self.det = det

    def __getitem__(self, key):
        return element_if_equal(self.det.getClockDivider(key))

    def __setitem__(self, key, value):
        set_proxy_using_dict(self.det.setClockDivider, key, value)

    def __repr__(self):
        rstr = ''
        for i in range(6):
            r = element_if_equal(self.__getitem__(i))
            if isinstance(r, list):
                rstr += ' '.join(f'{item}' for item in r)
            else:
                rstr += f'{i}: {r}\n'
        
        return rstr.strip('\n')


class MaxPhaseProxy:
    """
    Proxy class to allow for more intuitive reading clockdivider
    """
    def __init__(self, det):
        self.det = det

    def __getitem__(self, key):
        return element_if_equal(self.det.getMaxClockPhaseShift(key))

    def __repr__(self):
        rstr = ''
        for i in range(5):
            r = element_if_equal(self.__getitem__(i))
            if isinstance(r, list):
                rstr += ' '.join(f'{item}' for item in r)
            else:
                rstr += f'{i}: {r}\n'
        
        return rstr.strip('\n')

class ClkFreqProxy:
    """
    Proxy class to allow for more intuitive reading clockdivider
    """
    def __init__(self, det):
        self.det = det

    def __getitem__(self, key):
        return element_if_equal(self.det.getClockFrequency(key))

    def __repr__(self):
        rstr = ''
        for i in range(5):
            r = element_if_equal(self.__getitem__(i))
            if isinstance(r, list):
                rstr += ' '.join(f'{item}' for item in r)
            else:
                rstr += f'{i}: {r}\n'
        
        return rstr.strip('\n')

class PatNLoopProxy:
    """
    Proxy class to allow for more intuitive reading patnloop
    """
    def __init__(self, det):
        self.det = det

    def __getitem__(self, key):
        return element_if_equal(self.det.getPatternLoopCycles(key))

    def __setitem__(self, key, value):
        set_proxy_using_dict(self.det.setPatternLoopCycles, key, value)

    def __repr__(self):
        rstr = ''
        for i in range(3):
            r = element_if_equal(self.__getitem__(i))
            if isinstance(r, list):
                rstr += ' '.join(f'{item}' for item in r)
            else:
                rstr += f'{i}: {r}\n'
        
        return rstr.strip('\n')
