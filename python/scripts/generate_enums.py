"""
Code generator for enum bindings in the Python extension. 
Reads the sls_detector_defs.h and enums_in.cpp then outputs 
enums.cpp

"""


import re
import subprocess

from parse import remove_comments

def single_line_enum(line):
    sub = line[line.find('{')+1:line.find('}')]
    return sub.strip().split(',')

def extract_enums(lines):
    line_iter = iter(lines)
    enums = {}
    for line in line_iter:
        m = re.search("(?<=enum )\w+(?= {)", line)
        if m:
            enum_name = m.group()
            print(enum_name)
            # print(line)
            fields = []

            #deal with single line enums
            if '};' in line:
                fields = single_line_enum(line)
            else:
                #deal with multi line enums
                while True:
                    l  = next(line_iter)
                    if '};' in l:
                        break
                    m = re.search("\w+", l)
                    try:
                        # print('\t', m.group())
                        fields.append(m.group())

                    except:
                        pass
            fields = [f.strip() for f in fields]
            enums[enum_name] = fields
    return enums

def generate_enum_string(enums):
    data = []
    for key, value in enums.items():
        data.append(f'py::enum_<slsDetectorDefs::{key}>(Defs, "{key}")\n')
        for v in value:
            data.append(f'\t.value("{v}", slsDetectorDefs::{key}::{v})\n')
        data.append('.export_values();\n\n')
    return ''.join(data)

with open('../../slsSupportLib/include/sls/sls_detector_defs.h') as f:
    data = f.read()

data = remove_comments(data)
data = data.splitlines()
enums = extract_enums(data)
s = generate_enum_string(enums)

with open('../src/enums_in.cpp') as f:
    data = f.read()

text = data.replace('[[ENUMS]]', s)
warning = '/* WARINING This file is auto generated any edits might be overwritten without warning */\n\n'
with open('../src/enums.cpp', 'w') as f:
    f.write(warning)
    f.write(text)


# run clang format on the output
subprocess.run(['clang-format', '../src/enums.cpp', '-i'])