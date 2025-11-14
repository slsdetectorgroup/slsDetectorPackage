# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package

'''
This file is used to start up simulators, receivers and test python API, tests are less exhaustive than C++ tests.
'''

import traceback

from utils_for_test import (
    Log,
    LogLevel,
    RuntimeException,
    cleanup,
    startProcessInBackground,
    startReceiver,
    startDetectorVirtualServer,
    connectToVirtualServers,
    loadConfig, 
    loadBasicSettings,
    runProcessWithLogFile
)

LOG_PREFIX_FNAME = '/tmp/slsDetectorPackage_virtual_PythonAPI_test'
MAIN_LOG_FNAME = LOG_PREFIX_FNAME + '_log.txt'
PYTHONAPI_TEST_FNAME = LOG_PREFIX_FNAME + '_results_'

def startTests(fp):
    servers = [
        'moench',
    ]
    nmods = 2
    for server in servers:
        for ninterfaces in range(1,2):
            if ninterfaces == 2 and server != 'jungfrau' and server != 'moench':
                continue
            try:
                msg = f'Starting Python API Tests for {server}'
                if server == 'jungfrau' or server == 'moench':
                    msg += f' with {ninterfaces} interfaces'
                Log(LogLevel.INFOBLUE, msg)
                Log(LogLevel.INFOBLUE, msg, fp)
                cleanup(fp)
                startDetectorVirtualServer(server, nmods, fp)
                startReceiver(nmods, fp)
                d = loadConfig(name=server, log_file_fp=fp, num_mods=nmods, num_frames=1, num_interfaces=ninterfaces)
                loadBasicSettings(name=server, d=d, fp=fp)

                fname = PYTHONAPI_TEST_FNAME + server + '.txt'
                cmd = ['python', '-m', 'pytest', '../python/tests/test_pythonAPI.py']
                runProcessWithLogFile('Python API Tests for ' + server, cmd, fp, fname)
                Log(LogLevel.INFO, '\n')
            except Exception as e:
                raise RuntimeException(f'Python API Tests failed') from e

    Log(LogLevel.INFOGREEN, 'Passed all Python API tests for server ' + str(servers) + '\n')
  

if __name__ == '__main__':
    Log(LogLevel.INFOBLUE, '\nLog File: ' + MAIN_LOG_FNAME + '\n') 

    with open(MAIN_LOG_FNAME, 'w') as fp:
        try:
            startTests(fp)
            cleanup(fp)
        except Exception as e:
            with open(MAIN_LOG_FNAME, 'a') as fp_error:
                traceback.print_exc(file=fp_error)
            cleanup(fp)
            Log(LogLevel.ERROR, f'Tests Failed.')