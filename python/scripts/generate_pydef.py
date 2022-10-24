

import re
import subprocess

from parse import remove_comments, remove_ifdefs

with open('../../slsSupportLib/include/sls/sls_detector_defs.h') as f:
    data = f.read()

data = remove_comments(data)
data = data.splitlines()
# data = remove_ifdefs(data)


ignore = ['#define MYROOT', '#define __cplusplus']

defines = {}
for i, line in enumerate(data):
    if line.startswith('#define') and line not in ignore:
        _, name, value = line.split(maxsplit = 2)
        print(f'{name}={value}')
        defines[name]=value


warning = '#WARINING This file is auto generated any edits might be overwritten without warning\n\n'
with open('../slsdet/defines.py', 'w') as f:
    f.write(warning)
    for key, value in defines.items():
        f.write(f'{key}={value}\n')