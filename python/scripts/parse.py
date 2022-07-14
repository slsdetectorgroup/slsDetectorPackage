# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
import re
import subprocess
from subprocess import PIPE
import os

def clang_format_version():
    p = subprocess.run(['clang-format', '--version'], capture_output = True)
    ver = p.stdout.decode().split()[2]
    major = int(ver.split('.')[0])
    return major


def remove_comments(text):
    def replacer(match):
        s = match.group(0)
        if s.startswith('/'):
            return " " # note: a space and not an empty string
        else:
            return s
    pattern = re.compile(
        r'//.*?$|/\*.*?\*/|\'(?:\\.|[^\\\'])*\'|"(?:\\.|[^\\"])*"',
        re.DOTALL | re.MULTILINE
    )
    return re.sub(pattern, replacer, text)

def remove_ifdefs(lines):
    """Keeps C++ version of the code"""
    out = []
    it = iter(lines)
    skip = False
    for line in it:
        
        if "#ifdef __cplusplus" in line:
            line = next(it)

        if "#else" in line:
            skip = True

        if "#endif" in line:
            skip = False

        if not skip and "#endif" not in line:    
            out.append(line)
    return out


#based on ccsyspath: https://github.com/AndrewWalker/ccsyspath

def compiler_preprocessor_verbose(compiler, extraflags):
    """Capture the compiler preprocessor stage in verbose mode
    """
    lines = []
    with open(os.devnull, 'r', encoding='utf-8') as devnull:
        cmd = [compiler, '-E'] 
        cmd += extraflags
        cmd += ['-', '-v']
        p = subprocess.Popen(cmd, stdin=devnull, stdout=PIPE, stderr=PIPE)
        p.wait()
        lines = p.stderr.read()
        lines = lines.splitlines()
    return lines
    
def system_include_paths(compiler, cpp=True):
    extraflags = []
    if cpp:
        extraflags = b'-x c++'.split()
    lines = compiler_preprocessor_verbose(compiler, extraflags)
    lines = [ line.strip() for line in lines ]

    start = lines.index(b'#include <...> search starts here:')
    end   = lines.index(b'End of search list.')

    lines = lines[start+1:end]
    paths = []
    for line in lines:
        line = line.replace(b'(framework directory)', b'')
        line = line.strip()
        paths.append(line)
    paths = [p.decode('utf-8') for p in paths]
    return paths 