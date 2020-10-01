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

        # source udp ips must be same subnet at destintaion udp ips
        udp_srcip 192.168.1.112

        # destination udp ip picked up from rx_hostname (if auto)
        udp_dstip auto

For Multiple Modules
    .. code-block:: bash  

        # connects to mulitple modules
        hostname bchipxxx+bchipyyy+

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


Gui
----

Compile with SLS_USE_GUI=ON in cmake or -g option in cmk.sh script. One can also just use the conda binary. Refer :ref:`installation instructions<Installation>`.

.. code-block:: bash  

    slsDetectorGui


.. note ::
    | The streaming frequency (commmand: rx_zmqfreq) is set by default in the receiver to 1 (send every frame).
    | At Gui startup, this value is set to 0. It will set to use a timer of 500 ms. Hence, every frame will not be streamed to the gui. This is done to reduce the load for fast and large detectors for display purposes.
    | One can still change this setting in the gui in the Plot tab (Plotting frequency) or from the command line (rx_zmqfreq).
    | First frame is always streamed out, no matter if timer or frequency is used.