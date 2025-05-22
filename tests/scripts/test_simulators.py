# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
'''
This file is used to start up simulators, receivers and run all the tests on them and finally kill the simulators and receivers.
'''
import argparse
import sys, subprocess, time, traceback

from slsdet import Detector
from slsdet.defines import DEFAULT_TCP_RX_PORTNO

from utils_for_test import (
    Log,
    LogLevel,
    RuntimeException,
    checkIfProcessRunning,
    killProcess,
    cleanup,
    cleanSharedmemory,
    startProcessInBackground,
    runProcessWithLogFile,
    startDetectorVirtualServer,
    loadConfig,
    ParseArguments
)


LOG_PREFIX_FNAME = '/tmp/slsDetectorPackage_virtual_test'
MAIN_LOG_FNAME = LOG_PREFIX_FNAME + '_log.txt'
GENERAL_TESTS_LOG_FNAME = LOG_PREFIX_FNAME + '_results_general.txt'
CMD_TEST_LOG_PREFIX_FNAME = LOG_PREFIX_FNAME + '_results_cmd_'


def startReceiver(num_mods, fp):
    if num_mods == 1:
        cmd = ['slsReceiver']
    else:
        cmd = ['slsMultiReceiver', str(DEFAULT_TCP_RX_PORTNO), str(num_mods)]
        # in 10.0.0
        #cmd = ['slsMultiReceiver', '-p', str(DEFAULT_TCP_RX_PORTNO), '-n', str(num_mods)]
    startProcessInBackground(cmd, fp)
    time.sleep(1)

def startGeneralTests(fp):
    fname = GENERAL_TESTS_LOG_FNAME
    cmd = ['tests', '--abort', '-s']
    try:
        cleanup(fp)
        runProcessWithLogFile('General Tests', cmd, fp, fname)
    except Exception as e:
        raise RuntimeException(f'General tests failed.') from e


def startCmdTestsForAll(args, fp):
    for server in args.servers:
        try:
            num_mods = 2 if server == 'eiger' else 1
            fname = CMD_TEST_LOG_PREFIX_FNAME + server + '.txt'
            cmd = ['tests', '--abort', '[.cmdcall]', '-s']

            Log(LogLevel.INFOBLUE, f'Starting Cmd Tests for {server}')
            cleanup(fp)
            startDetectorVirtualServer(name=server, num_mods=num_mods, fp=fp)
            startReceiver(num_mods, fp)
            loadConfig(name=server, rx_hostname=args.rx_hostname, settingsdir=args.settingspath, fp=fp, num_mods=num_mods)
            runProcessWithLogFile('Cmd Tests for ' + server, cmd, fp, fname)
        except Exception as e:
            raise RuntimeException(f'Cmd Tests failed for {server}.') from e

    Log(LogLevel.INFOGREEN, 'Passed all tests for all detectors \n' + str(args.servers))


if __name__ == '__main__':
    args = ParseArguments('Automated tests with the virtual detector servers')
    if args.num_mods > 1:
        raise RuntimeException(f'Cannot support multiple modules at the moment (except Eiger).')

    Log(LogLevel.INFOBLUE, '\nLog File: ' + MAIN_LOG_FNAME + '\n') 

    with open(MAIN_LOG_FNAME, 'w') as fp:  
        try:
            startGeneralTests(fp)
            startCmdTestsForAll(args, fp)
            cleanup(fp)
        except Exception as e:
            with open(MAIN_LOG_FNAME, 'a') as fp_error:
                traceback.print_exc(file=fp_error)
            cleanup(fp)
            Log(LogLevel.ERROR, f'Tests Failed.')
