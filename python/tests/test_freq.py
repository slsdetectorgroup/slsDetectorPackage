from slsdet import Hz, MHz, kHz

def test_Hz():
    f = Hz(1)
    assert f.value == 1
    f = Hz(1 * 1000)
    assert f.value == 1000
    f = MHz(5)
    assert f.value == 5_000_000
    f = MHz(0.5)
    assert f.value == 500_000
    f = kHz(2.5)
    assert f.value == 2500
    f = kHz(5000)
    assert f.value == 5_000_000

def test_rounding_exact():
    f = MHz(1.234)
    assert f.value == round(1.234 * 1_000_000)

