from .detector import Detector

def view(name):
    names =  find(name)
    for n in names:
        print(n)

def find(name):
    return [n for n in dir(Detector) if name in n]