.. _using multiple detectors:

Using multiple detectors
==========================

The slsDetectorPackage supports using several detectors on the same computer.
This can either be two users, that need to use the same computer without interfering
with each other, or the same user that wants to use multiple detectors at the same time.
The detectors in turn can consist of multiple modules. For example, a 9M Jungfrau detector
consists of 18 modules which typically are addressed at once as a single detector.

.. note ::

    To address a single module of a multi-module detector you can use the module index. 
    
    - Command line: :ref:`cl-module-index-label`
    - Python: :ref:`py-module-index-label`


Coming back to multiple detectors we have two tools to our disposal:

#. Detector index 
#. The SLSDETNAME environment variable

They can be used together or separately depending on the use case.

Detector index
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

When configuring a detector you can specify a detector index. The default is 0. 

**Command line**

.. code-block:: bash

    # Given that we have two detectors (my-det and my-det2) that we want to use,
    # we can configure them with different indices.

    # Configure the first detector with index 0
    $ sls_detector_put hostname my-det
    
    # Set number of frames for detector 0 to 10
    $ sls_detector_put frames 10


    # 
    #Configure the second detector with index 1 (notice the 1- before hostname)
    $ sls_detector_put 1-hostname my-det2
    

    # Further configuration
    ...

    # Set number of frames for detector 1 to 19
    $ sls_detector_put 1-frames 19

    # Note that if we call sls_detector_get without specifying the index,
    # it will return the configuration of detector 0
    $ sls_detector_get frames
    10

The detector index is added to the name of the shared memory segment, so in this case
the shared memory segments would be:

.. code-block:: bash


    #First detector
    /dev/shm/slsDetectorPackage_detector_0
    /dev/shm/slsDetectorPackage_detector_0_module_0

    #Second detector
    /dev/shm/slsDetectorPackage_detector_1
    /dev/shm/slsDetectorPackage_detector_1_module_0


**Python**

The main difference between the command line and the Python API is that you set the index
when you create the detector object and you don't have to repeat it for every call.

The C++ API works int the same way. 

.. code-block:: python

    from slsdet import Detector
    

    # The same can be achieved in Python by creating a detector object with an index.
    # Again we have two detectors (my-det and my-det2) that we want to use:

    # Configure detector with index 0
    d = Detector()

    # If the detector has already been configured and has a shared memory
    # segment, you can omit setting the hostname again
    d.hostname = 'my-det'

    #Further configuration
    ...

    # Configure a second detector with index 1
    d2 = Detector(1)
    d2.hostname = 'my-det2'
   
    d.frames = 10
    d2.frames = 19
 

$SLSDETNAME
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To avoid interfering with other users on shared PCs it is best to always set the SLSDETNAME environmental variable.
Imagining a fictive user: Anna, we can set SLSDETNAME from the shell before configuring the detector:

**Command line**

.. code-block:: bash

    # Set the SLSDETNAME variable
    $ export SLSDETNAME=Anna

    # You can check that it is set
    $ echo $SLSDETNAME
    Anna

    # Now configures a detector with index 0 and prefixed with the name Anna
    # /dev/shm/slsDetectorPackage_detector_0_Anna
    $ sls_detector_put hostname my-det


.. tip ::

    Set SLSDETNAME in your .bashrc in order to not forget it when opening a new terminal.


**Python**

With python the best way is to set the SLSDETNAME from the command line before starting the python interpreter.

Bash:

.. code-block:: bash

    $ export SLSDETNAME=Anna

Python:

.. code-block:: python

    from slsdet import Detector
    
    # Now configures a detector with index 0 and prefixed with the name Anna
    # /dev/shm/slsDetectorPackage_detector_0_Anna
    d = Detector()
    d.hostname = 'my-det'

You can also set SLSDETNAME from within the Python interpreter, but you have to be aware that it will only
affect the current process and not the whole shell session.

.. code-block:: python

    import os
    os.environ['SLSDETNAME'] = 'Anna'

    # You can check that it is set
    print(os.environ['SLSDETNAME'])  # Output: Anna

    #Now SLSDETNAME is set to Anna but as soon as you exit the python interpreter
    # it will not be set anymore

.. note ::

    Python has two ways of reading environment variables: `**os.environ**` as shown above which throws a
    KeyError if the variable is not set and `os.getenv('SLSDETNAME')` which returns None if the variable is not set.

    For more details see the official python documentation on: https://docs.python.org/3/library/os.html#os.environ


Checking for other detectors
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If using shared accounts on a shared computer (which you anyway should not do), it is good practice to check
if there are other detectors configured by other users before configuring your own detector.

You can do this by listing the files in the shared memory directory `/dev/shm/` that start with `sls`. In this
example we can see that two single module detectors are configured one with index 0 and one with index 1.
SLSDETNAME is set to `Anna` so it makes sense to assume that she is the user that configured these detectors.


.. code-block:: bash

    # List the files in /dev/shm that starts with sls
    $ ls /dev/shm/sls*
    /dev/shm/slsDetectorPackage_detector_0_Anna
    /dev/shm/slsDetectorPackage_detector_0_module_0_Anna
    /dev/shm/slsDetectorPackage_detector_1_Anna
    /dev/shm/slsDetectorPackage_detector_1_module_0_Anna

We also provide a command: user, which gets information about the shared memory segment that
the client points to without doing any changes. 

.. code-block:: bash

    #in this case 3 simulated Mythen3 modules
    $ sls_detector_get user
    user 
    Hostname: localhost+localhost+localhost+
    Type: Mythen3
    PID: 1226078
    User: l_msdetect
    Date: Mon Jun  2 05:46:20 PM CEST 2025


Other considerations
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The shared memory is not the only way to interfere with other users. You also need to make sure that you are not
using the same:

* rx_tcpport
* Unique combination of udp_dstip and udp_dstport
* rx_zmqport
* zmqport

.. attention ::

    The computer that you are using need to have enough resources to run multiple detectors at the same time.
    This includes CPU and network bandwidth. Please coordinate with the other users!

