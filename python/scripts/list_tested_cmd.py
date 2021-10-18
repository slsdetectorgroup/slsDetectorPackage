# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
"""
Utility to find and list which command line functions have tests and
where the tests are located
"""
#local import for for parsing c++
import parse

#General python stuff
import os
import locale
import argparse
import subprocess
from pathlib import Path

#Realative path from this dir
path  = Path('../../slsDetectorSoftware/tests/')
parser = argparse.ArgumentParser()
parser.add_argument("-s", "--startswith", help="for filter", type = str, default=None)
args = parser.parse_args()

files = [f for f in os.listdir(path) if 'CmdProxy' in f]
tested = []
for fname in files:
    with open(path/fname) as f:
        data = f.read()
    data = parse.remove_comments(data)
    data = data.splitlines()
    for line in data:
        if 'TEST_CASE' in line or 'SECTION' in line:
            cmd = line.split("\"")[1]
            tested.append([cmd, fname])

out = subprocess.run(['g', 'list'], stdout = subprocess.PIPE, encoding=locale.getpreferredencoding())
all_cmd = out.stdout.splitlines()
all_cmd.pop(0)



if args.startswith is not None:
    all_cmd = [cmd for cmd in all_cmd if cmd.startswith(args.startswith)]
    tested = [cmd for cmd in tested if cmd[0].startswith(args.startswith)]

tn = [cmd[0] for cmd in tested]

not_tested = [cmd for cmd in all_cmd if cmd not in tn]
misnamed = [cmd for cmd in tn if cmd not in all_cmd]
tested = [cmd for cmd in tested if cmd[0] in all_cmd]


print("\nThe following commands are tested:")
for cmd in tested:
    print(f'{cmd[0]:>18} : {cmd[1]}')

print("\nThe following commands are NOT tested:")
for cmd in not_tested:
    print(cmd)

print(f"\nThe following {len(misnamed)} tests does not match commands and might be misnamed:")
for cmd in misnamed:
    print(cmd)
print(f'\nTests cover {len(tested)} of {len(all_cmd)} commands')