slsReceiver/ slsMultiReceiver
================================


SLS Receiver Header Format
--------------------------

It is 112 bytes and consists of:
    * 48 bytes of the SLS Detector Header (described in :ref:`the current detector header <detector udp header>`)
    * 64 bytes of packet mask

.. code-block:: cpp 
    
    typedef struct {
        uint64_t frameNumber;
        uint32_t expLength;
        uint32_t packetNumber;
        uint64_t detSpec1;
        uint64_t timestamp;
        uint16_t modId;
        uint16_t row;
        uint16_t column;
        uint16_t detSpec2;
        uint32_t detSpec3;
        uint16_t detSpec4;
        uint8_t detType;
        uint8_t version;
    } sls_detector_header;

    struct sls_receiver_header {
        sls_detector_header detHeader; /**< is the detector header */
        sls_bitset packetsMask;        /**< is the packets caught bit mask */
    };


.. note :: 

    | The packetNumber in the SLS Receiver Header will be modified to number of packets caught by receiver for that frame. For eg. Jungfrau will have 128 packets per frame. If it is less, then this is a partial frame due to missing packets.
    
    | Furthermore, the bit mask will specify which packets have been received.




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


**Binary file format**

This is the default file format. 


Each data file will consist of frames, each consisting of slsReceiver Header followed by data for 1 frame.


Master file is of ASCII format and will also include the format of the slsReceiver Header.


**HDF5 file formats**

#. Compile the package with HDF5 option enabled

    #. Using cmk script: ./cmk.sh -hj9 -d [path of hdf5 dir] (-d is optional and for custom installation)

    #. Enable using cmake **-DCMAKE_INSTALL_PREFIX=/path/to/hdf/installation** (optional) and **-DSLS_USE_HDF5=ON**

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
            
