
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

    

