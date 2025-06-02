# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
"""
Script to update VERSION file with semantic versioning if provided as an argument, or with 0.0.0 if no argument is provided.
"""

import sys
import os

from packaging.version import Version, InvalidVersion


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

def get_version():

    # Check at least one argument is passed
    if len(sys.argv) < 2:
        return "0.0.0"

    version = sys.argv[1]

    try:
        v = Version(version)  # normalizcheck if version follows PEP 440 specification
        #replace -
        return version.replace("-", ".")
    except InvalidVersion as e:
        print(f"Invalid version {version}. Version format must follow semantic versioning format of python PEP 440 version identification specification.")
        sys.exit(1)
    

def write_version_to_file(version):
    version_file_path = os.path.join(SCRIPT_DIR, "VERSION")
    with open(version_file_path, "w") as version_file:
        version_file.write(version)
    print(f"Version {version} written to VERSION file.")

# Main script
if __name__ == "__main__":

    version = get_version()
    write_version_to_file(version)
