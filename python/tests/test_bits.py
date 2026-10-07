from slsdet.bits import clearbit, setbit, flipbit
from slsdet import RegisterAddress
import numpy as np


def test_clearbit_on_python_int():
    val = 5  # 0b101
    r = clearbit(0, val)
    assert r == 4
    assert val == 5

def test_setbit_on_python_int():
    val = 5  # 0b101
    r = setbit(1, val)
    assert r == 7
    assert val == 5

def test_flipbit_on_python_int():
    val = 5  # 0b101
    r = flipbit(0, val)
    assert r == 4
    r2 = flipbit(1, val)
    assert r2 == 7
    assert val == 5

def test_setbit_doesnt_change_type():
    word = np.int32(5)
    ret = setbit(0, word)
    assert isinstance(ret, np.int32)

def test_clearbit_doesnt_change_type():
    word = np.uint8(5)
    ret = clearbit(0, word)
    assert isinstance(ret, np.uint8)

def test_setbit_on_array_element():
    arr = np.zeros(10, dtype=np.uint64)
    arr[5] = setbit(0, arr[5])
    arr[5] = setbit(1, arr[5])
    arr[5] = setbit(4, arr[5])
    assert arr[4] == 0
    assert arr[5] == 19  # 0b10011
    assert arr[6] == 0

def test_setbit_array():
    test_array = np.zeros(10, dtype=np.int32)
    test_array[0:3] = setbit(0, test_array[0:3])
    test_array[3:6] = setbit(1, test_array[3:6])
    test_array[6:9] = setbit(2, test_array[6:9])
    test_array[9] = setbit(3, test_array[9])
    assert all(test_array == np.array((1, 1, 1, 2, 2, 2, 4, 4, 4, 8), dtype=np.int32))

def test_clearbit_on_array_element():
    arr = np.zeros(10, dtype=np.uint64)
    arr[5] = setbit(0, arr[5])
    arr[5] = setbit(1, arr[5])
    arr[5] = setbit(4, arr[5])
    assert arr[4] == 0
    assert arr[5] == 19  # 0b10011
    assert arr[6] == 0

def test_clearbit_array():
    test_array = 5*np.ones(3, dtype=np.int8)
    test_array[:] = clearbit(0, test_array)
    assert all(test_array == 4)

def test_flipbit_on_array_element():
    arr = 5*np.ones(10, dtype=np.uint64) # 0b101
    arr[5] = flipbit(0, arr[5])
    arr[5] = flipbit(1, arr[5])
    arr[5] = flipbit(2, arr[5])
    assert arr[4] == 5
    assert arr[5] == 2  # 0b010
    assert arr[6] == 5

def test_flipbit_array():
    test_array = 7*np.ones(3, dtype=np.int8) # 0b111
    test_array[:] = flipbit(0, test_array)
    assert all(test_array == 6)  # 0b110

def test_RegisterAddress_addition():
    r = RegisterAddress(0x10)
    r2 = r + 0x5
    assert r2.value() == 0x15
    r3 = 0x5 + r
    assert r3.value() == 0x15
    r += 0x5
    assert r.value() == 0x15