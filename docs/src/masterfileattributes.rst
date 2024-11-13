
.. _master file attributes:
Master File Attributes
=======================

These attributes are the same in binary and HDF5 file, but vary depending on detector type.


Eiger
^^^^^

   +-----------------------+-------------------------------------------------+
   | **Key**               | **Description**                                 |
   +-----------------------+-------------------------------------------------+
   | Version               | Version of the master file                      |
   |                       | Current value:8.0                               |
   +-----------------------+-------------------------------------------------+
   | Timestamp             | Timestamp of creation of master file            |
   +-----------------------+-------------------------------------------------+
   | Detector Type         | Detector type                                   |
   +-----------------------+-------------------------------------------------+
   | Timing Mode           | Timing Mode                                     |
   +-----------------------+-------------------------------------------------+
   | Geometry              | Number of UDP ports in x and y dimension for    |
   |                       | complete detector                               |
   +-----------------------+-------------------------------------------------+
   | Image Size in bytes   | Image size in bytes per UDP port                |
   +-----------------------+-------------------------------------------------+
   | Pixels                | Number of pixels in x and y dimension           |
   |                       | per UDP port                                    |
   +-----------------------+-------------------------------------------------+
   | Max Frames Per File   | Maximum frames per file                         |
   +-----------------------+-------------------------------------------------+
   | Frame Discard Policy  | Receiever Frame discard policy                  |
   |                       | for partial frames                              |
   +-----------------------+-------------------------------------------------+
   | Frame Padding         | Receiver Frame padding enable                   |
   |                       | for partial frames                              |
   +-----------------------+-------------------------------------------------+
   | Scan Parameters       | Scanning mode on detector                       |
   +-----------------------+-------------------------------------------------+
   | Total Frames          | Total number of frames and triggers expected    |
   +-----------------------+-------------------------------------------------+
   | Receiver Roi          | Receiver ROI in file including xmax and ymax    |
   +-----------------------+-------------------------------------------------+
   | Dynamic Range         | Bits per pixel                                  |
   +-----------------------+-------------------------------------------------+
   | Ten Giga              | 10GbE enable for data                           |
   +-----------------------+-------------------------------------------------+
   | Exptime               | Exposure time                                   |
   +-----------------------+-------------------------------------------------+
   | Period                | Period between frames                           |
   +-----------------------+-------------------------------------------------+
   | Threshold Energy      | Threshold energy                                |
   +-----------------------+-------------------------------------------------+
   | Sub Exptime           | Sub exposure time in 32 bit mode                |
   +-----------------------+-------------------------------------------------+
   | Sub Period            | Sub period between frames in 32 bit mode        |
   +-----------------------+-------------------------------------------------+
   | Quad                  | Quad enable (hardware)                          |
   +-----------------------+-------------------------------------------------+
   | Number of rows        | Number of rows enabled for readout              |
   +-----------------------+-------------------------------------------------+
   | Rate Corrections      | Rate Corrections                                |
   +-----------------------+-------------------------------------------------+
   | Frames in File        | Number of frames written to file by Receiver 0  |
   +-----------------------+-------------------------------------------------+
   | Frame Header Format   | Expected frame header format for the data files |
   +-----------------------+-------------------------------------------------+


