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
