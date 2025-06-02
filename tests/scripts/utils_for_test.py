# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
'''
This file is used for common utils used for integration tests between simulators and receivers.
'''

import sys, subprocess, time, argparse
from enum import Enum
from colorama import Fore, Style, init

from slsdet import Detector, detectorSettings
from slsdet.defines import DEFAULT_TCP_RX_PORTNO, DEFAULT_UDP_DST_PORTNO
SERVER_START_PORTNO=1900

init(autoreset=True)


class LogLevel(Enum):
    INFO = 0
    INFORED = 1
    INFOGREEN = 2
    INFOBLUE = 3
    WARNING = 4
    ERROR = 5
    DEBUG = 6


LOG_LABELS = {
    LogLevel.WARNING: "WARNING: ",
    LogLevel.ERROR: "ERROR: ",
    LogLevel.DEBUG: "DEBUG: "
}


LOG_COLORS = {
    LogLevel.INFO: Fore.WHITE,
    LogLevel.INFORED: Fore.RED,
    LogLevel.INFOGREEN: Fore.GREEN,
    LogLevel.INFOBLUE: Fore.BLUE,
    LogLevel.WARNING: Fore.YELLOW,
    LogLevel.ERROR: Fore.RED,
    LogLevel.DEBUG: Fore.CYAN
}


def Log(level: LogLevel, message: str, stream=sys.stdout):
    color = LOG_COLORS.get(level, Fore.WHITE)
    label = LOG_LABELS.get(level, "")
    print(f"{color}{label}{message}{Style.RESET_ALL}", file=stream, flush=True)


class RuntimeException (Exception):
    def __init__ (self, message):
        Log(LogLevel.ERROR, message)
        super().__init__(message)


def checkIfProcessRunning(processName):
    cmd = f"pgrep -f {processName}"
    res = subprocess.getoutput(cmd)
    return res.strip().splitlines()


def killProcess(name, fp):
    pids = checkIfProcessRunning(name)
    if pids:
        Log(LogLevel.INFO, f"Killing '{name}' processes with PIDs: {', '.join(pids)}", fp)
        for pid in pids:
            try:
                p = subprocess.run(['kill', pid])
                if p.returncode != 0 and bool(checkIfProcessRunning(name)):
                    raise RuntimeException(f"Could not kill {name} with pid {pid}")
            except Exception as e:
                raise RuntimeException(f"Failed to kill process {name} pid:{pid}. Error: {str(e)}") from e
    #else:
    #    Log(LogLevel.INFO, 'process not running : ' + name)


def cleanSharedmemory(fp):
    Log(LogLevel.INFO, 'Cleaning up shared memory', fp)
    try:
        p = subprocess.run(['sls_detector_get', 'free'], stdout=fp, stderr=fp)
    except:
        raise RuntimeException('Could not free shared memory')


def cleanup(fp):
    Log(LogLevel.INFO, 'Cleaning up')
    Log(LogLevel.INFO, 'Cleaning up', fp)
    killProcess('DetectorServer_virtual', fp)
    killProcess('slsReceiver', fp)
    killProcess('slsMultiReceiver', fp)
    killProcess('slsFrameSynchronizer', fp)
    killProcess('frameSynchronizerPullSocket', fp)
    cleanSharedmemory(fp)


def startProcessInBackground(cmd, fp):
    Log(LogLevel.INFO, 'Starting up ' + ' '.join(cmd))
    Log(LogLevel.INFO, 'Starting up ' + ' '.join(cmd), fp)
    try:
        p = subprocess.Popen(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, restore_signals=False) 
    except Exception as e:
        raise RuntimeException(f'Failed to start {cmd}:{str(e)}') from e


def startProcessInBackgroundWithLogFile(cmd, fp, log_file_name: str):
    Log(LogLevel.INFOBLUE, 'Starting up ' +  ' '.join(cmd) + '. Log: ' +  log_file_name)
    Log(LogLevel.INFOBLUE, 'Starting up ' +  ' '.join(cmd) + '. Log: ' +  log_file_name, fp)
    try:
        with open(log_file_name, 'w') as log_fp:
            subprocess.Popen(cmd, stdout=log_fp, stderr=log_fp, text=True)
    except Exception as e:
        raise RuntimeException(f'Failed to start {cmd}:{str(e)}') from e


def checkLogForErrors(fp, log_file_path: str):
    try:
        with open(log_file_path, 'r') as log_file:
            for line in log_file:
                if 'Error' in line:
                    Log(LogLevel.ERROR, f"Error found in log: {line.strip()}")
                    Log(LogLevel.ERROR, f"Error found in log: {line.strip()}", fp)
                    raise RuntimeException("Error found in log file")
    except FileNotFoundError:
        print(f"Log file not found: {log_file_path}")
        raise
    except Exception as e:
        print(f"Exception while reading log: {e}")
        raise


