import parse
from pathlib import Path
import os
import locale
import argparse
path  = Path('../../slsDetectorSoftware/tests/')
import subprocess

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
        if 'TEST_CASE' in line:
            cmd = line.split("\"")[1]
            tested.append(cmd)

out = subprocess.run(['g', 'list'], stdout = subprocess.PIPE, encoding=locale.getpreferredencoding())
all_cmd = out.stdout.splitlines()
all_cmd.pop(0)



if args.startswith is not None:
    all_cmd = [cmd for cmd in all_cmd if cmd.startswith(args.startswith)]
    tested = [cmd for cmd in tested if cmd.startswith(args.startswith)]



not_tested = []
misnamed = []
for cmd in all_cmd:
    if cmd not in tested:
        not_tested.append(cmd)

for cmd in tested:
    if cmd not in all_cmd:
        misnamed.append(cmd)

print("\nThe following commands are tested:")
for cmd in tested:
    print(cmd)

print("\nThe following commands are NOT tested:")
for cmd in not_tested:
    print(cmd)

print(f"\nThe following {len(misnamed)} tests are misnamed and should be renamed:")
for cmd in misnamed:
    print(cmd)
print(f'\nTests cover {len(tested)} of {len(all_cmd)} commands')