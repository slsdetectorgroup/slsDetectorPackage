# SPDX-License-Identifier: LGPL-3.0-or-other
# Copyright (C) 2021 Contributors to the SLS Detector Package
from . import _slsdet

from ._slsdet import Pattern


class _StructViewMixin:
    """
    Exposes the fields of a numpy structured dtype view as attributes.
    """

    def _init_view(self, view):
        self.__dict__["view"] = view
        self.__dict__["names"] = view.dtype.names

    def __getattr__(self, name):
        if name in self.names:
            return self.view[name][0]
        else:
            raise KeyError(f"Key: {name} not found")

    def __setattr__(self, name, value):
        if name in self.names:
            self.view[name] = value
        else:
            raise KeyError(f"Key: {name} not found")

    # Provide custom dir for tab completion
    def __dir__(self):
        return self.names

    def copy(self):
        """
        Return a new instance with the same field values.
        """
        new = type(self)()
        new.view[:] = self.view
        return new

    __copy__ = copy


class patternParameters(_StructViewMixin, _slsdet.patternParameters):
    def __init__(self):
        super().__init__()
        self._init_view(self.numpy_view())


class Pattern(_StructViewMixin, _slsdet.Pattern):
    def __init__(self):
        super().__init__()
        self._init_view(self.data().numpy_view())
