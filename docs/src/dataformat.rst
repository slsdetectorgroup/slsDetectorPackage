Data Format
================================

Each UDP port creates its own output file, which contains the image data transmitted over that port. 

Jungfrau
-------------

Single Port Configuration
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. image:: images/Jungfrau_module.png
   :width: 700px
   :align: center
   :alt: Jungfrau Module Single Port Configuration

By default, only the outer 10GbE interface is enabled, transmitting the full image over a single UDP port. This results in one file per module containing the complete image.

Total image size = 524,288 bytes 
   - 8 chips (2 x 4 grid)
   - 256 x 256 pixels (chip size)
   - 2 bytes (pixel width)



Double Port Configuration
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. image:: images/Jungfrau_two_port.png
   :width: 500px
   :align: center
   :alt: Jungfrau Module Two Port Configuration

If both interfaces are enabled using the ``numinterfaces`` command on compatible hardware and firmware, the image splits into top and bottom halves sent over two UDP ports:

   - The top half transmits via the inner interface (``udp_dstport2`` and ``udp_dstip2``).

   - The bottom half uses the outer interface(``udp_dstport`` and ``udp_dstip``).

The number of files per module equals the active UDP ports—two files per module when both interfaces are used.

Image size per UDP port = 262,144 bytes
    - **Complete Image size / 2**



Special Cases
^^^^^^^^^^^^^^

.. image:: images/Jungfrau_read_rows.png
   :width: 500px
   :align: center
   :alt: Jungfrau Module Read Partial Rows Configuration


The number of image rows per port can be adjusted using the ``readnrows`` command. By default, 512 rows are read, but a smaller value centers the readout vertically (e.g., 8 rows reads 4 above and 4 below the center). Increasing the value symmetrically expands the region toward the top and bottom.


Total image size = 32,768 bytes 
   - 8 chips (2 x 4 grid)
   - **8** x 256 pixels (chip size: **8 rows**)
   - 2 bytes (pixel width)


Moench
-------------


Eiger
-------------


Mythen3 
-------------


Gotthard2
-------------