Jungfrau
^^^^^^^^

   +-----------------------+-------------------------------------------------+
   | **Key**               | **Description**                                 |
   +-----------------------+-------------------------------------------------+
   | Version               | Version of the master file                      |
   |                       | Current value:8.0                               |
   +-----------------------+-------------------------------------------------+
   | Timestamp             | Timestamp of creation of master file            |
   +-----------------------+-------------------------------------------------+
   | Detector Type         | Detector type                                   |
   +-----------------------+-------------------------------------------------+
   | Timing Mode           | Timing Mode                                     |
   +-----------------------+-------------------------------------------------+
   | Geometry              | Number of UDP ports in x and y dimension for    |
   |                       | complete detector                               |
   +-----------------------+-------------------------------------------------+
   | Image Size in bytes   | Image size in bytes per UDP port                |
   +-----------------------+-------------------------------------------------+
   | Pixels                | Number of pixels in x and y dimension           |
   |                       | per UDP port                                    |
   +-----------------------+-------------------------------------------------+
   | Max Frames Per File   | Maximum frames per file                         |
   +-----------------------+-------------------------------------------------+
   | Frame Discard Policy  | Receiever Frame discard policy                  |
   |                       | for partial frames                              |
   +-----------------------+-------------------------------------------------+
   | Frame Padding         | Receiver Frame padding enable                   |
   |                       | for partial frames                              |
   +-----------------------+-------------------------------------------------+
   | Scan Parameters       | Scanning mode on detector                       |
   +-----------------------+-------------------------------------------------+
   | Total Frames          | Total number of frames and triggers expected    |
   +-----------------------+-------------------------------------------------+
   | Receiver Roi          | Receiver ROI in file including xmax and ymax    |
   +-----------------------+-------------------------------------------------+
   | Exptime               | Exposure time                                   |
   +-----------------------+-------------------------------------------------+
   | Period                | Period between frames                           |
   +-----------------------+-------------------------------------------------+
   | Number of UDP         | Number of UDP Interfaces enabled per module     |
   | Interfaces            |                                                 |
   +-----------------------+-------------------------------------------------+
   | Number of rows        | Number of rows enabled for readout              |
   +-----------------------+-------------------------------------------------+
   | Frames in File        | Number of frames written to file by Receiver 0  |
   +-----------------------+-------------------------------------------------+
   | Frame Header Format   | Expected frame header format for the data files |
   +-----------------------+-------------------------------------------------+

Gotthard II
^^^^^^^^^^^^

   +-----------------------+-------------------------------------------------+
   | **Key**               | **Description**                                 |
   +-----------------------+-------------------------------------------------+
   | Version               | Version of the master file                      |
   |                       | Current value:8.0                               |
   +-----------------------+-------------------------------------------------+
   | Timestamp             | Timestamp of creation of master file            |
   +-----------------------+-------------------------------------------------+
   | Detector Type         | Detector type                                   |
   +-----------------------+-------------------------------------------------+
   | Timing Mode           | Timing Mode                                     |
   +-----------------------+-------------------------------------------------+
   | Geometry              | Number of UDP ports in x and y dimension for    |
   |                       | complete detector                               |
   +-----------------------+-------------------------------------------------+
   | Image Size in bytes   | Image size in bytes per UDP port                |
   +-----------------------+-------------------------------------------------+
   | Pixels                | Number of pixels in x and y dimension           |
   |                       | per UDP port                                    |
   +-----------------------+-------------------------------------------------+
   | Max Frames Per File   | Maximum frames per file                         |
   +-----------------------+-------------------------------------------------+
   | Frame Discard Policy  | Receiever Frame discard policy                  |
   |                       | for partial frames                              |
   +-----------------------+-------------------------------------------------+
   | Frame Padding         | Receiver Frame padding enable                   |
   |                       | for partial frames                              |
   +-----------------------+-------------------------------------------------+
   | Scan Parameters       | Scanning mode on detector                       |
   +-----------------------+-------------------------------------------------+
   | Total Frames          | Total number of frames and triggers expected    |
   +-----------------------+-------------------------------------------------+
   | Receiver Roi          | Receiver ROI in file including xmax and ymax    |
   +-----------------------+-------------------------------------------------+
   | Exptime               | Exposure time                                   |
   +-----------------------+-------------------------------------------------+
   | Period                | Period between frames                           |
   +-----------------------+-------------------------------------------------+
   | Burst Mode            | Burst mode of detector                          |
   +-----------------------+-------------------------------------------------+
   | Frames in File        | Number of frames written to file by Receiver 0  |
   +-----------------------+-------------------------------------------------+
   | Frame Header Format   | Expected frame header format for the data files |
   +-----------------------+-------------------------------------------------+

