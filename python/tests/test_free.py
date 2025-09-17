# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
'''
This file is used to start up simulators and test for freeing shm and accessing it from python.
Run this using: pytest -s test_free.py
'''

import pytest, sys

from slsdet import Detector, Ctb, freeSharedMemory
from utils_for_test import (
    Log,
    LogLevel,
    cleanup,
    startDetectorVirtualServer,
    connectToVirtualServers,
    SERVER_START_PORTNO
)

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

@pytest.fixture(scope="module", autouse=True)
def setup_simulator(det_config):
    """Fixture to start the detector server once and clean up at the end."""
    fp = sys.stdout

    cleanup(fp)
    startDetectorVirtualServer(det_config["name"], det_config["num_mods"], fp)

    Log(LogLevel.INFOBLUE, f'Waiting for server to start up and connect')
    connectToVirtualServers(det_config["name"], det_config["num_mods"])
    Log(LogLevel.INFOBLUE, f'Freeing shm before tests')
    freeSharedMemory()

    yield  # tests run here

    cleanup(fp)



def test_exptime_after_free_should_raise(setup_simulator):
    Log(LogLevel.INFOBLUE, f'\nRunning test_exptime_after_free_should_raise')


    d = Ctb() # creates multi shm (assuming no shm exists)
    d.hostname = f"localhost:{SERVER_START_PORTNO}" # hostname command creates mod shm, d maps to it

    d.free() # frees the shm, d should not map to it anymore

    # accessing invalid shm should throw
    with pytest.raises(Exception) as exc_info:
        _ = d.exptime

    Log(LogLevel.INFOGREEN, f"✅ Test passed, exception was: {exc_info.value}")
    assert str(exc_info.value) == "Shared memory is invalid or freed. Close resources before access."





def free_and_create_shm():
    k = Ctb() # opens existing shm if it exists
    k.hostname = f"localhost:{SERVER_START_PORTNO}" # free and recreate shm, maps to local shm struct


def test_exptime_after_not_passing_var_should_raise(setup_simulator):
    Log(LogLevel.INFOBLUE, f'\nRunning test_exptime_after_not_passing_var_should_raise')


    d = Ctb() # creates multi shm (assuming no shm exists)
    d.hostname = f"localhost:{SERVER_START_PORTNO}" # hostname command creates mod shm, d maps to it

    free_and_create_shm() # ctb() opens multi shm, hostname command frees and recreates mod shm but shm struct is local. d still maps to old shm struct

    # accessing invalid shm should throw
    with pytest.raises(Exception) as exc_info:
        _ = d.exptime

    Log(LogLevel.INFOGREEN, f"✅ Test passed, exception was: {exc_info.value}")
    assert str(exc_info.value) == "Shared memory is invalid or freed. Close resources before access."




def free_and_create_shm_passing_ctb_var(k):
    k = Ctb() # opens existing shm if it exists (disregards k as its new Ctb only local to this function)
    k.hostname = f"localhost:{SERVER_START_PORTNO}" # free and recreate shm, maps to local shm struct


def test_exptime_after_passing_ctb_var_should_raise(setup_simulator):
    Log(LogLevel.INFOBLUE, f'\nRunning test_exptime_after_passing_ctb_var_should_raise')

    d = Ctb() # creates multi shm (assuming no shm exists)
    d.hostname = f"localhost:{SERVER_START_PORTNO}" # hostname command creates mod shm, d maps to it

    free_and_create_shm_passing_ctb_var(d) # ctb() opens multi shm, hostname command frees and recreates mod shm but shm struct is local. d still maps to old shm struct

    # accessing invalid shm should throw
    with pytest.raises(Exception) as exc_info:
        _ = d.exptime

    Log(LogLevel.INFOGREEN, f"✅ Test passed, exception was: {exc_info.value}")
    assert str(exc_info.value) == "Shared memory is invalid or freed. Close resources before access."



def free_and_create_shm_returning_ctb():
    k = Ctb() # opens existing shm if it exists (disregards k as its new Ctb only local to this function)
    k.hostname = f"localhost:{SERVER_START_PORTNO}" # free and recreate shm, maps to local shm struct
    return k


def test_exptime_after_returning_ctb_should_raise(setup_simulator):
    Log(LogLevel.INFOBLUE, f'\nRunning test_exptime_after_returning_ctb_should_raise')

    d = Ctb() # creates multi shm (assuming no shm exists)

    d = free_and_create_shm_returning_ctb() # ctb() opens multi shm, hostname command frees and recreates mod shm but shm struct is local but returned. d now maps to the new sturct

    # this should not throw 
    exptime_val = d.exptime

    Log(LogLevel.INFOGREEN, f"✅ Test passed, exptime was: {exptime_val}")
    assert isinstance(exptime_val, float)

    free_and_create_shm_returning_ctb() # this time d is not updated, it maps to the old shm struct

    # accessing invalid shm should throw
    with pytest.raises(Exception) as exc_info:
        _ = d.exptime

    Log(LogLevel.INFOGREEN, f"✅ Test passed, exception was: {exc_info.value}")
    assert str(exc_info.value) == "Shared memory is invalid or freed. Close resources before access."






def test_hostname_twice_acess_old_should_raise(setup_simulator):
    Log(LogLevel.INFOBLUE, f'\nRunning test_hostname_twice_acess_old_should_raise')

    d = Ctb() # creates multi shm (assuming no shm exists)
    d.hostname = f"localhost:{SERVER_START_PORTNO}" # hostname command creates mod shm, d maps to it
    d.hostname = f"localhost:{SERVER_START_PORTNO}"  # Freeing and recreating shm while mapping d to it (old shm is out of scope)
    
    # this should not throw 
    exptime_val = d.exptime

    Log(LogLevel.INFOGREEN, f"✅ Test passed, exptime was: {exptime_val}")
    assert isinstance(exptime_val, float)




