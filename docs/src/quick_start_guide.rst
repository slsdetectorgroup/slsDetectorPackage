Quick Start Guide
=================

Detector
--------
Start up detector (with cooling if required). Ensure both control server and stop server is running on-board.

Or use a detector simulator to test. Click :ref:`here<Virtual Detector Servers>` for further instructions.

Receiver
--------

| One has to start the slsReceiver before loading config file or using any receiver commands (prefix: **rx_** )

For a Single Module
    .. code-block:: bash  

        # default port 1954
        slsReceiver

        # custom port 2012
        slsReceiver -t2012


For Multiple Modules
    .. code-block:: bash  

        # slsMultiReceiver [starting port] [number of receivers] [print each frame header for debugging]
        slsMultiReceiver 2012 2 0 


Client
------

Refer :ref:`Sample Config file` to create config file.

.. code-block:: bash  

    # load config file
    sls_detector_put config /path/sample.config

    # set number of frames
    sls_detector_put frames 5

    # acquire
    sls_detector_acquire


.. _Sample Config file:

Sample Config file
^^^^^^^^^^^^^^^^^^
There are sample config files for each detector in slsDetectorPackage/examples folder.

For a Single Module
    .. code-block:: bash  

        # connects to module
        hostname bchipxxx

        # connects to receiver at default port
        rx_hostname mpc1922
        # or to connect to specific port
        # rx_hostname mpc1922:2012

        # sets destination udp ports (not needed, default is 50001)
        udp_dstport 50012

        # 1g data out
        # source udp ips must be same subnet at destintaion udp ips
        # udp_srcip 192.168.1.112
        # destination udp ip picked up from rx_hostname (if auto)
        # udp_dstip auto

        # 10g data out
        udp_srcip 10.30.20.200
        udp_dstip 10.30.20.6

        # set file path
        fpath /tmp

For a Single Module with custom Receiver (not slsReceiver)
    .. code-block:: bash  

        # connects to module
        hostname bchipxxx

        # sets destination udp ports (not needed, default is 50001)
        udp_dstport 50012

        # source udp ips must be same subnet at destintaion udp ips
        udp_srcip 192.168.1.112

        # destination udp ip 
        udp_dstip 192.168.1.100

        # source udp mac 
        udp_srcmac aa:bb:cc:dd:ee:ff
        
        # destination udp mac 
        udp_dstmac 3c:ab:98:bf:50:60

        # set file path
        fpath /tmp

For Multiple Modules
    .. code-block:: bash  

        # connects to mulitple modules
        hostname bchipxxx+bchipyyy+

        # tcp port increases for each module (multi detector command)
        rx_tcpport 2012

        # connects to receivers at ports 2012 and 2014
        rx_hostname mpc1922

        # increasing udp ports (multi detector command)
        udp_dstport 50012

        # source udp ips must be same subnet at destintaion udp ips
        0:udp_srcip 192.168.1.112
        1:udp_srcip 192.168.1.114

        # destination udp ip picked up from rx_hostname (if auto)
        udp_dstip auto

        # set file path
        fpath /tmp

Gui
----

Compile with SLS_USE_GUI=ON in cmake or -g option in cmk.sh script. One can also just use the conda binary. Refer :ref:`installation instructions<Installation>`.

.. code-block:: bash  

    slsDetectorGui


.. note ::
    | The streaming high water mark (commmand: rx_zmqhwm) and the receiving high water mark (command: zmqhwm) is by default the lib zmq's default (currently 1000).
    | At Gui startup, these values are set to 2. Hence, for very fast detectors, many frames will be dropped to be able to view the latest in the gui.
    | One can still change this setting in the gui in the Plot tab (ZMQ Streaming), from the command line or API.
    | Both hwm's can be set to a -1 to use the lib's default.
    | Since the dummy end of acquisition packet streamed from receiver might also be lost, receiver restreams until gui acknowledges.