import filecmp
from pathlib import Path

import pytest

from pyctbgui.utils.numpyWriter.npy_writer import NumpyFileManager
import numpy as np


def test_create_new_file(tmp_path):
    npw = NumpyFileManager(tmp_path / 'tmp.npy', 'w', (400, 400), np.int32)
    npw.writeOneFrame(np.ones([400, 400], dtype=np.int32))
    npw.writeOneFrame(np.ones([400, 400], dtype=np.int32))
    npw.writeOneFrame(np.ones([400, 400], dtype=np.int32))
    npw.writeOneFrame(np.ones([400, 400], dtype=np.int32))
    npw.close()

    arr = np.load(tmp_path / 'tmp.npy')

    assert arr.dtype == np.int32
    assert arr.shape == (4, 400, 400)
    assert np.array_equal(arr, np.ones([4, 400, 400], dtype=np.int32))

    np.save(tmp_path / 'tmp2.npy', np.ones([4, 400, 400], dtype=np.int32))
    assert filecmp.cmp(tmp_path / 'tmp.npy', tmp_path / 'tmp2.npy')


def test_open_old_file(tmp_path):
    npw = NumpyFileManager(tmp_path / 'tmp.npy', 'w', (4000, ), np.float32)
    npw.writeOneFrame(np.ones(4000, dtype=np.float32))
    npw.writeOneFrame(np.ones(4000, dtype=np.float32))
    npw.close()
    npw2 = NumpyFileManager(tmp_path / 'tmp.npy', 'r+')
    assert npw2.frameCount == 2
    assert npw2.frameShape == (4000, )
    assert npw2.dtype == np.float32
    npw2.writeOneFrame(np.ones(4000, dtype=np.float32))
    del npw2
    np.save(tmp_path / 'tmp2.npy', np.ones([3, 4000], dtype=np.float32))
    assert filecmp.cmp(tmp_path / 'tmp.npy', tmp_path / 'tmp2.npy')


@pytest.mark.parametrize('mode', ['w', 'x'])
def test_init_parameters2(mode, tmp_path):
    # test opening files with missing parameters for write
    with pytest.raises(AssertionError):
        NumpyFileManager(tmp_path / 'abaababababa.npyx', mode)
    with pytest.raises(AssertionError):
        NumpyFileManager(tmp_path / 'abaababababa.npyx2', mode, frameShape=(12, 34))
    with pytest.raises(AssertionError):
        NumpyFileManager(tmp_path / 'abaababababa.npyx3', mode, dtype=np.int64)

    # opening new file with required parameters (this should work)
    NumpyFileManager(tmp_path / 'abaababababa.npyx3', mode, dtype=np.int64, frameShape=(6, 6))
    assert Path.is_file(tmp_path / 'abaababababa.npyx3')


def test_init_parameters(tmp_path):
    with pytest.raises(TypeError):
        NumpyFileManager()

    # test opening file that does not exist
    with pytest.raises(FileNotFoundError):
        NumpyFileManager(tmp_path / 'abaababababa.npyx')
    with pytest.raises(FileNotFoundError):
        NumpyFileManager(tmp_path / 'abaababababa.npyx2', frameShape=(12, 34))
    with pytest.raises(FileNotFoundError):
        NumpyFileManager(tmp_path / 'abaababababa.npyx3', dtype=np.int64)
    with pytest.raises(FileNotFoundError):
        NumpyFileManager(tmp_path / 'abaababababa.npyx3', dtype=np.int64, frameShape=(6, 6))

    # re-opening the same file
    NumpyFileManager(tmp_path / 'abaababababa.npyx3', 'w', dtype=np.int64, frameShape=(6, 6))
    NumpyFileManager(tmp_path / 'abaababababa.npyx3')

    # re-opening the file with wrong parameters
    with pytest.raises(AssertionError):
        NumpyFileManager(tmp_path / 'abaababababa.npyx3', frameShape=(6, 2))
    with pytest.raises(AssertionError):
        NumpyFileManager(tmp_path / 'abaababababa.npyx3', dtype=np.int32)
    with pytest.raises(AssertionError):
        NumpyFileManager(tmp_path / 'abaababababa.npyx3', dtype=np.float32, frameShape=(5, 5))

    # test resetting an existing file
    npw = NumpyFileManager(tmp_path / 'tmp4.npy', 'w', dtype=np.float32, frameShape=(5, 5))
    npw.writeOneFrame(np.ones((5, 5), dtype=np.float32))
    npw.close()
    assert np.load(tmp_path / 'tmp4.npy').shape == (1, 5, 5)
    npw = NumpyFileManager(tmp_path / 'tmp4.npy', 'w', dtype=np.int64, frameShape=(7, 7))
    npw.flush()
    assert np.load(tmp_path / 'tmp4.npy').shape == (0, 7, 7)

    # test adding frames with the wrong shape to an existing file
    with pytest.raises(ValueError, match=r'frame shape given \(9, 4, 4\) '):
        npw.writeOneFrame(np.ones((9, 4, 4)))


