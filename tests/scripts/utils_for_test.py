# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
'''
This file is used for common utils used for integration tests between simulators and receivers.
'''
import os
from pathlib import Path
import sys, subprocess, time, argparse
from enum import Enum
from colorama import Fore, Style, init
from datetime import timedelta
from contextlib import contextmanager

from slsdet import Detector, Ctb, detectorSettings, burstMode
from slsdet.defines import DEFAULT_UDP_DST_PORTNO
DET_START_TCP_PORTNO=1900
RX_START_TCP_PORTNO=2000
START_ZMQ_PORTNO=8000

LOG_PREFIX_FNAME = "/tmp/slsDetectorPackage_"

init(autoreset=True)

build_dir = Path(__file__).resolve().parents[2] / "build" / "bin"

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


def Log(level: LogLevel, message: str, stream=sys.stdout, both=False):
    color = LOG_COLORS.get(level, Fore.WHITE)
    label = LOG_LABELS.get(level, "")
    print(f"{color}{label}{message}{Style.RESET_ALL}", file=stream, flush=True)
    if both and stream != sys.stdout:
        print(f"{color}{label}{message}{Style.RESET_ALL}", file=sys.stdout, flush=True)



class RuntimeException (Exception):
    def __init__ (self, message):
        Log(LogLevel.ERROR, message)
        super().__init__(message)


@contextmanager
def optional_file(file_path=None, mode='w', quiet_mode=False):
    if file_path:
        f = open(file_path, mode)
        try:
            yield f
        finally:
            f.close()
    else:
        if quiet_mode:
            f = open(os.devnull, mode)
            try:
                yield f
            finally:
                f.close()
        else:
            yield sys.stdout


def checkIfProcessRunning(processName):
    res = subprocess.getoutput(f"pgrep -f {processName}")
    return res.strip().splitlines()


def killProcess(name, fp):
    '''
    Kill all processes matching name.
    Does not fail if process is already gone.
    '''
    Log(LogLevel.INFO, f"Attempting to kill '{name}' (if running)", fp)
    # pkill returns:
    # 0 -> process killed
    # 1 -> no process found OK
    subprocess.run(['pkill', '-f', name],
                   stdout=subprocess.DEVNULL,
                   stderr=subprocess.DEVNULL)


def cleanSharedmemory(fp):
    Log(LogLevel.INFO, 'Cleaning up shared memory', fp)
    try:
        p = subprocess.run([build_dir / 'sls_detector_get', 'free'], stdout=fp, stderr=fp)
    except Exception as e:
        raise RuntimeException(f'Could not free shared memory: {str(e)}')


def cleanup(fp):
    Log(LogLevel.INFO, 'Cleaning up', fp, True)
    killProcess('DetectorServer_virtual', fp)
    killProcess('slsReceiver', fp)
    killProcess('slsMultiReceiver', fp)
    killProcess('slsFrameSynchronizer', fp)
    killProcess('frameSynchronizerPullSocket', fp)
    cleanSharedmemory(fp)


def startProcessInBackground(cmd, fp, log_file_name: str, quiet_mode=False):
    info_text = 'Starting up ' + ' '.join(cmd)
    if log_file_name:
        info_text += '. Log: ' +  log_file_name

    Log(LogLevel.INFO, f'{info_text}', fp, True)

    try:
        with optional_file(log_file_name, 'w', quiet_mode) as log_fp:
            subprocess.Popen(cmd, stdout=log_fp, stderr=log_fp, text=True)
    except Exception as e:
        raise RuntimeException(f'Failed to start {cmd}:{str(e)}') from e


def checkLogForErrors(fp, log_file_path: str):
    try:
        with open(log_file_path, 'r') as log_file:
            for line in log_file:
                if 'Error' in line:
                    Log(LogLevel.ERROR, f"Error found in log: {line.strip()}", fp, True)
                    raise RuntimeException("Error found in log file")
    except FileNotFoundError:
        print(f"Log file not found: {log_file_path}")
        raise
    except Exception as e:
        print(f"Exception while reading log: {e}")
        raise


def checkLogForErrorsOrSummary(fp, lines, source_name=""):
    failed = False # if it found "failed" or "FAILED" in file
    failed_msg = ""
    printing_error = False # print every line in file after failure
    printing_summary = False # print summary if no failure


    for line in lines:
        line_stripped = line.rstrip()

        # Detect failure (case-insensitive)
        if not failed and (": FAILED:" in line or " failed\nassertions" in line):
            failed = True
            failed_msg = line_stripped
            printing_error = True
            Log(LogLevel.ERROR, line_stripped, fp)
            if source_name:
                Log(LogLevel.ERROR, f"Error log from file: {source_name}")
            Log(LogLevel.ERROR, "="*79)

        # After failure, log everything as ERROR
        if printing_error:
            print(f"{line_stripped}")
            continue

        # Summary delimiter
        if line_stripped.startswith("====="):
            printing_summary = True

        # No failure - print summary lines 
        if printing_summary:
            print(f"{line_stripped}")


    if failed:
        Log(LogLevel.ERROR, "="*79)
        raise RuntimeException(f'Test failed: {failed_msg}')
    else:
        print("="*79)



