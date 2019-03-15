#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Function decorators for the sls_detector.
"""
from .errors import DetectorError
import functools


def error_handling(func):
    """
    Check for errors registered by the slsDetectorSoftware
    """
    @functools.wraps(func)
    def wrapper(self, *args, **kwargs):
        
        # remove any previous errors
        self._api.clearErrorMask()
        
        # call function
        result = func(self, *args, **kwargs)
        
        # check for new errors
        m = self.error_mask
        if m != 0:
            msg = self.error_message
            self._api.clearErrorMask()
            raise DetectorError(msg)
        return result

    return wrapper


def property_error_handling(func):
    """
    Check for errors registered by the slsDetectorSoftware
    """

    @functools.wraps(func)
    def wrapper(self, *args, **kwargs):
        # remove any previous errors
        self._detector._api.clearErrorMask()

        # call function
        result = func(self, *args, **kwargs)

        # check for new errors
        m = self._detector.error_mask
        if m != 0:
            msg = self._detector.error_message
            self._detector._api.clearErrorMask()
            raise DetectorError(msg)
        return result

    return wrapper