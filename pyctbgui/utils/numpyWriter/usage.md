# Using numpyWriter module
## concept
numpyWriter is used to write and load huge numpy arrays that can't be fully loaded in RAM. 
It is designed to write frames of a constant shape (defined by user) and incrementally add to .npy and .npz files without accessing all of its contents

### NumpyFileManager
class to handle writing in .npy files frame by frame.
its positional parameter `file` can be of type: str,pathlib.Path, zipfile.ZipExtFile. This way we can use  NumpyFileManager to open files by getting their path or 
**in read mode** it can receiver file-like objects to read their data.

the complexity of initializing from file-like objects is added to be able to read from .npz files which are simply a zip of .npy files. Furthermore now we can save our files .npz files and read from them (even when compressed (⊙_⊙) ) without loading the whole .npy or .npz in memory.

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

# Numpy like interface
# NumpyFileManager is also subscriptable
npw[50:100] # returns the array arr[50:100]
npw[0] # returns the array arr[0]

# File like interface
# the npw class's cursors is initialized on the first frame
npw.read(5) # reads five frames and updates the cursor
npw.seek(99) # updates the cursor to point it to the 99-th frame

# to ensure that files are written to disk
npw.flush()

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

# NpzFileWriter is also subscriptable and returns a NumpyFileManager initialized 
# to open the the file with the given key inside the .npz file
npz = NpzFileWriter('tmp.npz', 'r')
npz.writeArray('adc', arr1)


npz['adc'] # returns a NumpyFileManager
npz['adc'][50:100] # returns the array from 50 to 100 
# note once a NumpyFileManager instance is created internally NpzFileWriter stores it
# this is done to avoid opening and closing the same file 
# also file-like interface can be used 
npz['adc'].read(5) # returns arr[:5]
npz['adc'].seek(100) # updates the cursor
npz['adc'].read(2) # returns arr[100:2]
```