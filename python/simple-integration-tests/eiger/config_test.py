#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Nov 14 16:49:07 2017

@author: l_frojdh
"""

fw_version = 23
detector_type = 'Eiger'
known_hostnames = ['beb083', 'beb098']
image_size = (512,1024) #rows, cols
module_geometry = (1,2) #horizontal, vertical

#Remember to change these in the settings file as well!
settings_path = '/home/l_frojdh/slsDetectorPackage/settingsdir/eiger'
file_path = '/home/l_frojdh/out'