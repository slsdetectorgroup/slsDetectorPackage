

.. _Installation:


Installation
===============

One can either install pre-built binaries using conda or build from source.

.. warning ::
    
    Before building from source make sure that you have the 
    :doc:`dependencies <../dependencies>` installed. If installing using conda, conda will 
    manage the dependencies. Avoid also installing packages with pip. 
   


Install binaries using conda
----------------------------------

Conda is not only useful to manage python environments but can also
be used as a user space package manager. Dates in the tag (for eg. 2020.07.23.dev0) 
are from the developer branch. Please use released tags for stability.

We have four different packages available:
    ==============     =============================================
    Package             Description
    ==============     =============================================
    slsdetlib           shared libraries and command line utilities 
    slsdetgui           GUI
    slsdet              Python bindings
    moenchzmq           moench
    ==============     =============================================

.. code-block:: bash

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
    ...


.. code-block:: bash

    #List available versions
    # lib and binaries
    conda search slsdetlib
    # python
    conda search slsdet
    # gui
    conda search slsdetgui
    # moench
    conda search moenchzmq




Build from source
----------------------

1. Download Source Code from github
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: bash

    git clone https://github.com/slsdetectorgroup/slsDetectorPackage.git --branch 6.1.1

.. note ::   

      For v6.x.x of slsDetectorPackage and older, refer :ref:`pybind11 notes on cloning. <pybind for different slsDetectorPackage versions>`  

.. _build from source using cmake:



2. Build from Source
^^^^^^^^^^^^^^^^^^^^^^^^^^

One can either build using cmake or use the in-built cmk.sh script.

Build using CMake
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: bash

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


Instead of the cmake command, one can use ccmake to get a list of options to configure and generate Makefiles at ease.

.. code-block:: bash

    # ccmake3 for some systems
    ccmake ..
 
    # choose the options
    # first press [c] - configure (until you see [g])
    # then press [g] - generate


===============================     ===============================
Example cmake options               Comment
===============================     ===============================
-DSLS_USE_PYTHON=ON                 Python
-DPython_FIND_VIRTUALENV=ONLY       Python from the conda env 
-DSLS_USE_GUI=ON                    GUI
-DSLS_USE_HDF5=ON                   HDF5
-DSLS_USE_SIMULATOR=ON              Simulator
===============================     ===============================

.. note ::   

    For v7.x.x of slsDetectorPackage and older, refer :ref:`zeromq notes for cmake option to hint library location. <zeromq for different slsDetectorPackage versions>` 


Build using in-built cmk.sh script
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


.. code-block:: bash

    The binaries are generated in slsDetectorPackage/build/bin directory.

    Usage: $0 [-b] [-c] [-d <HDF5 directory>] [-e] [-g] [-h] [-i] 
    [-j <Number of threads>] [-k <CMake command>] [-l <Install directory>] 
    [-m] [-n] [-p] [-r] [-s] [-t] [-u] [-z]  
    -[no option]: only make
    -b: Builds/Rebuilds CMake files normal mode
    -c: Clean
    -d: HDF5 Custom Directory
    -e: Debug mode
    -g: Build/Rebuilds gui
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

    #For rebuilding only certain sections
    ./cmk.sh -tg #only text client and gui
    ./cmk.sh -r #only receiver

.. note ::   

    For v7.x.x of slsDetectorPackage and older, refer :ref:`zeromq notes for cmk script option to hint library location. <zeromq for different slsDetectorPackage versions>` 


Build on old distributions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If your linux distribution doesn't come with a C++11 compiler (gcc>4.8) then 
it's possible to install a newer gcc using conda and build the slsDetectorPackage
using this compiler

.. code-block:: bash

    #Create an environment with the dependencies
    conda create -n myenv gxx_linux-64 cmake
    conda activate myenv

    # outside slsDetecorPackage folder
    mkdir build && cd build
    cmake ../slsDetectorPackage -DCMAKE_PREFIX_PATH=$CONDA_PREFIX
    make -j12


.. note ::   

    For v7.x.x of slsDetectorPackage and older, refer :ref:`zeromq notes for dependencies for conda. <zeromq for different slsDetectorPackage versions>` 



