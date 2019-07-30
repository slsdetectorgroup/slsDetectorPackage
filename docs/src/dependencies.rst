Dependencies
=========================

While we value few dependencies we require some libraries in 
order to not have to reinvent the wheel. Due to the...

-----------------------
Core
-----------------------
To use the basic building blocks you need 

 * Linux (currently no cross platform support)
 * CMake > 3.9 
 * C++11 compatible compiler. (We test with gcc and clang)
 * ZeroMQ version 4

-----------------------
GUI
-----------------------

 * Qt 4.8
 * Qwt 6

-----------------------
Python bindings
-----------------------

 * Python > 3.6
 * pybind11 (packaged in libs/)