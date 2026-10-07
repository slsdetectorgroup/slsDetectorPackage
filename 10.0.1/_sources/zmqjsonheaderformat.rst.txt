ZMQ: Json Header Format
========================


**Change in field names from slsDetectorPackage v6.x.x to v7.0.0**

* detSpec1 <- bunchId
* detSpec2 <- reserved
* detSpec3 <- debug
* detSpec4 <- roundRNumber


**Format**

    .. code-block:: bash 

        {
            "jsonversion": unsigned int,
            "bitmode": unsigned int,
            "fileIndex": unsigned long int,
            "detshape": [
                unsigned int,
                unsigned int
            ],
            "shape": [
                unsigned int,
                unsigned int
            ],
            "size": unsigned int,
            "acqIndex": unsigned long int,
            "frameIndex": unsigned long int,
            "progress": double,
            "fname": string,
            "data": unsigned int,
            "completeImage": unsigned int,

            "frameNumber": unsigned long long int,
            "expLength": unsigned int,
            "packetNumber": unsigned int,
            "detSpec1": unsigned long int,
            "timestamp": unsigned long int,
            "modId": unsigned int,
            "row": unsigned int,
            "column": unsigned int,
            "detSpec2": unsigned int,
            "detSpec3": unsigned int,
            "detSpec4": unsigned int,
            "detType": unsigned int,
            "version": unsigned int,
            
            "flipRows": unsigned int,
            "quad": unsigned int,
            "addJsonHeader": {
                string : string
            }
        }

   +--------------+----------------------------------------------+
   |   Field      |       Description                            |
   +--------------+----------------------------------------------+
   | jsonversion  | Version of the json header.                  |
   |              | Value at 4 for v6.x.x and v7.x.x             |
   +--------------+----------------------------------------------+
   | bitmode      | Bits per pixel [4|8|16|32]                   |
   +--------------+----------------------------------------------+
   | fileIndex    | Current file acquisition index               |
   +--------------+----------------------------------------------+
   | detshape     | Geometry of the entire detector              |
   +--------------+----------------------------------------------+
   | shape        | Geometry of the current port streamed out    |
   +--------------+----------------------------------------------+
   | size         | Size of image of current port in bytesout    |
   +--------------+----------------------------------------------+
   | acqIndex     | Frame number from the detector (redundant)   |
   +--------------+----------------------------------------------+
   | frameIndex   | Frame number of current acquisition          |
   |              | (Starting at 0)                              |
   +--------------+----------------------------------------------+
   | progress     | Progress of current acquisition in %         |
   +--------------+----------------------------------------------+
   | fname        | Current file name                            |
   +--------------+----------------------------------------------+
   | data         | 1 if there is data following                 |
   |              | 0 if dummy header                            |
   +--------------+----------------------------------------------+
   | completeImage| 1 if no missing packets for this frame       |
   |              | in this port, else 0                         |
   +--------------+----------------------------------------------+
   | frameNumber  | Frame number                                 |
   |              | [From detector udp header]                   |
   +--------------+----------------------------------------------+
   | expLength    | subframe number (32 bit eiger)               |
   |              | or real time exposure time in 100ns (others) |
   |              | [From detector udp header]                   |
   +--------------+----------------------------------------------+
   | packetNumber | Number of packets caught for that frame      |
   +--------------+----------------------------------------------+
   | detSpec1     | See :ref:`here<Detector Specific Fields>`    |
   |              | [From detector udp header]                   |
   +--------------+----------------------------------------------+
   | timestamp    | Timestamp with 10 MHz clock                  |
   |              | [From detector udp header]                   |
   +--------------+----------------------------------------------+
   | modId        | Module Id                                    |
   |              | [From detector udp header]                   |
   +--------------+----------------------------------------------+
   | row          | Row number in detector                       |
   |              | [From detector udp header]                   |
   +--------------+----------------------------------------------+
   | column       | Column number in detector                    |
   |              | [From detector udp header]                   |
   +--------------+----------------------------------------------+
   | detSpec2     | See :ref:`here<Detector Specific Fields>`    |
   |              | [From detector udp header]                   |
   +--------------+----------------------------------------------+
   | detSpec3     | See :ref:`here<Detector Specific Fields>`    |
   |              | [From detector udp header]                   |
   +--------------+----------------------------------------------+
   | detSpec4     | See :ref:`here<Detector Specific Fields>`    |
   |              | [From detector udp header]                   |
   +--------------+----------------------------------------------+
   | detType      | Detector type enum                           |
   | detSpec3     | See :ref:`Detector enum<detector enum>`      |
   |              | [From detector udp header]                   |
   +--------------+----------------------------------------------+
   | version      | Detector header version. At 2                |
   |              | [From detector udp header]                   |
   +--------------+----------------------------------------------+
   | flipRows     | 1 if rows should be flipped.                 |
   |              | Usually for Eiger bottom.                    |
   +--------------+----------------------------------------------+
   | quad         | 1 if its an Eiger quad.                      |
   +--------------+----------------------------------------------+
   | addJsonHeader| Optional custom parameters that is required  |
   |              | for processing code.                         |
   +--------------+----------------------------------------------+

