import pytest
import subprocess
import os
import sys
import time
sys.path.append(os.path.join(os.getcwd(), 'bin'))
from sls_detector import ExperimentalDetector

@pytest.fixture(scope="module")
def virtual_jf_detectors(request):
    print('Setting up virtual detectors')
    subprocess.run(["killall", "jungfrauDetectorServer_virtual"])
    virtual_jf_detectors = []
    virtual_jf_detectors.append(subprocess.Popen('bin/jungfrauDetectorServer_virtual'))
    time.sleep(5)
    def fin():
        print("Cleaning up virtual detectors")
        subprocess.run(["killall", "jungfrauDetectorServer_virtual"])
        
        

    request.addfinalizer(fin)
    return virtual_jf_detectors  # provide the fixture value


def test_hostname(virtual_jf_detectors):
    d = ExperimentalDetector()
    d.hostname = 'localhost'
    assert d.hostname == 'localhost'

def test_fwversion(virtual_jf_detectors):
    d = ExperimentalDetector()
    assert d.detectorversion == 0 #Firmware of virtual detector