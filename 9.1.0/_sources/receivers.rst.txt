Receivers
=================

Receiver processes can be run on same or different machines as the client, receives the data from the detector (via UDP packets).
When using the slsReceiver/ slsMultiReceiver, they can be further configured by the client control software (via TCP/IP) to set file name, file path, progress of acquisition etc.


To know more about detector receiver configuration, please check out :ref:`detector udp header and udp commands in the config file <detector udp header>`

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