def runProcess(name, cmd, fp, log_file_name = None, quiet_mode=False):
    
    info_text = 'Running ' +  name + '.'
    if log_file_name:
        info_text += ' Log: ' +  log_file_name
    Log(LogLevel.INFOBLUE, info_text, fp, True)
    Log(LogLevel.INFOBLUE, 'Cmd: ' + ' '.join(cmd), fp, True)

    error_log = None

    try:
        if log_file_name:
            with optional_file(log_file_name, 'w', quiet_mode) as log_fp:
                subprocess.run(cmd, stdout=log_fp, stderr=log_fp, check=True, text=True)
        else:
            capture = subprocess.run(cmd, check=True, text=True, capture_output=True)
            captured_log = capture.stdout.splitlines()

    except subprocess.CalledProcessError as e:
        print("error: ", str(e))
        if not log_file_name and e.stdout:
            captured_log = e.stdout.splitlines()
        pass
    except Exception as e:
        print("something else failed")
        Log(LogLevel.ERROR, f'Failed to run {name}:{str(e)}', fp)
        raise RuntimeException(f'Failed to run {name}:{str(e)}')
    
    if log_file_name:
        with optional_file(log_file_name, 'r') as log_fp:
            checkLogForErrorsOrSummary(fp, log_fp, log_file_name)
    else:
        checkLogForErrorsOrSummary(fp, captured_log)

    Log(LogLevel.INFOGREEN, name + ' successful!\n', fp, True)


def startDetectorVirtualServer(name :str, num_mods, fp, no_log_file = False, quiet_mode=False):
    for i in range(num_mods):
        port_no = DET_START_TCP_PORTNO + (i * 2)
        cmd = [str(build_dir / (name + 'DetectorServer_virtual')), '-p', str(port_no)]
        fname = LOG_PREFIX_FNAME + "virtual_det_" + name + "_" + str(DET_START_TCP_PORTNO) + ".txt"
        if no_log_file: 
            fname = None
        startProcessInBackground(cmd, fp, fname, quiet_mode)
        match name:
            case 'jungfrau':
                time.sleep(7)
            case 'gotthard2':
                time.sleep(5)
            case _:
                time.sleep(3)


def connectToVirtualServers(name, num_mods, ctb_object=False):
    try:
        if ctb_object:
            d = Ctb()
        else:
            d = Detector()
    except Exception as e:
        raise RuntimeException(f'Could not create Detector object for {name}. Error: {str(e)}') from e

    counts_sec = 5
    while (counts_sec != 0):
        try:
            d.virtual = [num_mods, DET_START_TCP_PORTNO] # sets the hostnames 
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

def startReceiver(num_mods, fp, no_log_file = False, quiet_mode=False):
    if num_mods == 1:
        cmd = [str(build_dir / 'slsReceiver'), '-p', str(RX_START_TCP_PORTNO)]
        fname = LOG_PREFIX_FNAME + "slsReceiver.txt"
    else:
        cmd = [str(build_dir / 'slsMultiReceiver'), '-p', str(RX_START_TCP_PORTNO), '-n', str(num_mods)]
        fname = LOG_PREFIX_FNAME + "slsMultiReceiver.txt"

    if no_log_file: 
        fname = None
    startProcessInBackground(cmd, fp, fname, quiet_mode)
    time.sleep(1)

