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
from slsdet import Detector, hdrParameters

from slsdet._slsdet import slsDetectorDefs

detectorType = slsDetectorDefs.detectorType
operationMode = slsDetectorDefs.operationMode

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


def test_hdrParameters():
    """ Test hdrParameters struct."""
    # default is disabled
    p = hdrParameters()
    assert p.enable == 0
    assert p.opMode == operationMode.SYNCHROTRON
    assert repr(p) == "[disabled]"

    # constructing with operation mode enables it
    p = hdrParameters(operationMode.SYNCHROTRON)
    assert p.enable == 1
    assert p.opMode == operationMode.SYNCHROTRON
    assert repr(p) == "[enabled synchrotron]"

    p = hdrParameters(operationMode.FEL)
    assert p.enable == 1
    assert p.opMode == operationMode.FEL
    assert repr(p) == "[enabled fel]"

    # read write
    p.enable = 0
    assert p.enable == 0
    assert p != hdrParameters(operationMode.FEL)
    p.enable = 1
    p.opMode = operationMode.SYNCHROTRON
    assert p == hdrParameters(operationMode.SYNCHROTRON)
    assert p != hdrParameters()

    # invalid arguments
    with pytest.raises(TypeError):
        hdrParameters(1)
    with pytest.raises(TypeError):
        hdrParameters(operationMode.FEL, 1)

    Log(LogLevel.INFOGREEN, f"✅ Test passed. Struct: hdrParameters")


@pytest.mark.detectorintegration
def test_hdr(session_simulator):
    """ Test hdr command."""
    det_type, num_interfaces, num_mods, d = session_simulator
    assert d is not None

    if d.type == detectorType.JUNGFRAU:
        prev_val = d.getHDR()

        # invalid arguments (only hdrParameters accepted)
        with pytest.raises(TypeError):
            d.hdr = 0
        with pytest.raises(TypeError):
            d.hdr = operationMode.SYNCHROTRON

        # TODO: test fel once chip version can be checked (only for chip v1.2 HDR)
        d.hdr = hdrParameters(operationMode.SYNCHROTRON)
        assert d.hdr == hdrParameters(operationMode.SYNCHROTRON)
        assert repr(d.hdr) == "[enabled synchrotron]"

        d.hdr = hdrParameters()
        assert d.hdr == hdrParameters()
        assert repr(d.hdr) == "[disabled]"

        # per module
        d.hdr = {0: hdrParameters(operationMode.SYNCHROTRON)}
        assert d.getHDR([0])[0] == hdrParameters(operationMode.SYNCHROTRON)
        if num_mods > 1:
            assert d.getHDR([1])[0] == hdrParameters()
            assert d.hdr == [hdrParameters(operationMode.SYNCHROTRON), hdrParameters()]

        for i in range(len(d)):
            d.setHDR(prev_val[i], [i])
    else:
        with pytest.raises(Exception):
            d.hdr
        with pytest.raises(Exception):
            d.hdr = hdrParameters()

    Log(LogLevel.INFOGREEN, f"✅ Test passed. Command: hdr")