Build slsDetectorGui (Qt5)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. Using pre-built binary on conda

    .. code-block:: bash

        conda create -n myenv slsdetgui=7.0.0
        conda activate myenv


2. Using system installation on RHEL7

    .. code-block:: bash

        yum install qt5-qtbase-devel.x86_64
        yum install qt5-qtsvg-devel.x86_64 

3. Using system installation on RHEL8

    .. code-block:: bash

        yum install qt5-qtbase-devel.x86_64
        yum install qt5-qtsvg-devel.x86_64 
        yum install expat-devel.x86_64

4. Using conda

    .. code-block:: bash

        #Add channels for dependencies and our library
        conda config --add channels conda-forge
        conda config --add channels slsdetectorgroup
        conda config --set channel_priority strict

        # create environment to compile
        # on rhel7
        conda create -n slsgui gxx_linux-64 gxx_linux-64 mesa-libgl-devel-cos6-x86_64 qt
        # on fedora or newer systems
        conda create -n slsgui qt

        # when using conda compilers, would also need libgl, but no need for it on fedora unless maybe using it with ROOT

        # activate environment
        conda activate slsgui

        # compile with cmake outside slsDetecorPackage folder
        mkdir build && cd build
        cmake ../slsDetectorPackage -DSLS_USE_GUI=ON
        make -j12

        # or compile with cmk.sh
        cd slsDetectorPackage
        ./cmk.sh -cbgj9

.. note ::   

    For v7.x.x of slsDetectorPackage and older, refer :ref:`zeromq notes for dependencies for conda. <zeromq for different slsDetectorPackage versions>` 



Build this documentation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The documentation for the slsDetectorPackage is build using a combination 
of Doxygen, Sphinx and Breathe. The easiest way to install the dependencies
is to use conda 

.. code-block:: bash

    conda create -n myenv python=3.12 sphinx sphinx_rtd_theme breathe doxygen numpy


.. code-block:: bash

    # using cmake or ccmake to enable DSLS_BUILD_DOCS
    # outside slsDetecorPackage folder
    mkdir build && cd build
    cmake ../slsDetectorPackage -DSLS_BUILD_DOCS=ON

    make docs # generate API docs and build Sphinx RST
    make rst # rst only, saves time in case the API did not change


Pybind and Zeromq
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. _pybind for different slsDetectorPackage versions:


| **Pybind11 for Python**
| v8.0.0+: 
|   pybind11 is built
|   * by default from tar file in repo (libs/pybind/v2.1x.0.tar.gz) 
|   * or use advanced option SLS_FETCH_PYBIND11_FROM_GITHUB [`link <https://github.com/pybind/pybind11>`__].
|      * v9.0.0+: pybind11 (v2.13.6)
|      * v8.x.x : pybind11 (v2.11.0)
|
| v7.x.x:
|   pybind11 packaged into 'libs/pybind'. No longer a submodule. No need for "recursive" or "submodule update".
| 
| Older versions:
|   pybind11 is a submodule. Must be cloned using "recursive" and updated when switching between versions using the following commands.

.. code-block:: bash

    # Note: Only for v6.x.x versions and older

    # clone using recursive to get pybind11 submodule
    git clone --recursive https://github.com/slsdetectorgroup/slsDetectorPackage.git

    # update submodule when switching between releases
    cd slsDetectorPackage
    git submodule update --init


.. _zeromq for different slsDetectorPackage versions:



| **Zeromq**
| v8.0.0+:
|   zeromq (v4.3.4) is built 
|   * by default from tar file in repo (libs/libzmq/libzmq-4.3.4.tar.gz) 
|   * or use advanced option SLS_FETCH_ZMQ_FROM_GITHUB [`link <https://github.com/zeromq/libzmq.git>`__].
|
| v7.x.x and older:
|   zeromq-devel must be installed and one can hint its location using
|   * cmake option:'-DZeroMQ_HINT=/usr/lib64' or 
|   * option '-q' in cmk.sh script: : ./cmk.sh -cbj5 -q /usr/lib64
|   * 'zeromq' dependency added when installing using conda