def loadConfig(name, rx_hostname = 'localhost', settingsdir = None, log_file_fp = None, num_mods = 1, num_frames = 1, num_interfaces = 1):
    Log(LogLevel.INFO, 'Loading config', log_file_fp, True)
    try:
        if name == 'ctb' or name == 'xilinx_ctb':
            d = connectToVirtualServers(name, num_mods, ctb_object=True)
        else: 
            d = connectToVirtualServers(name, num_mods)

        if name == 'jungfrau' or name == 'moench':
            d.numinterfaces = num_interfaces

        d.udp_dstport = DEFAULT_UDP_DST_PORTNO
        if d.numinterfaces == 2: 
            d.udp_dstport2 = DEFAULT_UDP_DST_PORTNO + 1

        d.rx_tcpport = RX_START_TCP_PORTNO
        d.rx_hostname = rx_hostname
        d.udp_dstip = 'auto'
        if name != "eiger":
            d.udp_srcip = 'auto'

        if num_interfaces == 2: 
            d.udp_dstip2 = 'auto'

        d.zmqport = START_ZMQ_PORTNO
        d.rx_zmqport = START_ZMQ_PORTNO

        if name == "jungfrau" or name == "moench" or name == "xilinx_ctb":
            d.powerchip = 1

        if name == "xilinx_ctb":
            d.configureTransceiver()

        if settingsdir is not None and name in ['eiger', 'mythen3']: 
            d.settingspath = settingsdir + '/' + name + '/'
            d.trimen = [4500, 5400, 6400] if name == 'eiger' else [4000, 6000, 8000, 12000]
            d.setThresholdEnergy(4500, detectorSettings.STANDARD) 
        

        d.frames = num_frames
      
    except Exception as e:
        raise RuntimeException(f'Could not load config for {name}. Error: {str(e)}') from e
    
    return d



# for easy acquire
def loadBasicSettings(name, d, fp):
    Log(LogLevel.INFO, 'Loading basic settings for ' + name, fp, True)
    try:
        # basic settings for easy acquire
        if name == "jungfrau":
            d.exptime = timedelta(microseconds = 200)
            d.readnrows = 512
        elif name == "moench":
            d.exptime = timedelta(microseconds = 200)
            d.readnrows = 400
        elif name == "eiger":
            d.exptime = timedelta(microseconds = 200)
            d.readnrows = 256
            d.dr = 16
        elif name == "mythen3":
            d.setExptime(-1, timedelta(microseconds = 200))
            d.dr = 16
            d.counters = [0, 1, 2]
        elif name == "gotthard2":
            d.exptime = timedelta(microseconds = 200)
            d.burstmode = burstMode.CONTINUOUS_EXTERNAL
            d.bursts = 1
            d.burstperiod = 0
        d.period = timedelta(milliseconds = 2)

    except Exception as e:
        raise RuntimeException(f'Could not load config for {name}. Error: {str(e)}') from e
    
def ParseArguments(description, default_num_mods=2, specific_tests=False, general_tests_option=False):
    parser = argparse.ArgumentParser(description)

    default_settings_path = Path(__file__).resolve().parents[2] / "settingsdir"

    parser.add_argument('rx_hostname', nargs='?', default='localhost',
                        help='Hostname/IP of the current machine')
    parser.add_argument('settingspath', nargs='?', default=str(default_settings_path),
                        help='Relative or absolute path to the settings directory')
    parser.add_argument('-n', '--num-mods', nargs='?', default=default_num_mods, type=int,
                        help='Number of modules to test with')
    parser.add_argument('-f', '--num-frames', nargs='?', default=1, type=int,
                        help='Number of frames to test with')
    parser.add_argument('-s', '--servers', nargs='*',
                        help='Detector servers to run')
    parser.add_argument('-nlf', '--no-log-file', action='store_true',
                        help='Dont write output to log file')
    parser.add_argument('-q', '--quiet', action='store_true',
                        help='Dont write to stdout when possible.')   
    if specific_tests:
        parser.add_argument('-t', '--tests', nargs='?', default ='[.detectorintegration]',
                        help = 'Test markers or specific test name to use for tests, default: [.detectorintegration]')
        
    if general_tests_option:
        parser.add_argument('-g', '--general-tests', action='store_true',
                        help = 'Enable general tests (no value needed)')
        
    args = parser.parse_args()


    # Set default server list if not provided
    if args.servers is None:
        args.servers = [
            'eiger',
            'jungfrau',
            'mythen3',
            'gotthard2',
            'ctb',
            'moench',
            'xilinx_ctb'
        ]

    msg = (
        'Arguments:\n'
        f'rx_hostname: {args.rx_hostname}\n'
        f"settingspath: '{args.settingspath}'\n"
        f"servers: '{' '.join(args.servers)}'\n"
        f"num_mods: '{args.num_mods}'\n"
        f"num_frames: '{args.num_frames}'"
    )

    if args.no_log_file:
        msg += f"\nLog File: Disabled"
    else:
        msg += f"\nLog File: Enabled"

    if args.quiet:
        msg += f"\nQuiet mode: Enabled"
    else:
        msg += f"\nQuiet mode: Disabled"

    if specific_tests:
        msg += f"\ntests: '{args.tests}'"

    if general_tests_option:
        msg += f"\ngeneral_tests: '{args.general_tests}'"

    Log(LogLevel.INFO, msg)


    return args   
