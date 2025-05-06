# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
'''
This file is used to start up simulators, receivers and run all the tests on them and finally kill the simulators and receivers.
'''
import argparse
import os, sys, subprocess, time

from slsdet import Detector, detectorType, detectorSettings
from slsdet.defines import DEFAULT_TCP_CNTRL_PORTNO, DEFAULT_TCP_RX_PORTNO, DEFAULT_UDP_DST_PORTNO
HALFMOD2_TCP_CNTRL_PORTNO=1955
HALFMOD2_TCP_RX_PORTNO=1957

from utils_for_test import Log, LogLevel

class RuntimeException (Exception):
    def __init__ (self, message):
        super().__init__(Log(LogLevel.INFORED, message))
    
def checkIfProcessRunning(processName):
    cmd = f"pgrep -f {processName}"
    res = subprocess.getoutput(cmd)
    return res.strip().splitlines()


def killProcess(name):
    pids = checkIfProcessRunning(name)
    if pids:
        Log(LogLevel.INFOGREEN, f"Killing '{name}' processes with PIDs: {', '.join(pids)}")
        for pid in pids:
            try:
                p = subprocess.run(['kill', pid])
                if p.returncode != 0 and bool(checkIfProcessRunning(name)):
                    raise RuntimeException(f"Could not kill {name} with pid {pid}")
            except Exception as e:
                raise RuntimeException(f"Failed to kill process {name} pid:{pid}. Exception occured: [code:{e}, msg:{e.stderr}]")
    #else:
    #    Log(LogLevel.INFO, 'process not running : ' + name)


def cleanup(fp):
    '''
    kill both servers, receivers and clean shared memory
    '''
    Log(LogLevel.INFOGREEN, 'Cleaning up...')
    killProcess('DetectorServer_virtual')
    killProcess('slsReceiver')
    killProcess('slsMultiReceiver')
    cleanSharedmemory(fp)

def cleanSharedmemory(fp):
    Log(LogLevel.INFOGREEN, 'Cleaning up shared memory...')
    try:
        p = subprocess.run(['sls_detector_get', 'free'], stdout=fp, stderr=fp)
    except:
        raise RuntimeException('Could not free shared memory')

def startProcessInBackground(name):
    try:
        # in background and dont print output
        p = subprocess.Popen(name.split(), stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, restore_signals=False) 
        Log(LogLevel.INFOGREEN, 'Starting up ' + name + ' ...')
    except Exception as e:
        raise RuntimeException(f'Could not start {name}:{e}')

def startServer(name):
    
    startProcessInBackground(name + 'DetectorServer_virtual')
    # second half
    if name == 'eiger':
        startProcessInBackground(name + 'DetectorServer_virtual -p' + str(HALFMOD2_TCP_CNTRL_PORTNO))
    tStartup = 6
    Log(LogLevel.INFO, 'Takes ' + str(tStartup) + ' seconds... Please be patient')
    time.sleep(tStartup)

def startReceiver(name):
    startProcessInBackground('slsReceiver')
    # second half
    if name == 'eiger':
        startProcessInBackground('slsReceiver -t' + str(HALFMOD2_TCP_RX_PORTNO))
    time.sleep(2)

def loadConfig(name, rx_hostname, settingsdir):
    Log(LogLevel.INFOGREEN, 'Loading config')
    try:
        d = Detector()
        if name == 'eiger':
            d.hostname = 'localhost:' + str(DEFAULT_TCP_CNTRL_PORTNO) + '+localhost:' + str(HALFMOD2_TCP_CNTRL_PORTNO)
            #d.udp_dstport = {2: 50003} 
            # will set up for every module
            d.udp_dstport = DEFAULT_UDP_DST_PORTNO
            d.udp_dstport2 = DEFAULT_UDP_DST_PORTNO + 1
            d.rx_hostname = rx_hostname + ':' + str(DEFAULT_TCP_RX_PORTNO) + '+' + rx_hostname + ':' + str(HALFMOD2_TCP_RX_PORTNO)
            d.udp_dstip = 'auto'
            d.trimen = [4500, 5400, 6400]
            d.settingspath = settingsdir + '/eiger/'
            d.setThresholdEnergy(4500, detectorSettings.STANDARD)
        else:
            d.hostname = 'localhost'
            d.rx_hostname = rx_hostname
            d.udp_dstip = 'auto'
            d.udp_dstip = 'auto'
            if d.type == detectorType.GOTTHARD:
                d.udp_srcip = d.udp_dstip
            else:
                d.udp_srcip = 'auto'
        if d.type == detectorType.JUNGFRAU or d.type == detectorType.MOENCH or d.type == detectorType.XILINX_CHIPTESTBOARD:
            d.powerchip = 1
        if d.type == detectorType.XILINX_CHIPTESTBOARD:
            d.configureTransceiver()
    except:
        raise RuntimeException('Could not load config for ' + name)

