# slsDetector package

The SLS Detectors Package is intended to control the detectors developed by the SLS Detectors Group. <br> 
The detectors currently supported are namely MYTHEN, GOTTHARD, EIGER, JUNGFRAU and MOENCH.<br>

## Installation

### Get binaries
Documentation to obtain the binaries via the conda package is available [here.](https://github.com/slsdetectorgroup/sls_detector_software)

### Get source code
One can also obtain the source code from this repository and compile as follows.
```
git clone https://github.com/slsdetectorgroup/slsDetectorPackage.git --branch 3.0.1

```
### Setup dependencies 
* Gui Client <br>
Requirements: Qt 4.8 and Qwt 6.0
```
    export QTDIR=/usr/local/Trolltech/
    export QWTDIR=/usr/local/qwt-6.0.1/
```
If either of them does not exist, the GUI client will not be built.

* Calibration wizards<br>
Requirements: ROOT
```
    export ROOTSYS=/usr/local/root-5.34
```

### Compilation 

#### Compile using script cmk.sh
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
 
For only make:
./cmk.sh

For make clean;make:
./cmk.sh -c

For using hdf5 without custom dir /blabla:
./cmk.sh -h -d /blabla

For rebuilding cmake without hdf5 
./cmk.sh -b

For using multiple cores to compile faster:<br>
(all these options work)<br>
./cmk.sh -j9<br>
./cmk.sh -cj9 #with clean<br>
./cmk.sh -hj9 #with hdf5<br>
./cmk.sh -j9 -h #with hdf<br>

For rebuilding only certain sections<br>
./cmk.sh -tg #only text client and gui<br>
./cmk.sh -r #only receiver<br>


#### Compile without script
Use cmake to create out-of-source builds, by creating a build folder parallel to source directory.
```
    $ cd ..
    $ mkdir slsDetectorPackage-build
    $ cd slsDetectorPackage-build
    $ cmake ../slsDetectorPackage -DUSE_TEXTCLIENT=ON -DUSE_RECEIVER=ON -DUSE_GUI=OFF -DCMAKE_BUILD_TYPE=Debug -DUSE_HDF5=OFF 
    $ make
```

Use the following as an example to compile statically and using specific hdf5 folder
```
    $ HDF5_ROOT=/opt/hdf5v1.10.0 cmake ../slsDetectorPackage -DUSE_TEXTCLIENT=ON -DUSE_RECEIVER=ON -DUSE_GUI=OFF -DCMAKE_BUILD_TYPE=Debug -DUSE_HDF5=ON
 ```  
The libraries and executables will be found at `bin` directory
```
    $ ls bin/
    gui_client  libSlsDetector.a  libSlsDetector.so  libSlsReceiver.a  libSlsReceiver.so
    sls_detector_acquire  sls_detector_get  slsDetectorGui  sls_detector_help  sls_detector_put  slsReceiver
```
