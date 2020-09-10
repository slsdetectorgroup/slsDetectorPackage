from .utils import element_if_equal
from .enums import dacIndex
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
        for i in range(7):
            r = element_if_equal(self.__getitem__(i))
            if isinstance(r, list):
                rstr += ' '.join(f'{item} mV' for item in r)
            else:
                rstr += f'{i}: {r} mV\n'
        
        return rstr.strip('\n')

    
