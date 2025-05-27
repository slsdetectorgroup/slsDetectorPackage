# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
'''
This file is used to start up simulators, frame synchronizer, pull sockets, acquire, test and kill them finally.
'''

import sys, time
import traceback, json

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
    startProcessInBackgroundWithLogFile,
    checkLogForErrors,
    startDetectorVirtualServer,
    loadConfig,
    ParseArguments
)

LOG_PREFIX_FNAME = '/tmp/slsFrameSynchronizer_test'
MAIN_LOG_FNAME = LOG_PREFIX_FNAME + '_log.txt'
PULL_SOCKET_PREFIX_FNAME = LOG_PREFIX_FNAME + '_pull_socket_'


def startFrameSynchronizerPullSocket(name, fp):
    fname = PULL_SOCKET_PREFIX_FNAME + name + '.txt'
    cmd = ['python', '-u', 'frameSynchronizerPullSocket.py']  
    startProcessInBackgroundWithLogFile(cmd, fp, fname)
    time.sleep(1)
    checkLogForErrors(fp, fname)
    


def startFrameSynchronizer(num_mods, fp):
    cmd = ['slsFrameSynchronizer', str(DEFAULT_TCP_RX_PORTNO), str(num_mods)]
    # in 10.0.0
    #cmd = ['slsFrameSynchronizer', '-p', str(DEFAULT_TCP_RX_PORTNO), '-n', str(num_mods)]
    startProcessInBackground(cmd, fp)
    time.sleep(1)


def acquire(fp, det):
    Log(LogLevel.INFO, 'Acquiring')
    Log(LogLevel.INFO, 'Acquiring', fp)
    det.acquire()


def testFramesCaught(name, det, num_frames):
    fnum = det.rx_framescaught[0]
    if fnum != num_frames:
        raise RuntimeException(f"{name} caught only {fnum}. Expected {num_frames}") 
    
    Log(LogLevel.INFOGREEN, f'Frames caught test passed for {name}')
    Log(LogLevel.INFOGREEN, f'Frames caught test passed for {name}', fp)


def testZmqHeadetTypeCount(name, det, num_mods, num_frames, fp):

    Log(LogLevel.INFO, f"Testing Zmq Header type count for {name}")
    Log(LogLevel.INFO, f"Testing Zmq Header type count for {name}", fp)
    htype_counts = {
        "header": 0,
        "series_end": 0,
        "module": 0
    }

    try:
        # get a count of each htype from file
        pull_socket_fname = PULL_SOCKET_PREFIX_FNAME + name + '.txt'
        with open(pull_socket_fname, 'r') as log_fp:
            for line in log_fp:
                line = line.strip()
                if not line or not line.startswith('{'):
                    continue
                try:
                    data = json.loads(line)
                    htype = data.get("htype")
                    if htype in htype_counts:
                        htype_counts[htype] += 1
                except json.JSONDecodeError:
                    continue

        # test if file contents matches expected counts
        num_ports_per_module = 1 if name == "gotthard2" else det.numinterfaces
        total_num_frame_parts = num_ports_per_module * num_mods * num_frames
        for htype, expected_count in [("header", num_mods), ("series_end", num_mods), ("module", total_num_frame_parts)]:
            if htype_counts[htype] != expected_count:
                msg = f"Expected {expected_count} '{htype}' entries, found {htype_counts[htype]}"
                raise RuntimeException(msg)
    except Exception as e:
        raise RuntimeException(f'Failed to get zmq header count type. Error:{str(e)}') from e
        
    Log(LogLevel.INFOGREEN, f"Zmq Header type count test passed for {name}")
    Log(LogLevel.INFOGREEN, f"Zmq Header type count test passed for {name}", fp)


def startTestsForAll(args, fp):
    for server in args.servers:
        try:
            Log(LogLevel.INFOBLUE, f'Synchronizer Tests for {server}')
            Log(LogLevel.INFOBLUE, f'Synchronizer Tests for {server}', fp)
            cleanup(fp)
            startDetectorVirtualServer(server, args.num_mods, fp)
            startFrameSynchronizerPullSocket(server, fp)
            startFrameSynchronizer(args.num_mods, fp)
            d = loadConfig(name=server, rx_hostname=args.rx_hostname, settingsdir=args.settingspath, fp=fp, num_mods=args.num_mods, num_frames=args.num_frames)
            acquire(fp, d)
            testFramesCaught(server, d, args.num_frames)
            testZmqHeadetTypeCount(server, d, args.num_mods, args.num_frames, fp)
            Log(LogLevel.INFO, '\n')
        except Exception as e:
            raise RuntimeException(f'Synchronizer Tests failed') from e

    Log(LogLevel.INFOGREEN, 'Passed all synchronizer tests for all detectors \n' + str(args.servers))
  

if __name__ == '__main__':
    args = ParseArguments(description='Automated tests to test frame synchronizer', default_num_mods=2)

    Log(LogLevel.INFOBLUE, '\nLog File: ' + MAIN_LOG_FNAME + '\n') 

    with open(MAIN_LOG_FNAME, 'w') as fp:
        try:
            startTestsForAll(args, fp)
            cleanup(fp)
        except Exception as e:
            with open(MAIN_LOG_FNAME, 'a') as fp_error:
                traceback.print_exc(file=fp_error)
            cleanup(fp)
            Log(LogLevel.ERROR, f'Tests Failed.')


