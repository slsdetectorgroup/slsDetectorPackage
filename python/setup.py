"""
Setup file for slsdet
Build upon the pybind11 example found here: https://github.com/pybind/python_example
"""

import os
import sys
sys.path.append('../libs/pybind11')
from setuptools import setup, find_packages
from pybind11.setup_helpers import Pybind11Extension, build_ext

__version__ = os.environ.get('GIT_DESCRIBE_TAG', 'developer')


def get_conda_path():
    """
    Keep this a function if we need some fancier logic later
    """
    print('Prefix: ', os.environ['CONDA_PREFIX'])
    return os.environ['CONDA_PREFIX']


#TODO migrate to CMake build? 
ext_modules = [
    Pybind11Extension(
        '_slsdet',
        ['src/main.cpp',
        'src/current.cpp',
        'src/enums.cpp',
        'src/detector.cpp',
        'src/network.cpp',
        'src/pattern.cpp',
        'src/scan.cpp',],
        include_dirs=[
            os.path.join('../libs/pybind11/include'),
            os.path.join(get_conda_path(), 'include'),

        ],
        libraries=['SlsDetector', 'SlsSupport', 'SlsReceiver', 'zmq'],
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