Mythen3
^^^^^^^


   +-----------------------+-------------------------------------------------+
   | **Key**               | **Description**                                 |
   +-----------------------+-------------------------------------------------+
   | Version               | Version of the master file                      |
   |                       | Current value:8.0                               |
   +-----------------------+-------------------------------------------------+
   | Timestamp             | Timestamp of creation of master file            |
   +-----------------------+-------------------------------------------------+
   | Detector Type         | Detector type                                   |
   +-----------------------+-------------------------------------------------+
   | Timing Mode           | Timing Mode                                     |
   +-----------------------+-------------------------------------------------+
   | Geometry              | Number of UDP ports in x and y dimension for    |
   |                       | complete detector                               |
   +-----------------------+-------------------------------------------------+
   | Image Size in bytes   | Image size in bytes per UDP port                |
   +-----------------------+-------------------------------------------------+
   | Pixels                | Number of pixels in x and y dimension           |
   |                       | per UDP port                                    |
   +-----------------------+-------------------------------------------------+
   | Max Frames Per File   | Maximum frames per file                         |
   +-----------------------+-------------------------------------------------+
   | Frame Discard Policy  | Receiever Frame discard policy                  |
   |                       | for partial frames                              |
   +-----------------------+-------------------------------------------------+
   | Frame Padding         | Receiver Frame padding enable                   |
   |                       | for partial frames                              |
   +-----------------------+-------------------------------------------------+
   | Scan Parameters       | Scanning mode on detector                       |
   +-----------------------+-------------------------------------------------+
   | Total Frames          | Total number of frames and triggers expected    |
   +-----------------------+-------------------------------------------------+
   | Receiver Roi          | Receiver ROI in file including xmax and ymax    |
   +-----------------------+-------------------------------------------------+
   | Dynamic Range         | Bits per pixel                                  |
   +-----------------------+-------------------------------------------------+
   | Ten Giga              | 10GbE enable for data                           |
   +-----------------------+-------------------------------------------------+
   | Period                | Period between frames                           |
   +-----------------------+-------------------------------------------------+
   | Counter Mask          | Mask of counters enabled                        |
   +-----------------------+-------------------------------------------------+
   | Exptime1              | Exposure time of counter 1                      |
   +-----------------------+-------------------------------------------------+
   | Exptime2              | Exposure time of counter 2                      |
   +-----------------------+-------------------------------------------------+
   | Exptime3              | Exposure time of counter 3                      |
   +-----------------------+-------------------------------------------------+
   | GateDelay1            | Gate delay of counter 1                         |
   +-----------------------+-------------------------------------------------+
   | GateDelay2            | Gate delay of counter 2                         |
   +-----------------------+-------------------------------------------------+
   | GateDelay3            | Gate delay of counter 3                         |
   +-----------------------+-------------------------------------------------+
   | Gates                 | Number of gates                                 |
   +-----------------------+-------------------------------------------------+
   | Threshold energies    | Threshold energy of all 3 counters              |
   +-----------------------+-------------------------------------------------+
   | Frames in File        | Number of frames written to file by Receiver 0  |
   +-----------------------+-------------------------------------------------+
   | Frame Header Format   | Expected frame header format for the data files |
   +-----------------------+-------------------------------------------------+


