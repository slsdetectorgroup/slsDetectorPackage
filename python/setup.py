# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
"""
Setup file for slsdet
Build upon the pybind11 example found here: https://github.com/pybind/python_example
"""

import os
import sys
from setuptools import setup, find_packages
from pybind11.setup_helpers import Pybind11Extension, build_ext


def read_version():
    version_file = os.path.join(os.path.dirname(__file__), "..", "VERSION")
    with open(version_file, "r") as f:
        return f.read().strip()

__version__ = read_version()


def get_conda_path():
    """
    Keep this a function if we need some fancier logic later
    """
    print('Prefix: ', os.environ['CONDA_PREFIX'])
    return os.environ['CONDA_PREFIX']


#TODO migrate to CMake build or fetch files from cmake? 
ext_modules = [
    Pybind11Extension(
        '_slsdet',
        ['src/main.cpp',
        'src/enums.cpp',
        'src/current.cpp',
        'src/detector.cpp',
        'src/network.cpp',
        'src/pattern.cpp',
        'src/scan.cpp',
        'src/duration.cpp',
        'src/DurationWrapper.cpp',
        'src/pedestal.cpp',
        ]
        
        
        ,
        include_dirs=[
            os.path.join(get_conda_path(), 'include'),

        ],
        libraries=['SlsDetector', 'SlsSupport', 'SlsReceiver'],
        library_dirs=[
            os.path.join(get_conda_path(), 'lib'),
        ],
        language='c++'
    ),
]

setup(
    name='slsdet',
    version=__version__,
    author='Erik Frojdh',
    author_email='erik.frojdh@psi.ch',
    url='https://github.com/slsdetectorgroup/slsDetectorPackage',
    description='Detector API for SLS Detector Group detectors',
    long_description='',
    packages=find_packages(exclude=['contrib', 'docs', 'tests']),
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
)