def test_get_item(tmp_path):
    rng = np.random.default_rng(seed=42)
    arr = rng.random((1000, 20, 20))
    npw = NumpyFileManager(tmp_path / 'tmp.npy', 'w', frameShape=(20, 20), dtype=arr.dtype)
    for frame in arr:
        npw.writeOneFrame(frame)

    assert np.array_equal(npw[50:100], arr[50:100])
    assert np.array_equal(npw[0:1], arr[0:1])
    assert np.array_equal(npw[0:10000], arr)
    assert np.array_equal(npw[999:1000], arr[999:1000])
    assert np.array_equal(npw[999:1005], arr[999:1000])
    assert np.array_equal(npw[49:300], arr[49:300])
    assert np.array_equal(npw[88:88], arr[88:88])
    assert np.array_equal(npw[0:0], arr[0:0])
    assert np.array_equal(npw[0:77], arr[0:77])
    assert np.array_equal(npw[77:0], arr[77:0])

    with pytest.raises(NotImplementedError):
        npw[-1:-3]
    with pytest.raises(NotImplementedError):
        npw[10:20:2]
    with pytest.raises(NotImplementedError):
        npw[-5:-87:5]
    with pytest.raises(NotImplementedError):
        npw[-5:-87:-5]

    with pytest.raises(NotImplementedError):
        npw.readFrames(-1, -4)
    with pytest.raises(NotImplementedError):
        npw.readFrames(0, -77)
    with pytest.raises(NotImplementedError):
        npw.readFrames(-5, -5)


def test_file_functions(tmp_path):
    rng = np.random.default_rng(seed=42)
    arr = rng.random((1000, 20, 20))
    npw = NumpyFileManager(tmp_path / 'tmp.npy', 'w', frameShape=(20, 20), dtype=arr.dtype)
    for frame in arr:
        npw.writeOneFrame(frame)
    assert np.array_equal(npw.read(10), arr[:10])
    assert np.array_equal(npw.read(10), arr[10:20])
    assert np.array_equal(npw.read(10), arr[20:30])
    npw.readFrames(500, 600)
    assert np.array_equal(npw.read(10), arr[30:40])
    npw.readFrames(0, 2)
    assert np.array_equal(npw.read(100), arr[40:140])
    npw.writeOneFrame(arr[700])
    assert np.array_equal(npw.read(5), arr[140:145])
    npw.seek(900)
    assert np.array_equal(npw.read(20), arr[900:920])
    npw.seek(5)
    npw.readFrames(500, 600)
    assert np.array_equal(npw.read(10), arr[5:15])


def test_with_statement(tmp_path):
    arr = np.ones((5, 5))
    with NumpyFileManager(tmp_path / 'tmp.npy', 'w', (5, 5), arr.dtype) as npw:
        npw.writeOneFrame(arr)
    np.save(tmp_path / 'tmp2.npy', np.expand_dims(arr, 0))
    assert filecmp.cmp(tmp_path / 'tmp2.npy', tmp_path / 'tmp.npy')
