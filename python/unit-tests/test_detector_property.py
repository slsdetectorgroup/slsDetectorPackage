import pytest
from sls_detector.detector_property import DetectorProperty

class Holder:
    """
    This class does nothing except hold values
    for testing of the DetectorProperty class
    """
    def __init__(self, N):
        self.values = [i for i in range(N)]
    def get(self, i):
        return self.values[i]
    def set(self, i,v):
        self.values[i] = v
    def nmod(self):
        return len(self.values)

@pytest.fixture
def p():
    h = Holder(5)
    return DetectorProperty(h.get, h.set, h.nmod, 'prop')

def test_initialization():
    def getf(i):
        return 5
    def setf():
        return
    def nmod():
        return 3
    name = 'a property'
    p = DetectorProperty(getf, setf, nmod, name)
    assert p.get == getf
    assert p.set == setf
    assert p.get_nmod == nmod
    assert p.__name__ == name

def test_get_single_value(p):
    assert p[2] == 2

def test_get_all_values(p):
    assert p[:] == [0, 1, 2, 3, 4]

def test_get_values_by_iterable(p):
    vals = p[1,3]
    assert vals == [1,3]

def test_set_single_value(p):
    p[2] = 7
    assert p[:] == [0,1,7,3,4]
 
def test_set_all(p):
     p[:] = 10
     assert p[:] == [10,10,10,10,10]

def test_set_all_by_list(p):
    p[:] = [7,8,9,10,11]
    assert p[:] == [7,8,9,10,11]

def test_set_all_bool(p):
    p[:] = True
    assert p[:] == [True]*5

def test_set_by_iter(p):
    keys = [2,4]
    vals = [18,23]
    p[keys] = vals
    assert p[:] == [0,1,18,3,23]

def test_set_by_iter_single_val(p):
    keys = [2,4]
    val = 9
    p[keys] = val
    assert p[:] == [0,1,9,3,9]

def test_print_values(p):
    assert repr(p) == 'prop: [0, 1, 2, 3, 4]'
