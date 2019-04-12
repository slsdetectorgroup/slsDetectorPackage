#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Testing setting dynamic range for Eiger. 
If the detector is not Eiger the tests are skipped
"""
import pytest
import config_test
from fixtures import detector, eiger, jungfrau, eigertest, jungfrautest
from sls_detector.errors import DetectorValueError


@eigertest
def test_set_dynamic_range_and_make_acq(eiger):
    eiger.exposure_time = 0.5
    eiger.n_frames = 2
    for dr in [4, 8, 16, 32]:
        eiger.dynamic_range = dr
        assert eiger.dynamic_range == dr
        eiger.acq()
        assert eiger.frames_caught == 2


@eigertest
def test_set_dynamic_range_raises(eiger):
    with pytest.raises(DetectorValueError):
        eiger.dynamic_range = 1
    with pytest.raises(DetectorValueError):
        eiger.dynamic_range = 75
    with pytest.raises(DetectorValueError):
        eiger.dynamic_range = -3
    with pytest.raises(DetectorValueError):
        eiger.dynamic_range = 12

@eigertest
def test_set_dynamic_range_reduces_speed(eiger):
    eiger.readout_clock = 'Full Speed'
    eiger.dynamic_range = 32
    assert eiger.dynamic_range == 32
    assert eiger.readout_clock == 'Quarter Speed'

    eiger.dynamic_range = 16
    assert eiger.dynamic_range == 16
    assert eiger.readout_clock == 'Half Speed'
