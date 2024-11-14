Chip Test Board Frame Format
================================

Contents of a frame
--------------------

Each frame consists of 3 types of data in the following order:

    .. code-block:: text

        # only data from enabled modes are included
        [ Analog Data ] 
        [ Digital Data ]
        [ Transceiver Data ]


Each Data type is further divided into:

    .. code-block:: text

        [ Sample 0 for all enabled Channels ] 
        [ Sample 1 for all enabled Channels ] 
        ... 
        [ Sample N for all enabled Channels ]


Digital data
-------------------

The chip test board sends out all digital data. 

Only the receiver can filter them using the command `rx_dbitlist <commandline.html#term-rx_dbitlist-all-or-i0-i1-i2-...>`_. 

    .. code-block:: text

        # filtered and reordered digital data from receiver
        # Any signal that is not a byte is filled with 0's to make up a byte

        [all samples of list signal 0] 
        [all samples of list signal 1] 
        ... 
        [all samples of list signal N]




Parameters of readout modes
---------------------------------

.. list-table:: 
   :widths: 25 40 20 30
   :header-rows: 1

   * - Readout mode
     - Enable Channels
     - Number of samples
     - Number of bytes
   * - Analog
     - 1G:  `adcenable <commandline.htmlterm-adcenable-bitmask>`_ 
            
       10G: `adcenable10g <commandline.htmlterm-adcenable10g-bitmask>`_  
     - `asamples <commandline.html#term-asamples-n_samples>`_
     - 2 bytes per channel,
           
       max 32 channels
   * - Digital
     - `rx_dbitlist <commandline.html#term-rx_dbitlist-all-or-i0-i1-i2-...>`_  
              
       [filtered only by receiver, module sends out all digital data]
     - `dsamples <commandline.html#term-dsamples-n_value>`_
     - 1 bit per signal,  
          
       max 64 signals
   * - Transceiver
     - `transceiverenable <commandline.html#term-transceiverenable-bitmask>`_
     - `tsamples <commandline.html#term-tsamples-n_value>`_
     - 8 bytes per channel,  
         
       max 4 channels


