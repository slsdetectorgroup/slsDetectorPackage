### Note

Please do not update to any xxxx.xx.xx.dev0 tags. They are not releases, but tags for internal usage.
Use only releases with tags such as x.x.x or x.x.x-rcx.

### Documentation
Detailed documentation on the latest release of 5.0.0 can be found in the [software wiki](https://slsdetectorgroup.github.io/devdoc/index.html) and on the [official site.](https://www.psi.ch/en/detectors/software)

### Binaries
Binaries for the slsDetectorPackage are available through conda. 
```
#Add conda channels
conda config --add channels conda-forge
conda config --add channels slsdetectorgroup

conda install slsdetlib   #only shared lib and command line
conda install slsdet      #python bindings (includes slsdetlib)
conda install slsdetgui   #gui (includes qt4)

#Install specific version
conda install slsdet=2020.03.02.dev0 #developer version from 3 March 2020

```

### Source code
One can also obtain the source code from this repository and compile.
```
git clone https://github.com/slsdetectorgroup/slsDetectorPackage.git

```
#### Dependencies 
* Lib: c++11 compiler (gcc=>4.8), ZeroMQ 4
* Gui: Qt 4.8 and Qwt 6.0
* Calibration wizards and ctbGUI: ROOT
* Optional: HDF5


#### Compilation 

Compiling can be done in two ways. Either with the convenience script
cmk.sh or directly with cmake for more control.

**1. Compile using script cmk.sh**<br>

These are mainly aimed at those not familiar with using ccmake and cmake. After compiling, the libraries and executables will be found in `slsDetectorPackage/build/bin` directory<br>

Usage: $0 [-c] [-b] [-p] [e] [t] [r] [g] [s] [u] [i] [m] [n] [-h] [z] [-d <HDF5 directory>] [-l Install directory] [-k <CMake command>] [-j <Number of threads>]
 -[no option]: only make
 -c: Clean
 -b: Builds/Rebuilds CMake files normal mode
 -p: Builds/Rebuilds Python API
 -h: Builds/Rebuilds Cmake files with HDF5 package
 -d: HDF5 Custom Directory
 -k: CMake command
 -l: Install directory
 -t: Build/Rebuilds only text client
 -r: Build/Rebuilds only receiver
 -g: Build/Rebuilds only gui
 -s: Simulator
 -u: Chip Test Gui
 -j: Number of threads to compile through
 -e: Debug mode
 -i: Builds tests
 -m: Manuals
 -n: Manuals without compiling doxygen (only rst)
 -z: Moench zmq processor

  
eg. Rebuild when you switch to a new build and compile in parallel:
./cmk.sh -bj5

**2. Compile without script**<br>
Use cmake to create out-of-source builds, by creating a build folder parallel to source directory. This would create a debug build with address sanitizers.
```
    $ mkdir build
    $ cd build
    $ cmake ../slsDetectorPackage  -DCMAKE_BUILD_TYPE=Debug -DSLS_USE_SANITIZER=ON
    $ make -j12 #or whatever number of threads wanted
```
