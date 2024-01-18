# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
'''
This file is used to start up simulators, receivers and run all the tests on them and finally kill the simulators and receivers.
'''
import argparse
import os, sys, subprocess, time, colorama, signal

from colorama import Fore
from slsdet import Detector, detectorType, detectorSettings
from slsdet.defines import DEFAULT_TCP_CNTRL_PORTNO, DEFAULT_TCP_RX_PORTNO, DEFAULT_UDP_DST_PORTNO
HALFMOD2_TCP_CNTRL_PORTNO=1955
HALFMOD2_TCP_RX_PORTNO=1957

colorama.init(autoreset=True)

class RuntimeException (Exception):
    def __init__ (self, message):
        super().__init__(Fore.RED + message)
    
def Log(color, message):
    print('\n' + color + message, flush=True)


def checkIfProcessRunning(processName):
    cmd = "ps -ef | grep " + processName
    print(cmd)
    res=subprocess.getoutput(cmd)
    print(res)
    # eg. of output
    #l_user  250506  243295  0 14:38 pts/5    00:00:00 /bin/sh -c ps -ef | grep slsReceiver
    #l_user  250508  250506  0 14:38 pts/5    00:00:00 grep slsReceiver

    print('how many')
    cmd = "ps -ef | grep " + processName + " | wc -l"
    print(cmd)
    res=subprocess.getoutput(cmd)
    print(res)

    if res == '2':
        return False
    return True


def killProcess(name):
    if checkIfProcessRunning(name):
        Log(Fore.GREEN, 'killing ' + name)
        p = subprocess.run(['killall', name])
        if p.returncode != 0:
            raise RuntimeException('killall failed for ' + name)
    else:
        print('process not running : ' + name)

def cleanup(name):
    '''
    kill both servers, receivers and clean shared memory
    '''
    Log(Fore.GREEN, 'Cleaning up...')
    killProcess(name + 'DetectorServer_virtual')
    killProcess('slsReceiver')
    killProcess('slsMultiReceiver')
    cleanSharedmemory()

def cleanSharedmemory():
    Log(Fore.GREEN, 'Cleaning up shared memory...')
    try:
        p = subprocess.run(['sls_detector_get', 'free'], stdout=fp, stderr=fp)
    except:
        Log(Fore.RED, 'Could not free shared memory')
        raise

def startProcessInBackground(name):
    try:
        # in background and dont print output
        p = subprocess.Popen(name.split(), stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, restore_signals=False) 
        Log(Fore.GREEN, 'Starting up ' + name + ' ...')
    except:
        Log(Fore.RED, 'Could not start ' + name)
        raise

def startServer(name):
    
    startProcessInBackground(name + 'DetectorServer_virtual')
    # second half
    if name == 'eiger':
        startProcessInBackground(name + 'DetectorServer_virtual -p' + str(HALFMOD2_TCP_CNTRL_PORTNO))
    tStartup = 6
    Log(Fore.WHITE, 'Takes ' + str(tStartup) + ' seconds... Please be patient')
    time.sleep(tStartup)

def startReceiver(name):
    startProcessInBackground('slsReceiver')
    # second half
    if name == 'eiger':
        startProcessInBackground('slsReceiver -t' + str(HALFMOD2_TCP_RX_PORTNO))
    time.sleep(2)

def loadConfig(name, rx_hostname, settingsdir):
    Log(Fore.GREEN, 'Loading config')
    try:
        d = Detector()
        if name == 'xilinx_ctb':
            d.hostname = 'localhost'
        elif name == 'eiger':
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
            if d.type == detectorType.GOTTHARD:
                d.udp_srcip = d.udp_dstip
            else:
                d.udp_srcip = 'auto'
        if d.type == detectorType.JUNGFRAU or d.type == detectorType.MOENCH:
            d.powerchip = 1
    except:
        Log(Fore.RED, 'Could not load config for ' + name)
        raise

def startCmdTests(name, fp, fname):
    Log(Fore.GREEN, 'Cmd Tests for ' + name)
    cmd = 'tests --abort [.cmdcall] -s -o ' + fname
    p = subprocess.run(cmd.split(), stdout=fp, stderr=fp, check=True, text=True)
    p.check_returncode()

    with open (fname, 'r') as f:
        for line in f:
            if "FAILED" in line:
                msg = 'Cmd tests failed for ' + name + '!!!'
                Log(Fore.RED, msg)
                raise Exception(msg)

    Log(Fore.GREEN, 'Cmd Tests successful for ' + name)

def startGeneralTests(fp, fname):
    Log(Fore.GREEN, 'General Tests')
    cmd = 'tests --abort -s -o ' + fname
    p = subprocess.run(cmd.split(), stdout=fp, stderr=fp, check=True, text=True)
    p.check_returncode()

    with open (fname, 'r') as f:
        for line in f:
            if "FAILED" in line:
                msg = 'General tests failed !!!'
                Log(Fore.RED, msg)
                raise Exception(msg)

    Log(Fore.GREEN, 'General Tests successful')



# parse cmd line for rx_hostname and settingspath using the argparse library
parser = argparse.ArgumentParser(description = 'automated tests with the virtual detector servers')
parser.add_argument('rx_hostname', help = 'hostname/ip of the current machine')
parser.add_argument('settingspath', help = 'Relative or absolut path to the settingspath')
parser.add_argument('-s', '--servers', help='Detector servers to run', nargs='*')
args = parser.parse_args()
if args.rx_hostname == 'localhost':
    raise RuntimeException('Cannot use localhost for rx_hostname for the tests (fails for rx_arping for eg.)')

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


Log(Fore.WHITE, 'Arguments:\nrx_hostname: ' + args.rx_hostname + '\nsettingspath: \'' + args.settingspath + '\'') 


# redirect to file
prefix_fname = '/tmp/slsDetectorPackage_virtual_test'
original_stdout = sys.stdout
original_stderr = sys.stderr
fname = prefix_fname + '_log.txt'
Log(Fore.BLUE, '\nLog File: ' + fname) 

with open(fname, 'w') as fp:

    # general tests
    file_results = prefix_fname + '_results_general.txt'
    Log(Fore.BLUE, 'General tests (results: ' + file_results + ')')
    sys.stdout = fp
    sys.stderr = fp
    Log(Fore.BLUE, 'General tests (results: ' + file_results + ')')
    startGeneralTests(fp, file_results)

    for server in servers:
        try:
            # print to terminal for progress
            sys.stdout = original_stdout
            sys.stderr = original_stderr
            file_results = prefix_fname + '_results_cmd_' + server + '.txt'
            Log(Fore.BLUE, 'Cmd tests for ' + server + ' (results: ' + file_results + ')')
            sys.stdout = fp
            sys.stderr = fp
            Log(Fore.BLUE, 'Cmd tests for ' + server + ' (results: ' + file_results + ')')
            
            # cmd tests for det
            cleanup(server)
            startServer(server)
            startReceiver(server)
            loadConfig(server, args.rx_hostname, args.settingspath)
            startCmdTests(server, fp, file_results)
            cleanup(server)
        except:
            Log(Fore.RED, 'Exception caught. Cleaning up.')
            cleanup(server)
            sys.stdout = original_stdout
            sys.stderr = original_stderr
            Log(Fore.RED, 'Cmd tests failed for ' + server + '!!!')
            raise


    Log(Fore.GREEN, 'Passed all tests for virtual detectors \n' + str(servers))

# redirect to terminal
sys.stdout = original_stdout
sys.stderr = original_stderr
Log(Fore.GREEN, 'Passed all tests for virtual detectors \n' + str(servers) + '\nYayyyy! :) ')