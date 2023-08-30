# Using numpyWriter module
## concept
numpyWriter is used to write and load huge numpy arrays that can't be fully loaded in RAM. 
It is designed to write frames of a constant shape (defined by user) and incrementally add to .npy and .npz files without accessing all of its contents

### NumpyFileManager
class to handle writing in .npy files frame by frame.
its positional parameter `file` can be of type: str,pathlib.Path, zipfile.ZipExtFile. This way we can use  NumpyFileManager to open files by getting their path or 
**in read mode** it can receiver file-like objects to read their data.

the complexity of reading from file-like objects is added to be able to read from .npz files which are simply a zip of .npy files. Furthermore now we can save our files .npz files and read from them (even when compressed (⊙_⊙) ) without loading the whole .npy or .npz in memory.

NumpyFileManager uses an internal buffer to store frames until bufferMax argument is reached. then it writes the buffer's content into a file
The function flushBuffer(strict=False) can be used to flash the buffer. setting strict to True will flush the file and execure os.fsync to ensure data is persisted in the disk


### NpzFileWriter
class used to handle .npz file functionalities. it can zip existing .npy files, write a whole array in an .npz file without loading the whole .npz in memory,
and read frames from .npy files inside the .npz file

## Usage

```python
# create .npy file
npw = NumpyFileManager('file.npy', 'w', (400, 400), np.int32)
npw.addFrame(np.ones([400, 400], dtype=np.int32))
npw.close()

# read frames from existing .npy file
npw = NumpyFileManager('file.npy')
# if arr is stored in the .npy file this statement will return arr[50:100]
npw.readFrames(50, 100)

# to ensure that files are written to disk
npw.flushBuffer(strict=True)

# zip existing .npy files (stored on disk) 
# filePaths: the paths to .npy files
# keys: name of the arrays inside of the .npz file
NpzFileWriter.zipNpyFiles('file.npz', filePaths, keys, compressed=True)

# add numpy arrays incrementally to a .npz file
with NpzFileWriter('tmp.npz', 'w', compress_file=True) as npz:
    npz.writeArray('adc', arr1)
    npz.writeArray('tx', arr2)

# read frames from adc.npy inside of tmp.npz
with NpzFileWriter('tmp.npz', 'r') as npz:
    frames = npz.readFrames('adc', 5, 8)

```