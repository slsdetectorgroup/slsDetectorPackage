# import clang.cindex
from clang import cindex
import subprocess

path = "/home/l_frojdh/sls/build/"
fpath = "/home/l_frojdh/sls/slsDetectorPackage/slsDetectorSoftware/src/Detector.cpp"
db = cindex.CompilationDatabase.fromDirectory(path)
index = cindex.Index.create()
args = db.getCompileCommands(fpath)
args = list(iter(args).__next__().arguments)[0:-1]


args = args + "-x c++ --std=c++11".split()

syspath = [
    "/usr/bin/../lib/gcc/x86_64-redhat-linux/9/../../../../include/c++/9",
    "/usr/bin/../lib/gcc/x86_64-redhat-linux/9/../../../../include/c++/9/x86_64-redhat-linux",
    "/usr/bin/../lib/gcc/x86_64-redhat-linux/9/../../../../include/c++/9/backward",
    "/usr/local/include",
    "/usr/lib64/clang/9.0.0/include",
    "/usr/include",
]
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
    return args


def visit(node):
    if node.kind == cindex.CursorKind.CLASS_DECL:
        if node.displayname == "Detector":
            for child in node.get_children():
                if (
                    child.kind == cindex.CursorKind.CXX_METHOD
                    and cindex.AccessSpecifier.PUBLIC
                ):
                    m.append(child)
                    args = get_arguments(child)
                    lines.append(f'.def({child.spelling}, &Detector::{child.spelling}, {args})')
    for child in node.get_children():
        visit(child)


visit(tu.cursor)


with open('../src/detector_in.cpp') as f:
    data = f.read()
s = ''.join(lines)
text = data.replace('[[FUNCTIONS]]', s)
warning = '/* WARINING This file is auto generated any edits might be overwritten without warning */\n\n'
with open('../src/detector.cpp', 'w') as f:
    f.write(warning)
    f.write(text)

# run clang format on the output
subprocess.run(['clang-format', '../src/detector.cpp', '-i'])