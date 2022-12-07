Dependencies
=========================

While we value few dependencies some libraries are required in 
order to not have to reinvent the wheel. Due to the state of package
management in C++ we decided to bundle some of them with our source
code. These are found in the libs/ directory. 

-----------------------
Core
-----------------------
To use the basic building blocks, meaning sls_detector_get/put and 
the shared libraries these are needed: 

 * Linux, preferably recent kernel (currently no cross platform support)
 * CMake > 3.12 
 * C++11 compatible compiler. (We test with gcc and clang)
 * ZeroMQ version 4

-----------------------
GUI
-----------------------

 * Qt 5.9
 * Qwt 6.1.5 (packaged in libs/)

-----------------------
Python bindings
-----------------------

 * Python > 3.6
 * pybind11 (packaged in libs/)


-----------------------
Moench executables
-----------------------

 * libtiff

-----------------------
Documentation
-----------------------

The documentation that you are reading now is built with 

 * Doxygen (to extract C++ classes etc.)
 * Breathe (Sphinx plugin to handle doxygen xml)
 * Sphinx with sphinx_rtd_theme 

-----------------------
Packaged in libs/
-----------------------

 * catch2 (unit testing)
 * rapidjson (streaming from receiver)
 * pybind11 (python bindings)  