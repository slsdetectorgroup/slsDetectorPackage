import _slsdet

class patternParameters(_slsdet.patternParameters):
    def __init__(self):
        super().__init__()
        self.view = self.numpy_view()
        self.names = self.view.dtype.names

    def __getattr__(self, name):
        if name in self.names:
            return self.view[name][0]
        else:
            raise KeyError(f"Key: {name} not found")

    def __setattr__(self, name, value):
        if name in ['view', 'names']:
            self.__dict__[name] = value
        elif name in self.names:
            self.view[name] = value
        else: 
            raise KeyError(f"Key: {name} not found")

    #Provide custom dir for tab completion
    def __dir__(self):
        return self.names