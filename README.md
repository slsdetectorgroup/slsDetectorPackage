# slsDetector package


## Installation

### Get source
The source code is organised into several submodules, and the top level module is
sls_detectors_package.

```
    $ git clone git@git.psi.ch:sls_detectors_software/sls_detectors_package.git
    $ cd sls_detectors_package
    $ ./checkout.sh
```

### Setup dependencies
The GUI client requires Qt 4.8 and Qwt 6.0
```
    export QTDIR=/usr/local/Trolltech/
    export QWTDIR=/usr/local/qwt-6.0.1/
```
If either of them does not exist, the GUI client will not be built.

The calibration wizards require ROOT
```
    export ROOTSYS=/usr/local/root-5.34
```

### Compile
Use cmake to create out-of-source builds, by creating an build folder parallel to source directory.
```
    $ cd ..
    $ mkdir sls_detectors_package-build
    $ cd sls_detectors_package-build
    $ cmake ../sls_detectors_package
    $ make
```

Use the following as an example to compile statically and using specific hdf5 folder
```
    $ HDF5_ROOT=/opt/hdf5v1.10.0 cmake -DHDF5_USE_STATIC_LIBRARIES=TRUE ../slsDetectorsPackage
 ```  
The libraries and executables will be found at `bin` directory
```
    $ ls bin/
    gui_client  libSlsDetector.a  libSlsDetector.so  libSlsReceiver.a  libSlsReceiver.so
    sls_detector_acquire  sls_detector_get  slsDetectorGui  sls_detector_help  sls_detector_put  slsReceiver
```
