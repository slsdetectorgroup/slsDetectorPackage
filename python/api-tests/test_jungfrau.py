import pytest
import datetime as dt
from slsdet import Detector, detectorType

"""
These tests are designed to work the API and catch
any changes in behavior or naming. Tests are expected
to pass with a virtual detector or a real one

"""


@pytest.fixture
def jf():
    from slsdet import Jungfrau
    return Jungfrau()


jungfrautest = pytest.mark.skipif(
    Detector().type != detectorType.JUNGFRAU, reason="Only valid for Jungfrau"
)


@jungfrautest
def test_storagecells(jf):
    for i in range(16):
        jf.storagecells = i
        assert jf.storagecells == i
    jf.storagecells = 0  # default

@jungfrautest
def test_storagecell_start(jf):
    for i in range(16):
        jf.storagecell_start = i
        assert jf.storagecell_start == i
    jf.storagecells = 15  # default

@jungfrautest
def test_storagecell_delay(jf):
    for t in [0.001, 0.0002, 0.0013]:
        jf.storagecell_delay = t
        assert jf.storagecell_delay == t
    jf.storagecell_delay = 0  # default

@jungfrautest
def test_temp_event(jf):
    # hard to test with virtual server
    assert jf.temp_event == 0

@jungfrautest
def test_temp_threshold(jf):
    for th in [0, 10, 43, 72]:
        jf.temp_threshold = th
        assert jf.temp_threshold == th
    jf.temp_threshold = 0

@jungfrautest
def test_auto_comp_disable(jf):
    for v in [True, False]:
        jf.auto_comp_disable = v
        assert jf.auto_comp_disable == v

@jungfrautest
def test_numinterfaces(jf):
    for n in [2, 1]:
        jf.numinterfaces = n
        assert jf.numinterfaces == n

@jungfrautest
def test_dr(jf):
    assert jf.dr == 16

@jungfrautest
def test_temp_control(jf):
    for v in [True, False]:
        jf.temp_control = v
        assert jf.temp_control == v

@jungfrautest
def test_startingfnum(jf):
    for n in [10, 127, 43321, 1]:
        jf.startingfnum = n
        assert jf.startingfnum == n

@jungfrautest
def test_selinterface(jf):
    for i in [1, 0]:
        jf.selinterface = i
        assert jf.selinterface == i