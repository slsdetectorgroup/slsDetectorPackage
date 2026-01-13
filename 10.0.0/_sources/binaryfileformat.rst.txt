.. _binary file format:

Binary File Format
====================

This is the default file format that can be configured using command `fformat <commandline.html#term-fformat-binary-hdf5>`_.

    .. code-block:: bash

        sls_detector_put fformat binary


Master File
--------------

* File Name: [fpath]/[fname]_master_[findex].json :ref:`Details here<file name format>`

* It is in json format and created for every acquisition.

* It contains :ref:`attributes<master file attributes>` relevant to the acquisition. This can vary with detector type shown in :ref:`master json file examples <json master file examples>` here.

* It shows the :ref:`**SLS Receiver Header** <sls receiver header format>` format used in data files.

* Enabled/disabled using command `fmaster <commandline.html#term-fmaster-0-1>`_. 


Data File
----------

* File Name: [fpath]/[fname]_dx_fy_[findex].raw :ref:`Details here<file name format>`

* It store multiple frames sequentially, with total number of frames determined by `rx_framesperfile <commandline.html#term-rx_framesperfile-n_frames>`_ parameter.

* Each frame includes a :ref:`**sls_receiver_header** <sls receiver header format>` structure, followed by the actual frame data.

* More details on :ref:`slsReceiverHeader<sls receiver header format>` and the actual image data is described in the :ref:`Detector Image Size and Format <data format>` section.


.. _json master file examples:

JSON Master File Examples
---------------------------------------------------

Eiger
^^^^^

.. code-block:: text

    {
        "Version": 7.2,
        "Timestamp": "Wed Nov 13 15:46:30 2024",
        "Detector Type": "Eiger",
        "Timing Mode": "auto",
        "Geometry": {
            "x": 2,
            "y": 1
        },
        "Image Size in bytes": 262144,
        "Pixels": {
            "x": 512,
            "y": 256
        },
        "Max Frames Per File": 10000,
        "Frame Discard Policy": "nodiscard",
        "Frame Padding": 1,
        "Scan Parameters": "[disabled]",
        "Total Frames": 1,
        "Receiver Roi": {
            "xmin": 4294967295,
            "xmax": 4294967295,
            "ymin": 4294967295,
            "ymax": 4294967295
        },
        "Dynamic Range": 16,
        "Ten Giga": 0,
        "Exptime": "1s",
        "Period": "1s",
        "Threshold Energy": -1,
        "Sub Exptime": "2.62144ms",
        "Sub Period": "2.62144ms",
        "Quad": 0,
        "Number of rows": 256,
        "Rate Corrections": "[0]",
        "Frames in File": 1,
        "Frame Header Format": {
            "Frame Number": "8 bytes",
            "SubFrame Number/ExpLength": "4 bytes",
            "Packet Number": "4 bytes",
            "Bunch ID": "8 bytes",
            "Timestamp": "8 bytes",
            "Module Id": "2 bytes",
            "Row": "2 bytes",
            "Column": "2 bytes",
            "Reserved": "2 bytes",
            "Debug": "4 bytes",
            "Round Robin Number": "2 bytes",
            "Detector Type": "1 byte",
            "Header Version": "1 byte",
            "Packets Caught Mask": "64 bytes"
        }
    }



Jungfrau
^^^^^^^^

