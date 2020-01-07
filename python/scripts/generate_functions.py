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


from parse import system_include_paths

default_build_path = "/home/l_frojdh/sls/build/" 
fpath = "../../slsDetectorSoftware/src/Detector.cpp"


parser = argparse.ArgumentParser()
parser.add_argument("-p", "--build_path", help="Path to the build database", type = str, default=default_build_path)
cargs = parser.parse_args()

db = cindex.CompilationDatabase.fromDirectory(cargs.build_path)
index = cindex.Index.create()
args = db.getCompileCommands(fpath)
args = list(iter(args).__next__().arguments)[0:-1]
args = args + "-x c++ --std=c++11".split()
syspath = system_include_paths('clang++')
incargs = ["-I" + inc for inc in syspath]
args = args + incargs


tu = index.parse(fpath, args=args)


m = []
ag = []

lines = []


def get_arguments(node):
    args = [a.type.spelling for a in node.get_arguments()]
    args = [
        "py::arg() = Positions{}" if item == "sls::Positions" else "py::arg()"
        for item in args
    ]
    args = ', '.join(args)
    if args:
        args = f', {args}'
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
                    args = get_arguments(child)
                    lines.append(f'.def(\"{child.spelling}\", &Detector::{child.spelling}{args})')
    for child in node.get_children():
        visit(child)


visit(tu.cursor)


with open('../src/detector_in.cpp') as f:
    data = f.read()
s = ''.join(lines)
s += ';'
text = data.replace('[[FUNCTIONS]]', s)
warning = '/* WARINING This file is auto generated any edits might be overwritten without warning */\n\n'
with open('../src/detector.cpp', 'w') as f:
    f.write(warning)
    f.write(text)

# run clang format on the output
subprocess.run(['clang-format', '../src/detector.cpp', '-i'])