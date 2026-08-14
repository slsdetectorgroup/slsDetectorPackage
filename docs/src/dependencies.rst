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

.. note::  
    
    For v9.x.x of slsDetectorPackage and older, C++11 compatible compiler.


Additionally the core requires the following dependencies: 

 * ZeroMQ 4.3.4 (packaged in libs)
 * rapidjson 1.1.0 (packaged in libs)

.. note:: 

    Both ZeroMQ and rapidjson are bundled in libs. One does not need to pre-install them on the system. Alternatively, one can fetch ZeroMQ from GitHub by passing the cmake options ``-DSLS_FETCH_ZEROMQ_FROM_GITHUB=ON``.

.. note:: 

    Refer :ref:`zeromq notes. <zeromq for different slsDetectorPackage versions>` 


------------------------------------
Dependencies to build Python module
------------------------------------

To build the python module the following dependencies are needed: 

 * Python >= 3.8
 * pybind11 2.13.8 (packaged in libs)
  
.. note:: 

    pybind11 is bundled in libs. One does not need to pre-install it on the system. Alternatively, one can fetch pybind11 from GitHub by passing the cmake option ``-DSLS_FETCH_PYBIND11_FROM_GITHUB=ON``. 

.. note::  

    Refer :ref:`pybind11 notes. <pybind for different slsDetectorPackage versions>`  

------------------------------------
Dependencies to build documentation
------------------------------------

To build this documentation that you are reading now the following dependencies are needed: 

 * Doxygen (to extract C++ classes etc.)
 * Breathe (Sphinx plugin to handle doxygen xml)
 * Sphinx with sphinx_rtd_theme

-------------------------
Dependencies to build GUI
-------------------------

To build the GUI the following dependencies are needed:

 * Qt 5.9
 * Qwt 6.1.5 (packaged in libs)

.. note:: 

    Qwt is bundled in libs. One does not need to pre-install it on the system.

------------------------------------------------------
Dependencies to build Moench and Jungfrau executables
------------------------------------------------------

To build the Moench and Jungfrau executables for preprocessing and calibration the following dependencies are needed:

 * libtiff

--------------------------------------------------
Dependencies to build Tests
--------------------------------------------------

To build the tests the following dependencies are needed:

 * Catch2 2.13.8 (packaged in libs)

.. note:: 
    
    Catch2 is bundled in libs. One does not need to pre-install it on the system. 
