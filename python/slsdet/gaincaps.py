

from . import _slsdet
gc = _slsdet.slsDetectorDefs.M3_GainCaps


class Mythen3GainCapsWrapper:
    """Holds M3_GainCaps enums and facilitates printing"""
    # 'M3_C10pre', 'M3_C15pre', 'M3_C15sh', 'M3_C225ACsh', 'M3_C30sh', 'M3_C50sh'
    all_bits = gc.M3_C10pre | gc.M3_C15pre | gc.M3_C15sh | gc.M3_C225ACsh | gc.M3_C30sh | gc.M3_C50sh
    all_caps = (gc.M3_C10pre, gc.M3_C15pre, gc.M3_C15sh, gc.M3_C225ACsh, gc.M3_C30sh, gc.M3_C50sh)
    def __init__(self, value = 0):
        self._validate(value)
        self.value = value


    def __eq__(self, other) -> bool:
        if isinstance(other, Mythen3GainCapsWrapper):
            return self.value == other.value
        else:
            return self.value == other

    
    def __ne__(self, other) -> bool:
        return not self.__eq__(other)

    def __str__(self) -> str:
        s = ', '.join(str(c).rsplit('_', 1)[1] for c in self.all_caps if self.value & c)
        return s
    
    def __repr__(self) -> str:
        return self.__str__()
    
    def _validate(self, value):
        """Check that only bits representing real capacitors are set"""
        if isinstance(value, gc):
            return True
        elif isinstance(value, int):
            if value & (~self.all_bits):
                raise ValueError(f"The value: {value} is not allowed for Mythen3GainCapsWrapper")
        else:
            raise ValueError("GainCaps can only be initialized from int or M3_GainCaps enum")