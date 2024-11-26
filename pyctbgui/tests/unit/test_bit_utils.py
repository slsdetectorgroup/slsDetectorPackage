import numpy as np

from pyctbgui import bit_utils as bt


def test_set_bit():
    num = np.int32(0)
    num = bt.set_bit(num, 5)
    assert num == 2**5


def test_remove_bit():
    num = np.int32(2**5)
    num = bt.remove_bit(num, 5)
    assert num == 0


def test_manipulate_bit():
    num = np.int32(0)
    num = bt.manipulate_bit(True, num, 5)  # True means setting bit
    assert num == 2**5


def test_manipulate_bit2():
    num = np.int32(2**8)
    num = bt.manipulate_bit(False, num, 8)  # False means clearing the bit
    assert num == 0
