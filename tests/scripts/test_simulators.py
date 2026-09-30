# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
'''
This file is used to start up simulators, receivers and run all the tests on them and finally kill the simulators and receivers.

It can be used to run all catch tests with tag [.detectorintegration]. 

Pass --tests <testname> to run specific tests only or --tests <testtag> to run all tests with that specific tag.

Pass --servers <server1> <server2> ... to run tests only for specific detector servers.
'''
import argparse
import sys, subprocess, time, traceback
from contextlib import contextmanager

from slsdet import Detector


from utils_for_test import (
    Log,
    LogLevel,
    RuntimeException,
    cleanup,
    runProcess,
    startReceiver,
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
        runProcess('General Tests', cmd, fp, fname)
    except Exception as e:
        raise RuntimeException(f'General tests failed.') from e

def startTestsForAll(args, fp):

    fname_template = LOG_PREFIX_FNAME + "_{}_{}.txt"

    
    test_filter = args.tests
    cmd = [str(build_dir / 'tests'), '--abort', test_filter, '-s'] 

    num_mods = args.num_mods 

    for server in args.servers:
        for curMods in range(1, num_mods + 1):
            if curMods == 2 and server in ['ctb', 'xilinx_ctb']:
                continue
            for ninterfaces in [1,2]: # always test both
                if ninterfaces == 2 and server != 'jungfrau' and server != 'moench':
                    continue

                if server == "eiger": 
                    curMods *= 2 # top and bottom half module 
                try:
                    fname = fname_template.format(args.tests, server) if not args.no_log_file else None

                    Log(LogLevel.INFOBLUE, f'Starting {args.tests} Tests for {server}, {ninterfaces} interfaces, {curMods} modules', fp, True)
                    cleanup(fp)
                    startDetectorVirtualServer(name=server, num_mods=curMods, fp=fp, no_log_file=args.no_log_file, quiet_mode=args.quiet)
                    startReceiver(curMods, fp, args.no_log_file, args.quiet)
                    d = loadConfig(name=server, rx_hostname=args.rx_hostname, settingsdir=args.settingspath, log_file_fp=fp, num_mods=curMods, num_interfaces=ninterfaces)
                    loadBasicSettings(name=server, d=d, fp=fp)
                    runProcess('Tests (' + args.tests + ') for ' + server, cmd, fp, fname, args.quiet)
                except Exception as e:
                    raise RuntimeException(f'Tests (' + args.tests + ') failed for ' + server + '.') from e

    Log(LogLevel.INFOGREEN, 'Passed all tests for all detectors \n' + str(args.servers))


if __name__ == '__main__':
    args = ParseArguments(description='Automated tests with the virtual detector servers', default_num_mods=2, specific_tests=True, general_tests_option=True)

    with optional_file(MAIN_LOG_FNAME if not args.no_log_file else None, 'w', args.quiet) as fp:  
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
            
