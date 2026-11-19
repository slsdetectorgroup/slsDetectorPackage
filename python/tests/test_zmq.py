import pytest 
import sys 
import time

from conftest import session_simulator

from slsdet import Detector
from slsdet.utils import element_if_equal

from slsdet._slsdet import slsDetectorDefs
from utils_for_test import (
    Log,
    LogLevel,
)

detectorType = slsDetectorDefs.detectorType

@pytest.mark.detectorintegration
@pytest.mark.parametrize(
    "session_simulator",
    [
        ("eiger", 1, 20)
    ],
    indirect=True,
)
def test_zmq_reconnect(session_simulator, request):
    """ Test changing zmq ports and zmq hwm with zmq sockets connected (reconnect). """
    det_type, num_interfaces, num_mods, d = session_simulator
    assert d is not None
    assert d.type == detectorType.EIGER


    d.rx_zmqstream = True
    assert d.rx_zmqstream == True

    port = 14000
    hwm = 2

    for _ in range(10):
        d.rx_zmqport = port
        d.rx_zmqhwm = hwm
        Log(LogLevel.INFOGREEN, f"Set zmqport={port}, zmqhwm={hwm}")

        port += 1000
        hwm += 5
        time.sleep(1)


    Log(LogLevel.INFOGREEN, f"✅ {request.node.name} passed")


