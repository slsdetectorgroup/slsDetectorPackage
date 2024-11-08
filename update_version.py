# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
"""
Script to update VERSION file with semantic versioning if provided as an argument, or with today's date in '0xyymmdd' hex format if no argument is provided.
"""

import sys
import re

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


# Main script
if __name__ == "__main__":

    version = get_version()
    write_version_to_file(version)
