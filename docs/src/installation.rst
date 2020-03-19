
Installation
==============================================

Build from source using CMake
---------------------------------

.. note :: 

    The default branch of our git repository is developer. It contains the 
    latest development version. It is expected to compile and work but 
    features might be added or tweaked. In some cases the API might also change
    without being communicated. If absolute stability of the API is needed please
    use one of the release versions. 

.. code-block:: bash

    git clone https://github.com/slsdetectorgroup/slsDetectorPackage.git
    mkdir build && cd build
    cmake ../slsDetectorPackage -DCMAKE_INSTALL_PREFIX=/your/install/path
    make -j12
    make install

Install binaries using conda
--------------------------------

.. code-block:: bash

    #Add channels for dependencies and our library
    conda config --add channels conda-forge
    conda config --add channels slsdetectorgroup
    conda config --set channel_priority strict

    #cerate an environment with our library, then activate
    conda create -n myenv slsdetlib=2020.03.18.dev2
    codna activate myenv

    #ready to use
    sls_detector_get exptime
    etc ...



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