# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
class Register:
    def __init__(self, detector):
        self._detector = detector

    def __getitem__(self, key):
        return self._detector.readRegister(key)

    def __setitem__(self, key, value):
        self._detector.writeRegister(key, value, False)

class Adc_register:
    def __init__(self, detector):
        self._detector = detector

    def __setitem__(self, key, value):
        self._detector.writeAdcRegister(key, value)

    def __getitem__(self, key):
        raise ValueError('Adc registers cannot be read back')