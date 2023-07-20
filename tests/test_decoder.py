import pytest
import numpy as np
import sys

from decoder import decode
def test_simple_decode():
    pixel_map = np.zeros((2,2), dtype = np.uint32)
    pixel_map.flat = [0,1,2,3]

    raw_data = np.zeros(4, dtype = np.uint16)
    raw_data.flat = [8,9,10,11]
    data = decode(raw_data, pixel_map)

    assert data[0,0] == 8
    assert data[0,1] == 9
    assert data[1,0] == 10
    assert data[1,1] == 11

    #Make sure we didn't mess up the reference counts
    assert sys.getrefcount(data) == 2
    assert sys.getrefcount(raw_data) == 2
    assert sys.getrefcount(pixel_map) == 2