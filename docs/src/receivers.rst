Receivers
=================


Custom Receiver
----------------

| When using  custom receiver with our package, ensure that **udp_dstmac** is also configured in the config file. This parameter is not required when using slsReceiver.

| Also ensure that there are no **rx_** commands in the config file. These commands are for configuring the slsReceiver.

| The UDP data format for the packets consist of a common header for all detectors, followed by the data for that one packet.

**The SLS Detector Header**

.. table:: <-------------------------------- 8 bytes -------------------------------->
    :align: center
    :widths: 30,30,30,30

    +--------------------------------------------------------------------+
    |frameNumber                                                         |
    +---------------------------------+----------------------------------+
    |expLength                        |packetNumber                      |
    +---------------------------------+----------------------------------+
    |bunchId                                                             |
    +--------------------------------------------------------------------+
    |timestamp                                                           |
    +----------------+----------------+----------------+-----------------+
    |modId           |row             |column          |reserved         |
    +----------------+----------------+----------------+--------+--------+
    |debug                            |roundRNumber    |detType |version |
    +---------------------------------+----------------+--------+--------+


slsReceiver
-------------

