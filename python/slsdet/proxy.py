# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
from .utils import element_if_equal
from .enums import dacIndex
from .defines import M3_MAX_PATTERN_LEVELS, MAX_PATTERN_LEVELS
from ._slsdet import slsDetectorDefs
detectorType = slsDetectorDefs.detectorType


def set_proxy_using_dict(func, key, value, unpack = False):
    if isinstance(value, dict) and all(isinstance(k, int) for k in value.keys()):
        if unpack:
            for dkey, dvalue in value.items():
                func(key, *dvalue, [dkey])
        else:
            for dkey, dvalue in value.items():
                func(key, dvalue, [dkey])
    else:
        if unpack:
            func(key, *value)
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
        return element_if_equal(self.det.getSlowADC(dac_index))/1000 #TODO! Multi module?

    def __repr__(self):
        rstr = ''
        for i,name in enumerate(self.det.getSlowADCNames()):
            r = element_if_equal(self.__getitem__(i))
            if isinstance(r, list):
                rstr += ' '.join(f'{item/1000} mV' for item in r)
            else:
                rstr += f'[{i}] {name}: {r/1000} mV\n'
        
        return rstr.strip('\n')
    
    def __getattr__(self, name):
        if name in self.det.getSlowADCNames():
            i = self.det.getSlowADCIndex(name)
            return element_if_equal(self.det.getSlowADC(i))
        else:
            raise ValueError(f"Could not find slow adc with name: {name}")



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
        num_clocks = 6
        if self.det.type == detectorType.MYTHEN3:
            num_clocks = 5
        for i in range(num_clocks):
            r = element_if_equal(self.__getitem__(i))
            if isinstance(r, list):
                rstr += ' '.join(f'{item}' for item in r)
            else:
                rstr += f'{i}: {r}\n'
        
        return rstr.strip('\n')

class ClkPhaseProxy:
    """
    Proxy class to allow for more intuitive reading clock phase
    """
    def __init__(self, det):
        self.det = det

    def __getitem__(self, key):
        return element_if_equal(self.det.getClockPhase(key))

    def __setitem__(self, key, value):
        set_proxy_using_dict(self.det.setClockPhase, key, value)

    def __repr__(self):
        rstr = ''
        if self.det.type == detectorType.MYTHEN3:
            num_clocks = 5
        for i in range(num_clocks):
            r = element_if_equal(self.__getitem__(i))
            if isinstance(r, list):
                rstr += ' '.join(f'{item}' for item in r)
            else:
                rstr += f'{i}: {r}\n'
        
        return rstr.strip('\n')

class MaxPhaseProxy:
    """
    Proxy class to allow for more intuitive reading max clock phase shift
    """
    def __init__(self, det):
        self.det = det

    def __getitem__(self, key):
        return element_if_equal(self.det.getMaxClockPhaseShift(key))

    def __repr__(self):
        rstr = ''
        if self.det.type == detectorType.MYTHEN3:
            num_clocks = 5
        for i in range(num_clocks):
            r = element_if_equal(self.__getitem__(i))
            if isinstance(r, list):
                rstr += ' '.join(f'{item}' for item in r)
            else:
                rstr += f'{i}: {r}\n'
        
        return rstr.strip('\n')

class ClkFreqProxy:
    """
    Proxy class to allow for more intuitive reading clock frequency
    """
    def __init__(self, det):
        self.det = det

    def __getitem__(self, key):
        return element_if_equal(self.det.getClockFrequency(key))

    def __repr__(self):
        rstr = ''
        if self.det.type == detectorType.MYTHEN3:
            num_clocks = 5
        for i in range(num_clocks):
            r = element_if_equal(self.__getitem__(i))
            if isinstance(r, list):
                rstr += ' '.join(f'{item}' for item in r)
            else:
                rstr += f'{i}: {r}\n'
        
        return rstr.strip('\n')

class PatLoopProxy:
    """
    Proxy class to allow for more intuitive reading patloop
    """
    def __init__(self, det):
        self.det = det

    def __getitem__(self, key):
        return element_if_equal(self.det.getPatternLoopAddresses(key))

    def __setitem__(self, key, value):
        set_proxy_using_dict(self.det.setPatternLoopAddresses, key, value, unpack = True)

    def __repr__(self):
        max_levels = MAX_PATTERN_LEVELS
        if self.det.type == slsDetectorDefs.detectorType.MYTHEN3:
            max_levels = M3_MAX_PATTERN_LEVELS
        rstr = ''
        for i in range(max_levels):
            r = self.__getitem__(i)
            if isinstance(r[0], list):
                part = ' '.join(f'{item}' for item in r)
                rstr += f'{i}: {part}\n'
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
        max_levels = MAX_PATTERN_LEVELS
        if self.det.type == slsDetectorDefs.detectorType.MYTHEN3:
            max_levels = M3_MAX_PATTERN_LEVELS
        rstr = ''
        for i in range(max_levels):
            r = element_if_equal(self.__getitem__(i))
            if isinstance(r, list):
                part = ', '.join(f'{item}' for item in r)
                rstr += f'{i}: {part}\n'
            else:
                rstr += f'{i}: {r}\n'
        
        return rstr.strip('\n')


class PatWaitProxy:
    """
    Proxy class to allow for more intuitive reading patwait
    """
    def __init__(self, det):
        self.det = det

    def __getitem__(self, key):
        return element_if_equal(self.det.getPatternWaitAddr(key))

    def __setitem__(self, key, value):
        set_proxy_using_dict(self.det.setPatternWaitAddr, key, value)

    def __repr__(self):
        max_levels = MAX_PATTERN_LEVELS
        if self.det.type == slsDetectorDefs.detectorType.MYTHEN3:
            max_levels = M3_MAX_PATTERN_LEVELS
        rstr = ''
        for i in range(max_levels):
            r = element_if_equal(self.__getitem__(i))
            if isinstance(r, list):
                part = ', '.join(f'{item}' for item in r)
                rstr += f'{i}: {part}\n'
            else:
                rstr += f'{i}: {r}\n'
        
        return rstr.strip('\n')

class PatWaitTimeProxy:
    """
    Proxy class to allow for more intuitive reading patwaittime
    """
    def __init__(self, det):
        self.det = det

    def __getitem__(self, key):
        return element_if_equal(self.det.getPatternWaitTime(key))

    def __setitem__(self, key, value):
        set_proxy_using_dict(self.det.setPatternWaitTime, key, value)

    def __repr__(self):
        max_levels = MAX_PATTERN_LEVELS
        if self.det.type == slsDetectorDefs.detectorType.MYTHEN3:
            max_levels = M3_MAX_PATTERN_LEVELS
        rstr = ''
        for i in range(max_levels):
            r = element_if_equal(self.__getitem__(i))
            if isinstance(r, list):
                part = ', '.join(f'{item}' for item in r)
                rstr += f'{i}: {part}\n'
            else:
                rstr += f'{i}: {r}\n'
        
        return rstr.strip('\n')
