# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
'''
This file is used to start up simulators and test for freeing shm and accessing it from python.
Run this using: pytest -s test_free.py
'''

import pytest, sys

from slsdet import Detector, Ctb
from utils_for_test import (
    Log,
    LogLevel,
    cleanup,
    startDetectorVirtualServer,
    connectToVirtualServers,
)


def test_exptime_after_free_should_raise():
    Log(LogLevel.INFO, f'Running Free Tests')

    fp = sys.stdout
    name = 'ctb'
    num_mods = 1

    cleanup(fp)
    startDetectorVirtualServer(name, num_mods, fp)

    connectToVirtualServers(name, num_mods, True)
    d = Ctb()
    d.exptime = 1
    connectToVirtualServers(name, num_mods, True)

    # here d still points to the old shared memory object and should throw
    with pytest.raises(Exception) as exc_info:
        _ = d.exptime

    # Print out the exception for confirmation
    Log(LogLevel.INFOGREEN, f"✅ Test passed, exception was: {exc_info.value}")

    assert str(exc_info.value) == "Shared memory is invalid or freed. Close resources before access."

    cleanup(fp)





