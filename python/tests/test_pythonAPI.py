import pytest 
import sys 

from conftest import test_with_simulators

from slsdet import Detector

@pytest.mark.withdetectorsimulators
def test_rx_ROI(test_with_simulators):
    """ Test setting and getting rx_ROI property of Detector class. """

    d = Detector()
    d.rx_roi = (0, 10, 10, 20)
    roi = d.rx_roi
    assert roi == [(0, 10, 10, 20)]
    
    d.rx_roi = [5,15,15,25]

    assert d.rx_roi == [(5,15,15,25)]

    d.rx_roi = [[0,10,0,20], [5,20,410,420]] #needs to be in second module 

    roi = d.rx_roi
    assert roi == [(0,10,0,20), (5,20,410,420)]

    d.clear_rx_roi() 
    roi = d.rx_roi
    assert roi == [(-1,-1,-1,-1)] 


