import pytest
import subprocess
import os
import sys
import time
import datetime as dt

sys.path.append(os.path.join(os.getcwd(), "bin"))
from sls_detector import ExperimentalDetector, detectorSettings

n_detectors = 3
start_port = 1952
port_step = 3


@pytest.fixture(scope="module")
def virtual_jf_detectors(request):
    """
    Fixture that is run once for the module
    will launch virtual servers and clean up
    after
    """
    print("Setting up virtual detectors")

    # Ensure that no detector servers are running
    subprocess.run(["killall", "jungfrauDetectorServer_virtual"])

    # Ensure no shared memory exists before tests start
    d = ExperimentalDetector()
    d.free()

    # Start servers
    virtual_jf_detectors = [
        subprocess.Popen(
            [
                "bin/jungfrauDetectorServer_virtual",
                "--port",
                f"{start_port+port_step*i}",
            ]
        )
        for i in range(n_detectors)
    ]

    # Allow server startup to complete
    time.sleep(3)

    def fin():
        print("Cleaning up virtual detectors")
        d = ExperimentalDetector()
        d.free()
        subprocess.run(["killall", "jungfrauDetectorServer_virtual"])

    request.addfinalizer(fin)
    return virtual_jf_detectors  # provide the fixture value


def test_shmid(virtual_jf_detectors):
    d = ExperimentalDetector()
    assert d.getShmId() == 0
    d.free()

    d = ExperimentalDetector(73)
    assert d.getShmId() == 73
    d.free()


def test_hostname(virtual_jf_detectors):
    d = ExperimentalDetector()
    d.hostname = "localhost"
    assert d.hostname == ["localhost"]

    d.hostname = [f"localhost:{start_port+i*port_step}" for i in range(n_detectors)]
    assert d.hostname == ["localhost"] * n_detectors


def test_fwversion(virtual_jf_detectors):
    d = ExperimentalDetector()
    assert d.detectorversion == 0  # Firmware of virtual detector
    assert d.getFirmwareVersion() == [0] * n_detectors


def test_len(virtual_jf_detectors):
    d = ExperimentalDetector()
    assert len(d) == n_detectors
    assert d.size() == n_detectors


def test_module_geometry(virtual_jf_detectors):
    d = ExperimentalDetector()
    geo = d.module_geometry
    assert geo.x == 1
    assert geo.y == 3


def test_module_size(virtual_jf_detectors):
    d = ExperimentalDetector()
    geo = d.module_size
    assert geo.x == 1024
    assert geo.y == 512


def test_settings(virtual_jf_detectors):
    d = ExperimentalDetector()
    assert d.settings == detectorSettings.DYNAMICGAIN

    gain_list = [
        detectorSettings.FIXGAIN1,
        detectorSettings.FIXGAIN2,
        detectorSettings.FORCESWITCHG1,
        detectorSettings.FORCESWITCHG2,
        detectorSettings.DYNAMICHG0,
        detectorSettings.DYNAMICGAIN,
    ]

    # Set all viable gain for Jungfrau to make sure nothing is crashing
    for gain in gain_list:
        d.settings = gain
        assert d.settings == gain

    d.setSettings(detectorSettings.FORCESWITCHG1, [1])
    assert d.settings == [
        detectorSettings.DYNAMICGAIN,
        detectorSettings.FORCESWITCHG1,
        detectorSettings.DYNAMICGAIN,
    ]

    d.settings = detectorSettings.DYNAMICGAIN
    assert d.settings == detectorSettings.DYNAMICGAIN

def test_frames(virtual_jf_detectors):
    d = ExperimentalDetector()
    d.frames = 10
    assert d.frames == 10

# def test_triggers(virtual_jf_detectors):

def test_exptime(virtual_jf_detectors):
    d = ExperimentalDetector()

    #default value
    assert d.exptime == 1e-5

    d.exptime = 1.5
    assert d.exptime == 1.5

    t = dt.timedelta(microseconds=10)
    d.exptime  = t
    assert d.exptime == 10e-6

def test_period(virtual_jf_detectors):
    d = ExperimentalDetector()

    #default value
    d.period = 0
    assert d.period == 0

    d.period = 1.5
    assert d.period == 1.5

    t = dt.timedelta(microseconds=10)
    d.period  = t
    assert d.period == 10e-6

    