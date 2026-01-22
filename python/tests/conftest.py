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

@pytest.fixture(scope="session")
def session_simulator(request):
    """
    Fixture to start the detector server once and clean up at the end.
    Expects request.param = (det_type, num_interfaces, num_mods)
    """
    det_type, num_interfaces, num_mods = request.param
    fp = sys.stdout

    # set up: once per server
    Log(LogLevel.INFOBLUE, 
        f'---- {det_type} | interfaces={num_interfaces} | modules={num_mods} ----', fp)
    
    cleanup(fp)
    startDetectorVirtualServer(det_type, num_mods, fp)
    startReceiver(num_mods, fp)

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

#helper fixture for servers
@pytest.fixture(scope='module') 
def setup_parameters(request): # only setup once per module if same parameters used for the scopes
    try:
        servers, nmods = request.param  # comes from @pytest.mark.parametrize(..., indirect=True)
        return servers, nmods
    except AttributeError:
        # fallback default if the test did not parametrize
        return (['eiger', 'jungfrau', 'mythen3', 'gotthard2', 'ctb', 'moench', 'xilinx_ctb'], 2)
    
@pytest.fixture(scope='module')
def test_with_simulators(setup_parameters):
    """ Fixture to automatically setup virtual detector servers for testing. """

    fp = sys.stdout 

    servers, nmods = setup_parameters
    print("servers:", servers)
    print("nmods:", nmods)
    try:
        for server in servers:
            for ninterfaces in range(1,2):
                if ninterfaces == 2 and server != 'jungfrau' and server != 'moench':
                    continue
                        
                msg = f'Starting Python API Tests for {server}'

                if server == 'jungfrau' or server == 'moench':
                    msg += f' with {ninterfaces} interfaces'

                Log(LogLevel.INFOBLUE, msg, fp)
                cleanup(fp)
                startDetectorVirtualServer(server, nmods, fp)
                startReceiver(nmods, fp)
                d = loadConfig(name=server, log_file_fp=fp, num_mods=nmods, num_frames=1, num_interfaces=ninterfaces)
                #loadBasicSettings(name=server, d=d, fp=fp)
                yield # run test 
        cleanup(fp) # teardown 
    except Exception as e:
        traceback.print_exc(file=fp)
        Log(LogLevel.ERROR, f'Tests Failed.', fp)
        cleanup(fp)
            

