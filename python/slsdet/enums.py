# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
"""
Automatically improt all enums from slsDetectorDefs and give an
alias with their name in the enum module. All names from the enum
module is later imported into slsdet

Example: detectorType = _slsdet.slsDetectorDefs.detectorType
Usage can later be:

from slsdet import detectorType
if dt === detectorType.EIGER:
    #do something

"""


from . import _slsdet
import slsdet._slsdet
for name, cls in slsdet._slsdet.slsDetectorDefs.__dict__.items():
    if isinstance(cls, type):
        exec(f'{name} = {cls.__module__}.{cls.__qualname__}')

