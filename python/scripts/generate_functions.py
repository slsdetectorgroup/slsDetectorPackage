# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
"""
This file is used to auto generate Python bindings for the 
sls::Detector class. The tool needs the libclang bindings
to be installed. 

When the Detector API is updated this file should be run
manually
"""
from clang import cindex
import subprocess
import argparse
import sys

from parse import system_include_paths, clang_format_version

required_version = 13
RED = '\033[91m'
ENDC = '\033[0m'
if (ver := clang_format_version()) != required_version:
    print(f'{RED}Clang format version {required_version} required, detected: {ver}. Bye!{ENDC}')
    sys.exit(1)


default_build_path = "/home/l_frojdh/sls/build/"
fpath = "../../slsDetectorSoftware/src/Detector.cpp"


parser = argparse.ArgumentParser()
parser.add_argument(
    "-p",
    "--build_path",
    help="Path to the build database",
    type=str,
    default=default_build_path,
)
cargs = parser.parse_args()

db = cindex.CompilationDatabase.fromDirectory(cargs.build_path)
index = cindex.Index.create()
args = db.getCompileCommands(fpath)
args = list(iter(args).__next__().arguments)[0:-1]
args = args + "-x c++ --std=c++11".split()
syspath = system_include_paths("clang++")
incargs = ["-I" + inc for inc in syspath]
args = args + incargs


tu = index.parse(fpath, args=args)


m = []
ag = []

lines = []

ag2 = []

cn = []

def get_arguments(node):
    args = [a.type.spelling for a in node.get_arguments()]
    args = [
        "py::arg() = Positions{}" if item == "sls::Positions" else "py::arg()"
        for item in args
    ]
    args = ", ".join(args)
    if args:
        args = f", {args}"
    return args

def get_arguments_with_default(node):
    args = []
    for arg in node.get_arguments():
        tokens = [t.spelling for t in arg.get_tokens()]
        # print(tokens)
        if '=' in tokens:
            if arg.type.spelling == "sls::Positions": #TODO! automate
                args.append("py::arg() = Positions{}")
            else:
                args.append('py::arg()' + ''.join(tokens[tokens.index('='):]))
        else:
            args.append('py::arg()')
    args = ", ".join(args)
    if args:
        args = f", {args}"
    return args


def get_fdec(node):
    args = [a.type.spelling for a in node.get_arguments()]
    if node.result_type.spelling:
        return_type = node.result_type.spelling
    else:
        return_type = 'void'
    
    if node.is_const_method():
        const = 'const'
    else:
        const = ''
    args = ", ".join(args)
    args = f'({return_type}(Detector::*)({args}){const})'
    return args


def visit(node):
    if node.kind == cindex.CursorKind.CLASS_DECL:
        if node.displayname == "Detector":
            for child in node.get_children():
                if (
                    child.kind == cindex.CursorKind.CXX_METHOD
                    and child.access_specifier == cindex.AccessSpecifier.PUBLIC
                ):
                    m.append(child)
                    # args = get_arguments(child)
                    args = get_arguments_with_default(child)
                    fs = get_fdec(child)
                    lines.append(
                        f'.def("{child.spelling}",{fs} &Detector::{child.spelling}{args})'
                    )
                    print(f'&Detector::{child.spelling}{args})')
                    cn.append(child)
    for child in node.get_children():
        visit(child)


visit(tu.cursor)


with open("../src/detector_in.cpp") as f:
    data = f.read()
s = "".join(lines)
s += ";"
text = data.replace("[[FUNCTIONS]]", s)
warning = "/* WARINING This file is auto generated any edits might be overwritten without warning */\n\n"
with open("../src/detector.cpp", "w") as f:
    f.write(warning)
    f.write(text)

# run clang format on the output
subprocess.run(["clang-format", "../src/detector.cpp", "-i"])

