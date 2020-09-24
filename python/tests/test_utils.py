#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Testing functions from utils.py
"""

import pytest
from slsdet.utils import *
from slsdet import IpAddr, MacAddr
import datetime as dt
import pathlib
from pathlib import Path


def test_iterable():
    assert is_iterable(5) == False
    assert is_iterable('abc') == True
    assert is_iterable([]) == True
    assert is_iterable(5.9) == False


def test_reduce_time_to_single_value_from_list():
    t = 3 * [dt.timedelta(seconds=1)]
    assert reduce_time(t) == 1


def test_reduce_time_to_single_value_from_list_of_lists():
    t = 3 * [dt.timedelta(seconds=3.3)]
    tt = 5 * t
    assert reduce_time(tt) == 3.3


def test_reduce_time_when_sublist_is_different():
    t = [
        dt.timedelta(seconds=1),
        dt.timedelta(seconds=2),
        dt.timedelta(seconds=1)
    ]
    tt = [t for i in range(4)]
    assert reduce_time(tt) == [1, 2, 1]


def test_convert_zero():
    assert eiger_register_to_time(0) == 0


def test_convert_smallest_unit():
    assert pytest.approx(eiger_register_to_time(0b1000), 1e-9) == 1e-8


def test_convert_second_smallest_unit():
    assert pytest.approx(eiger_register_to_time(0b10000), 1e-9) == 2e-8


def test_convert_one_ms_using_exponent():
    assert pytest.approx(eiger_register_to_time(0b1101), 1e-9) == 1e-3


def test_convert_five_seconds():
    assert pytest.approx(eiger_register_to_time(0b1001110001000101),
                         1e-9) == 5.0


def test_all_equal_int():
    assert all_equal([5, 5]) == True


def test_all_equal_fails():
    assert all_equal([5, 6]) == False


def test_all_equal_tuple():
    assert all_equal(('a', 'a', 'a')) == True


def test_all_equal_str():
    assert all_equal('aaa') == True


def test_all_equal_str_fails():
    assert all_equal('aaab') == False


def test_element_if_equal_int():
    assert element_if_equal([5, 5]) == 5


def test_element_if_equal_str():
    assert element_if_equal('hhh') == 'h'


def test_element_if_equal_int_fails():
    assert element_if_equal([5, 6, 7]) == [5, 6, 7]


def test_get_set_bits():
    assert (get_set_bits(0) == [])
    assert get_set_bits(7) == [0, 1, 2]


def test_list_to_mask():
    assert (list_to_bitmask([0, 1, 2]) == 7)
    assert (list_to_bitmask([]) == 0)
    assert (list_to_bitmask([0]) == 1)
    assert (list_to_bitmask([1]) == 2)
    assert (list_to_bitmask([3]) == 8)
    assert (list_to_bitmask([1, 1, 1]) == 2)


def test_make_timedelta_from_double():
    t = 1.7
    r = make_timedelta(t)
    assert t == r.total_seconds()
    assert r == dt.timedelta(seconds=t)


def test_make_timedelta_from_timedelta():
    t = dt.timedelta(minutes=1)
    r = make_timedelta(t)
    assert 60 == r.total_seconds()
    assert r == dt.timedelta(minutes=1)


def test_make_string_path_from_Path():
    pathstr = "/some/temp/path"
    p = pathlib.Path(pathstr)
    r = make_string_path(p)
    assert isinstance(r, str)
    assert r == p.as_posix()
    assert r == pathstr


def test_make_string_path_expand_user():
    pathstr = "~/tmp/virtual.config"
    home = pathlib.Path.home()
    expanded_str = pathstr.replace('~', home.as_posix())
    p = pathlib.Path(pathstr)
    rp = make_string_path(p)
    rs = make_string_path(pathstr)
    assert rp == expanded_str
    assert rs == expanded_str


def test_lhex_passing_list():
    values = [0, 1, 2, 3, 4]
    assert lhex(values) == ["0x0", "0x1", "0x2", "0x3", "0x4"]


def test_lhex_emty_list():
    assert lhex([]) == []


def test_make_ip_from_dict():
    arg = {0: 0, 1: "192.168.1.1"}
    res = make_ip(arg)
    assert res == {0: IpAddr("0.0.0.0"), 1: IpAddr("192.168.1.1")}
    assert res[0].str() == "0.0.0.0"
    assert res[1].str() == "192.168.1.1"


def test_make_ip_from_str():
    ip = "192.168.1.1"
    assert make_ip(ip).str() == ip


def test_make_ip_from_list():
    arg = ["192.168.1.1", "192.168.1.2", "127.0.0.1"]
    assert make_ip(arg) == [IpAddr(a) for a in arg]

def test_make_ip_from_tuple():
    arg = ("127.0.0.1")
    assert make_ip(arg) == (IpAddr(arg))

def test_make_mac_from_dict():
    arg = {6: "84:a9:aa:24:32:88", 12: "84:a9:3e:24:32:aa"}
    res = make_mac(arg)
    assert res == {
        6: MacAddr("84:a9:aa:24:32:88"),
        12: MacAddr("84:a9:3e:24:32:aa")
    }
    assert res[6].str() == "84:a9:aa:24:32:88"
    assert res[12].str() == "84:a9:3e:24:32:aa"


def test_make_mac_from_str():
    mac = "84:a9:aa:24:32:88"
    assert make_mac(mac) == MacAddr(mac)
    assert make_mac(mac).str() == mac


def test_make_mac_from_list():
    arg = ["84:a9:aa:24:32:88", "84:a9:3e:24:32:aa"]
    assert make_mac(arg) == [MacAddr(a) for a in arg]


def test_make_mac_from_tuple():
    arg = ("84:a9:aa:24:32:88", "84:a9:3e:24:32:aa")
    assert make_mac(arg) == (MacAddr("84:a9:aa:24:32:88"),
                             MacAddr("84:a9:3e:24:32:aa"))

def test_make_path_from_str():
    assert make_path("/") == Path("/")
    assert make_path("/home") == Path("/home")

def test_make_path_from_list():
    arg = ["/", "/home", "/another/path"]
    assert make_path(arg) == [Path(p) for p in arg]