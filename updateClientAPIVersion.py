# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2025 Contributors to the SLS Detector Package
"""
Script to update API VERSION for slsReceiverSoftware or slsDetectorSoftware 
""" 

import argparse
import os

from updateAPIVersion import update_api_version

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

parser = argparse.ArgumentParser(description = 'updates API version')
parser.add_argument('module_name', nargs="?", choices=["slsDetectorSoftware", "slsReceiverSoftware", "all"], default="all", help = 'module name to change api version options are: ["slsDetectorSoftware", "slsReceiverSoftware, "all"]')

if __name__ == "__main__": 
    args = parser.parse_args()

    if args.module_name == "all": 
        client_names = ["APILIB", "APIRECEIVER"]
        client_directories = [SCRIPT_DIR+"/slsDetectorSoftware", SCRIPT_DIR+"/slsReceiverSoftware"]
    elif args.module_name == "slsDetectorSoftware": 
        client_names = ["APILIB"]
        client_directories = [SCRIPT_DIR+"/slsDetectorSoftware"]
    else: 
        client_names = ["APIRECEIVER"]
        client_directories = [SCRIPT_DIR+"/slsReceiverSoftware"]
	
    for client_name, client_directory in zip(client_names, client_directories): 
        update_api_version(client_name, client_directory)
		

	
