Receivers
=================

Receiver processes can be run on same or different machines as the client, receives the data from the detector (via UDP packets).
When using the slsReceiver/ slsMultiReceiver, they can be further configured by the client control software (via TCP/IP) to set file name, file path, progress of acquisition etc.

Detector UDP Header
---------------------

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

UDP configuration in Config file
----------------------------------

#. UDP source port is hardcoded in detector server, starting at 32410.
#. **udp_dstport** : UDP destination port number. Port in receiver pc to listen to packets from the detector.
#. **udp_dstip** : IP address of UDP destination interface. IP address of interface in receiver pc to listen to packets from detector. If **auto** is used (only when using slsReceiver/ slsMultiReceiver), the IP of **rx_hostname** is picked up.
#. **udp_dstmac** : Mac address of UDP destination interface. MAC address of interface in receiver pc to list to packets from detector. Only required when using custom receiver, else slsReceiver/slsMultiReceiver picks it up from **udp_dstip**.
#. **udp_srcip** : IP address of UDP source interface. IP address of detector UDP interface to send packets from. Do not use for Eiger 1Gb interface (uses its hardware IP). For others, must be in the same subnet as **udp_dstip**.
#. **udp_srcmac** : MAC address of UDP source interface. MAC address of detector UDP interface to send packets from. Do not use for Eiger (uses hardware mac). For others, it is not necessary, but can help for switch and debugging to put unique values for each module.
 

Custom Receiver
----------------

| When using  custom receiver with our package, ensure that **udp_dstmac** is also configured in the config file. This parameter is not required when using slsReceiver.

| Cannot use "auto" for **udp_dstip**.

| Also ensure that there are no **rx_** commands in the config file. These commands are for configuring the slsReceiver.

Example of a custom receiver config file

* The main difference is the lack of **rx_** commands or file commands (eg. fwrite, fpath) and the udp_dstmac is required in config file.

.. code-block:: bash

    # detector hostname
    hostname bchip052

    # udp destination port (receiver)
    udp_dstport 50004

    # udp destination ip (receiver)
    udp_dstip 10.0.1.100

    # udp source ip (same subnet as udp_dstip)
    udp_srcip 10.0.1.184

    # udp destination mac
    udp_dstmac 22:47:d5:48:ad:ef
