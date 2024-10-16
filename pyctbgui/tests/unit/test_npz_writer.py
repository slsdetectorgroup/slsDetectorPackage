import filecmp
from pathlib import Path

import pytest

from pyctbgui.utils.numpyWriter.npy_writer import NumpyFileManager
import numpy as np

from pyctbgui.utils.numpyWriter.npz_writer import NpzFileWriter


@pytest.mark.parametrize('compressed', [True, False])
def test_incremental_npz(compressed, tmp_path):
    arr1 = np.ones((10, 5, 5))
    arr2 = np.zeros((10, 5, 5), dtype=np.int32)
    arr3 = np.ones((10, 5, 5), dtype=np.float32)
    with NpzFileWriter(tmp_path / 'tmp.npz', 'w', compress_file=compressed) as npz:
        npz.writeArray('adc', arr1)
        npz.writeArray('tx', arr2)
        npz.writeArray('signal', arr3)

    npzFile = np.load(tmp_path / 'tmp.npz')
    assert sorted(npzFile.files) == sorted(('adc', 'tx', 'signal'))
    assert np.array_equal(npzFile['adc'], np.ones((10, 5, 5)))
    assert np.array_equal(npzFile['tx'], np.zeros((10, 5, 5), dtype=np.int32))
    assert np.array_equal(npzFile['signal'], np.ones((10, 5, 5), dtype=np.float32))
    if compressed:
        np.savez_compressed(tmp_path / 'tmp2.npz', adc=arr1, tx=arr2, signal=arr3)
    else:
        np.savez(tmp_path / 'tmp2.npz', adc=arr1, tx=arr2, signal=arr3)

    assert filecmp.cmp(tmp_path / 'tmp2.npz', tmp_path / 'tmp.npz')


@pytest.mark.parametrize('compressed', [True, False])
def test_zipping_npy_files(compressed, tmp_path):
    data = {
        'arr1': np.ones((10, 5, 5)),
        'arr2': np.zeros((10, 5, 5), dtype=np.int32),
        'arr3': np.ones((10, 5, 5), dtype=np.float32)
    }
    filePaths = [tmp_path / (file + '.npy') for file in data.keys()]
    for file in data:
        np.save(tmp_path / (file + '.npy'), data[file])
    NpzFileWriter.zipNpyFiles(tmp_path / 'file.npz', filePaths, list(data.keys()), compressed=compressed)
    npz = np.load(tmp_path / 'file.npz')
    assert npz.files == list(data.keys())
    for file in data:
        assert np.array_equal(npz[file], data[file])

    if compressed:
        np.savez_compressed(tmp_path / 'numpy.npz', **data)
    else:
        np.savez(tmp_path / 'numpy.npz', **data)

    numpyz = np.load(tmp_path / 'numpy.npz')
    for file in data:
        assert np.array_equal(numpyz[file], npz[file])
    assert npz.files == numpyz.files

    # different files :(
    # for some reason numpy savez (most likely a trick with zipfile library) doesn't write the time
    # of last modification
    # assert filecmp.cmp(prefix / 'numpy.npz', prefix / 'file.npz')


@pytest.mark.parametrize('compressed', [True, False])
def test_npy_writer_with_zipfile_in_init(compressed, tmp_path):
    npz = NpzFileWriter(tmp_path / 'tmp.npz', 'w', compress_file=compressed)

    rng = np.random.default_rng(42)
    arr = rng.random((8000, 5, 5))
    npz.writeArray('adc', arr)
    with npz.file.open('adc.npy', mode='r') as outfile:
        npw = NumpyFileManager(outfile)
        assert np.array_equal(npw.readFrames(720, 7999), arr[720:7999])


def test_compression(tmp_path):
    arr = np.zeros((1000, 5, 5), dtype=np.int32)
    with NpzFileWriter(tmp_path / 'tmp.npz', 'w', compress_file=True) as npz:
        npz.writeArray('adc', arr)

    np.savez(tmp_path / 'tmp2.npz', adc=arr)

    assert Path(tmp_path / 'tmp2.npz').stat().st_size > Path(tmp_path / 'tmp.npz').stat().st_size


