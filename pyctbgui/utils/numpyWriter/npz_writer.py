from pathlib import Path
import shutil
import zipfile
import io

import numpy as np
from pyctbgui.utils.numpyWriter.npy_writer import NumpyFileManager


class NpzFileWriter:
    """
    Write data to npz file incrementally rather than compute all and write
    once, as in ``np.save``. This class can be used with ``contextlib.closing``
    to ensure closed after usage.
    """

    def __init__(self, tofile: str, mode='w', compress_file=False):
        """
        :param tofile: the ``npz`` file to write
        :param mode: must be one of {'x', 'w', 'a'}. See
               https://docs.python.org/3/library/zipfile.html for detail
        """
        self.__openedFiles = {}
        self.compression = zipfile.ZIP_DEFLATED if compress_file else zipfile.ZIP_STORED
        self.tofile = tofile
        self.mode = mode
        self.file = zipfile.ZipFile(self.tofile, mode=self.mode, compression=self.compression)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    def writeArray(self, key: str, data: np.ndarray | bytes) -> None:
        """
        overwrite existing data of name ``key``.

        :param key: the name of data to write
        :param data: the data
        """
        key += '.npy'
        with io.BytesIO() as cbuf:
            np.save(cbuf, data)
            cbuf.seek(0)
            with self.file.open(key, mode='w', force_zip64=True) as outfile:
                shutil.copyfileobj(cbuf, outfile)

    def readFrames(self, file: str, frameStart: int, frameEnd: int):
        file += '.npy'
        with self.file.open(file, mode='r') as outfile:
            npw = NumpyFileManager(outfile)
            return npw.readFrames(frameStart, frameEnd)

    @staticmethod
    def zipNpyFiles(filename: str,
                    files: list[str | Path],
                    fileKeys: list[str],
                    deleteOriginals=False,
                    compressed=False):
        compression = zipfile.ZIP_DEFLATED if compressed else zipfile.ZIP_STORED

        with zipfile.ZipFile(filename, mode='w', compression=compression, allowZip64=True) as zipf:
            for idx, file in enumerate(files):
                zipf.write(file, arcname=fileKeys[idx] + '.npy')
        if deleteOriginals:
            for file in files:
                Path.unlink(file)

    def __getitem__(self, item: str) -> NumpyFileManager:
        """
        returns NumpyFileManager file handling the .npy file under the key item inside of the .npz file
        @param item:
        @return:
        """
        if not isinstance(item, str):
            raise TypeError('given item is not of type str')
        if item not in self.__openedFiles:
            outfile = self.file.open(item + '.npy', mode='r')
            self.__openedFiles[item] = NumpyFileManager(outfile)
        return self.__openedFiles[item]

    def namelist(self):
        return sorted([key[:-4] for key in self.file.namelist()])

    def close(self):
        if hasattr(self, 'file') and self.file is not None:
            self.file.close()

    def __del__(self):
        self.close()