.. code-block:: text

    {
        "Version": 7.2,
        "Timestamp": "Wed Nov 13 13:03:53 2024",
        "Detector Type": "Jungfrau",
        "Timing Mode": "auto",
        "Geometry": {
            "x": 1,
            "y": 1
        },
        "Image Size in bytes": 1048576,
        "Pixels": {
            "x": 1024,
            "y": 512
        },
        "Max Frames Per File": 10000,
        "Frame Discard Policy": "nodiscard",
        "Frame Padding": 1,
        "Scan Parameters": "[disabled]",
        "Total Frames": 1000,
        "Receiver Roi": {
            "xmin": 4294967295,
            "xmax": 4294967295,
            "ymin": 4294967295,
            "ymax": 4294967295
        },
        "Exptime": "10us",
        "Period": "2ms",
        "Number of UDP Interfaces": 1,
        "Number of rows": 512,
        "Frames in File": 10,
        "Frame Header Format": {
            "Frame Number": "8 bytes",
            "SubFrame Number/ExpLength": "4 bytes",
            "Packet Number": "4 bytes",
            "Bunch ID": "8 bytes",
            "Timestamp": "8 bytes",
            "Module Id": "2 bytes",
            "Row": "2 bytes",
            "Column": "2 bytes",
            "Reserved": "2 bytes",
            "Debug": "4 bytes",
            "Round Robin Number": "2 bytes",
            "Detector Type": "1 byte",
            "Header Version": "1 byte",
            "Packets Caught Mask": "64 bytes"
        }
    }


Gotthard2
^^^^^^^^^^^^

.. code-block:: text

    {
        "Version": 7.2,
        "Timestamp": "Wed Nov 13 14:18:17 2024",
        "Detector Type": "Gotthard2",
        "Timing Mode": "auto",
        "Geometry": {
            "x": 1,
            "y": 1
        },
        "Image Size in bytes": 2560,
        "Pixels": {
            "x": 1280,
            "y": 1
        },
        "Max Frames Per File": 20000,
        "Frame Discard Policy": "nodiscard",
        "Frame Padding": 1,
        "Scan Parameters": "[disabled]",
        "Total Frames": 10,
        "Receiver Roi": {
            "xmin": 4294967295,
            "xmax": 4294967295,
            "ymin": 4294967295,
            "ymax": 4294967295
        },
        "Exptime": "0ns",
        "Period": "0ns",
        "Burst Mode": "burst_internal",
        "Frames in File": 10,
        "Frame Header Format": {
            "Frame Number": "8 bytes",
            "SubFrame Number/ExpLength": "4 bytes",
            "Packet Number": "4 bytes",
            "Bunch ID": "8 bytes",
            "Timestamp": "8 bytes",
            "Module Id": "2 bytes",
            "Row": "2 bytes",
            "Column": "2 bytes",
            "Reserved": "2 bytes",
            "Debug": "4 bytes",
            "Round Robin Number": "2 bytes",
            "Detector Type": "1 byte",
            "Header Version": "1 byte",
            "Packets Caught Mask": "64 bytes"
        }
    }

Mythen3
^^^^^^^

.. code-block:: text

    {
        "Version": 7.2,
        "Timestamp": "Wed Nov 13 14:39:14 2024",
        "Detector Type": "Mythen3",
        "Timing Mode": "auto",
        "Geometry": {
            "x": 1,
            "y": 1
        },
        "Image Size in bytes": 15360,
        "Pixels": {
            "x": 3840,
            "y": 1
        },
        "Max Frames Per File": 10000,
        "Frame Discard Policy": "nodiscard",
        "Frame Padding": 1,
        "Scan Parameters": "[disabled]",
        "Total Frames": 1,
        "Receiver Roi": {
            "xmin": 4294967295,
            "xmax": 4294967295,
            "ymin": 4294967295,
            "ymax": 4294967295
        },
        "Dynamic Range": 32,
        "Ten Giga": 1,
        "Period": "2ms",
        "Counter Mask": "0x7",
        "Exptime1": "0.1s",
        "Exptime2": "0.1s",
        "Exptime3": "0.1s",
        "GateDelay1": "0ns",
        "GateDelay2": "0ns",
        "GateDelay3": "0ns",
        "Gates": 1,
        "Threshold Energies": "[-1, -1, -1]",
        "Frames in File": 1,
        "Frame Header Format": {
            "Frame Number": "8 bytes",
            "SubFrame Number/ExpLength": "4 bytes",
            "Packet Number": "4 bytes",
            "Bunch ID": "8 bytes",
            "Timestamp": "8 bytes",
            "Module Id": "2 bytes",
            "Row": "2 bytes",
            "Column": "2 bytes",
            "Reserved": "2 bytes",
            "Debug": "4 bytes",
            "Round Robin Number": "2 bytes",
            "Detector Type": "1 byte",
            "Header Version": "1 byte",
            "Packets Caught Mask": "64 bytes"
        }
    }


