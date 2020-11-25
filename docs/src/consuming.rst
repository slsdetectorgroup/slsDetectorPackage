Consuming slsDetectorPackage
===============================

Depending on how you want to build your integration with 
slsDetectorPackage there are a few different ways to 
consume our package. The recommended way is to use one of the 
CMake approaches. 

One can test with :ref:`detector simulators<Virtual Detector Servers>` before testing the API with a real detector or when a real detector is not at hand.

CMake: slsDetectorPackage as submodule in your project
---------------------------------------------------------------

If you are using CMake to build your integration and want to build everything
in one go, we support adding slsDetectorPackage as a subfolder in your cmake project. 

A minimal CMakeLists.txt could look like this: 

.. code-block:: cmake

    project(myDetectorIntegration)
    cmake_minimum_required(VERSION 3.12)
    add_subdirectory(slsDetectorPackage)

    #Add your executable
    add_executable(example main.cpp)
    target_compile_features(example PRIVATE cxx_std_11)

    #Link towards slsDetectorShared
    target_link_libraries(example slsDetectorShared)

A fully working example can be found at:

https://github.com/slsdetectorgroup/cmake-subfolder-example


CMake: find_package(slsDetectorPackage)
------------------------------------------

If you have compiled and installed slsDetectorPackage we also support
find_package in CMake. If installed in a system wide location no path
should be needed, otherwise specify cmake prefix path. 

.. code-block:: cmake 

    cmake_minimum_required(VERSION 3.12)
    project(myintegration)

    find_package(slsDetectorPackage 5.0 REQUIRED)
    add_executable(example main.cpp)
    target_link_libraries(example slsDetectorShared)


Then assuming the slsDetectorPackage is installed in /path/to/sls/install
you should be able to configure and build your project in this way. 

.. code-block:: bash

    cmake ../path/to/your/source -DCMAKE_PREFIX_PATH=/path/to/sls/install
    make


A minimal example is available at: https://github.com/slsdetectorgroup/minimal-cmake


CMake: find_package and conda
----------------------------------

.. note::

    conda can also be used for installing dependencies such as zmq, Qt4 etc. 

find_package(slsDetectorPackage) also works if you have installed slsDetectorPackage using conda.
The only difference is that you point CMake to $CONDA_PREFIX 


.. code-block:: bash

    #assuming myenv contains slsdetlib
    conda activate myenv
    cmake ../path/to/your/source -DCMAKE_PREFIX_PATH=$CONDA_PREFIX
    make

Depending on your system compiler you might also have to install gxx_linux-64 to compiled.

No tools minimal approach
-----------------------------

While not recommended it is still possible to specify the include and library paths
manually when invoking g++. This can sometimes be handy for a quick try. 

.. code-block:: cpp

    #include "sls/Detector.h"
    #include <iostream>
    int main(){

        sls::Detector det;

        //Get all values and print them
        std::cout << "Hostname: " << det.getHostname() << "\n";
        std::cout << "Type: " << det.getDetectorType() << "\n";
        std::cout << "Udp ip: " << det.getSourceUDPIP() << "\n";


        //Get mac addr 
        const int module = 0;
        auto mac = det.getSourceUDPMAC()[module];
        std::cout << "Mac addr of module "<< module <<  " is " <<  mac.str() << '\n'; 

    }     


.. code-block:: bash

    g++ -I/install/path/include/  -L/install/path/lib64/ myapp.cpp -lSlsDetector -lSlsSupport -Wl,-rpath=../install/path/lib64