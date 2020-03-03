### Documentation
Detailed documentation can be found on the [official site.](https://www.psi.ch/detectors/users-support)

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
Use cmake to create out-of-source builds, by creating a build folder parallel to source directory. This would crete a debug build with address sanitizers.
```
    $ mkdir build
    $ cd build
    $ cmake ../slsDetectorPackage  -DCMAKE_BUILD_TYPE=Debug -DSLS_USE_SANITIZER=ON
    $ make -j12 #or whatever number of threads wanted
```
