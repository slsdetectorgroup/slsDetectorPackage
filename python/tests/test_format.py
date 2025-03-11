from slsdet import format as fmt
import numpy as np


def test_hexFormat():
    assert fmt.hexFormat(500, 0) == "0x1f4"
    assert fmt.hexFormat(500, 4) == "0x01f4"
    assert fmt.hexFormat(500, 5) == "0x001f4"

    assert fmt.hexFormat(np.uint64(1), 0) == "0x1"
    assert fmt.hexFormat(np.int64(100), 4) == "0x0064"


def test_hexFormat_nox():
    assert fmt.hexFormat_nox(500, 0) == "1f4"
    assert fmt.hexFormat_nox(500, 4) == "01f4"
    assert fmt.hexFormat_nox(500, 5) == "001f4"


def test_binFormat():
    assert fmt.binFormat(123, 0) == "0b1111011"
    assert fmt.binFormat(123, 1) == "0b1111011"
    assert fmt.binFormat(123, 8) == "0b01111011"
    assert fmt.binFormat(123, 10) == "0b0001111011"


def test_binFormat_nob():
    assert fmt.binFormat_nob(123, 0) == "1111011"
    assert fmt.binFormat_nob(123, 1) == "1111011"
    assert fmt.binFormat_nob(123, 8) == "01111011"
    assert fmt.binFormat_nob(123, 10) == "0001111011"


def test_decFormat():
    assert fmt.decFormat(5, 0) == "5"
    assert fmt.decFormat(53, 5) == "00053"


def test_hex_formatting():
    assert fmt.to_hex(5, width = 4) == '0x0005'