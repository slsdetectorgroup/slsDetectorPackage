# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2025 Contributors to the SLS Detector Package
"""
Script to update API VERSION file based on the version in VERSION file.
"""

import argparse
import sys
import os
import re
import time
from datetime import datetime


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

API_FILE = SCRIPT_DIR + "/slsSupportLib/include/sls/versionAPI.h"

VERSION_FILE = SCRIPT_DIR + "/VERSION"

parser = argparse.ArgumentParser(description = 'updates API version')
parser.add_argument('api_module_name', choices=["APILIB", "APIRECEIVER", "APICTB", "APIGOTTHARD2", "APIMOENCH", "APIEIGER", "APIXILINXCTB", "APIJUNGFRAU", "APIMYTHEN3"], help = 'module name to change api version options are: ["APILIB", "APIRECEIVER", "APICTB", "APIGOTTHARD2", "APIMOENCH", "APIEIGER", "APIXILINXCTB", "APIJUNGFRAU", "APIMYTHEN3"]')
parser.add_argument('api_dir', help = 'Relative or absolute path to the module code')

def update_api_file(new_api : str, api_module_name : str, api_file_name : str): 

    regex_pattern = re.compile(rf'#define\s+{api_module_name}\s+')
    with open(api_file_name, "r") as api_file:
        lines = api_file.readlines()

    with open(api_file_name, "w") as api_file:
        for line in lines:
            if regex_pattern.match(line):
                api_file.write(f'#define {api_module_name} "{new_api}"\n')
            else:
                api_file.write(line)

def get_latest_modification_date(directory : str):
    latest_time = 0
    latest_date = None

    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith(".o"):
                continue
            full_path = os.path.join(root, file)
            try:
                mtime = os.path.getmtime(full_path)
                if mtime > latest_time:
                    latest_time = mtime
            except FileNotFoundError:
                continue
    
    latest_date = datetime.fromtimestamp(latest_time).strftime("%y%m%d")            

    return latest_date


def update_api_version(api_module_name : str, api_dir : str):
    api_date = get_latest_modification_date(api_dir)
    api_date = "0x"+str(api_date)

    with open(VERSION_FILE, "r") as version_file: 
        api_version = version_file.read().strip()

    api_version = api_version + " " + api_date #not sure if we should give an argument option version_branch 

    update_api_file(api_version, api_module_name, API_FILE)

    print(f"updated {api_module_name} api version to: {api_version}")

if __name__ == "__main__":

    args = parser.parse_args() 

    api_dir = SCRIPT_DIR + "/" + args.api_dir
    

    update_api_version(args.api_module_name, api_dir)


