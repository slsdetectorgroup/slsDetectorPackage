"""
Implementation of a naive (not thread safe) singleton design pattern
taken from: https://refactoring.guru/design-patterns/singleton/python/example
"""


class SingletonMeta(type):
    _instances = {}

    def __call__(cls, *args, **kwargs):
        if cls not in cls._instances:
            instance = super().__call__(*args, **kwargs)
            cls._instances[cls] = instance
        return cls._instances[cls]
