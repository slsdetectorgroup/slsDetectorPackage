from collections.abc import Iterable
import numpy as np

class DetectorProperty:
    """
    Base class for a detector property that should be accessed by name and index
    """
    def __init__(self, get_func, set_func, nmod_func, name):
        self.get = get_func
        self.set = set_func
        self.get_nmod = nmod_func
        self.__name__ = name

    def __getitem__(self, key):
        if key == slice(None, None, None):
            return [self.get(i) for i in range(self.get_nmod())]
        elif isinstance(key, Iterable):
            return [self.get(k) for k in key]
        else:
            return self.get(key)

    def __setitem__(self, key, value):
        #operate on all values
        if key == slice(None, None, None):
            if isinstance(value, (np.integer, int)):
                for i in range(self.get_nmod()):
                    self.set(i, value)
            elif isinstance(value, Iterable):
                for i in range(self.get_nmod()):
                    self.set(i, value[i])
            else:
                raise ValueError('Value should be int or np.integer not', type(value))
        
        #Iterate over some
        elif isinstance(key, Iterable):
            if isinstance(value, Iterable):
                for k,v in zip(key, value):
                    self.set(k,v)

            elif isinstance(value, int):
                for k in key:
                    self.set(k, value)

        #Set single value
        elif isinstance(key, int):
            self.set(key, value)

    def __repr__(self):
        s = ', '.join(str(v) for v in self[:])
        return '{}: [{}]'.format(self.__name__, s)