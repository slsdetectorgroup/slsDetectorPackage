## Documentation
Detailed documentation for every release including installation can be found in the [Documentation](https://slsdetectorgroup.github.io/slsDetectorPackage/index.html).

Firmware compatiblity can be found in [firmware page](https://github.com/slsdetectorgroup/slsDetectorFirmware)

## Installation

The slsDetectorPackage provides core detector software implemented in C++, along with Python bindings packaged as the slsdet Python extension module. Choose the option that best fits your environment and use case.

1. **Install pre-built binaries using conda (Recommended)**: Install pre-built binaries for the C++ client, receiver, GUI and the Python API (`slsdet`), simplifying setup across platforms.

2. **Pip**: Install only the Python extension module, either by downloading the pre-built library from PyPI or by building the extension locally from source. Available only from v9.2.0 onwards.

3. **Build from source**: Compile the entire package yourself, including both the C++ core and the Python bindings, for maximum control and customization. However, make sure that you have the dependencies installed. If installing using conda, conda will manage the [Dependencies](#31-dependencies). Avoid installing packages with pip and conda simultaneously.

### 1. Install pre-built binaries using conda (Recommended)
Conda is not only useful to manage python environments but can also
be used as a user space package manager. Dates in the tag (for eg. 2020.07.23.dev0) 
are from the developer branch. Please use released tags for stability.

We have three different packages available:
* **slsdetlib** Shared libraries and command line utilities 
* **slsdetgui** GUI
* **slsdet** Python bindings
* **moenchzmq** Moench

```
#Add channels for dependencies and our library
conda config --add channels conda-forge
conda config --add channels slsdetectorgroup
conda config --set channel_priority strict

#create and activate an environment with our library
#replace 6.1.1 with the required tag
conda create -n myenv slsdetlib=6.1.1
conda activate myenv

#ready to use
sls_detector_get exptime
etc ...
```

```
# List available versions
# lib and binaries
conda search slsdetlib
# python
conda search slsdet
# gui
conda search slsdetgui
# moench
conda search moenchzmq
```

## 2. Pip
The Python extension module `slsdet` can be installed using pip. This is available from v9.2.0 onwards.

```
#Install the Python extension module from PyPI
pip install slsdet

# or install the python extension locally from source
git clone https://github.com/slsdetectorgroup/slsDetectorPackage.git --branch 9.2.0
cd slsDetectorPackage
pip install .
```

## 3. Build from source

### 3.1. Dependencies

While we value few dependencies some libraries are required in order to not have to reinvent the wheel. Due to the state of package management in C++ we decided to bundle some of them with our source code. These are found in the libs/ directory.

#### Core

To use the basic building blocks, meaning sls_detector_get/put and the shared libraries these are needed:

* Linux, preferably recent kernel (currently no cross platform support)

* CMake >= 3.14

* C++17 compatible compiler. (We test with gcc and clang)

> **Note:** For v9.x.x of slsDetectorPackage and older, C++11 compatible compiler.

#### Python bindings

* Python >= 3.8

* pybind11 2.13.6 (packaged in libs)

> **Note:** Refer [pybind11 notes](#4-pybind-and-zeromq).

#### ZeroMQ

* Zeromq 4.3.4 (packaged in libs)

> **Note:** Refer [zeromq notes](#4-pybind-and-zeromq).

#### GUI

* Qt 5 (>= 5.9) **or** Qt 6 (>= 6.2), components Widgets, Gui, Core and
  Concurrent.  Qt 6 is the default; Qt 5 keeps older distributions such as
  RHEL 7 buildable.
  compile with qt5 with -g flag, or qt6 with -G flag


* libpng and zlib

The GUI plotting (1D/2D plots, colour maps, scale panel) is implemented with
plain Qt Widgets painting; Qwt, ROOT and OpenGL are no longer dependencies and
the bundled Qwt copy has been removed. The Plot settings tab can be detached
with **View -> Detach Plot Settings** (or **Ctrl+Shift+D**), so the
configuration stays reachable while an acquisition is started/stopped.

#### Moench executables

* libtiff

#### Documentation

The documentation is built with

* Doxygen (to extract C++ classes etc.)

* Breathe (Sphinx plugin to handle doxygen xml)

* Sphinx with sphinx_rtd_theme

#### Packaged in libs/

* catch2 (unit testing)

* rapidjson (streaming from receiver)

* pybind11 (python bindings)

* libzmq (streaming to/from receiver)

### 3.2. Download Source Code from github
```
git clone https://github.com/slsdetectorgroup/slsDetectorPackage.git --branch 7.0.0
```

> **Note:** For v6.x.x of slsDetectorPackage and older, refer [pybind11 notes on cloning](#4-pybind-and-zeromq).


### 3.3. Build from source

One can either build using cmake or use the in-built cmk.sh script.

### 3.3.1. Build using CMake

```
# outside slsDetecorPackage folder
mkdir build && cd build

# configure & generate Makefiles using cmake
# by listing all your options (alternately use ccmake described below)
# cmake3 for some systems
cmake ../slsDetectorPackage -DCMAKE_INSTALL_PREFIX=/your/install/path

# compiled to the build/bin directory
make -j12 #or whatever number of cores you are using to build

# install headers and libs in /your/install/path directory
make install
```

Instead of the cmake command, one can use ccmake to get a list of options to configure and generate Makefiles at ease.


```
# ccmake3 for some systems
ccmake ..
 
# choose the options
# first press [c] - configure (unil you see [g])
# then press [g] - generate
```

|Example cmake options|Comment|
|---|---|
| -DSLS_USE_PYTHON=ON            | Python                                 |   
| -DPython_FIND_VIRTUALENV=ONLY  | Python from only the conda env         | 
| -DSLS_USE_GUI=ON               | GUI                                    |
| -DSLS_GUI_QT_MAJOR=5           | GUI with Qt5 instead of the Qt6 default|
| -DSLS_USE_HDF5=ON              | HDF5                                   |
| -DSLS_USE_SIMULATOR=ON         | Simulator                              |

> **Note:** For v7.x.x of slsDetectorPackage and older, refer [zeromq notes for cmake option to hint library location](#4-pybind-and-zeromq).

### 3.3.2. Build using in-built cmk.sh script

```
The binaries are generated in slsDetectorPackage/build/bin directory.

Usage: $0 [-b] [-c] [-d <HDF5 directory>] [-e] [-g] [-G] [-h] [-i]
[-j <Number of threads>] [-k <CMake command>] [-l <Install directory>]
[-m] [-n] [-p] [-r] [-s] [-t] [-u] [-z]
-[no option]: only make
-b: Builds/Rebuilds CMake files normal mode
-c: Clean
-d: HDF5 Custom Directory
-e: Debug mode
-g: Build/Rebuilds gui with Qt5
-G: Build/Rebuilds gui with Qt6
-h: Builds/Rebuilds Cmake files with HDF5 package
-i: Builds tests
-j: Number of threads to compile through
-k: CMake command
-l: Install directory
-m: Manuals
-n: Manuals without compiling doxygen (only rst)
-p: Builds/Rebuilds Python API
-r: Build/Rebuilds only receiver
-s: Simulator
-t: Build/Rebuilds only text client
-u: Chip Test Gui
-z: Moench zmq processor


# display all options
./cmk.sh -?

# new build and compile in parallel (recommended basic option):
./cmk.sh -cbj5

# new build, python and compile in parallel:
./cmk.sh -cbpj5

# new build including the GUI (see 3.5 for the Qt packages)
./cmk.sh -cbgj9  # Qt 5 GUI (RHEL 7/older)
./cmk.sh -cbGj9  # Qt 6 GUI

# Switching between -g (Qt5) and -G (Qt6) removes the build directory
# automatically, so no separate -c is needed.

#For rebuilding only certain sections
./cmk.sh -tg #text client and Qt5 gui
./cmk.sh -tG #text client and Qt6 gui
./cmk.sh -r #only receiver
```

> **Note:** For v7.x.x of slsDetectorPackage and older, refer [zeromq notes for cmk script option to hint library location](#4-pybind-and-zeromq).

### 3.4. Build on old distributions using conda

If your linux distribution doesn't come with a C++17 compiler (gcc>8) then 
it's possible to install a newer gcc using conda and build the slsDetectorPackage
using this compiler

```
#Create an environment with the dependencies
conda create -n myenv gxx_linux-64 cmake zmq
conda activate myenv

# outside slsDetecorPackage folder
mkdir build && cd build
cmake ../slsDetectorPackage -DCMAKE_PREFIX_PATH=$CONDA_PREFIX
make -j12
```

> **Note:** For v7.x.x of slsDetectorPackage and older, refer [zeromq notes for dependencies for conda](#4-pybind-and-zeromq).


### 3.5. Build slsDetectorGui (Qt 5 or Qt 6)

The GUI builds against Qt 5 (>= 5.9) or Qt 6 (>= 6.2), selected with
`-DSLS_GUI_QT_MAJOR=5|6` (default 6).  Use Qt 5 on older distributions such as
RHEL 7; Qt 6 is the default elsewhere.  Qwt, expat and Qt6Svg are no longer
needed; the plotting is done with plain Qt Widgets painting.

With cmk.sh, use `-g` for Qt 5 and `-G` for Qt 6:
```
./cmk.sh -cbgj9     # Qt 5 GUI
./cmk.sh -cbGj9     # Qt 6 GUI
```

1. Using pre-built binary on conda
```
conda create -n myenv slsdetgui=7.0.0
conda activate myenv
```

2. Using system installation on RHEL 7 / RHEL 8 / RHEL 9 / Fedora
```
# Qt 6 (RHEL 8/9, Fedora)
dnf install qt6-qtbase-devel

# Qt 5 (RHEL 7 and older)
dnf install qt5-qtbase-devel
```

3. Using system installation on Debian/Ubuntu
```
apt install qt6-base-dev   # Qt 6
apt install qtbase5-dev    # Qt 5
```

4. Using conda
```
#Add channels for dependencies and our library
conda config --add channels conda-forge
conda config --add channels slsdetectorgroup
conda config --set channel_priority strict

# create environment to compile (qt6-main is the Qt 6 base package on conda-forge)
# The packaged conda recipe builds the Qt 6 GUI; for Qt 5 use the system Qt 5 packages.
conda create -n slsgui zeromq gxx_linux-64 qt6-main

# activate environment
conda activate slsgui

# compile with cmake outside slsDetecorPackage folder
mkdir build && cd build
cmake ../slsDetectorPackage -DSLS_USE_GUI=ON -DSLS_GUI_QT_MAJOR=6 -DCMAKE_PREFIX_PATH=$CONDA_PREFIX
make -j12

# or compile with cmk.sh (Qt 6; use -g instead of -G for Qt 5)
cd slsDetectorPackage
./cmk.sh -cbGj9
```

> **Note:** For v7.x.x of slsDetectorPackage and older, refer [zeromq notes for dependencies for conda](#4-pybind-and-zeromq).

### 3.6. Build documentation from package
The documentation for the slsDetectorPackage is build using a combination 
of Doxygen, Sphinx and Breathe. The easiest w(#4-pybind-and-zeromq)ay to install the dependencies
is to use conda 

```
conda create -n myenv python=3.12 sphinx sphinx_rtd_theme breathe doxygen numpy
```

```
# using cmake or ccmake to enable DSLS_BUILD_DOCS
# outside slsDetecorPackage folder
mkdir build && cd build
cmake ../slsDetectorPackage -DSLS_BUILD_DOCS=ON

make docs # generate API docs and build Sphinx RST
make rst # rst only, saves time in case the API did not change
```



## 4. Pybind and Zeromq

### Pybind11 for Python
**v8.0.0+**:
pybind11 is built
* by default from tar file in repo (libs/pybind/v2.1x.0.tar.gz)
* or use advanced option SLS_FETCH_PYBIND11_FROM_GITHUB [link].
   * v9.0.0+: pybind11 (v2.13.6)
   * v8.x.x : pybind11 (v2.11.0)

**v7.x.x**:
pybind11 packaged into ‘libs/pybind’. No longer a submodule. No need for “recursive” or “submodule update”.

**Older versions**:
pybind11 is a submodule. Must be cloned using “recursive” and updated when switching between versions using the following commands.

```
# Note: Only for v6.x.x versions and older

# clone using recursive to get pybind11 submodule
git clone --recursive https://github.com/slsdetectorgroup/slsDetectorPackage.git

# update submodule when switching between releases
cd slsDetectorPackage
git submodule update --init
```

### Zeromq
**v8.0.0+**:
zeromq (v4.3.4) is built
* by default from tar file in repo (libs/libzmq/libzmq-4.3.4.tar.gz)
* or use advanced option SLS_FETCH_ZMQ_FROM_GITHUB [link].

**v7.x.x and older**:
zeromq-devel must be installed and one can hint its location using
* cmake option:’-DZeroMQ_HINT=/usr/lib64’ or
* option ‘-q’ in cmk.sh script: : ./cmk.sh -cbj5 -q /usr/lib64
* ‘zeromq’ dependency added when installing using conda


## Support
    dhanya.thattil@psi.ch
    erik.frojdh@psi.ch
    alice.mazzoleni@psi.ch