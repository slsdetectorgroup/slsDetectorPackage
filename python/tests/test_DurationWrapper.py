import pytest

from slsdet import DurationWrapper

#import the compiled extension to use test functions for the automatic conversion
from slsdet import _slsdet


def test_default_construction_of_DurationWrapper():
    """Test default construction of DurationWrapper"""
    t = DurationWrapper()
    assert t.count() == 0
    assert t.total_seconds() == 0

def test_construction_of_DurationWrapper():
    """Test construction of DurationWrapper with total_seconds"""
    t = DurationWrapper(5)
    assert t.count() == 5e9
    assert t.total_seconds() == 5

def test_set_count_on_DurationWrapper():
    """Test set_count on DurationWrapper"""
    t = DurationWrapper()
    t.set_count(10)
    assert t.count() == 10
    assert t.total_seconds() == 10e-9
    t.set_count(0)
    assert t.count() == 0
    assert t.total_seconds() == 0


def test_return_a_DurationWrapper_from_cpp():
    """Test returning a DurationWrapper from C++"""
    t = _slsdet.test_return_DurationWrapper()
    assert t.count() == 1.3e9
    assert t.total_seconds() == 1.3

def test_call_a_cpp_function_with_a_duration_wrapper():
    """C++ functions can accept a DurationWrapper"""
    t = DurationWrapper(5)
    assert _slsdet.test_duration_to_ns(t) == 5e9
    
def test_call_a_cpp_function_converting_number_to_DurationWrapper():
    """int and float can be converted to std::chrono::nanoseconds"""
    assert _slsdet.test_duration_to_ns(0) == 0
    assert _slsdet.test_duration_to_ns(3) == 3e9
    assert _slsdet.test_duration_to_ns(1.3) == 1.3e9
    assert _slsdet.test_duration_to_ns(10e-9) == 10

def test_call_a_cpp_function_with_datetime_timedelta():
    """datetime.timedelta can be converted to std::chrono::nanoseconds"""
    import datetime
    t = datetime.timedelta(seconds=5)
    assert _slsdet.test_duration_to_ns(t) == 5e9
    t = datetime.timedelta(seconds=0)
    assert _slsdet.test_duration_to_ns(t) == 0
    t = datetime.timedelta(seconds=1.3)
    assert _slsdet.test_duration_to_ns(t) == 1.3e9