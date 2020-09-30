


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
    


Installation
==============================================

Build from source using CMake
---------------------------------

Note that on some systems, for example RH7,  cmake v3+ is available under the cmake3 alias.
It is also required to clone with the option --recursive to get the git submodules used
in the package. 


.. code-block:: bash

    git clone --recursive https://github.com/slsdetectorgroup/slsDetectorPackage.git
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
.. code-block:: bash

    # new build and make with 9 parallel threads
    ./cmk.sh -cbj9

    # build with python
    ./cmk.sh -bpj9

    # build with GUI
    ./cmk.sh -bgj9

    # build with hdf5
    ./cmk.sh -hj9 -d [path of hdf5 dir]

    # get all options
    ./cmk.sh -?


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