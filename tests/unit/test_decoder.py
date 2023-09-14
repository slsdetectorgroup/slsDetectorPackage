import numpy as np
import sys

from pyctbgui.utils import decoder
from pyctbgui.utils.pixelmap import moench04_analog, matterhorn_transceiver


def test_simple_decode():
    pixel_map = np.zeros((2, 2), dtype=np.uint32)
    pixel_map.flat = [0, 1, 2, 3]

    raw_data = np.zeros(4, dtype=np.uint16)
    raw_data.flat = [8, 9, 10, 11]
    data = decoder.decode(raw_data, pixel_map)

    assert data[0, 0] == 8
    assert data[0, 1] == 9
    assert data[1, 0] == 10
    assert data[1, 1] == 11

    # Make sure we didn't mess up the reference counts
    assert sys.getrefcount(data) == 2
    assert sys.getrefcount(raw_data) == 2
    assert sys.getrefcount(pixel_map) == 2


def test_compare_python_and_c_decoding():
    """
    Check that the python and C implementation give the same results
    provides a regression test in case we change the C implementation
    """
    pixel_map = moench04_analog()
    raw_data = np.arange(400 * 400, dtype=np.uint16)
    c_data = decoder.decode(raw_data, pixel_map)
    py_data = decoder.moench04(raw_data)
    assert (c_data == py_data).all()

    pixel_map = matterhorn_transceiver()
    raw_data = np.arange(48 * 48, dtype=np.uint16)
    c_data = decoder.decode(raw_data, pixel_map)
    py_data = decoder.matterhorn(raw_data)
    assert (c_data == py_data).all()
