import subprocess
import locale
out = subprocess.run(['g', 'list'], stdout = subprocess.PIPE, encoding=locale.getpreferredencoding())
cmd = out.stdout.splitlines()
cmd.pop(0)

from sls_detector import Detector, Eiger, Ctb

pycmd = dir(Detector)+dir(Eiger)+dir(Ctb)

for c in cmd:
    if c not in pycmd:
        print(c)