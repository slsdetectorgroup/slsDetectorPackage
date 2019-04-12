from .decorators import error_handling, property_error_handling
class Register:
    def __init__(self, detector):
        self._detector = detector

    @property_error_handling
    def __getitem__(self, key):
        return self._detector._api.readRegister(key)

    def __setitem__(self, key, value):
        self._detector._api.writeRegister(key, value)

class Adc_register:
    def __init__(self, detector):
        self._detector = detector

    def __setitem__(self, key, value):
        self._detector._api.writeAdcRegister(key, value)