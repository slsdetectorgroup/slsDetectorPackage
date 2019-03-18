import pytest
import config_test
import time
from sls_detector.errors import DetectorValueError

from fixtures import eiger, eigertest





@eigertest
def test_set_matrix_reset(eiger):
    eiger.eiger_matrix_reset = False
    assert eiger.eiger_matrix_reset == False
    eiger.eiger_matrix_reset = True
    assert eiger.eiger_matrix_reset == True

@eigertest
def test_set_tx_delay_left_single(eiger):
    eiger.tx_delay.left[0] = 130
    assert eiger.tx_delay.left[0] == 130
    eiger.tx_delay.left[1] = 150
    assert eiger.tx_delay.left[1] == 150
    eiger.tx_delay.left[0] = 0
    eiger.tx_delay.left[1] = 0
    assert eiger.tx_delay.left[0] == 0
    assert eiger.tx_delay.left[1] == 0

@eigertest
def test_set_tx_delay_right_single(eiger):
    eiger.tx_delay.right[0] = 130
    assert eiger.tx_delay.right[0] == 130
    eiger.tx_delay.right[1] = 150
    assert eiger.tx_delay.right[1] == 150
    eiger.tx_delay.right[0] = 0
    eiger.tx_delay.right[1] = 0
    assert eiger.tx_delay.right[0] == 0
    assert eiger.tx_delay.right[1] == 0

@eigertest
def test_set_tx_delay_frame_single(eiger):
    eiger.tx_delay.frame[0] = 500
    eiger.tx_delay.frame[1] = 600
    assert eiger.tx_delay.frame[0] == 500
    assert eiger.tx_delay.frame[1] == 600

    eiger.tx_delay.frame[0] = 0
    eiger.tx_delay.frame[1] = 0
    assert eiger.tx_delay.frame[0] == 0
    assert eiger.tx_delay.frame[1] == 0

@eigertest
def test_tx_delay_from_list(eiger):
    eiger.tx_delay.left = [123,456]
    assert eiger.tx_delay.left[:] == [123,456]
    eiger.tx_delay.right = [789,100]
    assert eiger.tx_delay.right[:] == [789,100]
    eiger.tx_delay.frame = [1000,90000]
    assert eiger.tx_delay.frame[:] == [1000,90000]

    eiger.tx_delay.left = [0, 0]
    eiger.tx_delay.right = [0, 0]
    eiger.tx_delay.frame = [0, 0]
    assert eiger.tx_delay.left[:] == [0, 0]
    assert eiger.tx_delay.right[:] == [0, 0]
    assert eiger.tx_delay.frame[:] == [0, 0]

@eigertest
def test_acitve(eiger):
    eiger.file_write = False
    eiger.reset_frames_caught()
    eiger.active[1] = False
    eiger.acq()
    assert eiger._api.getFramesCaughtByReceiver(1) == 0
    assert eiger._api.getFramesCaughtByReceiver(0) == 1
    eiger.active = True
    time.sleep(0.5)
    eiger.acq()
    assert eiger.frames_caught == 1

@eigertest
def test_set_default_settings(eiger):
    eiger.default_settings()
    assert eiger.n_frames == 1
    assert eiger.exposure_time == 1
    assert eiger.period == 0
    assert eiger.n_cycles == 1
    assert eiger.dynamic_range == 16

@eigertest
def test_flowcontrol10g(eiger):
    eiger.flowcontrol_10g = True
    assert eiger.flowcontrol_10g == True
    eiger.flowcontrol_10g = False
    assert eiger.flowcontrol_10g == False

@eigertest
def test_read_vcmp(eiger):
    eiger.vthreshold = 1500
    assert eiger.vcmp[:]  == [1500]*4*eiger.n_modules

@eigertest
def test_set_vcmp(eiger):
    eiger.vcmp = [1000,1100,1200,1300,1400,1500,1600,1700]
    assert eiger.vcmp[:] == [1000,1100,1200,1300,1400,1500,1600,1700]
    eiger.vthreshold = 1500

#Disabled only works with receiver on the same pc
# @eigertest
# def test_setup500k():
#     from sls_detector import Eiger, free_shared_memory
#     free_shared_memory()
#     d = Eiger()
#     d.setup500k(config_test.known_hostnames)
#     d.acq()
#     assert d.rx_tcpport == [1954,1955]
#     assert d.frames_caught == 1
#     #could assert more setting but if the frame is caught it worked...