Consuming slsDetectorPackage
===============================

Depending on how you want to build your integration with 
slsDetectorPackage there are a few different ways to 
consume the package. 



CMake with submodule in your project
---------------------------------------

If you are using CMake to build your integration and want to build everything
in one go we support adding slsDetectorPackage as a subfolder in your cmake project. 
a minimal example would be. 

.. code-block:: cmake

    project(myDetectorIntegration)
    cmake_minimum_required(VERSION 3.12)
    add_subdirectory(slsDetectorPackage)

    #Add your executable
    add_executable(example main.cpp)
    target_compile_features(example PRIVATE cxx_std_11)

    #Link towards slsDetectorShared
    target_link_libraries(example slsDetectorShared)

An example that also uses git submodules is available in our github repo

https://github.com/slsdetectorgroup/cmake-subfolder-example


No tools minimal approach
-----------------------------

.. code-block:: c++

    #include "Detector.h"
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

    }⏎       

.. code-block:: bash

    g++ -I/install/path/include/  -L/install/path/lib64/ myapp.cpp -lSlsDetector -lSlsSupport -Wl,-rpath=../install/path/lib64