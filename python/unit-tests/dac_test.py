#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Testing setting and getting dacs from the detector
"""
from unittest.mock import Mock, call
import pytest
from pytest_mock import mocker
import numpy as np

from sls_detector import Eiger
from sls_detector import DetectorApi


def test_get_vrf_for_three_mod(mocker):
    m2= mocker.patch.object(DetectorApi, 'getNumberOfDetectors', autospec=True)
    m2.return_value = 3
    m = mocker.patch.object(DetectorApi, 'getDac', autospec=True)
    m.return_value = 1560
    d = Eiger()
    vrf = d.dacs.vrf[:]
    assert vrf == [1560, 1560, 1560]
    
def test_set_vrf_for_three_mod_same_value(mocker):
    m2= mocker.patch.object(DetectorApi, 'getNumberOfDetectors', autospec=True)
    m2.return_value = 3
    m = mocker.patch.object(DetectorApi, 'setDac', autospec=True)
#    m.return_value = 1560
    d = Eiger()
    d.dacs.vrf[:] = 1500
    calls = [call('vrf', 0, 1500), call('vrf', 1, 1500), call('vrf', 2, 1500)]
    m.assert_has_calls(calls)
    assert m.call_count == 3
    
def test_set_vrf_for_four_mod_different_value(mocker):
    m2= mocker.patch.object(DetectorApi, 'getNumberOfDetectors', autospec=True)
    m2.return_value = 4
    m = mocker.patch.object(DetectorApi, 'setDac', autospec=True)
#    m.return_value = 1560
    d = Eiger()
    d.dacs.vrf =  [1500, 1600, 1800, 1502]
    calls = [call('vrf', 0, 1500), 
             call('vrf', 1, 1600), 
             call('vrf', 2, 1800),
             call('vrf', 3, 1502)]
    m.assert_has_calls(calls)
    assert m.call_count == 4
    
def test_set_vrf_for_four_mod_different_value_slice(mocker):
    m2= mocker.patch.object(DetectorApi, 'getNumberOfDetectors', autospec=True)
    m2.return_value = 4
    m = mocker.patch.object(DetectorApi, 'setDac', autospec=True)
#    m.return_value = 1560
    d = Eiger()
    d.dacs.vrf[:] =  [1500, 1600, 1800, 1502]
    calls = [call('vrf', 0, 1500), 
             call('vrf', 1, 1600), 
             call('vrf', 2, 1800),
             call('vrf', 3, 1502)]
    m.assert_has_calls(calls)
    assert m.call_count == 4
    
def test_set_vcp_single_call(mocker):
    m2= mocker.patch.object(DetectorApi, 'getNumberOfDetectors', autospec=True)
    m2.return_value = 2
    m = mocker.patch.object(DetectorApi, 'setDac', autospec=True)
#    m.return_value = 1560
    d = Eiger()
    d.dacs.vcp[1] =  1637
    m.assert_called_once_with('vcp', 1, 1637)
    
def test_iterate_on_index_call_vcn(mocker):
    m2= mocker.patch.object(DetectorApi, 'getNumberOfDetectors', autospec=True)
    m2.return_value = 10
    m = mocker.patch.object(DetectorApi, 'setDac', autospec=True)
#    m.return_value = 1560
    d = Eiger()
    d.dacs.vcn[0,3,8] = 1532
    calls = [call('vcn', 0, 1532), 
             call('vcn', 3, 1532), 
             call('vcn', 8, 1532)]
    m.assert_has_calls(calls)
    assert m.call_count == 3

def test_set_dac_from_element_in_numpy_array(mocker):
    m2= mocker.patch.object(DetectorApi, 'getNumberOfDetectors', autospec=True)
    m2.return_value = 2
    m = mocker.patch.object(DetectorApi, 'setDac', autospec=True)  
    d = Eiger()
    
    vrf = np.array((1600,1700,1800))
    d.dacs.vrf = vrf[0]
    calls = [call('vrf', 0, 1600),
             call('vrf', 1, 1600),]
    m.assert_has_calls(calls)
    assert m.call_count == 2

def test_set_dac_from_element_in_numpy_array_using_slice(mocker):
    m2= mocker.patch.object(DetectorApi, 'getNumberOfDetectors', autospec=True)
    m2.return_value = 2
    m = mocker.patch.object(DetectorApi, 'setDac', autospec=True)  
    d = Eiger()
    
    vrf = np.array((1600,1700,1800))
    d.dacs.vrf[:] = vrf[0]
    calls = [call('vrf', 0, 1600),
             call('vrf', 1, 1600),]
    m.assert_has_calls(calls)
    assert m.call_count == 2
    
def test_set_eiger_default(mocker):
    m2= mocker.patch.object(DetectorApi, 'getNumberOfDetectors', autospec=True)
    m2.return_value = 2
    m = mocker.patch.object(DetectorApi, 'setDac', autospec=True)
#    m.return_value = 1560
    d = Eiger()
    d.dacs.set_default()
    calls = [call('vsvp', 0, 0),
             call('vsvp', 1, 0),
             call('vtr', 0, 2500),
             call('vtr', 1, 2500),
             call('vrf', 0, 3300),
             call('vrf', 1, 3300),
             call('vrs', 0, 1400),
             call('vrs', 1, 1400),
             call('vsvn', 0, 4000),
             call('vsvn', 1, 4000),
             call('vtgstv', 0, 2556),
             call('vtgstv', 1, 2556),
             call('vcmp_ll', 0, 1500),
             call('vcmp_ll', 1, 1500),
             call('vcmp_lr', 0, 1500),
             call('vcmp_lr', 1, 1500),
             call('vcall', 0, 4000),
             call('vcall', 1, 4000),
             call('vcmp_rl', 0, 1500),
             call('vcmp_rl', 1, 1500),
             call('rxb_rb', 0, 1100),
             call('rxb_rb', 1, 1100),
             call('rxb_lb', 0, 1100),
             call('rxb_lb', 1, 1100),
             call('vcmp_rr', 0, 1500),
             call('vcmp_rr', 1, 1500),
             call('vcp', 0, 200),
             call('vcp', 1, 200),
             call('vcn', 0, 2000),
             call('vcn', 1, 2000),
             call('vis', 0, 1550),
             call('vis', 1, 1550),
             call('iodelay', 0, 660),
             call('iodelay', 1, 660)]

    m.assert_has_calls(calls)
    assert m.call_count == 17*2
    
def test_set_eiger_set_from_array_call_count(mocker):
    import numpy as np
    m2= mocker.patch.object(DetectorApi, 'getNumberOfDetectors', autospec=True)
    m2.return_value = 3
    m = mocker.patch.object(DetectorApi, 'setDac', autospec=True)
#    m.return_value = 1560
    d = Eiger()
    d.dacs.set_from_array( np.zeros((17,3)))
    assert m.call_count == 17*3
    
def test_get_fpga_temp(mocker):
    m2= mocker.patch.object(DetectorApi, 'getNumberOfDetectors', autospec=True)
    m2.return_value = 2
    m = mocker.patch.object(DetectorApi, 'getAdc', autospec=True)
    m.return_value = 34253
    d = Eiger()
    t = d.temp.fpga[:]
    assert t == [34.253, 34.253]