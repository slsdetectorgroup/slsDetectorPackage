#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
General tests for the Jungfrau detector.

NOTE! Uses hostnames from config_test
"""

import pytest
import config_test
import tests

import os
dir_path = os.path.dirname(os.path.realpath(__file__))

pytest.main(['-x', '-s', os.path.join(dir_path, 'tests/test_load_config.py')])            #Test 1
pytest.main(['-x', '-s', os.path.join(dir_path, 'tests/test_overtemperature.py')])        #Test 2
