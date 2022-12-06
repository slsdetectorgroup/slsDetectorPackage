.. _detector specific fields:

Detector Specific Fields
========================

Please check out :ref:`the current detector header <detector udp header>` to see 
where the detector specific fields are placed.



Eiger
------

.. table:: Detector Specific Field
   
   +----------+------------------------------+
   | expLength| Sub Frame Number             |
   +----------+------------------------------+
   | detSpec1 | 0x0                          |
   +----------+------------------------------+
   | detSpec2 | 0x0                          |
   +----------+------------------------------+
   | detSpec3 | e14a                         |
   +----------+------------------------------+
   | detSpec4 | Round Robin Interface Number |
   +----------+------------------------------+


Jungfrau
---------

.. table:: Detector Specific Field

   +----------+------------------------------+
   | detSpec1 | Bunch Id [#]_                |
   +----------+------------------------------+
   | detSpec2 | 0                            |
   +----------+------------------------------+
   | detSpec3 | DAQ info                     |
   +----------+------------------------------+
   | detSpec4 | 0                            |
   +----------+------------------------------+


.. table:: DAQ Info Field

   +----------+--------------------+----------------------------------------------+
   |   Bits   |       Name         |   Description                                |
   +----------+--------------------+-----+----------------------------------------+
   | 0        | High gain          |  1  | High Gain enabled                      |
   |          |                    +-----+----------------------------------------+
   |          |                    |  0  | High Gain disabled                     |
   +----------+--------------------+-----+----------------------------------------+
   | 1        | Fix gain stage 1   |  1  | Gain stage 1 fixed. The switch that    |
   |          |                    |     | selects the gains stage 1 is active all|
   |          |                    |     | the time.                              |
   |          |                    +-----+----------------------------------------+
   |          |                    |  0  | Gain stage 1 unset. The switch that    |
   |          |                    |     | selects the gains stage 1 is inactive  |
   |          |                    |     | all the time.                          |
   +----------+--------------------+-----+----------------------------------------+
   | 2        | Fix gain stage 2   |  1  | Gain stage 2 fixed. The switch that    |
   |          |                    |     | selects the gains stage 2 is active all|
   |          |                    |     | the time.                              |
   |          |                    +-----+----------------------------------------+
   |          |                    |  0  | Gain stage 2 unset. The switch that    |
   |          |                    |     | selects the gains stage 2 is inactive  |
   |          |                    |     | all the time.                          |
   +----------+--------------------+-----+----------------------------------------+
   | 4        | Comparator reset   |  1  | On-chip comparator in reset state.     | 
   |          |                    |     | Dynamic-gain switching is therefore    |
   |          |                    |     | disabled.                              |
   |          |                    +-----+----------------------------------------+
   |          |                    |  0  | On-chip comparator active.             |
   +----------+--------------------+-----+-----+-----+----------------------------+
   | 7-5      | Jungfrau chip      |Bit 7|Bit 6|Bit 5| Description                |
   |          | version            +-----+-----+-----+----------------------------+
   |          |                    | 0   |   0 |  0  | v1.0                       |
   |          |                    +-----+-----+-----+----------------------------+
   |          |                    | 0   |   0 |  1  | v1.1                       |
   |          |                    +-----+-----+-----+----------------------------+
   |          |                    | 0   |   1 |  X  | Reserved                   |
   |          |                    +-----+-----+-----+----------------------------+
   |          |                    | 1   |   X |  X  | Reserved                   |
   +----------+--------------------+-----+-----+-----+----------------------------+
   | 11-8     | Storage cell select|Storage cell used for this exposure. This     |
   |          |                    |field defines the storage cell that was used  |
   |          |                    |to acquire the data of this frame             |
   +----------+--------------------+-----+----------------------------------------+
   | 12       | Force switching    |  1  | Forced switching to gain stage 1 at the|
   |          | to gain stage 1    |     | start of the exposure period.          |
   |          |                    +-----+----------------------------------------+
   |          |                    |  0  | Disabled forced gain switching to gain |
   |          |                    |     | stage 1. Dynamic gain switching        |
   |          |                    |     | conditions apply.                      |
   +----------+--------------------+-----+----------------------------------------+
   | 13       | Force switching    |  1  | Forced switching to gain stage 2 at the|
   |          | to gain stage 2    |     | start of the exposure period.          |
   |          |                    +-----+----------------------------------------+
   |          |                    | 0   | Disabled forced gain switching to gain |
   |          |                    |     | stage 2. Dynamic gain switching        |
   |          |                    |     | conditions apply.                      |
   +----------+--------------------+-----+-----+-----+----------------------------+
   | 23-16    |  10-Gigabit event  |The 8-bit event code contains value of the    |
   |          |  code              |event received over the 10 GbE interface by   |
   |          |                    |JUNGFRAU detector at the moment of the frame  |
   |          |                    |acquisition.                                  |
   +----------+--------------------+-----+----------------------------------------+
   | 31       | External input flag|  1  | External input flag detected in the    |
   |          |                    |     | last exposure.                         |
   |          |                    +-----+----------------------------------------+
   |          |                    |  0  | External input flag not detected in the|
   |          |                    |     |  last exposure.                        |
   +----------+--------------------+-----+----------------------------------------+



Gotthard2
----------

.. table:: Detector Specific Field

   +----------+------------------------------+
   | detSpec1 | Train Id [#]_                |
   +----------+------------------------------+
   | detSpec2 | Bunch Id [#]_                |
   +----------+------------------------------+
   | detSpec3 | 0                            |
   +----------+------------------------------+
   | detSpec4 | 0                            |
   +----------+------------------------------+


Mythen3
----------

.. table:: Detector Specific Field

   +----------+------------------------------+
   | detSpec1 | 0                            |
   +----------+------------------------------+
   | detSpec2 | 0                            |
   +----------+------------------------------+
   | detSpec3 | 0                            |
   +----------+------------------------------+
   | detSpec4 | 0                            |
   +----------+------------------------------+


.. [#] **Bunch Id**: bunch identification number received by the detector at the moment of frame acquisition.
.. [#] **Train Id**: train identification number received by the detector at the moment of frame acquisition.
.. [#] **Bunch Id**: bunch identification number to identify every single exposure during a burst acquisition.
