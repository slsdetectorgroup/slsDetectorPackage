import numpy as np


def _bitmask(bit, word):
    dtype = word.dtype if hasattr(word, 'dtype') else np.uint64
    if bit >= np.iinfo(dtype).bits:
        raise ValueError(f"bit {bit} out of range for {np.dtype(dtype).name}")
    return np.dtype(dtype).type(1 << bit)

def setbit(bit, word):
    """
    Set the bit at position bit in word(s).
    """
    return word | _bitmask(bit, word)

def clearbit(bit, word):
    """
    Clear the bit at position bit in word(s).
    """
    return word & ~_bitmask(bit, word)

def flipbit(bit, word):
    """
    Flip the bit at position bit in word(s).
    """
    return word ^ _bitmask(bit, word)