import pytest 
import sys 

from conftest import test_with_simulators

from slsdet import Detector

from utils_for_test import (
    Log,
    LogLevel,
)

@pytest.mark.detectorintegration
@pytest.mark.parametrize("setup_parameters", [(["moench"], 2)], indirect=True)
def test_rx_ROI_moench(test_with_simulators, setup_parameters):
    """ Test setting and getting rx_ROI property of Detector class for moench. """

    d = Detector()
    d.rx_roi = (0, 10, 10, 20)
    roi = d.rx_roi
    assert roi == [(0, 10, 10, 20)]
    
    d.rx_roi = [5,15,15,25]

    assert d.rx_roi == [(5,15,15,25)]

    d.rx_roi = [[0,10,0,20], [5,20,410,420]] 

    roi = d.rx_roi
    assert roi == [(0,10,0,20), (5,20,410,420)]

    d.rx_clearroi() 
    roi = d.rx_roi
    assert roi == [(-1,-1,-1,-1)] 

@pytest.mark.detectorintegration
@pytest.mark.parametrize("setup_parameters", [(["mythen3"], 1)], indirect=True)
def test_rx_ROI_mythen(test_with_simulators, setup_parameters):
    """ Test setting and getting rx_ROI property of Detector class for mythen. """

    d = Detector()
    d.rx_roi = (0, 10)
    roi = d.rx_roi
    assert roi == [(0, 10, -1, -1)]

    #d.rx_roi = [[5,15, 0, 1]] # not allowed for mythen3

    d.rx_roi = [0,10, -1, -1]

    assert d.rx_roi == [(0,10,-1,-1)]
    
