# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
'''
This file is used to start up simulators, frame synchronizer, pull sockets, acquire, test and kill them finally.
'''

import sys, time
import traceback, json

from slsdet import Detector

from utils_for_test import (
    Log,
    LogLevel,
    RuntimeException,
    cleanup,
    startProcessInBackground,
    checkLogForErrors,
    startDetectorVirtualServer,
    loadConfig,
    loadBasicSettings,
    ParseArguments, 
    build_dir,
    optional_file,
    RX_START_TCP_PORTNO
)

LOG_PREFIX_FNAME = '/tmp/slsFrameSynchronizer_test'
MAIN_LOG_FNAME = LOG_PREFIX_FNAME + '_log.txt'
PULL_SOCKET_PREFIX_FNAME = LOG_PREFIX_FNAME + '_pull_socket_'
SYNCHRONIZER_SUFFIX_FNAME = LOG_PREFIX_FNAME + '_synchronizer.txt'


def startFrameSynchronizerPullSocket(name, fp, no_log_file = False, quiet_mode=False):
    cmd = ['python', '-u', 'frameSynchronizerPullSocket.py']  
    fname = PULL_SOCKET_PREFIX_FNAME + name + '.txt'
    if no_log_file:
        fname = None
    startProcessInBackground(cmd, fp, fname, quiet_mode)
    time.sleep(1)
    if not no_log_file:
        checkLogForErrors(fp, fname)
    


def startFrameSynchronizer(num_mods, fp, no_log_file = False, quiet_mode=False):
    cmd = [str(build_dir / 'slsFrameSynchronizer'), '-p', str(RX_START_TCP_PORTNO), '-n', str(num_mods)]
    fname = SYNCHRONIZER_SUFFIX_FNAME
    if no_log_file:
        fname = None
    startProcessInBackground(cmd, fp, fname, quiet_mode)
    time.sleep(1)


def acquire(fp, det):
    Log(LogLevel.INFO, 'Acquiring', fp, True)
    det.acquire()


def testFramesCaught(name, det, num_frames):
    fnum = det.rx_framescaught[0]
    if fnum != num_frames:
        raise RuntimeException(f"{name} caught only {fnum}. Expected {num_frames}") 
    
    Log(LogLevel.INFOGREEN, f'Frames caught test passed for {name}', fp, True)


def testZmqHeadetTypeCount(name, det, num_mods, num_frames, fp):

    Log(LogLevel.INFO, f"Testing Zmq Header type count for {name}", fp, True)
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
        
    Log(LogLevel.INFOGREEN, f"Zmq Header type count test passed for {name}", fp, True)


def startTestsForAll(args, fp):
    for server in args.servers:
        try:
            Log(LogLevel.INFOBLUE, f'Synchronizer Tests for {server}', fp, True)
            cleanup(fp)
            startDetectorVirtualServer(server, args.num_mods, fp, args.quiet)
            startFrameSynchronizerPullSocket(server, fp, args.no_log_file, args.quiet)
            startFrameSynchronizer(args.num_mods, fp, args.no_log_file, args.quiet)
            d = loadConfig(name=server, rx_hostname=args.rx_hostname, settingsdir=args.settingspath, log_file_fp=fp, num_mods=args.num_mods, num_frames=args.num_frames)
            loadBasicSettings(name=server, d=d, fp=fp)
            acquire(fp, d)
            testFramesCaught(server, d, args.num_frames)
            testZmqHeadetTypeCount(server, d, args.num_mods, args.num_frames, fp)
            Log(LogLevel.INFO, '\n')
        except Exception as e:
            raise RuntimeException(f'Synchronizer Tests failed') from e

    Log(LogLevel.INFOGREEN, 'Passed all synchronizer tests for all detectors \n' + str(args.servers))
  

if __name__ == '__main__':
    args = ParseArguments(description='Automated tests to test frame synchronizer', default_num_mods=2)

    if args.no_log_file:
        raise RuntimeException("Cannot run frame synchronizer test without files")

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