def startCmdTests(name, fp, fname):
    Log(LogLevel.INFOGREEN, 'Cmd Tests for ' + name)
    cmd = 'tests --abort [.cmdcall] -s -o ' + fname
    try:
        subprocess.run(cmd.split(), stdout=fp, stderr=fp, check=True, text=True)
    except subprocess.CalledProcessError as e:
        pass

    with open (fname, 'r') as f:
        for line in f:
            if "FAILED" in line:
                msg = 'Cmd tests failed for ' + name + '!!!'
                sys.stdout = original_stdout
                Log(LogLevel.ERROR, f"{msg}\n{line}")
                sys.stdout = fp
                raise Exception(msg)

    Log(LogLevel.INFOGREEN, 'Cmd Tests successful for ' + name)

def startGeneralTests(fp, fname):
    Log(LogLevel.INFOGREEN, 'General Tests')
    cmd = 'tests --abort -s -o ' + fname
    try:
        subprocess.run(cmd.split(), stdout=fp, stderr=fp, check=True, text=True)
    except subprocess.CalledProcessError as e:
        pass

    with open (fname, 'r') as f:
        for line in f:
            if "FAILED" in line:
                msg = 'General tests failed !!!'
                sys.stdout = original_stdout
                Log(LogLevel.ERROR, msg + '\n' + line)
                sys.stdout = fp
                raise Exception(msg)

    Log(LogLevel.INFOGREEN, 'General Tests successful')



# parse cmd line for rx_hostname and settingspath using the argparse library
parser = argparse.ArgumentParser(description = 'automated tests with the virtual detector servers')
parser.add_argument('rx_hostname', nargs='?', default='localhost', help = 'hostname/ip of the current machine')
parser.add_argument('settingspath', nargs='?', default='../../settingsdir', help = 'Relative or absolut path to the settingspath')
parser.add_argument('-s', '--servers', help='Detector servers to run', nargs='*')
args = parser.parse_args()

if args.servers is None:
    servers = [
        'eiger',
        'jungfrau',
        'mythen3',
        'gotthard2',
        'gotthard',
        'ctb',
        'moench',
        'xilinx_ctb'
    ]
else:
    servers = args.servers


Log(LogLevel.INFO, 'Arguments:\nrx_hostname: ' + args.rx_hostname + '\nsettingspath: \'' + args.settingspath + '\nservers: \'' + ' '.join(servers) + '\'') 


# redirect to file
prefix_fname = '/tmp/slsDetectorPackage_virtual_test'
original_stdout = sys.stdout
original_stderr = sys.stderr
fname = prefix_fname + '_log.txt'
Log(LogLevel.INFOBLUE, '\nLog File: ' + fname) 

with open(fname, 'w') as fp:



    # general tests
    file_results = prefix_fname + '_results_general.txt'
    Log(LogLevel.INFOBLUE, 'General tests (results: ' + file_results + ')')
    sys.stdout = fp
    sys.stderr = fp
    Log(LogLevel.INFOBLUE, 'General tests (results: ' + file_results + ')')

    try:
        cleanup(fp)
        startGeneralTests(fp, file_results)
        cleanup(fp)

        testError = False
        for server in servers:
            try:
                # print to terminal for progress
                sys.stdout = original_stdout
                sys.stderr = original_stderr
                file_results = prefix_fname + '_results_cmd_' + server + '.txt'
                Log(LogLevel.INFOBLUE, 'Cmd tests for ' + server + ' (results: ' + file_results + ')')
                sys.stdout = fp
                sys.stderr = fp
                Log(LogLevel.INFOBLUE, 'Cmd tests for ' + server + ' (results: ' + file_results + ')')
                
                # cmd tests for det
                cleanup(fp)
                startServer(server)
                startReceiver(server)
                loadConfig(server, args.rx_hostname, args.settingspath)
                startCmdTests(server, fp, file_results)
                cleanup(fp)
                
            except Exception as e:
                # redirect to terminal
                sys.stdout = original_stdout
                sys.stderr = original_stderr
                Log(LogLevel.INFORED, f'Exception caught while testing {server}. Cleaning up...')
                testError = True
                break

        # redirect to terminal
        sys.stdout = original_stdout
        sys.stderr = original_stderr
        if not testError:
            Log(LogLevel.INFOGREEN, 'Passed all tests for virtual detectors \n' + str(servers))


    except Exception as e:
        # redirect to terminal
        sys.stdout = original_stdout
        sys.stderr = original_stderr
        Log(LogLevel.INFORED, f'Exception caught with general testing. Cleaning up...')
        cleanSharedmemory(sys.stdout)
        