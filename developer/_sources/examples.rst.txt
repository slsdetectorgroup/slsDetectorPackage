.. _Cplusplus Api Examples:



Examples
===========

Setup 
------------

The examples here assume that you have compiled and installed slsDetectorPackage
to ~/sls/install and that the option for SLS_USE_SIMULATOR was enabled. This also builds
the virtual detector servers that we will be using for testing. 

We also add ~/sls/detector/install/bin to the path for convenience. 

Compile examples
-------------------

The source code of the examples is available at:
https://github.com/slsdetectorgroup/api-examples


.. code-block:: bash

    git clone https://github.com/slsdetectorgroup/api-examples.git
    mkdir build && cd build
    cmake ../api-examples -DCMAKE_PREFIX_PATH=~/sls/detector/install
    make

Below follows a short description of what is included in the examples.


Running a config file [e1]
-----------------------------


.. code-block:: cpp

    #include "sls/Detector.h"
    ...
    sls::Detector det;
    det.loadConfig("path/to/config/file.config");



To configure the connection between PC and detector the easiest 
is to run a config file. For this example we first launch a virtual Jungfrau server and
then set up the detector. 

**Launch a virtual detector server**

.. code-block:: bash

    jungfrauDetectorServer_virtual

This launches a virtual Jungfrau detector server. As default it uses port 1952 and 1953
for communication over TCP. Most commands go on 1952 and only a few such as stop and status on 1953. 

**Run example to configure**

.. code-block:: bash

    ./e1-config one_det_no_receiver.config
    - 12:01:06.371 INFO: Shared memory deleted /slsDetectorPackage_multi_0_sls_0
    - 12:01:06.371 INFO: Shared memory deleted /slsDetectorPackage_multi_0
    - 12:01:06.372 INFO: Shared memory created /slsDetectorPackage_multi_0
    - 12:01:06.376 INFO: Loading configuration file: one_det_no_receiver.config
    - 12:01:06.376 INFO: Adding detector localhost
    - 12:01:06.377 INFO: Shared memory created /slsDetectorPackage_multi_0_sls_0
    - 12:01:06.377 INFO: Checking Detector Version Compatibility
    - 12:01:06.378 INFO: Detector connecting - updating!
    hostname [localhost]


    Jungfrau detector with 1 modules configured


Using the return type sls::Result [e2]
-----------------------------------------

Since many our detectors have multiple modules we cannot return
a single value when reading from the Detector. Hostname, Ip and also
for example exposure time can differ between modules. 

Therefore we return Result<T> which is a thin wrapper around
std::vector. 

.. code-block:: cpp

    sls::Result<int> res1{1, 1, 1};
    std::cout << "res1: " << res1 << '\n';
    res1.squash();
    # return -1 if different
    res1.squash(-1);
    # throw exception with custom message if different
    res1.tsquash("Values are different);



Setting exposure time [e3]
-----------------------------------------

For setting times, like exposure time, period, delay etc. 
we use std::chrono::duration. 

Example 3 shows how to set and read exposure time as well
as converting to floating point. 

.. code-block:: cpp

    #include "sls/Detector.h"
    #include <chrono>
    ...
    std::chrono::microseconds t0{500};
    det.setExptime(t0);



