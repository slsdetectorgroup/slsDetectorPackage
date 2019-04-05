import pytest
import config_test
from fixtures import detector, eiger, jungfrau, eigertest, jungfrautest
from sls_detector.errors import DetectorValueError




@eigertest
@pytest.mark.local
def test_set_path(eiger, tmpdir):
    import os
    path = os.path.join(tmpdir.dirname, tmpdir.basename)
    eiger.file_path = path
    assert eiger.file_path == path

@eigertest
@pytest.mark.local
def test_set_path_and_write_files(eiger, tmpdir):
    import os
    prefix = 'testprefix'
    path = os.path.join(tmpdir.dirname, tmpdir.basename)
    eiger.file_path = path
    eiger.file_write = True
    eiger.exposure_time = 0.1
    eiger.n_frames = 1
    eiger.timing_mode = 'auto'
    eiger.file_name = prefix
    eiger.file_index = 0
    eiger.acq()

    files = [f.basename for f in tmpdir.listdir()]

    assert len(files) == 5
    assert (prefix+'_d0_0.raw' in files) == True
    assert (prefix+'_d1_0.raw' in files) == True
    assert (prefix+'_d2_0.raw' in files) == True
    assert (prefix+'_d3_0.raw' in files) == True

def test_set_discard_policy(detector):
    detector.frame_discard_policy = 'nodiscard'
    assert detector.frame_discard_policy == 'nodiscard'
    detector.frame_discard_policy = 'discardpartial'
    assert detector.frame_discard_policy == 'discardpartial'
    detector.frame_discard_policy = 'discardempty'
    assert detector.frame_discard_policy == 'discardempty'

def test_set_discard_policy_raises(detector):
    with pytest.raises(ValueError):
        detector.frame_discard_policy = 'adjfvadksvsj'

def test_set_frames_perfile(detector):
    detector.frames_per_file = 5000
    assert detector.frames_per_file == 5000