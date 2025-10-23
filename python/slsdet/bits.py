import numpy as np


def setbit(bit, word):
    if isinstance(word, np.generic):
        mask = word.dtype.type(1)
        mask = mask << bit
    else:
        mask = 1 << bit
    return word | mask


def setbit_arr(bit, arr):
    arr |= arr.dtype.type(1 << bit)


def clearbit(bit, word):
    """
    Clear the bit at position bit in word.
    Two paths to avoid converting the types.  
    """
    if isinstance(word, np.generic):
        mask = word.dtype.type(1)
        mask = ~(mask << bit)
    else:
        mask = ~(1 << bit)
    return word & mask


def clearbit_arr(bit, arr):
    arr &= arr.dtype.type(~(1 << bit))