#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Tests for trimbit and dac related functions
"""
import pytest
import config_test
from fixtures import detector, eiger, jungfrau, eigertest, jungfrautest
from sls_detector.errors import DetectorValueError


@eigertest
def test_set_trimbits(eiger):
    """Limited values due to time"""
    for i in [17, 32, 60]:
        print(i)
        eiger.trimbits = i
        assert eiger.trimbits == i

@eigertest
def test_set_trimbits_raises_on_too_big(eiger):
    with pytest.raises(DetectorValueError):
        eiger.trimbits = 75

@eigertest
def test_set_trimbits_raises_on_negative(eiger):
    with pytest.raises(DetectorValueError):
        eiger.trimbits = -5


# @jungfrautest
# def test_jungfrau(jungfrau):
#     """Example of a test that is not run with Eiger connected"""
#     pass
