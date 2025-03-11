from slsdet.bits import clearbit, clearbit_arr, setbit, setbit_arr
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
    assert arr[5] == 19  # 0b10011


def test_setbit_arr():
    arr = np.zeros(10, dtype=np.int32)
    setbit_arr(3, arr[3:9])
    assert all(arr == np.array((0, 0, 0, 8, 8, 8, 8, 8, 8, 0), dtype=np.int32))


def test_clearbit_arr():
    arr = np.array((5, 5, 5), dtype=np.int8)
    clearbit_arr(0, arr)
    assert all(arr == (4, 4, 4))