Moench
^^^^^^

   +-----------------------+-------------------------------------------------+
   | **Key**               | **Description**                                 |
   +-----------------------+-------------------------------------------------+
   | Version               | Version of the master file                      |
   |                       | Current value:8.0                               |
   +-----------------------+-------------------------------------------------+
   | Timestamp             | Timestamp of creation of master file            |
   +-----------------------+-------------------------------------------------+
   | Detector Type         | Detector type                                   |
   +-----------------------+-------------------------------------------------+
   | Timing Mode           | Timing Mode                                     |
   +-----------------------+-------------------------------------------------+
   | Geometry              | Number of UDP ports in x and y dimension for    |
   |                       | complete detector                               |
   +-----------------------+-------------------------------------------------+
   | Image Size in bytes   | Image size in bytes per UDP port                |
   +-----------------------+-------------------------------------------------+
   | Pixels                | Number of pixels in x and y dimension           |
   |                       | per UDP port                                    |
   +-----------------------+-------------------------------------------------+
   | Max Frames Per File   | Maximum frames per file                         |
   +-----------------------+-------------------------------------------------+
   | Frame Discard Policy  | Receiever Frame discard policy                  |
   |                       | for partial frames                              |
   +-----------------------+-------------------------------------------------+
   | Frame Padding         | Receiver Frame padding enable                   |
   |                       | for partial frames                              |
   +-----------------------+-------------------------------------------------+
   | Scan Parameters       | Scanning mode on detector                       |
   +-----------------------+-------------------------------------------------+
   | Total Frames          | Total number of frames and triggers expected    |
   +-----------------------+-------------------------------------------------+
   | Receiver Roi          | Receiver ROI in file including xmax and ymax    |
   +-----------------------+-------------------------------------------------+
   | Exptime               | Exposure time                                   |
   +-----------------------+-------------------------------------------------+
   | Period                | Period between frames                           |
   +-----------------------+-------------------------------------------------+
   | Number of UDP         | Number of UDP Interfaces enabled per module     |
   | Interfaces            |                                                 |
   +-----------------------+-------------------------------------------------+
   | Number of rows        | Number of rows enabled for readout              |
   +-----------------------+-------------------------------------------------+
   | Frames in File        | Number of frames written to file by Receiver 0  |
   +-----------------------+-------------------------------------------------+
   | Frame Header Format   | Expected frame header format for the data files |
   +-----------------------+-------------------------------------------------+

Gotthard I
^^^^^^^^^^^

   +-----------------------+-------------------------------------------------+
   | **Key**               | **Description**                                 |
   +-----------------------+-------------------------------------------------+
   | Version               | Version of the master file                      |
   |                       | Current value:8.0                               |
   +-----------------------+-------------------------------------------------+
   | Timestamp             | Timestamp of creation of master file            |
   +-----------------------+-------------------------------------------------+
   | Detector Type         | Detector type                                   |
   +-----------------------+-------------------------------------------------+
   | Timing Mode           | Timing Mode                                     |
   +-----------------------+-------------------------------------------------+
   | Geometry              | Number of UDP ports in x and y dimension for    |
   |                       | complete detector                               |
   +-----------------------+-------------------------------------------------+
   | Image Size in bytes   | Image size in bytes per UDP port                |
   +-----------------------+-------------------------------------------------+
   | Pixels                | Number of pixels in x and y dimension           |
   |                       | per UDP port                                    |
   +-----------------------+-------------------------------------------------+
   | Max Frames Per File   | Maximum frames per file                         |
   +-----------------------+-------------------------------------------------+
   | Frame Discard Policy  | Receiever Frame discard policy                  |
   |                       | for partial frames                              |
   +-----------------------+-------------------------------------------------+
   | Frame Padding         | Receiver Frame padding enable                   |
   |                       | for partial frames                              |
   +-----------------------+-------------------------------------------------+
   | Scan Parameters       | Scanning mode on detector                       |
   +-----------------------+-------------------------------------------------+
   | Total Frames          | Total number of frames and triggers expected    |
   +-----------------------+-------------------------------------------------+
   | Receiver Roi          | Receiver ROI in file including xmax and ymax    |
   +-----------------------+-------------------------------------------------+
   | Exptime               | Exposure time                                   |
   +-----------------------+-------------------------------------------------+
   | Period                | Period between frames                           |
   +-----------------------+-------------------------------------------------+
   | Detector Roi          | Roi in detector restricted to an ADC.           |
   |                       | Includes xmax                                   |
   +-----------------------+-------------------------------------------------+
   | Burst Mode            | Burst mode of detector                          |
   +-----------------------+-------------------------------------------------+
   | Frames in File        | Number of frames written to file by Receiver 0  |
   +-----------------------+-------------------------------------------------+
   | Frame Header Format   | Expected frame header format for the data files |
   +-----------------------+-------------------------------------------------+

