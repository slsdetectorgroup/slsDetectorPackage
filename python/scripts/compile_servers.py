# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
import subprocess
import os
import sys
from pathlib import Path
import shutil as sh
from argparse import ArgumentParser

class color:
    HEADER = "\033[95m"
    BLUE = "\033[94m"
    CYAN = "\033[96m"
    GREEN = "\033[92m"
    YELLOW = "\033[93m"
    RED = "\033[91m"
    ENDC = "\033[0m"
    BOLD = "\033[1m"
    UNDERLINE = "\033[4m"
    MAGENTA = "\033[35m"

    @staticmethod
    def red(s):
        return f"{color.RED}{s}{color.ENDC}"

    @staticmethod
    def green(s):
        return f"{color.GREEN}{s}{color.ENDC}"

def add_to_path():
    paths = [
        "/opt/uClinux/bfin-uclinux/bin",
        "/opt/nios2-gcc/bin",
        "/opt/eldk-5.1/powerpc-4xx-softfloat/sysroots/i686-eldk-linux/usr/bin/ppc405-linux",
    ]
    os.environ["PATH"] += os.pathsep + os.pathsep.join(paths)


def rc_to_string(rc):
    if rc == 0:
        return color.green("OK")
    else:
        return color.red("FAIL")



parser = ArgumentParser()
parser.add_argument('-t', '--tag', help = 'Tag added to server file name', default='developer')
parser.add_argument('-g', '--git', help='Add new servers to the git repo', action="store_true")
args = parser.parse_args()


servers = [
#     "eigerDetectorServer",
#     "jungfrauDetectorServer",
      "mythen3DetectorServer",
#     "gotthard2DetectorServer",
#     "gotthardDetectorServer",
#     "ctbDetectorServer",
#     "moenchDetectorServer",
]


server_root = Path("../../slsDetectorServers/").resolve()

add_to_path()
for server in servers:
    bin_name = f"{server}_{args.tag}"
    path = server_root / server
    print(f"{bin_name} - ", end="")
    os.chdir(path)
    try: 
        sh.rmtree(path/'bin')
    except FileNotFoundError:
        pass
    p = subprocess.run(["make"], stdout=subprocess.DEVNULL)
    print(rc_to_string(p.returncode))
    if p.returncode == 0:
        sh.move(f"bin/{server}", f"bin/{bin_name}")
        if args.git:
            print("Adding to git")
            subprocess.run(['git', 'add', 'bin', '-f'])
