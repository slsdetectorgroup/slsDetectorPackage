from slsdet import Hz
'''
def test_Hz():
    f = Hz(1)
    assert f.value() == 1e6
    f = Hz('1MHz')
    assert f.value() == 1e6
    f = Hz('5000kHz')
    assert f.value() == 5e6
    assert Hz(1) == 1
    assert Hz(1e6) == 1e6
    assert Hz(1, 'MHz') == 1e6
    assert Hz(0.5, 'GHz') == 0.5e9
'''