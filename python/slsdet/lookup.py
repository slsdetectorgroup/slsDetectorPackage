from .detector import Detector

def view(name, det = Detector):
    names =  find(name, det)
    for n in names:
        print(n)

def find(name, det = Detector):
    return [n for n in dir(det) if name.lower() in n.lower()]