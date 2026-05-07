import pytest
import sys
import traceback

from pathlib import Path

current_dir = Path(__file__).resolve().parents[2]

scripts_dir = current_dir / "tests" / "scripts"

sys.path.append(str(scripts_dir))

from utils_for_test import (
    Log,
    LogLevel,
    cleanup,
    startReceiver,
    startDetectorVirtualServer,
    loadConfig, 
    loadBasicSettings,
    connectToVirtualServers,
    DEFAULT_UDP_DST_PORTNO,
)

def pytest_addoption(parser):
    parser.addoption(
        "--detector-integration", action="store_true", default=False, help="Run tests that require detectors"
    )

def pytest_configure(config):
    config.addinivalue_line("markers", "detectorintegration: mark test as needing detectors to run")

def pytest_collection_modifyitems(config, items):
    if config.getoption("--detector-integration"):
        return
    skip = pytest.mark.skip(reason="need --detector-integration option to run")
    for item in items:
        if "detectorintegration" in item.keywords:
            item.add_marker(skip)


DEFAULT_SIMULATOR_CONFIGS = [
    ("eiger", 1, 1),
    ("eiger", 1, 2),
    ("jungfrau", 1, 1),
    ("jungfrau", 1, 2),
    ("jungfrau", 2, 1),
    ("jungfrau", 2, 2),
    ("mythen3", 1, 1),
    ("mythen3", 1, 2),
    ("gotthard2", 1, 1),
    ("gotthard2", 1, 2),
    ("moench", 1, 1),
    ("moench", 1, 2),
    ("ctb", 1, 1),
    ("xilinx_ctb", 1, 1),
]

SIMULATOR_IDS = [f"{det_type}_{num_interface}if_{num_mod}mod" for det_type, num_interface, num_mod in DEFAULT_SIMULATOR_CONFIGS]

'''
for more specific parameters
@pytest.mark.detectorintegration
@pytest.mark.parametrize(
    "session_simulator",
    [
        ("ctb", 1, 1),
        ("xilinx_ctb", 1, 1),
    ],
    indirect=True,
)
def test_define_reg(session_simulator):
    det_type, num_interfaces, num_mods, d = session_simulator
'''
@pytest.fixture(scope="session")
def session_simulator(request):
    """
    Fixture to start the detector server once and clean up at the end.
    Expects request.param = (det_type, num_interfaces, num_mods)
    """
    try: 
        det_type, num_interfaces, num_mods = request.param
        fp = sys.stdout

        # set up: once per server
        Log(LogLevel.INFOBLUE, 
            f'---- {det_type} | interfaces={num_interfaces} | modules={num_mods} ----', fp)
    
        cleanup(fp)
        startDetectorVirtualServer(det_type, num_mods, fp, True, True)
        startReceiver(num_mods, fp, True)

        Log(LogLevel.INFOBLUE, f'Waiting for server to start up and connect', fp)
        d = loadConfig(
            name=det_type, 
            log_file_fp=fp, 
            num_mods=num_mods, 
            num_frames=1, 
            num_interfaces=num_interfaces,
        )

        loadBasicSettings(name=det_type, d=d, fp=fp)

        yield det_type, num_interfaces, num_mods, d

        cleanup(fp)

    except Exception as e:
        Log(LogLevel.ERROR, f'Tests Failed.', fp)
        cleanup(fp)

def pytest_generate_tests(metafunc):
    if "session_simulator" not in metafunc.fixturenames:
        return  # nothing to do

    # Check if session_simulator is already parametrized
    markers = metafunc.definition.iter_markers(name="parametrize")
    for m in markers:
        if m.args and m.args[0] == "session_simulator":
            return  # already parametrized, skip defaults

    # Apply default configs only for session_simulator
    metafunc.parametrize(
        "session_simulator",
        DEFAULT_SIMULATOR_CONFIGS,
        ids=SIMULATOR_IDS,
        indirect=True
    )


