# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
"""
Script to update VERSION file with semantic versioning if provided as an argument, or with 0.0.0 if no argument is provided.
Script also updates VERSION in RELEASE.md and adds the new version to version.yaml
"""

import sys
from pathlib import Path
from datetime import datetime
from tempfile import template
import yaml
import argparse

from packaging.version import Version, InvalidVersion

ROOT_DIR = Path(__file__).resolve().parent.parent 

SCRIPT_DIR = Path(__file__).resolve().parent.parent 

def get_version(version : str) -> str:

    try:
        v = Version(version)  # normalizcheck if version follows PEP 440 specification
        #replace -
        return version.replace("-", ".")
    except InvalidVersion as e:
        print(f"Invalid version {version}. Version format must follow semantic versioning format of python PEP 440 version identification specification.")
        sys.exit(1)
    

def write_version_to_file(version):
    version_file_path = Path(SCRIPT_DIR / "VERSION")
    with open(version_file_path, "w") as version_file:
        version_file.write(version)
    print(f"✓ Version {version} written to VERSION file.")


def extract_release_type(version: str) -> str: 
    """Extract release type from version string."""
    parts = version.split('.')
    if len(parts) != 3:
        raise ValueError(f"Version {version} does not follow semantic versioning format of major.minor.patch")
    major, minor, patch = map(int, parts)
    if minor == 0 and patch == 0:
        return "Major"
    elif patch == 0:
        return "Minor"
    else:
        return "Bug Fix"
    
def get_previous_version(data_path: Path, version : str) -> str:
    """Get the previous version from the versions YAML file."""
    with open(data_path, 'r') as f:
        data = yaml.safe_load(f)

    prev_version = data['versions'][0]["version"]
    if prev_version == version: #already updated to new version 
        prev_version = data['versions'][1]["version"]

    return prev_version

def add_version_to_version_file(data_path: Path, version: str, release_type: str, date: str, has_docs: bool = True):
    """Add a new version to the YAML data file."""

    with open(data_path, 'r') as f:
        data = yaml.safe_load(f)

    # check if version has already been added
    prev_version = data['versions'][0]["version"]
    if version == prev_version:
        return 
    
    # Add to table (check if not already present)
    new_entry = {
            'version': version,
            'type': release_type,
            'date': date,
            'has_docs': has_docs
    }
    data["versions"].insert(0, new_entry)

    with open(data_path, 'w') as f:
        yaml.dump(data, f, default_flow_style=False, sort_keys=False)
    
    print(f"✓ Added version {new_entry} to version data")


def main(): 
    parser = argparse.ArgumentParser(description='Update version release notes')
    parser.add_argument('--version', required=True, type=str, help='version to write to VERSION file')
    parser.add_argument('--update_version_only', action="store_true", help='Only update version in VERSION file, do not generate release notes')
    args = parser.parse_args()

    version = get_version(args.version)
    write_version_to_file(version)

    if not args.update_version_only:
        release_type = extract_release_type(version)
        prev_version = get_previous_version(ROOT_DIR / "docs/main_index/versions.yaml", version)

        # Read template
        template = Path(ROOT_DIR / "RELEASE.md").read_text()

        # Substitute version and date
        output = template.replace("{{VERSION}}", version)
        output = output.replace("{{DATE}}", datetime.now().strftime("%Y-%m-%d"))
        output = output.replace("{{RELEASE_TYPE}}", release_type)
        output = output.replace("{{PREVIOUS_VERSION}}", prev_version)

        # Write output
        Path(ROOT_DIR / "RELEASE.md").write_text(output)
        print(f"✓ Generated RELEASE.md for version {version}")

        #add version to versions.yaml file
        add_version_to_version_file(ROOT_DIR / "docs/main_index/versions.yaml", version, release_type, datetime.now().strftime("%d.%m.%Y"))

# Main script
if __name__ == "__main__":
    main()
