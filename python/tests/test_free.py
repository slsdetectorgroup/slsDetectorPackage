# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
'''
This file is used to start up simulators and test for freeing shm and accessing it from python.
Run this using: pytest -s test_free.py
'''

import pytest, sys

from pathlib import Path

current_dir = Path(__file__).resolve().parents[2]

scripts_dir = current_dir / "tests" / "scripts"

sys.path.append(str(scripts_dir))


from slsdet import Detector, Ctb, freeSharedMemory

from utils_for_test import (
    Log,
    LogLevel,
    SERVER_START_PORTNO
)

from conftest import session_simulator

@pytest.mark.detectorintegration
@pytest.mark.parametrize("session_simulator",[("ctb", 1, 1)],indirect=True)
def test_exptime_after_free_should_raise(session_simulator):
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

@pytest.mark.detectorintegration
@pytest.mark.parametrize("session_simulator",[("ctb", 1, 1)],indirect=True)
def test_exptime_after_not_passing_var_should_raise(session_simulator):
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

@pytest.mark.detectorintegration
@pytest.mark.parametrize("session_simulator",[("ctb", 1, 1)],indirect=True)
def test_exptime_after_passing_ctb_var_should_raise(session_simulator):
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

@pytest.mark.detectorintegration
@pytest.mark.parametrize("session_simulator",[("ctb", 1, 1)],indirect=True)
def test_exptime_after_returning_ctb_should_raise(session_simulator):
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

@pytest.mark.detectorintegration
@pytest.mark.parametrize("session_simulator",[("ctb", 1, 1)],indirect=True)
def test_hostname_twice_acess_old_should_raise(session_simulator):
    Log(LogLevel.INFOBLUE, f'\nRunning test_hostname_twice_acess_old_should_raise')

    d = Ctb() # creates multi shm (assuming no shm exists)
    d.hostname = f"localhost:{SERVER_START_PORTNO}" # hostname command creates mod shm, d maps to it
    d.hostname = f"localhost:{SERVER_START_PORTNO}"  # Freeing and recreating shm while mapping d to it (old shm is out of scope)
    
    # this should not throw 
    exptime_val = d.exptime

    Log(LogLevel.INFOGREEN, f"✅ Test passed, exptime was: {exptime_val}")
    assert isinstance(exptime_val, float)




