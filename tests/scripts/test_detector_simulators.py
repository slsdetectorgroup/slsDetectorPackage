# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package

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
    loadBasicSettings,
    ParseArguments
)


if __name__ == '__main__':
    args = ParseArguments(description='Automated tests with the virtual detector servers', default_num_mods=1, markers=True, general_tests_option=False)
    if args.num_mods > 1:
        raise RuntimeException(f'Cannot support multiple modules at the moment (except Eiger).')

    Log(LogLevel.INFOBLUE, '\nLog File: ' + MAIN_LOG_FNAME + '\n') 

    with open(MAIN_LOG_FNAME, 'w') as fp:  
        try:
            if args.general_tests:
                startGeneralTests(fp)
            startCmdTestsForAll(args, fp)
            cleanup(fp)
        except Exception as e:
            with open(MAIN_LOG_FNAME, 'a') as fp_error:
                traceback.print_exc(file=fp_error)
            cleanup(fp)
            Log(LogLevel.ERROR, f'Tests Failed.')
