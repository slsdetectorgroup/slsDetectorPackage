# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
"""
Code generator for enum bindings in the Python extension. 
Reads the sls_detector_defs.h and enums_in.cpp then outputs 
enums.cpp

"""


import re
import subprocess

from parse import remove_comments, remove_ifdefs


allow_bitwise_op = ["streamingInterface", "M3_GainCaps"]

op_key = {"operator|": "|", 
          "operator&" : "&"}

def single_line_enum(line):
    sub = line[line.find('{')+1:line.find('}')]
    return sub.strip().split(',')

def extract_enums(lines):

    # deal with enum class streamingInterface : int32_t
    # and normal enum burstMode {

    line_iter = iter(lines)
    enums = {}
    for line in line_iter:
        #Hack away class enum defs
        if "class" in line:
            line = line.replace("class", "")
            line = line.replace(line[line.find(':'):line.find('{')], "")
            line = line.replace("  ", " ")

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


    #Loop again to find operators
    for key in enums:
        for line in lines:
            if key in line and "operator" in line:
                pos = line.find("operator")
                op_type = line[pos:pos+9]
                enums[key].append(op_type)
    return enums


def generate_enum_string(enums):
    data = []
    for key, value in enums.items():
        if key in allow_bitwise_op:
            tag=", py::arithmetic()"
        else:
            tag=""
        data.append(f'py::enum_<slsDetectorDefs::{key}>(Defs, "{key}"{tag})\n')
        operators = []
        for v in value:
            if "operator" not in v:
                data.append(f'\t.value("{v}", slsDetectorDefs::{key}::{v})\n')
            else:
                operators.append(v)
        data.append('\t.export_values()')

        #Here add the operators 
        for op in operators:
            data.append(f"\n\t.def(py::self {op_key[op]} slsDetectorDefs::{key}())")

        data.append(';\n\n')
    return ''.join(data)


# def remove_ifdefs(lines):
#     """Keeps C++ version of the code"""
#     out = []
#     it = iter(lines)
#     skip = False
#     for line in it:
        
#         if "#ifdef __cplusplus" in line:
#             line = next(it)

#         if "#else" in line:
#             skip = True

#         if "#endif" in line:
#             skip = False

#         if not skip and "#endif" not in line:    
#             out.append(line)
#     return out
    

with open('../../slsSupportLib/include/sls/sls_detector_defs.h') as f:
    data = f.read()

data = remove_comments(data)
data = data.splitlines()
data = remove_ifdefs(data)
enums = extract_enums(data)


s = generate_enum_string(enums)

# print(s)
# for i, line in enumerate(data):
#     print(i, line)


with open('../src/enums_in.cpp') as f:
    data = f.read()

text = data.replace('[[ENUMS]]', s)
warning = '/* WARINING This file is auto generated any edits might be overwritten without warning */\n\n'
with open('../src/enums.cpp', 'w') as f:
    f.write(warning)
    f.write(text)


# run clang format on the output
subprocess.run(['clang-format', '../src/enums.cpp', '-i'])