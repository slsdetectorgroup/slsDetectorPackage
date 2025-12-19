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

#helper fixture for servers
@pytest.fixture
def servers(request):
    """ Fixture to get server and num interaface from test marker. """
    try:
        return request.param  # comes from @pytest.mark.parametrize(..., indirect=True)
    except AttributeError:
        # fallback default if the test did not parametrize
        return [['eiger', 1], ['jungfrau', 1], ['jungfrau', 2], ['mythen3',1], ['gotthard2',1], ['ctb',1], ['moench',1], ['moench',2],['xilinx_ctb',1]]
    return request.param  

@pytest.fixture() 
def test_with_specific_simulator(servers):
    """ Fixture to automatically setup virtual detector servers for testing. """

    LOG_PREFIX_FNAME = '/tmp/slsDetectorPackage_virtual_PythonAPI_test'
    MAIN_LOG_FNAME = LOG_PREFIX_FNAME + '_log.txt'

    with open(MAIN_LOG_FNAME, 'w') as fp:
        try:
            nmods = 2
            server, ninterface = servers 
            msg = f'Starting Python API Tests for {server}'

            if server == 'jungfrau' or server == 'moench':
                msg += f' with {ninterface} interfaces'

            Log(LogLevel.INFOBLUE, msg, fp)
            cleanup(fp)
            startDetectorVirtualServer(server, nmods, fp)
            startReceiver(nmods, fp)
            d = loadConfig(name=server, log_file_fp=fp, num_mods=nmods, num_frames=1, num_interfaces=ninterface)
            loadBasicSettings(name=server, d=d, fp=fp)
            yield # run test 
            cleanup(fp) # teardown 
        except Exception as e:
            with open(MAIN_LOG_FNAME, 'a') as fp_error:
                traceback.print_exc(file=fp_error)
                Log(LogLevel.ERROR, f'Tests Failed.', fp)
            cleanup(fp)

@pytest.fixture(scope="module", params=[['eiger', 1], ['jungfrau', 1], ['jungfrau', 2], ['mythen3',1], ['gotthard2',1], ['ctb',1], ['moench',1], ['moench',2],['xilinx_ctb',1]])
def test_with_simulators(request):
    """ Fixture to automatically setup virtual detector servers for testing. """

    LOG_PREFIX_FNAME = '/tmp/slsDetectorPackage_virtual_PythonAPI_test'
    MAIN_LOG_FNAME = LOG_PREFIX_FNAME + '_log.txt'

    server, ninterfaces = request.param

    with open(MAIN_LOG_FNAME, 'w') as fp:
        try:
            nmods = 2      
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
            

