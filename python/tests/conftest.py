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
            

