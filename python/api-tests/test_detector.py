import pytest
import datetime as dt
from slsdet import Detector, timingMode, detectorType

not_eiger = pytest.mark.skipif(
    Detector().type == detectorType.EIGER, reason="Does not work for eiger"
)


@pytest.fixture
def det():
    from slsdet import Detector

    return Detector()


def test_frames(det):
    for n in [1, 100, 3245, 10000]:
        det.frames = n
        assert det.frames == n
    det.frames = 1


def test_triggers(det):
    for n in [1, 100, 3245, 10000]:
        det.triggers = n
        assert det.triggers == n
    det.triggers = 1


def test_exptime(det):
    det.exptime = 1
    assert det.exptime == 1
    det.exptime = dt.timedelta(milliseconds=10)
    assert det.exptime == 0.01
    det.exptime = 1


def test_period(det):
    det.period = 3.2
    assert det.period == 3.2

    p = dt.timedelta(microseconds=1020)
    det.period = p
    assert det.period == 0.001020
    r = det.getPeriod()
    assert r[0] == p
    det.period = 0
    assert det.period == 0


def test_lock(det):
    for l in [True, False]:
        det.lock = l
        assert det.lock == l


def test_timing(det):
    # auto and trigger is available for all det
    for m in [timingMode.TRIGGER_EXPOSURE, timingMode.AUTO_TIMING]:
        det.timing = m
        assert det.timing == m

@not_eiger
def test_delay(det):
    det.delay = 1
    assert det.delay == 1

    t  = dt.timedelta(microseconds=1)
    det.delay = t
    assert det.delay == t.total_seconds()

    r = det.getDelayAfterTrigger()[0]
    assert r == t

    det.delay = 0
    assert det.delay == 0


@not_eiger
def test_delayl(det):
    assert det.delayl == 0