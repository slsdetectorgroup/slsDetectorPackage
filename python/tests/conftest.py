import pytest
import sys
import traceback

from pathlib import Path

current_dir = Path(__file__).resolve().parents[2]

scripts_dir = current_dir / "tests" / "scripts"

sys.path.append(str(scripts_dir))

print(sys.path)

from utils_for_test import (
    Log,
    LogLevel,
    cleanup,
    startReceiver,
    startDetectorVirtualServer,
    loadConfig, 
    loadBasicSettings,
)

def pytest_addoption(parser):
    parser.addoption(
        "--with-detector-simulators", action="store_true", default=False, help="Run tests that require detector simulators"
    )

def pytest_configure(config):
    config.addinivalue_line("markers", "withdetectorsimulators: mark test as needing detector simulators to run")

def pytest_collection_modifyitems(config, items):
    if config.getoption("--with-detector-simulators"):
        return
    skip = pytest.mark.skip(reason="need --with-detector-simulators option to run")
    for item in items:
        if "withdetectorsimulators" in item.keywords:
            item.add_marker(skip)

@pytest.fixture
def test_with_simulators():
    """ Fixture to automatically setup virtual detector servers for testing. """

    LOG_PREFIX_FNAME = '/tmp/slsDetectorPackage_virtual_PythonAPI_test'
    MAIN_LOG_FNAME = LOG_PREFIX_FNAME + '_log.txt'

    with open(MAIN_LOG_FNAME, 'w') as fp:
        try:
            servers = ['moench']
            nmods = 2
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
                    loadBasicSettings(name=server, d=d, fp=fp)
                    yield # run test 
            cleanup(fp) # teardown 
        except Exception as e:
            with open(MAIN_LOG_FNAME, 'a') as fp_error:
                traceback.print_exc(file=fp_error)
                Log(LogLevel.ERROR, f'Tests Failed.', fp)
            cleanup(fp)
            

