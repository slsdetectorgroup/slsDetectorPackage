import pytest, sys, traceback

from pathlib import Path
current_dir = Path(__file__).resolve().parents[2]
scripts_dir = current_dir / "tests" / "scripts"
sys.path.append(str(scripts_dir))
print(sys.path)

from utils_for_test import (
    Log,
    LogLevel,
)
from slsdet import Detector

from slsdet._slsdet import slsDetectorDefs

detectorType = slsDetectorDefs.detectorType

@pytest.fixture(
    scope="session", 
    params=['ctb', 'xilinx_ctb', 'mythen3', 'jungfrau']
)
def simulator(request):
    """Fixture to start the detector server once and clean up at the end."""
    det_name = request.param
    num_mods = 1
    fp = sys.stdout

@pytest.mark.detectorintegration
def test_parameters_file(session_simulator, request):
    """ Test using test_parameters_file."""
    det_type, num_interfaces, num_mods, d = session_simulator
    assert d is not None

    with open("/tmp/params.det", "w") as f:
        f.write("frames 2\n")
        f.write("fwrite 1\n")

    # this should not throw
    d.parameters = "/tmp/params.det"

    assert d.frames == 2    
    assert d.fwrite == 1

    Log(LogLevel.INFOGREEN, f"✅ Test passed. Command: parameters")


@pytest.mark.detectorintegration
def test_include_file(session_simulator, request):
    """ Test using test_include_file."""
    det_type, num_interfaces, num_mods, d = session_simulator
    assert d is not None
    
    with open("/tmp/params.det", "w") as f:
        f.write("frames 3\n")
        f.write("fwrite 0\n")

    # this should not throw
    d.include = "/tmp/params.det"

    assert d.frames == 3    
    assert d.fwrite == 0

    Log(LogLevel.INFOGREEN, f"✅ Test passed. Command: include")

@pytest.mark.detectorintegration
@pytest.mark.parametrize("session_simulator",[("moench", 1, 2)],indirect=True)
def test_type(session_simulator):

    d = Detector()
    assert d.type == detectorType.MOENCH

@pytest.mark.detectorintegration
@pytest.mark.parametrize("session_simulator",[("moench", 1, 2), ("jungfrau", 1, 2)],indirect=True)
def test_numinterfaces(session_simulator):

    d = Detector()
    assert d.numinterfaces == 1

