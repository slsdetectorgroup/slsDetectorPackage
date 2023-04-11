# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
'''
This file is used to start up simulators, receivers and run all the tests on them and finally kill the simulators and receivers.
'''
import os, sys, subprocess, time, colorama, signal, psutil

from colorama import Fore
from slsdet import Detector, detectorType, detectorSettings
from slsdet.defines import DEFAULT_TCP_CNTRL_PORTNO, DEFAULT_TCP_RX_PORTNO, DEFAULT_UDP_DST_PORTNO
HALFMOD2_TCP_CNTRL_PORTNO=1955
HALFMOD2_TCP_RX_PORTNO=1957
HALFMOD2_TCP_RX_PORTNO=1957
import shlex

colorama.init(autoreset=True)

class RuntimeException (Exception):
    def __init__ (self, message):
        super().__init__(Fore.RED + message)

def handler(signum, frame):
        pid, status = os.waitpid(-1, os.WNOHANG)

def checkIfProcessRunning(processName):
    '''
    Check if there is any running process that contains the given name processName.
    https://gist.github.com/Sanix-Darker/8cbed2ff6f8eb108ce2c8c51acd2aa5a
    '''
    # Iterate over the all the running process
    for proc in psutil.process_iter():
        try:
            # Check if process name contains the given name string.
            if processName.lower() in proc.name().lower():
                return True
        except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
            pass
    return False;

def killProcess(name):
    if checkIfProcessRunning(name):
        p = subprocess.run(['killall', name])
        if p.returncode != 0:
            raise RuntimeException('error in killall ' + name)

def cleanup(name):
    '''
    kill both servers and receivers
    '''
    killProcess(name + 'DetectorServer_virtual')
    killProcess('slsReceiver')
    killProcess('slsMultiReceiver')

def startProcessInBackground(name):
    try:
        # in background and dont print output
        p = subprocess.Popen(shlex.split(name), stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL) 
        print(Fore.GREEN + 'Starting up ' + name + ' ...')
    except:
        print(Fore.RED + 'Could not start ' + name)
        raise

def startServer(name):
    startProcessInBackground(name + 'DetectorServer_virtual')
    # second half
    if name == 'eiger':
        startProcessInBackground(name + 'DetectorServer_virtual -p' + str(HALFMOD2_TCP_CNTRL_PORTNO))
    tStartup = 6
    print('Takes {} seconds... Please be patient'.format(tStartup))
    time.sleep(tStartup)

def startReceiver(name):
    startProcessInBackground('slsReceiver')
    # second half
    if name == 'eiger':
        startProcessInBackground('slsReceiver -t' + str(HALFMOD2_TCP_RX_PORTNO))
    time.sleep(2)

def loadConfig(name, rx_hostname, settingsdir):
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
            if d.type == detectorType.GOTTHARD:
                d.udp_srcip = d.udp_dstip
            else:
                d.udp_srcip = 'auto'
        if d.type == detectorType.JUNGFRAU or d.type == detectorType.MOENCH:
            d.powerchip = 1
    except:
        print(Fore.RED + 'Could not load config for ' + name)
        raise

def startCmdTests(name):
    try:
        p = subprocess.run(["tests", "--abort", "[.cmd]"])
        if p.returncode != 0:
            raise Exception 
    except:
        print(Fore.RED + "tests failed for " + name) 
        raise


#print('Nargs:{}, args:{}'.format(len(sys.argv), str(sys.argv)))

# argument for rx_hostname
if len(sys.argv) != 3:
    raise RuntimeException('No argument for rx_hostname or settingsdir. Expected [script_name] [rx_hostname] [settingsdir(rel or abs)]')
if sys.argv[1] == 'localhost':
    raise RuntimeException('Cannot use localhost for rx_hostname for the tests (fails for rx_arping for eg.)')
rx_hostname = sys.argv[1]
settingsdir = sys.argv[2]

# handle zombies
signal.signal(signal.SIGCHLD, handler)   
servers = [
     "eiger",
     "jungfrau",
     "mythen3",
     "gotthard2",
     "gotthard",
     "ctb",
     "moench",
]
for server in servers:
    try:
        print(Fore.BLUE + 'Starting tests for {}'.format(server))
        cleanup(server)
        startServer(server)
        startReceiver(server)
        loadConfig(server, rx_hostname, settingsdir)
        startCmdTests(server)
        #start normal tests for one? if not detector type dependent
        cleanup(server)
    except:
        cleanup(server)
        raise
print(Fore.GREEN + 'Passed all tests for virtual detectors {} Yayyyy! :) '.format(servers))