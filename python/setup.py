# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
"""
Setup file for slsdet
Build upon the pybind11 example found here: https://github.com/pybind/python_example
"""

import os
import sys
from pathlib import Path
# sys.path.append('../libs/pybind')
from setuptools import setup, find_packages


__version__ = os.environ.get('GIT_DESCRIBE_TAG', 'developer')




setup(
    name='slsdet',
    version=__version__,
    author='Erik Frojdh',
    author_email='erik.frojdh@psi.ch',
    url='https://github.com/slsdetectorgroup/slsDetectorPackage',
    description='Detector API for SLS Detector Group detectors',
    long_description='',
    packages=find_packages(exclude=['contrib', 'docs', 'tests']),
    package_data={"":["*.so"]},
    include_package_data=True,
    zip_safe=False,
)