def runProcessWithLogFile(name, cmd, fp, log_file_name):
    Log(LogLevel.INFOBLUE, 'Running ' +  name + '. Log: ' +  log_file_name)
    Log(LogLevel.INFOBLUE, 'Running ' +  name + '. Log: ' +  log_file_name, fp)
    Log(LogLevel.INFOBLUE, 'Cmd: ' + ' '.join(cmd), fp)
    try:
        with open(log_file_name, 'w') as log_fp:
            subprocess.run(cmd, stdout=log_fp, stderr=log_fp, check=True, text=True)
    except subprocess.CalledProcessError as e:
        pass    
    except Exception as e:
        Log(LogLevel.ERROR, f'Failed to run {name}:{str(e)}', fp)
        raise RuntimeException(f'Failed to run {name}:{str(e)}')
    
    with open (log_file_name, 'r') as f:
        for line in f:
            if "FAILED" in line:
                raise RuntimeException(f'{line}')

    Log(LogLevel.INFOGREEN, name + ' successful!\n')
    Log(LogLevel.INFOGREEN, name + ' successful!\n', fp)


def startDetectorVirtualServer(name :str, num_mods, fp):
    for i in range(num_mods):
        port_no = SERVER_START_PORTNO + (i * 2)
        cmd = [name + 'DetectorServer_virtual', '-p', str(port_no)]
        if name == 'gotthard':
            cmd += ['-m', '1']
        startProcessInBackgroundWithLogFile(cmd, fp, "/tmp/virtual_det_" + name + str(i) + ".txt")
        match name:
            case 'jungfrau':
                time.sleep(7)
            case 'gotthard2':
                time.sleep(5)
            case _:
                time.sleep(3)


def connectToVirtualServers(name, num_mods):
    try:
        d = Detector()
    except Exception as e:
        raise RuntimeException(f'Could not create Detector object for {name}. Error: {str(e)}') from e

    counts_sec = 5
    while (counts_sec != 0):
        try:
            d.virtual = [num_mods, SERVER_START_PORTNO]
            break
        except Exception as e:
            # stop server still not up, wait a bit longer
            if "Cannot connect to" in str(e):
                Log(LogLevel.WARNING, f'Still waiting for {name} virtual server to be up...{counts_sec}s left')
                time.sleep(1)
                counts_sec -= 1
            else:
                raise

    return d


def loadConfig(name, rx_hostname, settingsdir, fp, num_mods = 1, num_frames = 1):
    Log(LogLevel.INFO, 'Loading config')
    Log(LogLevel.INFO, 'Loading config', fp)
    try:
        d = connectToVirtualServers(name, num_mods)
        d.udp_dstport = DEFAULT_UDP_DST_PORTNO
        if name == 'eiger':
            d.udp_dstport2 = DEFAULT_UDP_DST_PORTNO + 1

        d.rx_hostname = rx_hostname
        d.udp_dstip = 'auto'
        if name != "eiger":
            if name == "gotthard":
                d.udp_srcip = d.udp_dstip
            else:
                d.udp_srcip = 'auto'

        if name == "jungfrau" or name == "moench" or name == "xilinx_ctb":
            d.powerchip = 1

        if name == "xilinx_ctb":
            d.configureTransceiver()

        if name == "eiger":
            d.trimen = [4500, 5400, 6400]
            d.settingspath = settingsdir + '/eiger/'
            d.setThresholdEnergy(4500, detectorSettings.STANDARD)

        d.frames = num_frames
    except Exception as e:
        raise RuntimeException(f'Could not load config for {name}. Error: {str(e)}') from e
    
    return d


def ParseArguments(description, default_num_mods=1):
    parser = argparse.ArgumentParser(description)

    parser.add_argument('rx_hostname', nargs='?', default='localhost',
                        help='Hostname/IP of the current machine')
    parser.add_argument('settingspath', nargs='?', default='../../settingsdir',
                        help='Relative or absolute path to the settings directory')
    parser.add_argument('-n', '--num-mods', nargs='?', default=default_num_mods, type=int,
                        help='Number of modules to test with')
    parser.add_argument('-f', '--num-frames', nargs='?', default=1, type=int,
                        help='Number of frames to test with')
    parser.add_argument('-s', '--servers', nargs='*',
                        help='Detector servers to run')

    args = parser.parse_args()

    # Set default server list if not provided
    if args.servers is None:
        args.servers = [
            'eiger',
            'jungfrau',
            'mythen3',
            'gotthard2',
            'gotthard',
            'ctb',
            'moench',
            'xilinx_ctb'
        ]

    Log(LogLevel.INFO, 'Arguments:\n' + 
        'rx_hostname: ' + args.rx_hostname + 
        '\nsettingspath: \'' + args.settingspath + 
        '\nservers: \'' + ' '.join(args.servers) + 
        '\nnum_mods: \'' + str(args.num_mods) + 
        '\nnum_frames: \'' + str(args.num_frames) + '\'') 

    return args   
