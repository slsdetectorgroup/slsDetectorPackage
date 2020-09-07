from .utils import element_if_equal

class JsonProxy:
    """
    Proxy class to allow for intuitive setting and getting of rx_jsonpara
    This class is returned by Detectr.rx_jsonpara
    """
    def __init__(self, det):
        self.det = det

    def __getitem__(self, key):
        return element_if_equal(self.det.getAdditionalJsonParameter(key))

    def __setitem__(self, key, value):
        self.det.setAdditionalJsonParameter(key, str(value))

    def __repr__(self):
        r = element_if_equal(self.det.getAdditionalJsonHeader())
        if isinstance(r, list):
            rstr = ''
            for i, list_item in enumerate(r):
                list_item = dict(list_item)
                rstr += ''.join([f'{i}:{key}: {value}\n' for key, value in list_item.items()])

            return rstr.strip('\n')
        else:
            r = dict(r)
            return '\n'.join([f'{key}: {value}' for key, value in r.items()])

    
