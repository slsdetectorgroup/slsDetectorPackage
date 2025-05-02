# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
'''
This file is used to start up simulators, receivers and run all the tests on them and finally kill the simulators and receivers.
'''
import argparse
import os, sys, subprocess, time, colorama
import shlex

from colorama import Fore, Style
from slsdet import Detector, detectorType, detectorSettings
from slsdet.defines import DEFAULT_TCP_RX_PORTNO, DEFAULT_UDP_DST_PORTNO
SERVER_START_PORTNO=1900


colorama.init(autoreset=True)

def Log(color, message):
    print(f"{color}{message}{Style.RESET_ALL}", flush=True)

class RuntimeException (Exception):
    def __init__ (self, message):
        super().__init__(Log(Fore.RED, message))
    
def checkIfProcessRunning(processName):
    cmd = f"pgrep -f {processName}"
    res = subprocess.getoutput(cmd)
    return res.strip().splitlines()


def killProcess(name):
    pids = checkIfProcessRunning(name)
    if pids:
        Log(Fore.GREEN, f"Killing '{name}' processes with PIDs: {', '.join(pids)}")
        for pid in pids:
            try:
                p = subprocess.run(['kill', pid])
                if p.returncode != 0 and bool(checkIfProcessRunning(name)):
                    raise RuntimeException(f"Could not kill {name} with pid {pid}")
            except Exception as e:
                Log(Fore.RED, f"Failed to kill process {name} pid:{pid}. Exception occured: [code:{e}, msg:{e.stderr}]")
                raise               
    #else:
    #    Log(Fore.WHITE, 'process not running : ' + name)


def cleanup(fp):
    '''
    kill both servers, receivers and clean shared memory
    '''
    Log(Fore.GREEN, 'Cleaning up...')
    killProcess('DetectorServer_virtual')
    killProcess('slsReceiver')
    killProcess('slsMultiReceiver')
    killProcess('slsFrameSynchronizer')
    killProcess('frameSynchronizerPullSocket')
    cleanSharedmemory(fp)

def cleanSharedmemory(fp):
    Log(Fore.GREEN, 'Cleaning up shared memory...')
    try:
        p = subprocess.run(['sls_detector_get', 'free'], stdout=fp, stderr=fp)
    except:
        Log(Fore.RED, 'Could not free shared memory')
        raise

def startProcessInBackground(name):
    try:
        # in background and dont print output
        p = subprocess.Popen(shlex.split(name), stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, restore_signals=False) 
        Log(Fore.GREEN, 'Starting up ' + name + ' ...')
    except Exception as e:
        Log(Fore.RED, f'Could not start {name}:{e}')
        raise

def startServers(name, num_mods):
    for i in range(num_mods):
        port_no = SERVER_START_PORTNO + (i * 2)
        startProcessInBackground(name + 'DetectorServer_virtual -p' + str(port_no))
        time.sleep(6)

def startFrameSynchronizerPullSocket():
    startProcessInBackground('python frameSynchronizerPullSocket.py')
    tStartup = 4
    Log(Fore.WHITE, 'Takes ' + str(tStartup) + ' seconds... Please be patient')
    time.sleep(tStartup)
    if not checkIfProcessRunning('frameSynchonizerPull'):
        Log(Fore.RED, "Could not start pull socket. Its not running.")
        raise

def startFrameSynchronizer(num_mods):
    Log(Fore.GREEN, "Going to start frame synchonizer")
    # in 10.0.0
    #startProcessInBackground('slsFrameSynchronizer -n ' + str(num_mods) + ' -p ' + str(DEFAULT_TCP_RX_PORTNO))
    startProcessInBackground('slsFrameSynchronizer ' + str(DEFAULT_TCP_RX_PORTNO) + ' ' + str(num_mods))
    tStartup = 1 * num_mods
    Log(Fore.WHITE, 'Takes ' + str(tStartup) + ' seconds... Please be patient')
    time.sleep(tStartup)

