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
    assert f.value == round(1_234_000)


def test_mul():
    c = MHz(1)
    assert (c * 2).value == 2_000_000
    assert (c * 4).value == 4_000_000


def test_rmul():
    c = MHz(1)
    assert (2 * c).value == 2_000_000
    assert (4 * c).value == 4_000_000

    c = c * 2
    assert c.value == 2_000_000

    for rc in [1, 2, 4, 8]:
        c = rc * c
    assert c.value == 128_000_000    


def test_div():
    c = MHz(1)
    assert (c / 2).value == 500_000

def test_eq():
    assert MHz(1) == MHz(1)
    assert MHz(1) != MHz(2)
    assert MHz(1) == kHz(1000)