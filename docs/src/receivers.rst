Receivers
=================


Custom Receiver
----------------

| When using  custom receiver with our package, ensure that **udp_dstmac** is also configured in the config file. This parameter is not required when using slsReceiver.

| Also ensure that there are no **rx_** commands in the config file. These commands are in configuring the slsReceiver.

UDP Data Format
^^^^^^^^^^^^^^^^^
The UDP data format for the packets is common for all the detectors. It consists of the 

#. SLS Detector Header of 48 bytes
    .. list-table:: 

        * - frameNumber
        * - expLength
          - packetNumber
        * - bunchId
        * - timestamp
        * - modId
          - row
          - column
          - reserved
        * - debug
          - roundRNumber
          - detType
          - version

+----------------------------------------------------+
|                     frameNumber                    |
+-----------------------+----------------------------+
|    expLength          |         packetNumber       |
+-----------------------+----------------------------+
|                       bunchId                      |
+----------------------------------------------------+
|                      timestamp                     |
+----------+------------+------------+---------------+
| modId    |   row      | column     | reserved      |
+----------+------------+------------+-------+-------+
|      debug            |roundRNumber|detType|version|
+-----------------------+------------+-------+-------+

#. Data for one one packet

slsReceiver
-------------

