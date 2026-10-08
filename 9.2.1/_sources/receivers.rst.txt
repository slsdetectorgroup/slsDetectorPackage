Custom Receiver
=================

The receiver essentially listens to UDP data packets sent out by the detector.

To know more about detector receiver setup in the config file, please check out :ref:`the detector-receiver UDP configuration in the config file<detector udp header config>` and the :ref:`detector udp format<detector udp header>`.


| Please note the following when using a custom receiver:

* **udp_dstmac** must be configured in the config file. This parameter is not required when using an in-built receiver.

* Cannot use "auto" for **udp_dstip**.

* No **rx_** commands in the config file. These commands are for configuring the slsReceiver.



The main difference is the lack of **rx_** commands or file commands (eg. **f**write, **f**path) and the **udp_dstmac** is required in config file.

Example of a custom receiver config file

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
