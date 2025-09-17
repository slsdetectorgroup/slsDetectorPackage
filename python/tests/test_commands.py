# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
'''
This file is used to start up simulators and test for freeing shm and accessing it from python.
Run this using: pytest -s test_free.py
'''

from time import time
import pytest, sys, time

from slsdet import Detector, Ctb, freeSharedMemory
from slsdet.defines import DEFAULT_TCP_RX_PORTNO
from utils_for_test import (
    Log,
    LogLevel,
    cleanup,
    startDetectorVirtualServer,
    startProcessInBackground,
    loadConfig,
    loadBasicSettings
)

def startReceiver(num_mods, fp):
    if num_mods == 1:
        cmd = ['slsReceiver']
    else:
        cmd = ['slsMultiReceiver', str(DEFAULT_TCP_RX_PORTNO), str(num_mods)]
        # in 10.0.0
        #cmd = ['slsMultiReceiver', '-p', str(DEFAULT_TCP_RX_PORTNO), '-n', str(num_mods)]
    startProcessInBackground(cmd, fp)
    time.sleep(1)

'''
scope = module =>Once per test file/module 
to share expensive setup like startDetectorVirtualServer
'''
@pytest.fixture(scope="module")
def det_config():
    return {
        "name": "ctb",
        "num_mods": 1
    }

# autouse is false to pass explictly
@pytest.fixture(scope="module", autouse=False)
def setup_simulator(det_config):
    """Fixture to start the detector server once and clean up at the end."""
    fp = sys.stdout

    cleanup(fp)
    # server
    startDetectorVirtualServer(det_config["name"], det_config["num_mods"], fp)
    # receiver
    startReceiver(det_config["num_mods"], fp)
    # config and basic settings
    d = loadConfig(name=det_config["name"], rx_hostname="localhost", settingsdir="", fp=fp, num_mods=det_config["num_mods"])
    loadBasicSettings(name=det_config["name"], d=d, fp=fp)

    yield d # tests run here

    cleanup(fp)



def test_parameters_file(setup_simulator):
    d = setup_simulator
    Log(LogLevel.INFOBLUE, f'\nRunning test_parameters_file')
    
    assert isinstance(d, Detector)

    with open("/tmp/params.det", "w") as f:
        f.write("frames 2\n")
        f.write("fwrite 1\n")

    # this should not throw
    d.parameters = "/tmp/params.det"

    assert d.frames == 2    
    assert d.fwrite == 1

    Log(LogLevel.INFOGREEN, f"✅ Test passed. Command: parameters")


def test_include_file(setup_simulator):
    d = setup_simulator
    Log(LogLevel.INFOBLUE, f'\test_include_file test_parameters_file')
    
    assert isinstance(d, Detector)

    with open("/tmp/params.det", "w") as f:
        f.write("frames 3\n")
        f.write("fwrite 0\n")

    # this should not throw
    d.include = "/tmp/params.det"

    assert d.frames == 3    
    assert d.fwrite == 0

    Log(LogLevel.INFOGREEN, f"✅ Test passed. Command: include")