def loadConfig(name, num_mods, rx_hostname, settingsdir, num_frames):
    Log(Fore.GREEN, 'Loading config')
    try:
        d = Detector()
        d.virtual = [num_mods, SERVER_START_PORTNO]
        d.udp_dstport = DEFAULT_UDP_DST_PORTNO

        if name == 'eiger':
            d.udp_dstport2 = DEFAULT_UDP_DST_PORTNO + 1

        d.rx_hostname = rx_hostname
        d.udp_dstip = 'auto'
        d.udp_srcip = 'auto'

        if name == 'eiger':
            d.trimen = [4500, 5400, 6400]
            d.settingspath = settingsdir + '/eiger/'
            d.setThresholdEnergy(4500, detectorSettings.STANDARD)
        elif d.type == detectorType.JUNGFRAU or d.type == detectorType.MOENCH or d.type == detectorType.XILINX_CHIPTESTBOARD:
            d.powerchip = 1

        if d.type == detectorType.XILINX_CHIPTESTBOARD:
            d.configureTransceiver()

        d.frames = num_frames
    except Exception as e:
        Log(Fore.RED, f'Could not load config for {name}. Error: {str(e)}')
        raise

def startTests(name, fp, fname, num_frames):
    Log(Fore.GREEN, 'Tests for ' + name)
    cmd = 'tests --abort [.cmdcall] -s -o ' + fname
    
    d = Detector()
    d.acquire()
    fnum = d.rx_framescaught[0]
    if fnum == num_frames:
        Log(Fore.RED, "{name} caught only {fnum}. Expected {num_frames}") 
        raise

    Log(Fore.GREEN, 'Tests successful for ' + name)


# parse cmd line for rx_hostname and settingspath using the argparse library
parser = argparse.ArgumentParser(description = 'automated tests with the virtual detector servers')
parser.add_argument('rx_hostname', nargs='?', default='localhost', help = 'hostname/ip of the current machine')
parser.add_argument('settingspath', nargs='?', default='../../settingsdir', help = 'Relative or absolut path to the settingspath')
parser.add_argument('-n', '--num-mods', nargs='?', default=2, type=int, help = 'Number of modules to test with')
parser.add_argument('-f', '--num-frames', nargs='?', default=1, type=int, help = 'Number of frames to test with')
parser.add_argument('-s', '--servers', nargs='*', help='Detector servers to run')
args = parser.parse_args()

if args.servers is None:
    servers = [
        #'eiger',
        'jungfrau',
        #'mythen3',
        #'gotthard2',
        #'ctb',
        #'moench',
        #'xilinx_ctb'
    ]
else:
    servers = args.servers


Log(Fore.WHITE, 'Arguments:\nrx_hostname: ' + args.rx_hostname + '\nsettingspath: \'' + args.settingspath + '\nservers: \'' + ' '.join(servers) + '\nnum_mods: \'' + str(args.num_mods) + '\nnum_frames: \'' + str(args.num_frames) + '\'') 


# redirect to file
prefix_fname = '/tmp/slsFrameSynchronizer_test'
original_stdout = sys.stdout
original_stderr = sys.stderr
fname = prefix_fname + '_log.txt'
Log(Fore.BLUE, '\nLog File: ' + fname) 

with open(fname, 'w') as fp:

    try:
        cleanup(fp)

        testError = False
        for server in servers:
            try:
                # print to terminal for progress
                sys.stdout = original_stdout
                sys.stderr = original_stderr
                file_results = prefix_fname + '_results_cmd_' + server + '.txt'
                Log(Fore.BLUE, 'Synchonizer tests for ' + server + ' (results: ' + file_results + ')')
                sys.stdout = fp
                sys.stderr = fp
                Log(Fore.BLUE, 'Synchonizer tests for ' + server + ' (results: ' + file_results + ')')
                
                # cmd tests for det
                cleanup(fp)
                startServers(server, args.num_mods)
                startFrameSynchronizerPullSocket()
                startFrameSynchronizer(args.num_mods)
                loadConfig(server, args.num_mods, args.rx_hostname, args.settingspath, args.num_frames)
                startTests(server, fp, file_results, args.num_frames)
                cleanup(fp)
                
            except Exception as e:
                # redirect to terminal
                sys.stdout = original_stdout
                sys.stderr = original_stderr
                Log(Fore.RED, f'Exception caught while testing {server}. Cleaning up...')
                testError = True
                break

        # redirect to terminal
        sys.stdout = original_stdout
        sys.stderr = original_stderr
        if not testError:
            Log(Fore.GREEN, 'Passed all sync tests\n' + str(servers))


    except Exception as e:
        # redirect to terminal
        sys.stdout = original_stdout
        sys.stderr = original_stderr
        Log(Fore.RED, f'Exception caught with general testing. Cleaning up...')
        cleanSharedmemory(sys.stdout)
        