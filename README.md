### Note

Please do not update to any xxxx.xx.xx.dev0 tags. They are not releases, but tags for internal usage.
Use only releases with tags such as x.x.x or x.x.x-rcx.

### Documentation
#### 5.0.0 - Latest Release
Detailed documentation on the latest release can be found in the [software wiki](https://slsdetectorgroup.github.io/devdoc/index.html) and on the [official site](https://www.psi.ch/en/detectors/software).

#### Older Releases
Documentation is found in the package.

### Binaries
Binaries for the slsDetectorPackage are available through conda. 
```
#Add conda channels
conda config --add channels conda-forge
conda config --add channels slsdetectorgroup
conda config --set channel_priority strict

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

Refer [this page](https://slsdetectorgroup.github.io/devdoc/dependencies.html)  for dependencies.


#### Compilation 

Compiling can be done in two ways. Either with the convenience script
cmk.sh or directly with cmake for more control.

**1. Compile using script cmk.sh**<br>

These are mainly aimed at those not familiar with using ccmake and cmake. 
```
    The binaries are generated in slsDetectorPackage/build/bin directory.

    Usage: ./cmk.sh [-c] [-b] [-p] [e] [t] [r] [g] [s] [u] [i] [m] [n] [-h] [z] [-d <HDF5 directory>] [-l Install directory] [-k <CMake command>] [-j <Number of threads>]
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

    
    # get all options
    ./cmk.sh -?

    # new build  and compile in parallel:
    ./cmk.sh -bj5
```
 
**2. Compile without script**<br>
Use cmake to create out-of-source builds, by creating a build folder parallel to source directory. This would create a debug build with address sanitizers.
```
    $ mkdir build
    $ cd build
    $ cmake ../slsDetectorPackage  -DCMAKE_BUILD_TYPE=Debug -DSLS_USE_SANITIZER=ON
    $ make -j12 #or whatever number of threads wanted
```

To install binaries using CMake
```
    git clone --recursive https://github.com/slsdetectorgroup/slsDetectorPackage.git
    mkdir build && cd build
    cmake ../slsDetectorPackage -DCMAKE_INSTALL_PREFIX=/your/install/path
    make -j12 #or whatever number of cores you are using to build
    make install
```


### Support
    dhanya.thattil@psi.ch
    erik.frojdh@psi.ch