Chip Test Board
^^^^^^^^^^^^^^^


   +-----------------------+-------------------------------------------------+
   | **Key**               | **Description**                                 |
   +-----------------------+-------------------------------------------------+
   | Version               | Version of the master file                      |
   |                       | Current value:8.0                               |
   +-----------------------+-------------------------------------------------+
   | Timestamp             | Timestamp of creation of master file            |
   +-----------------------+-------------------------------------------------+
   | Detector Type         | Detector type                                   |
   +-----------------------+-------------------------------------------------+
   | Timing Mode           | Timing Mode                                     |
   +-----------------------+-------------------------------------------------+
   | Geometry              | Number of UDP ports in x and y dimension for    |
   |                       | complete detector                               |
   +-----------------------+-------------------------------------------------+
   | Image Size in bytes   | Image size in bytes per UDP port                |
   +-----------------------+-------------------------------------------------+
   | Pixels                | Number of pixels in x and y dimension           |
   |                       | per UDP port                                    |
   +-----------------------+-------------------------------------------------+
   | Max Frames Per File   | Maximum frames per file                         |
   +-----------------------+-------------------------------------------------+
   | Frame Discard Policy  | Receiever Frame discard policy                  |
   |                       | for partial frames                              |
   +-----------------------+-------------------------------------------------+
   | Frame Padding         | Receiver Frame padding enable                   |
   |                       | for partial frames                              |
   +-----------------------+-------------------------------------------------+
   | Scan Parameters       | Scanning mode on detector                       |
   +-----------------------+-------------------------------------------------+
   | Total Frames          | Total number of frames and triggers expected    |
   +-----------------------+-------------------------------------------------+
   | Receiver Roi          | Receiver ROI in file including xmax and ymax    |
   +-----------------------+-------------------------------------------------+
   | Exptime               | Exposure time                                   |
   +-----------------------+-------------------------------------------------+
   | Period                | Period between frames                           |
   +-----------------------+-------------------------------------------------+
   | Ten Giga              | Ten giga enable                                 |
   +-----------------------+-------------------------------------------------+
   | ADC Mask              | Mask of channels enabled in ADC                 |
   +-----------------------+-------------------------------------------------+
   | Analog Flag           | Analog readout enable                           |
   +-----------------------+-------------------------------------------------+
   | Analog Samples        | Number of analog samples                        |
   +-----------------------+-------------------------------------------------+
   | Digital Flag          | Digital readout enable                          |
   +-----------------------+-------------------------------------------------+
   | Digital Samples       | Number of digital samples                       |
   +-----------------------+-------------------------------------------------+
   | Dbit Offset           | Digital offset of valid data in bytes           |
   +-----------------------+-------------------------------------------------+
   | Dbit Bitset           | Digital 64 bit mask of bits enabled in receiver |
   +-----------------------+-------------------------------------------------+
   | Transceiver Mask      | Mask of channels enabled in Transceiver         |
   +-----------------------+-------------------------------------------------+
   | Transceiver Flag      | Transceiver readout enable                      |
   +-----------------------+-------------------------------------------------+
   | Transceiver Samples   | Number of transceiver samples                   |
   +-----------------------+-------------------------------------------------+
   | Frames in File        | Number of frames written to file by Receiver 0  |
   +-----------------------+-------------------------------------------------+
   | Frame Header Format   | Expected frame header format for the data files |
   +-----------------------+-------------------------------------------------+