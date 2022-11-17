.. _detector udp header:


Config file
============

Commands to configure the UDP in the config file:

Source Port
-----------
    Hardcoded in detector server, starting at 32410.

udp_srcip - Source IP
---------------------
    IP address of detector UDP interface to send packets from. Do not use for Eiger 1Gb interface (uses its hardware IP). For others, must be in the same subnet as **udp_dstip**.

udp_srcmac - Source MAC
-----------------------
    MAC address of detector UDP interface to send packets from. Do not use for Eiger (uses hardware mac). For others, it is not necessary, but can help for switch and debugging to put unique values for each module.
 

udp_dstport - Desintation Port
-------------------------------
    Port in receiver pc to listen to packets from the detector.

udp_dstip - Destination IP
--------------------------
    IP address of interface in receiver pc to listen to packets from detector. If **auto** is used (only when using slsReceiver/ slsMultiReceiver), the IP of **rx_hostname** is picked up.

udp_dstmac - Destination MAC
----------------------------
    MAC address of interface in receiver pc to list to packets from detector. Only required when using custom receiver, else slsReceiver/slsMultiReceiver picks it up from **udp_dstip**.

