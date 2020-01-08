from .experimental import Detector
from .utils import element_if_equal
from .dacs import DetectorDacs
import _sls_detector
dacIndex = _sls_detector.slsDetectorDefs.dacIndex
from .detector_property import DetectorProperty

class CtbDacs(DetectorDacs):
    """
    Eiger specific dacs
    """
    _dacs = [('dac0',  dacIndex(0), 0, 4000,    1400),
             ('dac1',  dacIndex(1), 0, 4000,    1200),
             ('dac2',  dacIndex(2), 0, 4000,    900),
             ('dac3',  dacIndex(3), 0, 4000,    1050),
             ('dac4',  dacIndex(4), 0, 4000,     1400),
             ('dac5',  dacIndex(5), 0, 4000,    655),
             ('dac6',  dacIndex(6), 0, 4000,    2000),
             ('dac7',  dacIndex(7), 0, 4000,     1400),
             ('dac8',  dacIndex(8), 0, 4000,    850),
             ('dac9',  dacIndex(9), 0, 4000,    2000),
             ('dac10', dacIndex(10), 0, 4000,    2294),
             ('dac11', dacIndex(11), 0, 4000,    983),
             ('dac12', dacIndex(12), 0, 4000,    1475),
             ('dac13', dacIndex(13), 0, 4000,    1200),
             ('dac14', dacIndex(14), 0, 4000,    1600),
             ('dac15', dacIndex(15), 0, 4000,    1455),
             ('dac16', dacIndex(16), 0, 4000,       0),
             ('dac17', dacIndex(17), 0, 4000,    1000),
            ]
    _dacnames = [_d[0] for _d in _dacs]

from .utils import element
class Ctb(Detector):
    def __init__(self, id = 0):
        super().__init__(id)
        self._frozen = False 
        self._dacs = CtbDacs(self)
    
    @property
    def dacs(self):
        return self._dacs

    @property
    @element
    def v_a(self):
        return self.getDAC(dacIndex.V_POWER_A, True)

    @v_a.setter
    def v_a(self, value):
        self.setDAC(dacIndex.V_POWER_A, value, True)

    @property
    @element
    def v_b(self):
        return self.getDAC(dacIndex.V_POWER_B, True)

    @v_b.setter
    def v_b(self, value):
        self.setDAC(dacIndex.V_POWER_B, value, True)

    @property
    @element
    def v_c(self):
        return self.getDAC(dacIndex.V_POWER_C, True)

    @v_c.setter
    def v_c(self, value):
        self.setDAC(dacIndex.V_POWER_C, value, True)

    @property
    @element
    def v_d(self):
        return self.getDAC(dacIndex.V_POWER_D, True)

    @v_d.setter
    def v_d(self, value):
        self.setDAC(dacIndex.V_POWER_D, value, True)

    @property
    @element
    def v_io(self):
        return self.getDAC(dacIndex.V_POWER_IO, True)

    @v_io.setter
    def v_io(self, value):
        self.setDAC(dacIndex.V_POWER_IO, value, True)

    @property
    @element
    def v_limit(self):
        return self.getDAC(dacIndex.V_LIMIT, True)

    @v_limit.setter
    def v_limit(self, value):
        self.setDAC(dacIndex.V_LIMIT, value, True)

    @property
    @element
    def im_a(self):
        return self.getMeasuredCurrent(dacIndex.I_POWER_A)

    @property
    @element
    def im_b(self):
        return self.getMeasuredCurrent(dacIndex.I_POWER_B)

    @property
    @element
    def im_c(self):
        return self.getMeasuredCurrent(dacIndex.I_POWER_C)

    @property
    @element
    def im_d(self):
        return self.getMeasuredCurrent(dacIndex.I_POWER_D)

    @property
    @element
    def im_io(self):
        return self.getMeasuredCurrent(dacIndex.I_POWER_IO)