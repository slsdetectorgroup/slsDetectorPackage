# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
"""
This file is used to auto generate Python bindings for the 
sls::Detector class. The tool needs the libclang bindings
to be installed. 

When the Detector API is updated this file should be run
manually.
"""
from clang import cindex
import subprocess
import argparse
import sys
import time
from pathlib import Path
from parse import system_include_paths, clang_format_version

REDC = "\033[91m"
GREENC = "\033[92m"
ENDC = "\033[0m"


def red(msg):
    return f"{REDC}{msg}{ENDC}"


def green(msg):
    return f"{GREENC}{msg}{ENDC}"


def check_clang_format_version(required_version):
    if (ver := clang_format_version()) != required_version:
        msg = red(
            f"Clang format version {required_version} required, detected: {ver}. Bye!"
        )
        print(msg)
        sys.exit(1)
    else:
        msg = green(f"Found clang-format version {ver}")
        print(msg)


def check_for_compile_commands_json(path):
    # print(f"Looking for compile data base in: {path}")
    compile_data_base_file = path / "compile_commands.json"
    if not compile_data_base_file.exists():
        msg = red(f"No compile_commands.json file found in {path}. Bye!")
        print(msg)
        sys.exit(1)
    else:
        msg = green(f"Found: {compile_data_base_file}")
        print(msg)


default_build_path = "/home/l_frojdh/sls/build/"
fpath = "../../slsDetectorSoftware/src/Detector.cpp"


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
        if "=" in tokens:
            if arg.type.spelling == "sls::Positions":  # TODO! automate
                args.append("py::arg() = Positions{}")
            else:
                args.append("py::arg()" + "".join(tokens[tokens.index("=") :]))
        else:
            args.append("py::arg()")
    args = ", ".join(args)
    if args:
        args = f", {args}"
    return args


def get_fdec(node):
    args = [a.type.spelling for a in node.get_arguments()]
    if node.result_type.spelling:
        return_type = node.result_type.spelling
    else:
        return_type = "void"

    if node.is_const_method():
        const = "const"
    else:
        const = ""
    args = ", ".join(args)
    args = f"({return_type}(Detector::*)({args}){const})"
    return args


def time_return_lambda(node, args):
    names = ['a', 'b', 'c', 'd']
    fa = [a.type.spelling for a in node.get_arguments()]
    ca = ','.join(f'{arg} {n}' for arg, n in zip(fa, names))
    na = ','.join(names[0:len(fa)])
    s = f'CppDetectorApi.def("{node.spelling}",[](sls::Detector& self, {ca}){{ auto r = self.{node.spelling}({na}); \n return std::vector<sls::Duration>(r.begin(), r.end()); }}{args});'
    return s


def visit(node):
    if node.kind == cindex.CursorKind.CLASS_DECL:
        if node.displayname == "Detector":
            for child in node.get_children():
                # Skip assignment operators
                if child.kind == cindex.CursorKind.CXX_METHOD and child.spelling == "operator=":
                    continue
                if (
                    child.kind == cindex.CursorKind.CXX_METHOD
                    and child.access_specifier == cindex.AccessSpecifier.PUBLIC
                ):
                    m.append(child)
                    args = get_arguments_with_default(child)
                    fs = get_fdec(child)
                    lines.append(
                        f'CppDetectorApi.def("{child.spelling}",{fs} &Detector::{child.spelling}{args});'
                    )
                    if cargs.verbose:
                        print(f"&Detector::{child.spelling}{args})")
                    cn.append(child)

    for child in node.get_children():
        visit(child)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-p",
        "--build_path",
        help="Path to the build database",
        type=Path,
        default=default_build_path,
    )
    parser.add_argument(
        "-v",
        "--verbose",
        help="more output",
        action="store_true",
    )
    cargs = parser.parse_args()

    check_clang_format_version(12)
    check_for_compile_commands_json(cargs.build_path)

    print("Parsing functions in Detector.h - ", end="", flush=True)
    t0 = time.perf_counter()
    # parse functions
    db = cindex.CompilationDatabase.fromDirectory(cargs.build_path)
    index = cindex.Index.create()
    args = db.getCompileCommands(fpath)
    args = list(iter(args).__next__().arguments)[0:-1]
    args = args + "-x c++ --std=c++11".split()
    syspath = system_include_paths("clang++")
    incargs = ["-I" + inc for inc in syspath]
    args = args + incargs
    tu = index.parse(fpath, args=args)
    visit(tu.cursor)
    print(green("OK"))
    print(f"Parsing took {time.perf_counter()-t0:.3f}s")

    print("Read detector_in.cpp - ", end="")
    with open("../src/detector_in.cpp") as f:
        data = f.read()
    s = "".join(lines)
    s += ";"
    text = data.replace("[[FUNCTIONS]]", s)
    warning = "/* WARINING This file is auto generated any edits might be overwritten without warning */\n\n"
    print(green("OK"))
    print("Writing to detector.cpp - ", end="")
    with open("../src/detector.cpp", "w") as f:
        f.write(warning)
        f.write(text)
    print(green("OK"))

    # run clang format on the output
    print("Running clang format on generated source -", end="")
    subprocess.run(["clang-format", "../src/detector.cpp", "-i"])
    print(green(" OK"))

    print("Changes since last commit:")
    subprocess.run(["git", "diff", "../src/detector.cpp"])
