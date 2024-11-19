# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
"""
Script to update VERSION file with semantic versioning if provided as an argument, or with 0.0.0 if no argument is provided.
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


def update_release_in_header(version):
    # Path to the versionAPI.h file
    header_file_path = "slsSupportLib/include/sls/versionAPI.h"
    
    try:
        with open(header_file_path, "r") as file:
            content = file.read()

        # Replace the version number next to #define RELEASE with the new version
        # This line is modified to always replace whatever value is after #define RELEASE
        new_content = re.sub(r'#define RELEASE\s+".*?"', f'#define RELEASE      "{version}"', content)

        # If the version has changed, write the updated content back to the file
        if new_content != content:
            with open(header_file_path, "w") as file:
                file.write(new_content)
            print(f"RELEASE version updated to {version} in {header_file_path}")
        else:
            print(f"RELEASE version is already set to {version} in {header_file_path}")
    
    
    except FileNotFoundError:
        print(f"Error: The file {header_file_path} was not found.")
        sys.exit(1)

# Main script
if __name__ == "__main__":

    version = get_version()
    write_version_to_file(version)
    update_release_in_header(version)