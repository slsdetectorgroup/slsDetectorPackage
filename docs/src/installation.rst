


.. note :: 

    The default branch of our git repository is developer. It contains the 
    latest development version. It is expected to compile and work but 
    features might be added or tweaked. In some cases the API might also change
    without being communicated. If absolute stability of the API is needed please
    use one of the release versions. 

.. warning ::
    
    Before building from source make sure that you have the 
    :doc:`dependencies <../dependencies>` installed. If installing using conda, conda will 
    manage the dependencies.
    

.. _Installation:

Installation
==============================================

.. _build from source using cmake:

Build from source using CMake
---------------------------------

Note that on some systems, for example RH7,  cmake v3+ is available under the cmake3 alias.
It is also required to clone with the option --recursive to get the pybind11 submodules used
in the package. (Only needed for older versions than v7.0.0)


.. code-block:: bash

    git clone --recursive https://github.com/slsdetectorgroup/slsDetectorPackage.git
    
    # if older than v7.0.0 and using python, update pybind11 submodules
    cd slsDetectorPackage
    git submodule update --init

    mkdir build && cd build
    cmake ../slsDetectorPackage -DCMAKE_INSTALL_PREFIX=/your/install/path
    make -j12 #or whatever number of cores you are using to build
    make install

The easiest way to configure options is to use the ccmake utility. 

.. code-block:: bash

    #from the build directory
    ccmake .


Build using cmk.sh script
-------------------------
These are mainly aimed at those not familiar with using ccmake and cmake.

.. code-block:: bash

    The binaries are generated in slsDetectorPackage/build/bin directory.

    Usage: ./cmk.sh [-b] [-c] [-d <HDF5 directory>] [e] [g] [-h] [i] [-j <Number of threads>] [-k <CMake command>] [-l <Install directory>] [m] [n] [-p] [-q <Zmq hint directory>] [r] [s] [t] [u] [z]  
    -[no option]: only make
    -b: Builds/Rebuilds CMake files normal mode
    -c: Clean
    -d: HDF5 Custom Directory
    -e: Debug mode
    -g: Build/Rebuilds only gui
    -h: Builds/Rebuilds Cmake files with HDF5 package
    -i: Builds tests
    -j: Number of threads to compile through
    -k: CMake command
    -l: Install directory
    -m: Manuals
    -n: Manuals without compiling doxygen (only rst)
    -p: Builds/Rebuilds Python API
    -q: Zmq hint directory
    -r: Build/Rebuilds only receiver
    -s: Simulator
    -t: Build/Rebuilds only text client
    -u: Chip Test Gui
    -z: Moench zmq processor

    
    # get all options
    ./cmk.sh -?

    # new build  and compile in parallel:
    ./cmk.sh -bj5


Install binaries using conda
--------------------------------

Conda is not only useful to manage python environments but can also
be used as a user space package manager. 

We have three different packages available:

 * **slsdetlib**, shared libraries and command line utilities 
 * **slsdetgui**, GUI
 * **slsdet**, Python bindings


.. code-block:: bash

    #Add channels for dependencies and our library
    conda config --add channels conda-forge
    conda config --add channels slsdetectorgroup
    conda config --set channel_priority strict

    #cerate an environment with our library, then activate
    #replace 2020.07.20.dev0 with the required tag
    conda create -n myenv slsdetlib=2020.07.23.dev0
    conda activate myenv

    #ready to use
    sls_detector_get exptime
    etc ...


.. code-block:: bash

    #List available versions
    conda search slsdet


Build from source on old distributions
-----------------------------------------

If your linux distribution doesn't come with a C++11 compiler (gcc>4.8) then 
it's possible to install a newer gcc using conda and build the slsDetectorPackage
using this compiler

.. code-block:: bash

    #Create an environment with the dependencies
    conda create -n myenv gxx_linux-64 cmake zmq
    conda activate myenv
    cmake ../slsDetectorPackage -DCMAKE_PREFIX_PATH=$CONDA_PREFIX
    make -j12


Build this documentation
-------------------------------

The documentation for the slsDetectorPackage is build using a combination 
of Doxygen, Sphinx and Breathe. The easiest way to install the dependencies
is to use conda 

.. code-block:: bash

    conda create -n myenv python sphinx sphinx_rtd_theme

Then enable the option SLS_BUILD_DOCS to create the targets

.. code-block:: bash

    make docs # generate API docs and build Sphinx RST
    make rst # rst only, saves time in case the API did not change