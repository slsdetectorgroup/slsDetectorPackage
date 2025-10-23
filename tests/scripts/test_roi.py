# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
'''
This file is used to start up simulators, receivers and test roi for every detector in many configurations.
'''

import sys, time
import traceback

from slsdet import Detector, burstMode
from slsdet.defines import DEFAULT_TCP_RX_PORTNO, DEFAULT_UDP_DST_PORTNO
from datetime import timedelta


from utils_for_test import (
    Log,
    LogLevel,
    RuntimeException,
    cleanup,
    startProcessInBackground,
    startDetectorVirtualServer,
    connectToVirtualServers,
    loadBasicSettings,
    runProcessWithLogFile
)

LOG_PREFIX_FNAME = '/tmp/slsDetectorPackage_virtual_roi_test'
MAIN_LOG_FNAME = LOG_PREFIX_FNAME + '_log.txt'
ROI_TEST_FNAME = LOG_PREFIX_FNAME + '_results_'

def startReceiver(num_mods, fp):
    if num_mods == 1:
        cmd = ['slsReceiver']
    else:
        cmd = ['slsMultiReceiver', str(DEFAULT_TCP_RX_PORTNO), str(num_mods)]
        # in 10.0.0
        #cmd = ['slsMultiReceiver', '-p', str(DEFAULT_TCP_RX_PORTNO), '-n', str(num_mods)]
    startProcessInBackground(cmd, fp)
    time.sleep(1)


def loadConfigForRoi(name, fp, num_mods = 1, num_interfaces = 1):
    Log(LogLevel.INFO, 'Loading config')
    Log(LogLevel.INFO, 'Loading config', fp)
    try:
        d = connectToVirtualServers(name, num_mods)
        
        if name == 'jungfrau' or name == 'moench':
            d.numinterfaces = num_interfaces

        d.udp_dstport = DEFAULT_UDP_DST_PORTNO
        if name == 'eiger' or name == 'jungfrau' or name == 'moench':
            d.udp_dstport2 = DEFAULT_UDP_DST_PORTNO + 1

        d.rx_hostname = 'localhost'
        d.udp_dstip = 'auto'
        if name != "eiger":
            d.udp_srcip = 'auto'
        if name == 'jungfrau' or name == 'moench':
            d.udp_dstip2 = 'auto'
            d.powerchip = 1

        d.frames = 5

    except Exception as e:
        raise RuntimeException(f'Could not load config for {name}. Error: {str(e)}') from e
    
    return d

def startTestsForAll(fp):
    servers = [
        'eiger',
        'jungfrau',
        'mythen3',
        'gotthard2',
        'moench',
    ]
    nmods = 2
    for server in servers:
        for ninterfaces in range(1, 2):
            if ninterfaces == 2 and server != 'jungfrau' and server != 'moench':
                continue
            try:
                msg = f'Starting Roi Tests for {server}'
                if server == 'jungfrau' or server == 'moench':
                    msg += f' with {ninterfaces} interfaces'
                Log(LogLevel.INFOBLUE, msg)
                Log(LogLevel.INFOBLUE, msg, fp)
                cleanup(fp)
                startDetectorVirtualServer(server, nmods, fp)
                startReceiver(nmods, fp)
                d = loadConfigForRoi(name=server, fp=fp, num_mods=nmods, num_interfaces=ninterfaces)
                loadBasicSettings(name=server, d=d, fp=fp)

                fname = ROI_TEST_FNAME + server + '.txt'
                cmd = ['tests', 'rx_roi', '--abort', '-s']
                runProcessWithLogFile('Roi Tests for ' + server, cmd, fp, fname)
                Log(LogLevel.INFO, '\n')
            except Exception as e:
                raise RuntimeException(f'Roi Tests failed') from e

    Log(LogLevel.INFOGREEN, 'Passed all Roi tests for all detectors \n' + str(servers))
  

if __name__ == '__main__':
    Log(LogLevel.INFOBLUE, '\nLog File: ' + MAIN_LOG_FNAME + '\n') 

    with open(MAIN_LOG_FNAME, 'w') as fp:
        try:
            startTestsForAll(fp)
            #TODO: check master file as well for both json and hdf5 as well
            cleanup(fp)
        except Exception as e:
            with open(MAIN_LOG_FNAME, 'a') as fp_error:
                traceback.print_exc(file=fp_error)
            cleanup(fp)
            Log(LogLevel.ERROR, f'Tests Failed.')


