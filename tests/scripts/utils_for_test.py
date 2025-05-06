# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
'''
This file is used for common utils used for integration tests between simulators and receivers.
'''

import sys
from enum import Enum
from colorama import Fore, Style, init

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