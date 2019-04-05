#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Thu Dec 14 17:13:55 2017

@author: l_frojdh
"""


class DetectorError(Exception):
    """
    This error should be used when something fails
    on the detector side
    """
    pass


class DetectorSettingDoesNotExist(Exception):
    """This error should be used when the setting does not exist"""
    pass


class DetectorValueError(Exception):
    """This error should be used when the set value is outside the allowed range"""
    pass
