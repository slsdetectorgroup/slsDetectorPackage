#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Testing functions from utils.py
"""

import pytest
from sls_detector.utils import *


def test_convert_zero():
    assert eiger_register_to_time(0) == 0

def test_convert_smallest_unit():
    assert pytest.approx(eiger_register_to_time(0b1000), 1e-9) == 1e-8

def test_convert_second_smallest_unit():
    assert pytest.approx(eiger_register_to_time(0b10000), 1e-9) == 2e-8

def test_convert_one_ms_using_exponent():
    assert pytest.approx(eiger_register_to_time(0b1101), 1e-9) == 1e-3

def test_convert_five_seconds():
    assert pytest.approx(eiger_register_to_time(0b1001110001000101), 1e-9) == 5.0

def test_all_equal_int():
    assert all_equal([5,5]) == True

def test_all_equal_fails():
    assert all_equal([5,6])  == False

def test_all_equal_tuple():
    assert all_equal(('a', 'a', 'a')) == True

def test_all_equal_str():
    assert all_equal('aaa') == True

def test_all_equal_str_fails():
    assert all_equal('aaab') == False



def test_element_if_equal_int():
    assert element_if_equal([5,5]) == 5

def test_element_if_equal_str():
    assert element_if_equal('hhh') == 'h'

def test_element_if_equal_int_fails():
    assert element_if_equal([5, 6, 7]) == [5, 6, 7]

def test_get_set_bits():
    assert(get_set_bits(0) == [])
    assert get_set_bits(7) == [0, 1, 2]
    
def test_list_to_mask():
    assert(list_to_bitmask([0,1,2]) == 7)
    assert(list_to_bitmask([]) == 0)
    assert(list_to_bitmask([0]) == 1)
    assert(list_to_bitmask([1]) == 2)
    assert(list_to_bitmask([3]) == 8)
    assert(list_to_bitmask([1,1,1]) == 2)