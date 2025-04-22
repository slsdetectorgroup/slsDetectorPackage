# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
"""
Script to update VERSION file with semantic versioning if provided as an argument, or with 0.0.0 if no argument is provided.
"""

import sys
import re
import os
import toml
import yaml
from jinja2 import Template, Undefined

def get_version():

    # Check at least one argument is passed
    if len(sys.argv) < 2:
        return "0.0.0"

    version = sys.argv[1]
    
    # Validate that the version argument matches semantic versioning format (X.Y.Z)
    if not re.match(r'^\d+\.\d+\.\d+$', version):
        print("Error: Version argument must be in semantic versioning format (X.Y.Z)")
        sys.exit(1)
    
    return version


def write_version_to_file(version):
    with open("VERSION", "w") as version_file:
        version_file.write(version)
    print(f"Version {version} written to VERSION file.")

def define_environment_variable(version):
    os.environ["VERSION"] = version

def update_pyproject_toml_file(version):
    pyproject = toml.load("pyproject.toml")
    pyproject["project"]["version"] = version
    toml.dump(pyproject, open("pyproject.toml", "w")) #write back
    print(f"Version in pyproject.toml set to {version}")

class NullUndefined(Undefined):
  def __getattr__(self, key):
    return ''



# Main script
if __name__ == "__main__":

    version = get_version()
    write_version_to_file(version)
    define_environment_variable(version)
    update_pyproject_toml_file(version)
