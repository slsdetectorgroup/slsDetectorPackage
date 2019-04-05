### Documentation
Detailed documentation can be found on the [official site.](https://www.psi.ch/detectors/users-support)

### Binaries
Documentation to obtain the binaries via the conda package is available for [lib](https://github.com/slsdetectorgroup/sls_detector_lib) and [gui](https://github.com/slsdetectorgroup/sls_detector_gui)

### Source code
One can also obtain the source code from this repository and compile while realizing the setup dependencies as required.
```
git clone https://github.com/slsdetectorgroup/slsDetectorPackage.git

```
#### Setup dependencies 
* Gui Client <br>
Requirements: Qt 4.8 and Qwt 6.0
```
    export QTDIR=/usr/local/Trolltech/
    export QWTDIR=/usr/local/qwt-6.0.1/
```
If either of them does not exist, the GUI client will not be built.

* Advanced user Calibration wizards<br>
Requirements: ROOT
```
    export ROOTSYS=/usr/local/root-5.34
```

#### Compilation 

Compiling can be done in two ways.

**1. Compile using script cmk.sh**<br>

After compiling, the libraries and executables will be found in `slsDetectorPackage/build/bin` directory<br>

Usage: [-c] [-b] [-h] [-d HDF5 directory] [-j]<br>
 * -[no option]: only make<br>
 * -c: Clean<br>
 * -b: Builds/Rebuilds CMake files normal mode<br>
 * -h: Builds/Rebuilds Cmake files with HDF5 package<br>
 * -d: HDF5 Custom Directory<br>
 * -t: Build/Rebuilds only text client<br>
 * -r: Build/Rebuilds only receiver<br>
 * -g: Build/Rebuilds only gui<br>
 * -j: Number of threads to compile through<br>
 * -e: Debug mode
 
Basic Option:
./cmk.sh -b
 
For only make:
./cmk.sh

For make clean;make:
./cmk.sh -c

For using hdf5 without custom dir /blabla:
./cmk.sh -h -d /blabla

For rebuilding cmake without hdf5 
./cmk.sh -b

For using multiple cores to compile faster:
./cmk.sh -j9<br>


For rebuilding only certain sections<br>
./cmk.sh -tg #only text client and gui<br>
./cmk.sh -r #only receiver<br>


**2. Compile without script**<br>
Use cmake to create out-of-source builds, by creating a build folder parallel to source directory.
```
    $ cd ..
    $ mkdir slsDetectorPackage-build
    $ cd slsDetectorPackage-build
    $ cmake ../slsDetectorPackage  -DCMAKE_BUILD_TYPE=Debug -DSLS_USE_HDF5=OFF 
    $ make
```

Use the following as an example to compile statically and using specific hdf5 folder
```
    $ HDF5_ROOT=/opt/hdf5v1.10.0 cmake ../slsDetectorPackage -DCMAKE_BUILD_TYPE=Debug -DSLS_USE_HDF5=ON
 ```  
After compiling, the libraries and executables will be found at `bin` directory
```
    $ ls bin/
    gui_client  libSlsDetector.a  libSlsDetector.so  libSlsReceiver.a  libSlsReceiver.so
    sls_detector_acquire  sls_detector_get  slsDetectorGui  sls_detector_help  sls_detector_put  slsReceiver slsMultiReceiver
```