@pytest.mark.parametrize('compressed', [True, False])
@pytest.mark.parametrize('isPath', [True, False])
@pytest.mark.parametrize('deleteOriginals', [True, False])
def test_delete_files(compressed, isPath, deleteOriginals, tmp_path):
    data = {
        'arr1': np.ones((10, 5, 5)),
        'arr2': np.zeros((10, 5, 5), dtype=np.int32),
        'arr3': np.ones((10, 5, 5), dtype=np.float32)
    }
    filePaths = [tmp_path / (file + '.npy') for file in data.keys()]
    for file in data:
        np.save(tmp_path / (file + '.npy'), data[file])
    path = tmp_path / 'file.npz'
    path = str(path) if isPath else path
    NpzFileWriter.zipNpyFiles(path,
                              filePaths,
                              list(data.keys()),
                              deleteOriginals=deleteOriginals,
                              compressed=compressed)
    if deleteOriginals:
        for file in filePaths:
            assert not Path.exists(file)
    else:
        for file in filePaths:
            assert Path.exists(file)


def test_npz_read_frames(tmp_path):
    rng = np.random.default_rng(seed=42)
    arr1 = rng.random((10, 5, 5))

    with NpzFileWriter(tmp_path / 'tmp.npz', 'w') as npz:
        npz.writeArray('adc', arr1)

    with NpzFileWriter(tmp_path / 'tmp.npz', 'r') as npz:
        frames = npz.readFrames('adc', 5, 8)
        assert np.array_equal(frames, arr1[5:8])


def test_file_modes(tmp_path):
    rng = np.random.default_rng(seed=42)
    arr1 = rng.random((10, 5, 5))

    # check reopening with mode w
    with NpzFileWriter(tmp_path / 'tmp.npz', 'w') as npz:
        npz.writeArray('adc', arr1)

    with NpzFileWriter(tmp_path / 'tmp.npz', 'w') as npz:
        assert npz.file.namelist() == []

    # check reopening with mode x
    with NpzFileWriter(tmp_path / 'tmp.npz', 'w') as npz:
        npz.writeArray('adc', arr1)

    with pytest.raises(FileExistsError):
        with NpzFileWriter(tmp_path / 'tmp.npz', 'x'):
            pass
    # check reopening with mode r
    with NpzFileWriter(tmp_path / 'tmp.npz', 'r') as npz:
        assert np.array_equal(npz.readFrames('adc', 4, 6), arr1[4:6])
        with pytest.raises(ValueError, match=r'write\(\) requires mode \'w\'\, \'x\'\, or \'a\''):
            npz.writeArray('adc2', arr1)


@pytest.mark.filterwarnings('ignore::UserWarning')
def test_file_mode_a(tmp_path):
    rng = np.random.default_rng(seed=42)
    arr1 = rng.random((10, 5, 5))
    # check reopening with mode a
    with NpzFileWriter(tmp_path / 'tmp.npz', 'w') as npz:
        npz.writeArray('adc', arr1)

    with NpzFileWriter(tmp_path / 'tmp.npz', 'a') as npz:
        npz.writeArray('adc2', arr1)
        npz.writeArray('adc', arr1)


@pytest.mark.parametrize('compressed', [True, False])
def test_get_item(compressed, tmp_path):
    rng = np.random.default_rng(seed=42)
    arr1 = rng.random((10, 5, 5))
    arr2 = rng.random((3, 2, 2))
    # check reopening with mode a
    npz = NpzFileWriter(tmp_path / 'tmp.npz', 'w', compress_file=compressed)
    npz.writeArray('adc1', arr1)
    npz.writeArray('adc2', arr2)
    npz.writeArray('adc3', arr1)
    assert np.array_equal(npz['adc1'].read(3), arr1[:3])
    assert np.array_equal(npz['adc2'].read(1), arr2[:1])
    assert np.array_equal(npz['adc2'].read(1), arr2[1:2])
    assert np.array_equal(npz['adc2'].read(1), arr2[2:3])
    assert np.array_equal(npz['adc1'].read(3), arr1[3:6])
    assert np.array_equal(npz['adc1'].read(3), arr1[6:9])


@pytest.mark.parametrize('compressed', [True, False])
def test_namelist(compressed, tmp_path):
    rng = np.random.default_rng(seed=42)
    arr1 = rng.random((10, 5, 5))
    arr2 = rng.random((3, 2, 2))
    # check reopening with mode a
    npz = NpzFileWriter(tmp_path / 'tmp.npz', 'w', compress_file=compressed)
    npz.writeArray('adc1', arr1)
    npz.writeArray('adc2', arr2)
    npz.writeArray('adc3', arr1)
    assert npz.namelist() == ['adc1', 'adc2', 'adc3']
