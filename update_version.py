# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
"""
Script to update VERSION file with semantic versioning if provided as an argument, or with 0.0.0 if no argument is provided.
"""

import sys
import re
import toml
from packaging.version import Version, InvalidVersion

def get_version():

    # Check at least one argument is passed
    if len(sys.argv) < 2:
        return "0.0.0"

    version = sys.argv[1]
    
    try:
        v = Version(version)  # normalize according to PEP 440 specification
        return v
    except InvalidVersion as e:
        print(f"Invalid version {version}. Version format must follow semantic versioning format of python PEP 440 version identification specification.")
        sys.exit(1)
    

def write_version_to_file(version):
    with open("VERSION", "w") as version_file:
        version_file.write(str(version))
    print(f"Version {version} written to VERSION file.")

def update_pyproject_toml_file(version):
    pyproject = toml.load("pyproject.toml")
    pyproject["project"]["version"] = str(version)
    toml.dump(pyproject, open("pyproject.toml", "w")) #write back
    print(f"Version in pyproject.toml set to {version}")

# Main script
if __name__ == "__main__":

    version = get_version()
    write_version_to_file(version)
    update_pyproject_toml_file(version)
