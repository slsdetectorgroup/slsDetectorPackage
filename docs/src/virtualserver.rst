.. _Virtual Detector Servers:
Simulators
===========

Compilation
-----------

* Using CMake, turn on the option 
    .. code-block:: bash  
        
        SLS_USE_SIMULATOR=ON

* Using cmk.sh script,
     .. code-block:: bash  
        
        ./cmk.sh -bsj9 # option -s is for simulator


Binaries
^^^^^^^^
    .. code-block:: bash  

        eigerDetectorServer_virtual
        jungfrauDetectorServer_virtual
        gotthardDetectorServer_virtual
        gotthard2DetectorServer_virtual
        mythen3DetectorServer_virtual
        moenchDetectorServer_virtual
        ctbDetectorServer_virtual


Arguments
---------

The arguments are the same as the :ref:`normal server arguments<Detector Server Arguments>`.

When using multiple modules, use different ports for each virtual server.
    .. code-block:: bash  

        # will start control server at port 1912 and stop server at port 1913
        jungfrauDetectorServer --port 1912 &

        # will start second control server at port 1914 and stop server at port 1915
        jungfrauDetectorServer --port 1914 &


Client
------

.. code-block:: bash  

    # hostname should include the port (if not default)
    sls_detector_put hostname localhost:1912+localhost:1914+

    # or use virtual command, instead of hostname
    # connects to 2 servers at localhost 
    # (control servers: 1912, 1914; stop servers: 1913, 1915)
    sls_detector_put virtual 2 1912 

Use the same in the config file.
Detector API has a method 'isVirtualDetectorServer' to check if on-board detector server is virtual.


Sample Config file
^^^^^^^^^^^^^^^^^^
There are sample config files for each detector in slsDetectorPackage/examples folder.

For a Single Module (Basic)
    .. code-block:: bash  

        hostname localhost
        rx_hostname localhost
        udp_dstip auto


For a Single Module (With Options)
    .. code-block:: bash  

        # connects to control port 1912
        hostname localhost:1912+

        # connects to receiver at ports 2012
        rx_hostname mpc1922:2012+

        # sets destination udp ports (not needed, default is 50001)
        udp_dstport 50012

        # source udp ips must be same subnet at destintaion udp ips
        udp_srcip 192.168.1.112

        # destination udp ip picked up from rx_hostname (if auto)
        udp_dstip auto

        # set file path
        fpath /tmp

For Multiple Modules
    .. code-block:: bash  

        # connects to control ports 1912, 1914 and stop ports 1913, 1915
        virtual 2 1912
        # or hostname localhost:1912+localhost:1914+

        # connects to receivers at ports 2012 and 2014
        rx_hostname mpc1922:2012+mpc1922:2013+

        # sets differernt destination udp ports
        0:udp_dstport 50012
        1:udp_dstport 50014

        # source udp ips must be same subnet at destintaion udp ips
        0:udp_srcip 192.168.1.112
        1:udp_srcip 192.168.1.114

        # destination udp ip picked up from rx_hostname (if auto)
        udp_dstip auto

        # set file path
        fpath /tmp

Receivers
----------
Same as if you would use an actual detector

For a Single Module
    .. code-block:: bash  

        slsReceiver -t2012


For Multiple Modules
    .. code-block:: bash  

        # slsMultiReceiver [starting port] [number of receivers] [print each frame header for debugging]
        slsMultiReceiver 2012 2 0 

Gui
----
| Same as if you would use an actual detector.
| Compile with SLS_USE_GUI=ON in cmake or -g option in cmk.sh script.

.. code-block:: bash  

    slsDetectorGui

Limitations
-----------

#. Data coming out of virtual server is fake. 

#. A stop will stop the virtual acquisition only at the start of every new frame.

#. Triggers are counted as number of virtual frames. trigger command to send a software trigger (Mythen3 & Eiger) is not implemented in virtual server.

#. firmware version and serial number will give 0.
