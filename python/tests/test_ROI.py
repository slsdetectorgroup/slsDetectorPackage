import pytest 
import sys 

from conftest import session_simulator

from slsdet import Detector

from slsdet._slsdet import slsDetectorDefs

detectorType = slsDetectorDefs.detectorType

@pytest.mark.detectorintegration
def test_rx_ROI(session_simulator):
    """ Test rx_ROI property of Detector class. """
    det_type, num_interfaces, num_mods, d = session_simulator
    assert d is not None

    if d.type == detectorType.CHIPTESTBOARD or d.type == detectorType.XILINX_CHIPTESTBOARD:
        pytest.skip("Skipping ROI test for ctb/xilinx_ctb detector types.")
    
    if(d.type == detectorType.MYTHEN3 or d.type == detectorType.GOTTHARD2):
        d.rx_roi = (0, 10)
        roi = d.rx_roi
        assert roi == [(0, 10, -1, -1)]

        #d.rx_roi = [[5,15, 0, 1]] # not allowed for mythen3

        d.rx_roi = [0,10, -1, -1]

        assert d.rx_roi == [(0,10,-1,-1)]
        d.rx_clearroi()
    else: 

        d.rx_roi = (0, 10, 10, 20)
        roi = d.rx_roi
        assert roi == [(0, 10, 10, 20)]
    
        d.rx_roi = [5,15,15,25]

        assert d.rx_roi == [(5,15,15,25)]

        if d.nmod > 1 and (d.type != detectorType.JUNGFRAU) or (d.numinterfaces == 2 and d.type != detectorType.EIGER): 
            d.rx_roi = [[0,10,0,20], [5,20,410,420]] 

            roi = d.rx_roi
            assert roi == [(0,10,0,20), (5,20,410,420)] #in same file for jungfrau 

        d.rx_clearroi() 
        roi = d.rx_roi
        assert roi == [(-1,-1,-1,-1)] 







    
    
