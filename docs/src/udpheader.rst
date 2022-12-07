.. _detector udp header:

Format
=======

The UDP data format for the packets consist of a common header for all detectors, followed by the data for that one packet.


Current Version
---------------------------

**v2.0 (slsDetectorPackage v7.0.0+)**

.. table:: <---------------------------------------------------- 8 bytes ---------------------------------------------------->
    :align: center
    :widths: 30,30,30,15,15

    +---------------------------------------------------------------+
    |                          frameNumber                          |
    +-------------------------------+-------------------------------+
    |            expLength          |         packetNumber          |
    +-------------------------------+-------------------------------+
    |                         **detSpec1**                          |
    +---------------------------------------------------------------+
    |                           timestamp                           |
    +---------------+---------------+---------------+---------------+
    |     modId     |      row      |     column    |  **detSpec2** |
    +---------------+---------------+---------------+-------+-------+
    |          **detSpec3**         |  **detSpec4** |detType|version|
    +-------------------------------+---------------+-------+-------+


.. note :: 

    Since there is no difference in the format of the UDP header from the detector
    from the previous version (v2.0), the version number stays the same.
    
    Only the struture member names have changed in sls_detector_defs.h



Description
------------

* **Detector specific field** descriptions are found :ref:`here<detector specific fields>`.

* **frameNumber**: framenumber to which the current packet belongs to.

* **expLength**: measured exposure time of the frame in tenths of microsecond. It is instead the sub frame number for Eiger.

* **packetNumber**: packet number of the frame to which the current data belongs to.

* **timestamp**: time measured at the start of frame exposure since the start of the current measurement. It is expressed in tenths of microsecond.

* **modId**: module ID picked up from det_id_[detector type].txt on the detector cpu.

* **row**: row position of the module in the detector system. It is calculated by the order of the module in hostname command, as well as the detsize command.

* **column**: column position of the module in the detector system.  It is calculated by the order of the module in hostname command, as well as the detsize command.

* **detType**: detector type from enum of detectorType in the package.

* **version**: current version of the detector header (0x2).


Detector Enum
--------------

    ================    ========
    Detector Type        Value
    ================    ========
    GENERIC             0
    EIGER               1
    GOTTHARD            2    
    JUNGFRAU            3    
    CHIPTESTBOARD       4        
    MOENCH              5
    MYTHEN3             6
    GOTTHARD2           7    
    ================    ========



Previous Versions
-----------------
**v2.0 (Package v4.0.0 -  6.x.x)**

.. table:: <---------------------------------------------------- 8 bytes ---------------------------------------------------->
    :align: center
    :widths: 30,30,30,15,15

    +---------------------------------------------------------------+
    |                          frameNumber                          |
    +-------------------------------+-------------------------------+
    |            expLength          |         packetNumber          |
    +-------------------------------+-------------------------------+
    |                            bunchid                            |
    +---------------------------------------------------------------+
    |                           timestamp                           |
    +---------------+---------------+---------------+---------------+
    |     modId     |    **row**    |   **column**  |  **reserved** |
    +---------------+---------------+---------------+-------+-------+
    |             debug             |  roundRNumber |detType|version|
    +-------------------------------+---------------+-------+-------+

**v1.0 (Package v3.0.0 -  3.1.5)**

.. table:: <---------------------------------------------------- 8 bytes ---------------------------------------------------->
    :align: center
    :widths: 30,30,30,15,15

    +---------------------------------------------------------------+
    |                          frameNumber                          |
    +-------------------------------+-------------------------------+
    |            expLength          |         packetNumber          |
    +-------------------------------+-------------------------------+
    |                            bunchid                            |
    +---------------------------------------------------------------+
    |                           timestamp                           |
    +---------------+---------------+---------------+---------------+
    |     modId     |    xCoord     |     yCoord    |    zCoord     |
    +---------------+---------------+---------------+-------+-------+
    |             debug             |  roundRNumber |detType|version|
    +-------------------------------+---------------+-------+-------+

