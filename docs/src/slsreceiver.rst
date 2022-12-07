slsReceiver/ slsMultiReceiver
================================

| One has to start the slsReceiver before loading config file or using any receiver commands (prefix: **rx_** )

For a Single Module
    .. code-block:: bash  

        # default port 1954
        slsReceiver

        # custom port 2012
        slsReceiver -t2012


For Multiple Modules
    .. code-block:: bash  

        # each receiver (for each module) requires a unique tcp port (if all on same machine)

        # using slsReceiver in multiple consoles
        slsReceiver
        slsReceiver -t1955

        # slsMultiReceiver [starting port] [number of receivers]
        slsMultiReceiver 2012 2

        # slsMultiReceiver [starting port] [number of receivers] [print each frame header for debugging]
        slsMultiReceiver 2012 2 1


Client Commands 
-----------------

| One can remove **udp_dstmac** from the config file, as the slsReceiver fetches this from the **udp_ip**.

| One can use "auto" for **udp_dstip** if one wants to use default ip of **rx_hostname**.

| The first command to the receiver (**rx_** commands) should be **rx_hostname**. The following are the different ways to establish contact.

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

        # multi modules with custom ports on same rxr pc
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


File format
--------------

Master file is in json format.

The file name format is [fpath]/[fname]_dx_fy_[findex].raw, where x is module index and y is file index. **fname** is file name prefix and by default "run". **fpath** is '/' by default.


Each acquisition will have an increasing acquisition index or findex (if file write enabled). This can be retrieved by using **findex** command. 


Each acquisition can have multiple files (the file index number **y**), with **rx_framesperfile** being the maximum number of frames per file. The default varies for each detector type.


Some file name examples:

    .. code-block:: bash

        # first file
        path-to-file/run_d0_f0_0.raw

        # second file after reaching max frames in first file
        path-to-file/run_d0_f1_0.raw
        
        # second acquisition, first file
        path-to-file/run_d0_f0_1.raw


Each acquisition will create a master file that can be enabled/disabled using **fmaster**. This should have parameters relevant to the acquisition.


SLS Receiver Header consist of SLS Detector Header + 64 bytes of bitmask, altogether 112 bytes. The packetNumber in the sls detector header part, will be updated to number of packets caught by receiver for that frame. Furthermore, the bit mask will specify which packets have been received.

**Binary file format**

This is the default file format. 


Each data file will consist of frames, each consisting of slsReceiver Header followed by data for 1 frame.


Master file is of ASCII format and will also include the format of the slsReceiver Header.


**HDF5 file formats**

#. Compile the package with HDF5 option enabled

    #. Using cmk script: ./cmk.sh -hj9 -d [path of hdf5 dir]

    #. Enable using cmake **-DCMAKE_INSTALL_PREFIX=/path/to/hdf/installation** and **-DSLS_USE_HDF5=ON**

#. Start Receiver process

#. Load config file

#. Set file format from client or in config file
    .. code-block:: bash

        sls_detector_put fformat hdf5


| For multiple, modules, a virtual file linking all the modules is created. Both the data files and virtual files are linked in the master file.


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
            
