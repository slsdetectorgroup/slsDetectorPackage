# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
"""
This file is used to auto generate Python bindings for the 
sls::Detector class. The tool needs the libclang bindings
to be installed. 

When the Detector API is updated this file should be run
manually.

Tested with libclang 12 and 20-23. The output is formatted with
clang-format 12 which conda can't install next to a newer libclang,
there use: pip install clang-format==12.0.1
"""
import os
from clang import cindex
import subprocess
import argparse
import sys
import time
import ctypes.util, re  # to check libclang version 
from pathlib import Path
from parse import system_include_paths, clang_format_version

REDC = "\033[91m"
GREENC = "\033[92m"
YELLOWC = "\033[93m"
ENDC = "\033[0m"

def yellow(msg):
    return f"{YELLOWC}{msg}{ENDC}"

def red(msg):
    return f"{REDC}{msg}{ENDC}"


def green(msg):
    return f"{GREENC}{msg}{ENDC}"


def find_libclang():
    """Find libclang in the current Conda/Mamba environment."""
    conda_prefix = os.environ.get("CONDA_PREFIX")
    if conda_prefix:
        # Match the name exactly, lib also holds libclang-cpp and libclang_rt.*
        for name in ("libclang.so", "libclang.dylib"):
            path = os.path.join(conda_prefix, "lib", name)
            if os.path.exists(path):
                return path

    # fallback: system-wide search
    path = ctypes.util.find_library("clang")
    if path:
        return path

    raise FileNotFoundError("libclang not found in CONDA_PREFIX or system paths.")


def check_libclang_version(supported):
    # Use already-loaded libclang, or let cindex resolve it
    lib = ctypes.CDLL(cindex.Config.library_file or ctypes.util.find_library("clang"))

    # Get version string
    lib.clang_getClangVersion.restype = ctypes.c_void_p
    version_ptr = lib.clang_getClangVersion()
    version_str = ctypes.cast(version_ptr, ctypes.c_char_p).value.decode()

    # Parse and check version
    match = re.search(r"version\s+(\d+)", version_str)
    version = int(match.group(1)) if match else None
    if version not in supported:
        versions = ", ".join(str(v) for v in supported)
        msg = red(f"libclang version {version or '?'} found, but one of {versions} required. Bye!")
        print(msg)
        sys.exit(1)
    msg = green(f"Found libclang version {version}")
    print(msg)


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


def check_for_parse_errors(tu):
    # Types that libclang fails to parse silently turn into int. Errors without
    # a location come from compiler flags that clang does not know and are harmless
    errors = [
        d
        for d in tu.diagnostics
        if d.severity >= cindex.Diagnostic.Error and d.location.file
    ]
    if errors:
        print(red("FAILED"))
        for d in errors:
            print(red(f"{d.location.file}:{d.location.line}: {d.spelling}"))
        print(red("Errors while parsing, the generated types can't be trusted. Bye!"))
        sys.exit(1)


default_build_path = "/home/l_frojdh/sls/build/"
fpath = "../../slsDetectorSoftware/src/Detector.cpp"
supported_libclang_versions = (12, 20, 21, 22, 23)


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


def qualified_name(decl):
    """Name of a declaration including its enclosing scopes, e.g. sls::Positions"""
    parts = []
    while decl is not None and decl.kind != cindex.CursorKind.TRANSLATION_UNIT:
        if decl.spelling:
            parts.append(decl.spelling)
        decl = decl.semantic_parent
    return "::".join(reversed(parts))


def referenced_types(node):
    """Declarations of the types named in the return type of a method or in a parameter"""
    for child in node.get_children():
        if child.kind == cindex.CursorKind.TYPE_REF:
            yield child.referenced
        elif node.kind == child.kind == cindex.CursorKind.PARM_DECL:
            # parameters of a function pointer
            yield from referenced_types(child)


def type_spelling(node, type):
    """
    Spelling of the return type of a method or the type of a parameter.

    libclang 12 printed a type name written without scope as sls::Positions,
    newer versions give it as written in the source (Positions). To be
    independent of the version we look up the declarations and add the scope
    ourselves. Names already written with a scope (defs::xy) are left as they are.
    """
    spelling = type.spelling
    for decl in referenced_types(node):
        unqualified = rf"(?<![\w:]){re.escape(decl.spelling)}(?![\w:])"
        spelling = re.sub(unqualified, qualified_name(decl), spelling)
    return spelling


def get_compile_args(build_path):
    """Arguments to parse Detector.cpp with, from the compilation database"""
    db = cindex.CompilationDatabase.fromDirectory(build_path)
    args = list(next(iter(db.getCompileCommands(fpath))).arguments)
    # Drop the compiler and the source file. Newer versions of libclang put
    # "--" before the file and anything we add after that is read as a file name
    args = args[1:-1]
    if args and args[-1] == "--":
        args.pop()
    args += "-x c++ --std=c++17".split()
    for inc in system_include_paths("clang++"):
        args += ["-isystem", inc]
    return args


def get_arguments_with_default(node):
    args = []
    for arg in node.get_arguments():
        tokens = [t.spelling for t in arg.get_tokens()]
        # print(tokens)
        if "=" in tokens:
            if type_spelling(arg, arg.type) == "sls::Positions":  # TODO! automate
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
    args = [type_spelling(a, a.type) for a in node.get_arguments()]
    if node.result_type.spelling:
        return_type = type_spelling(node, node.result_type)
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

    loc = node.location
    # skip if ndoe is outside project directory
    if loc.file and not str(loc.file).startswith(str(cargs.build_path.parent)):
        return
    
    '''
    # to see which file was causing the error (not in Detector.h, so skipping others in the above code)
    try:
        kind = node.kind
    except ValueError as e:
        loc = node.location
        file_name = loc.file.name if loc.file else "<unknown file>"
        msg = yellow(f"\nWarning: skipping node with unknown CursorKind id {node._kind_id} at {file_name}:{loc.line}:{loc.column}")
        print(msg)
        return 
    '''
        
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
    
    libclang_path = find_libclang()
    cindex.Config.set_library_file(libclang_path)
    check_libclang_version(supported_libclang_versions)
    check_clang_format_version(12)
    check_for_compile_commands_json(cargs.build_path)

    print("Parsing functions in Detector.h - ", end="", flush=True)
    t0 = time.perf_counter()
    # parse functions
    index = cindex.Index.create()
    tu = index.parse(
        fpath,
        args=get_compile_args(cargs.build_path),
        # we only need the declarations
        options=cindex.TranslationUnit.PARSE_SKIP_FUNCTION_BODIES,
    )
    check_for_parse_errors(tu)
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
