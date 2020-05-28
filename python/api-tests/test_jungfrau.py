import pytest
import datetime as dt

@pytest.fixture
def jf():
    from slsdet import Jungfrau
    return Jungfrau()


def test_storagecells(jf):
    for i in range(16):
        jf.storagecells = i
        assert jf.storagecells == i
    jf.storagecells = 0 #default

def test_storagecell_start(jf):
    for i in range(16):
        jf.storagecell_start = i
        assert jf.storagecell_start == i
    jf.storagecells = 15 #default


def test_storagecell_delay(jf):
    for t in [0.001, 0.0002, 0.0013]:
        jf.storagecell_delay = t
        assert jf.storagecell_delay == t
    jf.storagecell_delay = 0 #default