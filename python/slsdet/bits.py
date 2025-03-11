import numpy as np


def setbit(bit, word):
    mask = 1 << bit
    if isinstance(word, np.generic):
        mask = word.dtype.type(mask)
    return word | mask


def setbit_arr(bit, arr):
    arr |= arr.dtype.type(1 << bit)


def clearbit(bit, word):
    mask = ~(1 << bit)
    if isinstance(word, np.generic):
        mask = word.dtype.type(mask)
    return word & mask


def clearbit_arr(bit, arr):
    arr &= arr.dtype.type(~(1 << bit))