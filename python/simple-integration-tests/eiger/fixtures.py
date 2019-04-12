import pytest

from sls_detector import Detector

@pytest.fixture
def detector():
    from sls_detector import Detector
    return Detector()

@pytest.fixture
def eiger():
    from sls_detector import Eiger
    d = Eiger()
    d.n_frames = 1
    d.exposure_time = 1
    d.file_write = False
    return d


@pytest.fixture
def jungfrau():
    from sls_detector import Jungfrau
    return Jungfrau()

detector_type = Detector().detector_type
eigertest = pytest.mark.skipif(detector_type != 'Eiger', reason = 'Only valid for Eiger')
jungfrautest = pytest.mark.skipif(detector_type != 'Jungfrau', reason = 'Only valid for Jungfrau')