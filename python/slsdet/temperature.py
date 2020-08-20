from functools import partial
from collections.abc import Iterable
class Temperature:
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
        degree_sign = u"\N{DEGREE SIGN}"
        # r_str = ["{:14s}: ".format(self.name)]
        # r_str += [
        #     "{:6.2f}{:s}C, ".format(self.get(i) / 1000, degree_sign)
        #     for i in range(self.get_nmod())
        # ]
        tempstr = ''.join([f'{item:5d}{degree_sign}C' for item in self.get()])
        return f'{self.name:15s}:{tempstr}'
        # return "".join(r_str).strip(", ")


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
    #     dacstr = ''.join([f'{item:5d}' for item in self.get()])
    #     return f'{self.__name__:10s}:{dacstr}'
        # return "\n".join([str(t) for t in self])