Moench
^^^^^^

.. code-block:: text

    {
        "Version": 7.2,
        "Timestamp": "Wed Nov 13 14:41:32 2024",
        "Detector Type": "Moench",
        "Timing Mode": "auto",
        "Geometry": {
            "x": 1,
            "y": 1
        },
        "Image Size in bytes": 320000,
        "Pixels": {
            "x": 400,
            "y": 400
        },
        "Max Frames Per File": 100000,
        "Frame Discard Policy": "discardpartial",
        "Frame Padding": 1,
        "Scan Parameters": "[disabled]",
        "Total Frames": 1,
        "Receiver Roi": {
            "xmin": 4294967295,
            "xmax": 4294967295,
            "ymin": 4294967295,
            "ymax": 4294967295
        },
        "Exptime": "10us",
        "Period": "2ms",
        "Number of UDP Interfaces": 1,
        "Number of rows": 400,
        "Frames in File": 1,
        "Frame Header Format": {
            "Frame Number": "8 bytes",
            "SubFrame Number/ExpLength": "4 bytes",
            "Packet Number": "4 bytes",
            "Bunch ID": "8 bytes",
            "Timestamp": "8 bytes",
            "Module Id": "2 bytes",
            "Row": "2 bytes",
            "Column": "2 bytes",
            "Reserved": "2 bytes",
            "Debug": "4 bytes",
            "Round Robin Number": "2 bytes",
            "Detector Type": "1 byte",
            "Header Version": "1 byte",
            "Packets Caught Mask": "64 bytes"
        }
    }

Chip Test Board
^^^^^^^^^^^^^^^

.. code-block:: text

    {
        "Version": 7.2,
        "Timestamp": "Wed Nov 13 15:32:59 2024",
        "Detector Type": "ChipTestBoard",
        "Timing Mode": "auto",
        "Geometry": {
            "x": 1,
            "y": 1
        },
        "Image Size in bytes": 48018,
        "Pixels": {
            "x": 3,
            "y": 1
        },
        "Max Frames Per File": 20000,
        "Frame Discard Policy": "nodiscard",
        "Frame Padding": 1,
        "Scan Parameters": "[disabled]",
        "Total Frames": 1,
        "Receiver Roi": {
            "xmin": 4294967295,
            "xmax": 4294967295,
            "ymin": 4294967295,
            "ymax": 4294967295
        },
        "Exptime": "0ns",
        "Period": "0.18s",
        "Ten Giga": 0,
        "ADC Mask": "0x2202",
        "Analog Flag": 1,
        "Analog Samples": 8003,
        "Digital Flag": 0,
        "Digital Samples": 1000,
        "Dbit Offset": 0,
        "Dbit Reorder": 1, 
        "Dbit Bitset": 0,
        "Transceiver Mask": "0x3",
        "Transceiver Flag": 0,
        "Transceiver Samples": 1,
        "Frames in File": 1,
        "Frame Header Format": {
            "Frame Number": "8 bytes",
            "SubFrame Number/ExpLength": "4 bytes",
            "Packet Number": "4 bytes",
            "Bunch ID": "8 bytes",
            "Timestamp": "8 bytes",
            "Module Id": "2 bytes",
            "Row": "2 bytes",
            "Column": "2 bytes",
            "Reserved": "2 bytes",
            "Debug": "4 bytes",
            "Round Robin Number": "2 bytes",
            "Detector Type": "1 byte",
            "Header Version": "1 byte",
            "Packets Caught Mask": "64 bytes"
        }
    }
