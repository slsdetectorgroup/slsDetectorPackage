import pytest

from slsdet import Mythen3GainCapsWrapper
from slsdet.enums import M3_GainCaps #this is the c++ enum


def test_comapre_with_int():
    c = Mythen3GainCapsWrapper(128) #C10pre
    assert c == 128
    assert c != 5
    assert c != 1280

def test_compare_with_other():
    a = Mythen3GainCapsWrapper(128)
    b = Mythen3GainCapsWrapper(1<<10)
    c = Mythen3GainCapsWrapper(128)
    assert a!=b
    assert (a==b) == False
    assert a==c

def test_can_be_default_constructed():
    c = Mythen3GainCapsWrapper() 
    assert c == 0

