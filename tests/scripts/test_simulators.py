# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
'''
This file is used to start up simulators, receivers and run all the tests on them and finally kill the simulators and receivers.

It can be used to run all catch tests with tag [.cmdcall]. 

Pass --tests <testname> to run specific tests only or --tesst <testtag> to run all tests with that specific tag.

Pass --servers <server1> <server2> ... to run tests only for specific detector servers.
'''
import argparse
import sys, subprocess, time, traceback
from contextlib import contextmanager

from slsdet import Detector
from slsdet.defines import DEFAULT_TCP_RX_PORTNO


from utils_for_test import (
    Log,
    LogLevel,
    RuntimeException,
    cleanup,
    runProcess,
    startReceiver,
    runProcessWithLogFile,
    runProcess, 
    startDetectorVirtualServer,
    loadConfig,
    loadBasicSettings,
    ParseArguments, 
    build_dir,
    optional_file
)

LOG_PREFIX_FNAME = '/tmp/slsDetectorPackage_virtual_test'
MAIN_LOG_FNAME = LOG_PREFIX_FNAME + '_log.txt'
GENERAL_TESTS_LOG_FNAME = LOG_PREFIX_FNAME + '_results_general.txt'

def startGeneralTests(fp):
    fname = GENERAL_TESTS_LOG_FNAME
    cmd = [str(build_dir / 'tests'), '--abort', '-s']
    try:
        cleanup(fp)
        runProcessWithLogFile('General Tests', cmd, fp, fname)
    except Exception as e:
        raise RuntimeException(f'General tests failed.') from e

def startTestsForAll(args, fp, advanced_test_settings=None):

    fname_template = LOG_PREFIX_FNAME + "_{}_{}.txt"

    
    test_filter = args.tests
    if args.disable_xilinx_ctb:
        test_filter += " ~[disable_xilinx_ctb]"
    if args.disable_jungfrau: 
        test_filter += " ~[disable_jungfrau]"

    cmd = [str(build_dir / 'tests'), '--abort', test_filter, '-s'] 

    for server in args.servers:
        for ninterfaces in range(1, 2): # always test both
            if ninterfaces == 2 and server != 'jungfrau' and server != 'moench':
                continue
            try:
                fname = fname_template.format(args.tests, server) if not args.no_log_file else None

                
        
                Log(LogLevel.INFOBLUE, f'Starting {args.tests} Tests for {server}')
                cleanup(fp)
                startDetectorVirtualServer(name=server, num_mods=args.num_mods, fp=fp)
                startReceiver(args.num_mods, fp)
                d = loadConfig(name=server, rx_hostname=args.rx_hostname, settingsdir=args.settingspath, log_file_fp=fp, num_mods=args.num_mods, num_interfaces=ninterfaces)
                loadBasicSettings(name=server, d=d, fp=fp)
                if advanced_test_settings is not None:
                    advanced_test_settings(name=server, detector=d, log_file_fp=fp) # special settings for specific tests 
        
                if args.no_log_file:
                    runProcess('Tests (' + args.tests + ') for ' + server, cmd, fp)
                else:
                    runProcessWithLogFile('Tests (' + args.tests + ') for ' + server, cmd, fp, fname)
            except Exception as e:
                raise RuntimeException(f'Tests (' + args.tests + ') failed for ' + server + '.') from e

    Log(LogLevel.INFOGREEN, 'Passed all tests for all detectors \n' + str(args.servers))


if __name__ == '__main__':
    args = ParseArguments(description='Automated tests with the virtual detector servers', default_num_mods=2, specific_tests=True, general_tests_option=True)
    if args.num_mods > 2:
        raise RuntimeException(f'Cannot support multiple modules at the moment (except Eiger).')
    
    with optional_file(MAIN_LOG_FNAME if not args.no_log_file else None, 'w') as fp:  
        try:
            if args.general_tests:
                startGeneralTests(fp)
            startTestsForAll(args, fp)
            cleanup(fp)
        except Exception as e:
            traceback.print_exc(file=fp)
            cleanup(fp)
            Log(LogLevel.ERROR, f'Tests Failed.')
            raise e
            
