#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Tests regarding exposure time and period of the detector
Set and get test as well as test for duration and on detector 
measurement of the time.
"""
import pytest
import config_test
from fixtures import detector, eiger, jungfrau, eigertest, jungfrautest
from sls_detector.errors import DetectorValueError, DetectorError
import time


testdata_times = [1e-8, 0.001, 0.5, 3.125, 5.0, 600, 784]
@pytest.mark.parametrize("t", testdata_times)
def test_set_and_get_exposure_time(eiger, t):
    """
    Test that the exposure time we set in the detector
    is the same as the one read back
    """
    eiger.exposure_time = t
    assert eiger.exposure_time == t


def test_negative_exposure_time_raises_error(eiger):
    with pytest.raises(DetectorValueError):
        eiger.exposure_time = -15


testdata_times = [0.001, 0.0025, 0.005, 5]
@pytest.mark.parametrize("t", testdata_times)
def test_set_subexptime(eiger, t):
    eiger.sub_exposure_time = t
    assert eiger.sub_exposure_time == t


testdata_times = [-5,6,7,50]
@pytest.mark.parametrize("t", testdata_times)
def test_set_subextime_too_large_or_neg(eiger, t):
    with pytest.raises((DetectorError, DetectorValueError)):
        eiger.sub_exposure_time = t



testdata_times = [0.2, 0.5, 1, 2, 5, 7]
@pytest.mark.slow
@pytest.mark.parametrize("t", testdata_times)
def test_measure_exposure_time_from_python(eiger, t):
    """ 
    The main idea with this test is to make sure the overhead of a
    single acq is less than tol[s]. This test also catches stupid bugs
    that would for example not change the exposure time or make acquire 
    not blocking. 
    """
    tol = 0.5
    eiger.dynamic_range = 16
    eiger.file_write = False
    eiger.n_frames = 1
    eiger.exposure_time = t
    assert eiger.exposure_time == t
    t0 = time.time()
    eiger.acq()
    duration = time.time()-t0
    assert duration < (t+tol)


testdata_times = [0.5, 1, 3, 5]


@pytest.mark.slow
@pytest.mark.parametrize("t", testdata_times)
def test_measure_period_from_python_and_detector(eiger, t):
    tol = 0.5
    nframes = 5
    eiger.dynamic_range = 16
    eiger.file_write = False
    eiger.n_frames = nframes
    eiger.exposure_time = 0.001
    eiger.period = t
    t0 = time.time()
    eiger.acq()
    duration = time.time()-t0
    assert duration < t*(nframes-1)+tol
    for mp in eiger.measured_period:
        assert pytest.approx(mp, 1e-5) == t


testdata_times = [0.001, 0.002, 0.003, 0.005, 0.01]
@pytest.mark.parametrize("t", testdata_times)
def test_measure_subperiod_nonparallel(eiger, t):
    readout_time = 500e-6
    eiger.dynamic_range = 32
    eiger.file_write = False
    eiger.flags = 'nonparallel'
    eiger.n_frames = 1
    eiger.period = 0
    eiger.exposure_time = 0.5
    eiger.sub_exposure_time = t
    eiger.sub_deadtime = 0
    eiger.acq()
    for mp in eiger.measured_subperiod:
        assert pytest.approx(mp, abs=1e-5) == t+readout_time


@pytest.mark.parametrize("t", testdata_times)
def test_measure_subperiod_parallel(eiger, t):
    readout_time = 12e-6
    eiger.dynamic_range = 32
    eiger.file_write = False
    eiger.flags = 'parallel'
    eiger.n_frames = 1
    eiger.period = 0
    eiger.exposure_time = 0.5
    eiger.sub_exposure_time = t
    eiger.sub_deadtime = 0
    eiger.acq()
    for mp in eiger.measured_subperiod:
        assert pytest.approx(mp, abs=1e-5) == t+readout_time


@pytest.mark.parametrize("t", testdata_times)
def test_measure_subperiod_parallel_when_changing_deadtime(eiger, t):
    readout_time = 12e-6
    exposure_time = 0.001
    eiger.dynamic_range = 32
    eiger.file_write = False
    eiger.flags = 'parallel'
    eiger.n_frames = 1
    eiger.period = 0
    eiger.exposure_time = 0.5
    eiger.sub_exposure_time = exposure_time
    eiger.sub_deadtime = t
    eiger.acq()
    for mp in eiger.measured_subperiod:
        assert pytest.approx(mp, abs=1e-5) == t+exposure_time