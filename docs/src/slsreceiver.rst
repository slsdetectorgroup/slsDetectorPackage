In-built Receiver   
================================



The receiver essentially listens to UDP data packets sent out by the detector. It's main features are:

- **Listening**: Receives UDP data from the detector.
- **Writing to File**: Optionally writes received data to disk.
- **Streaming via ZMQ**: Optionally streams out the data using ZeroMQ.

Each of these operations runs asynchronously and in parallel for each UDP port.


.. note ::   

    * Can be run on the same or different machine as the client.
    * Can be configured by the client. (set file name/ discard policy, get progress etc.)
    * Has to be started before the client runs any receiver specific command.


Receiver Variants
-----------------
There are three main receiver types. How to start them is described :ref:`below<Starting up the Receiver>`.


+----------------------+------------------+-----------------------------------------+--------------------+--------------------+---------------------+
| Receiver Type        | Modules Supported| Internal Architecture                   | ZMQ Streaming      | ZMQ Synchronization| Image Reconstruction|
+======================+==================+=========================================+====================+====================+=====================+
| slsReceiver          | 1                | Threads per port                        | Disabled by default| No                 | No                  |
+----------------------+------------------+-----------------------------------------+--------------------+--------------------+---------------------+
| slsMultiReceiver     | Multiple         | Multiple child processes of slsReceiver | Disabled by default| No                 | No                  |
+----------------------+------------------+-----------------------------------------+--------------------+--------------------+---------------------+
| slsFrameSynchronizer | Multiple         | Multi-threading of slsReceiver          | Enabled            | Yes, across ports  | No                  |
+----------------------+------------------+-----------------------------------------+--------------------+--------------------+---------------------+


.. _Starting up the Receiver:

Starting up the Receiver
-------------------------
For a Single Module
    .. code-block:: bash  

        # default port 1954
        slsReceiver

        # custom port 2012
        slsReceiver -t2012


For Multiple Modules
    .. code-block:: bash  

        # each receiver (for each module) requires a unique tcp port (if all on same machine)

        # using slsReceiver in multiple consoles (one for each module)
        slsReceiver
        slsReceiver -t1955

        # slsMultiReceiver [starting port] [number of receivers/modules]
        slsMultiReceiver 2012 2
        slsFrameSynchronizer 2012 2


        # slsMultiReceiver [starting port] [number of receivers/modules] [print for debugging]
        slsMultiReceiver 2012 2 1
        slsFrameSynchronizer 2012 2 1



Client Commands 
-----------------

* Client commands to the receiver begin with **rx_** or **f_** (file commands).

* **rx_hostname** has to be the first command to the receiver so the client knows which reciever process to communicate with.

* Can use 'auto' for **udp_dstip** if using 1GbE interface or the :ref:`virtual simulators<Virtual Detector Servers>`.


To know more about detector receiver setup in the config file, please check out :ref:`the detector-receiver UDP configuration in the config file<detector udp header config>` and the :ref:`detector udp format<detector udp header>`.


The following are the different ways to establish contact using **rx_hostname** command.

    .. code-block:: bash  

        # default receiver tcp port (1954)
        rx_hostname xxx

        # custom receiver port
        rx_hostname xxx:1957

        # custom receiver port
        rx_tcpport 1954
        rx_hostname xxx

        # multi modules with custom ports
        rx_hostname xxx:1955+xxx:1956+
        
        
        # multi modules using increasing tcp ports when using multi detector command
        rx_tcpport 1955
        rx_hostname xxx

        # or specify multi modules with custom ports on same rxr pc
        0:rx_tcpport 1954
        1:rx_tcpport 1955
        2:rx_tcpport 1956
        rx_hostname xxx

        # multi modules with custom ports on different rxr pc
        0:rx_tcpport 1954
        0:rx_hostname xxx
        1:rx_tcpport 1955
        1:rx_hostname yyy


| Example commands:

    .. code-block:: bash 

        # to get a list of receiver commands (these dont include file commands)
        sls_detector_get list | grep rx_

        # some file commands are:
        fwrite
        foverwrite
        findex
        fpath
        fname
        fmaster
        fformat

        # to get help on a single commands
        sls_detector_get -h rx_framescaught


Example of a config file using in-built receiver

.. code-block:: bash

    # detector hostname
    hostname bchip052+bchip053+

    # udp destination port (receiver)
    # sets increasing destination udp ports starting at 50004
    udp_dstport 50004

    # udp destination ip (receiver)
    0:udp_dstip 10.0.1.100
    1:udp_dstip 10.0.2.100

    # udp source ip (same subnet as udp_dstip)
    0:udp_srcip 10.0.1.184
    1:udp_srcip 10.0.2.184

    # udp destination mac - not required (picked up from udp_dstip)
    #udp_dstmac 22:47:d5:48:ad:ef

    # connects to receivers at increasing tcp port starting at 1954
    rx_hostname mpc3434
    # same as rx_hostname mpc3434:1954+mpc3434:1955+



Performance 
-------------

Please refer to Receiver PC Tuning options and slsReceiver Tuning under `Troubleshooting <https://slsdetectorgroup.github.io/devdoc/troubleshooting.html>`_.


Using Callbacks
----------------

One can get a callback in the receiver for each frame to:
    * manipulate the data that will be written to file, or
    * disable file writing in slsReceiver and take care of the data for each call back

When handling callbacks, the control should be returned as soon as possible, to prevent packet loss from fifo being full.

**Example**
    * `main cpp file <https://github.com/slsdetectorgroup/api-examples/blob/master/e4-receiver_callbacks.cpp>`_ 
    * `cmake file <https://github.com/slsdetectorgroup/api-examples/blob/master/CMakeLists.txt>`_. 
    * how to install the slsDetectorPackage with cmake is provided :ref:`here <build from source using cmake>`.
    * compile the example **e4-rxr** by:

        .. code-block:: bash

            cmake ../path/to/your/source -DCMAKE_PREFIX_PATH=/path/to/sls/install
            make
            
