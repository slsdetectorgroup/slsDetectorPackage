# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
'''
This file is used to start up simulators, receivers and run all the tests on them and finally kill the simulators and receivers.
'''
import argparse
import os, sys, subprocess, time, colorama
import shlex, traceback, json

from colorama import Fore, Style
from slsdet import Detector, detectorType, detectorSettings
from slsdet.defines import DEFAULT_TCP_RX_PORTNO, DEFAULT_UDP_DST_PORTNO
SERVER_START_PORTNO=1900


colorama.init(autoreset=True)

def Log(color, message, stream=sys.stdout):
    print(f"{color}{message}{Style.RESET_ALL}", file=stream, flush=True)

class RuntimeException (Exception):
    def __init__ (self, message):
        super().__init__(Log(Fore.RED, message))
    
def checkIfProcessRunning(processName):
    cmd = f"pgrep -f {processName}"
    res = subprocess.getoutput(cmd)
    return res.strip().splitlines()


def killProcess(name, fp):
    pids = checkIfProcessRunning(name)
    if pids:
        Log(Fore.WHITE, f"Killing '{name}' processes with PIDs: {', '.join(pids)}", fp)
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
    Log(Fore.WHITE, 'Cleaning up')
    Log(Fore.WHITE, 'Cleaning up', fp)
    killProcess('DetectorServer_virtual', fp)
    killProcess('slsReceiver', fp)
    killProcess('slsMultiReceiver', fp)
    killProcess('slsFrameSynchronizer', fp)
    killProcess('frameSynchronizerPullSocket', fp)
    cleanSharedmemory(fp)

def cleanSharedmemory(fp):
    Log(Fore.WHITE, 'Cleaning up shared memory...', fp)
    try:
        p = subprocess.run(['sls_detector_get', 'free'], stdout=fp, stderr=fp)
    except:
        Log(Fore.RED, 'Could not free shared memory')
        raise

def startProcessInBackground(name, fp):
    try:
        # in background and dont print output
        p = subprocess.Popen(shlex.split(name), stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, restore_signals=False) 
        Log(Fore.WHITE, 'Starting up ' + name + ' ...', fp)
    except Exception as e:
        Log(Fore.RED, f'Could not start {name}:{e}')
        raise

def startServers(name, num_mods):
    Log(Fore.WHITE, 'Starting server')
    for i in range(num_mods):
        port_no = SERVER_START_PORTNO + (i * 2)
        startProcessInBackground(name + 'DetectorServer_virtual -p' + str(port_no), fp)
        time.sleep(6)

def startFrameSynchronizerPullSocket(fname, fp):
    Log(Fore.WHITE, 'Starting sync pull socket')
    Log(Fore.WHITE, f"Starting up Synchronizer pull socket. Log: {fname}", fp)
    Log(Fore.WHITE, f"Synchronizer pull socket log: {fname}")
    cmd = ['python', '-u', 'frameSynchronizerPullSocket.py']  
    try:
        with open(fname, 'w') as fp:
            subprocess.Popen(cmd, stdout=fp, stderr=fp, text=True)
    except Exception as e:
        Log(Fore.RED, f"failed to start synchronizer pull socket: {e}")
        raise

def startFrameSynchronizer(num_mods, fp):
    Log(Fore.WHITE, 'Starting frame synchronizer')
    # in 10.0.0
    #startProcessInBackground('slsFrameSynchronizer -n ' + str(num_mods) + ' -p ' + str(DEFAULT_TCP_RX_PORTNO))
    startProcessInBackground('slsFrameSynchronizer ' + str(DEFAULT_TCP_RX_PORTNO) + ' ' + str(num_mods), fp)
    tStartup = 1 * num_mods
    time.sleep(tStartup)

def loadConfig(name, num_mods, rx_hostname, settingsdir, num_frames, fp):
    Log(Fore.WHITE, 'Loading config')
    Log(Fore.WHITE, 'Loading config', fp)
    try:
        d = Detector()
        d.virtual = [num_mods, SERVER_START_PORTNO]
        d.udp_dstport = DEFAULT_UDP_DST_PORTNO

        if name == 'eiger':
            d.udp_dstport2 = DEFAULT_UDP_DST_PORTNO + 1

        d.rx_hostname = rx_hostname
        d.udp_dstip = 'auto'
        if name != "eiger":
            d.udp_srcip = 'auto'

        if name == "jungfrau" or name == "moench" or name == "xilinx_ctb":
            d.powerchip = 1

        if d.type == detectorType.XILINX_CHIPTESTBOARD:
            d.configureTransceiver()

        d.frames = num_frames
    except Exception as e:
        Log(Fore.RED, f'Could not load config for {name}. Error: {str(e)}')
        raise

def validate_htype_counts(log_path, num_mods, num_ports_per_module, num_frames):
    htype_counts = {
        "header": 0,
        "series_end": 0,
        "module": 0
    }

    # get a count of each htype from file
    with open(log_path, 'r') as f:
        for line in f:
            line = line.strip()
            if not line or not line.startswith('{'):
                continue
            try:
                data = json.loads(line)
                htype = data.get("htype")
                if htype in htype_counts:
                    htype_counts[htype] += 1
            except json.JSONDecodeError:
                continue  # or log malformed line

    for htype, expected_count in [("header", num_mods), ("series_end", num_mods), ("module", num_ports_per_module * num_mods * num_frames)]:
        if htype_counts[htype] != expected_count:
            msg = f"Expected {expected_count} '{htype}' entries, found {htype_counts[htype]}"
            Log(Fore.RED, msg)
            raise RuntimeError(msg)

def startTests(name, num_mods, num_frames, fp, file_pull_socket):
    Log(Fore.WHITE, 'Tests for ' + name)
    Log(Fore.WHITE, 'Tests for ' + name, fp)
    cmd = 'tests --abort [.cmdcall] -s -o ' + fname
    
    d = Detector()
    num_ports_per_module = d.numinterfaces
    if name == "gotthard2":
        num_ports_per_module = 1
    d.acquire()
    fnum = d.rx_framescaught[0]
    if fnum != num_frames:
        Log(Fore.RED, f"{name} caught only {fnum}. Expected {num_frames}") 
        raise

    validate_htype_counts(file_pull_socket, num_mods, num_ports_per_module, num_frames)
    Log(Fore.GREEN, f"Log file htype checks passed for {name}", fp)


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
        'eiger',
        'jungfrau',
        'mythen3',
        'gotthard2',
        'ctb',
        'moench',
        'xilinx_ctb'
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
        testError = False
        for server in servers:
            try:
                Log(Fore.BLUE, '\nSynchonizer tests for ' + server, fp)
                Log(Fore.BLUE, '\nSynchonizer tests for ' + server)
                
                # cmd tests for det
                cleanup(fp)
                startServers(server, args.num_mods)
                file_pull_socket = prefix_fname + '_pull_socket_' + server + '.txt'
                startFrameSynchronizerPullSocket(file_pull_socket, fp)
                startFrameSynchronizer(args.num_mods, fp)
                loadConfig(server, args.num_mods, args.rx_hostname, args.settingspath, args.num_frames, fp)
                startTests(server, args.num_mods, args.num_frames, fp, file_pull_socket)
                cleanup(fp)
                
            except Exception as e:
                # redirect to terminal
                sys.stdout = original_stdout
                sys.stderr = original_stderr
                Log(Fore.RED, f'Exception caught while testing {server}. Cleaning up...')
                with open(fname, 'a') as fp_error:
                    traceback.print_exc(file=fp_error)  # This will log the full traceback

                testError = True
                cleanup(fp)
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
        cleanup(fp)
        