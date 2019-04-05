import pytest
import config_test
import time
from sls_detector.errors import DetectorValueError
import os
from fixtures import eiger, eigertest


testdata_th = [0,333,500,1750,2000]

@eigertest
@pytest.mark.parametrize("th", testdata_th)
def test_set_vthreshold(eiger, th):
    eiger.vthreshold = th
    assert eiger.vthreshold == th

@eigertest
def test_vthreshold_with_different_vcmp(eiger):
    #When vcmp is different for the chip vthreshold should return -1
    eiger.vthreshold = 1500
    eiger.dacs.vcmp_ll = 1400
    assert eiger.vthreshold == -1

@eigertest
def test_set_settingsdir(eiger):
    path = os.path.dirname( os.path.realpath(__file__) )
    path = os.path.join(path, 'settingsdir')
    eiger.settings_path = path
    assert eiger.settings_path == path

@eigertest
def test_set_trimmed_energies(eiger):
    en = [5000,6000,7000]
    eiger.trimmed_energies = en
    assert eiger.trimmed_energies == en


#TODO! add checks for vcmp as well and improve naming
#TODO! remove dependency on beb number
testdata_en = [(5000, 500),(5500,750),(6000,1000),(6200,1100),(7000,1500)]
@eigertest
@pytest.mark.parametrize('val', testdata_en)
def test_set_energy_threshold(eiger, val):
    eiger.settings = 'standard'
    eiger.threshold = val[0]
    assert eiger.threshold == val[0]
    assert eiger.dacs.vrf[0] == val[1]