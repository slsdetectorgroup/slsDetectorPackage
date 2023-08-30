import filecmp
import os
from pathlib import Path

import pytest

from pyctbgui.utils.numpyWriter.npy_writer import NumpyFileManager
import numpy as np

prefix = Path('tests/.tmp/')


def __clean_tmp_dir(path=prefix):
    if Path.is_dir(path):
        for file in os.listdir(path):
            Path.unlink(path / file)
    else:
        Path.mkdir(path)


def test_create_new_file():
    __clean_tmp_dir()
    npw = NumpyFileManager(prefix / 'tmp.npy', 'w', (400, 400), np.int32)
    npw.addFrame(np.ones([400, 400], dtype=np.int32))
    npw.addFrame(np.ones([400, 400], dtype=np.int32))
    npw.addFrame(np.ones([400, 400], dtype=np.int32))
    npw.addFrame(np.ones([400, 400], dtype=np.int32))
    npw.close()

    arr = np.load(prefix / 'tmp.npy')

    assert arr.dtype == np.int32
    assert arr.shape == (4, 400, 400)
    assert np.array_equal(arr, np.ones([4, 400, 400], dtype=np.int32))

    np.save(prefix / 'tmp2.npy', np.ones([4, 400, 400], dtype=np.int32))
    assert filecmp.cmp(prefix / 'tmp.npy', prefix / 'tmp2.npy')


def test_open_old_file():
    __clean_tmp_dir()
    npw = NumpyFileManager(prefix / 'tmp.npy', 'w', (4000, ), np.float32)
    npw.addFrame(np.ones(4000, dtype=np.float32))
    npw.addFrame(np.ones(4000, dtype=np.float32))
    npw.close()
    npw2 = NumpyFileManager(prefix / 'tmp.npy')
    assert npw2.frameCount == 2
    assert npw2.frameShape == (4000, )
    assert npw2.dtype == np.float32
    assert len(npw2.buffer) == 0
    npw2.addFrame(np.ones(4000, dtype=np.float32))
    del npw2
    np.save(prefix / 'tmp2.npy', np.ones([3, 4000], dtype=np.float32))
    assert filecmp.cmp(prefix / 'tmp.npy', prefix / 'tmp2.npy')


def test_buffer():
    __clean_tmp_dir()
    npw = NumpyFileManager(prefix / 'tmp.npy', 'w', (2, 2, 2), np.clongdouble, bufferMax=3)
    assert npw.bufferCount == 0
    npw.addFrame(np.ones((2, 2, 2), dtype=np.clongdouble))
    assert npw.bufferCount == 1
    npw.addFrame(np.ones((2, 2, 2), dtype=np.clongdouble))
    assert npw.bufferCount == 2
    npw.addFrame(np.ones((2, 2, 2), dtype=np.clongdouble))
    assert npw.bufferCount == 3
    npw.addFrame(np.ones((2, 2, 2), dtype=np.clongdouble))
    assert npw.bufferCount == 0
    npw.flushBuffer(strict=True)
    assert np.array_equal(np.load(prefix / 'tmp.npy'), np.ones((4, 2, 2, 2), dtype=np.clongdouble))
    del npw

    np.save(prefix / 'tmp2.npy', np.ones((4, 2, 2, 2), dtype=np.clongdouble))
    assert filecmp.cmp(prefix / 'tmp.npy', prefix / 'tmp2.npy')


@pytest.mark.parametrize('mode', ['w', 'x'])
def test_init_parameters2(mode):
    __clean_tmp_dir()
    # test opening files with missing parameters for write
    with pytest.raises(AssertionError):
        NumpyFileManager(prefix / 'abaababababa.npyx', mode)
    with pytest.raises(AssertionError):
        NumpyFileManager(prefix / 'abaababababa.npyx2', mode, frameShape=(12, 34))
    with pytest.raises(AssertionError):
        NumpyFileManager(prefix / 'abaababababa.npyx3', mode, dtype=np.int64)

    # opening new file with required parameters (this should work)
    NumpyFileManager(prefix / 'abaababababa.npyx3', mode, dtype=np.int64, frameShape=(6, 6))
    assert Path.is_file(prefix / 'abaababababa.npyx3')


def test_init_parameters():
    __clean_tmp_dir()
    with pytest.raises(TypeError):
        NumpyFileManager()

    # test opening file that does not exist
    with pytest.raises(FileNotFoundError):
        NumpyFileManager(prefix / 'abaababababa.npyx')
    with pytest.raises(FileNotFoundError):
        NumpyFileManager(prefix / 'abaababababa.npyx2', frameShape=(12, 34))
    with pytest.raises(FileNotFoundError):
        NumpyFileManager(prefix / 'abaababababa.npyx3', dtype=np.int64)
    with pytest.raises(FileNotFoundError):
        NumpyFileManager(prefix / 'abaababababa.npyx3', dtype=np.int64, frameShape=(6, 6))

    # re-opening the same file
    NumpyFileManager(prefix / 'abaababababa.npyx3', 'w', dtype=np.int64, frameShape=(6, 6))
    NumpyFileManager(prefix / 'abaababababa.npyx3')

    # re-opening the file with wrong parameters
    with pytest.raises(AssertionError):
        NumpyFileManager(prefix / 'abaababababa.npyx3', frameShape=(6, 2))
    with pytest.raises(AssertionError):
        NumpyFileManager(prefix / 'abaababababa.npyx3', dtype=np.int32)
    with pytest.raises(AssertionError):
        NumpyFileManager(prefix / 'abaababababa.npyx3', dtype=np.float32, frameShape=(5, 5))

    # test resetting an existing file
    npw = NumpyFileManager(prefix / 'tmp4.npy', 'w', dtype=np.float32, frameShape=(5, 5))
    npw.addFrame(np.ones((5, 5), dtype=np.float32))
    npw.close()
    assert np.load(prefix / 'tmp4.npy').shape == (1, 5, 5)
    npw = NumpyFileManager(prefix / 'tmp4.npy', 'w', dtype=np.int64, frameShape=(7, 7))
    npw.flushBuffer(strict=True)
    assert np.load(prefix / 'tmp4.npy').shape == (0, 7, 7)

    # test adding frames with the wrong shape to an existing file
    with pytest.raises(AssertionError):
        npw.addFrame(np.ones((9, 4, 4)))


def test_read_frames():
    __clean_tmp_dir()
    rng = np.random.default_rng(seed=42)
    arr = rng.random((10000, 20, 20))
    npw = NumpyFileManager(prefix / 'tmp.npy', 'w', frameShape=(20, 20), dtype=arr.dtype)
    for frame in arr:
        npw.addFrame(frame)
    npw.flushBuffer(strict=True)
    assert np.array_equal(npw.readFrames(50, 100), arr[50:100])
    assert np.array_equal(npw.readFrames(0, 1), arr[0:1])
    assert np.array_equal(npw.readFrames(0, 10000), arr)
    assert np.array_equal(npw.readFrames(9999, 10000), arr[9999:10000])
    assert np.array_equal(npw.readFrames(9999, 10005), arr[9999:10000])
    assert np.array_equal(npw.readFrames(499, 3000), arr[499:3000])
