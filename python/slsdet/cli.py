import os
import subprocess
import sys
from importlib.resources import files



def _program_exit(exe_name: str) -> int:
    exe = files("slsdet").joinpath("data", "bin", exe_name)

    #Replace the current process with the executable, passing the command line arguments
    os.execl(str(exe), str(exe), *sys.argv[1:])

def get() -> int:
    exe_name = "sls_detector_get"
    return _program_exit(exe_name)

def put() -> int:
    exe_name = "sls_detector_put"
    return _program_exit(exe_name)

def acquire() -> int:
    exe_name = "sls_detector_acquire"
    return _program_exit(exe_name)

def receiver() -> int:
    exe_name = "slsReceiver"
    return _program_exit(exe_name)