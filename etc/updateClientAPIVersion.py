# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2025 Contributors to the SLS Detector Package
"""
script to update API VERSION for slsReceiverSoftware or slsDetectorSoftware 
""" 

import argparse
from pathlib import Path

from updateAPIVersion import update_api_version

ROOT_DIR = Path(__file__).resolve().parent.parent 

parser = argparse.ArgumentParser(description = 'updates API version')
parser.add_argument('module_name', nargs="?", choices=["slsDetectorSoftware", "slsReceiverSoftware", "all"], default="all", help = 'module name to change api version options are: ["slsDetectorSoftware", "slsReceiverSoftware, "all"]')

if __name__ == "__main__": 
    args = parser.parse_args()

    if args.module_name == "all": 
        client_names = ["APILIB", "APIRECEIVER"]
        client_directories = [ROOT_DIR / "slsDetectorSoftware", ROOT_DIR / "slsReceiverSoftware"]
    elif args.module_name == "slsDetectorSoftware": 
        client_names = ["APILIB"]
        client_directories = [ROOT_DIR / "slsDetectorSoftware"]
    else: 
        client_names = ["APIRECEIVER"]
        client_directories = [ROOT_DIR / "slsReceiverSoftware"]
	
    for client_name, client_directory in zip(client_names, client_directories): 
        update_api_version(client_name, client_directory)
		

	
