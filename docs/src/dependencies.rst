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
 * CMake >= 3.14 
 * C++17 compatible compiler. (We test with gcc and clang)

.. note ::  
    
    For v9.x.x of slsDetectorPackage and older, C++11 compatible compiler.


-----------------------
Python bindings
-----------------------

 * Python >= 3.8
 * pybind11 2.13.6 (packaged in libs)

.. note ::  

    Refer :ref:`pybind11 notes. <pybind for different slsDetectorPackage versions>`  

-----------------------
ZeroMQ
-----------------------

 * Zeromq 4.3.4 (packaged in libs)

.. note ::   

    Refer :ref:`zeromq notes. <zeromq for different slsDetectorPackage versions>` 

-----------------------
GUI
-----------------------

 * Qt 5 (>= 5.9) **or** Qt 6 (>= 6.2), components Widgets, Gui, Core and
   Concurrent.  Qt 6 is the default; Qt 5 is kept for older distributions
   such as RHEL 7.
 * libpng and zlib

The GUI plotting (1D/2D plots, colour maps, scale panel) is implemented with
plain Qt Widgets painting. Qwt, ROOT and OpenGL are not required and the
bundled Qwt copy has been removed from the sources.

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
 * libzmq (streaming to/from receiver)