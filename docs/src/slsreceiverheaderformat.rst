
.. _sls receiver header format:

SLS Receiver Header Format
====================================================

It is 112 bytes and consists of:
    * 48 bytes of the SLS Detector Header 
    * 64 bytes of packet mask

.. code-block:: cpp 
    
    typedef struct {
        uint64_t frameNumber;
        uint32_t expLength;
        uint32_t packetNumber;
        uint64_t detSpec1;
        uint64_t timestamp;
        uint16_t modId;
        uint16_t row;
        uint16_t column;
        uint16_t detSpec2;
        uint32_t detSpec3;
        uint16_t detSpec4;
        uint8_t detType;
        uint8_t version;
    } sls_detector_header;

    struct sls_receiver_header {
        sls_detector_header detHeader; /**< is the detector header */
        sls_bitset packetsMask;        /**< is the packets caught bit mask */
    };



| **sls_detector_header** (described in :ref:`the current detector header <detector udp header>`)
    
| The **packetNumber** from detector UDP header is modified in **sls_receiver_header** to number of packets caught by receiver for that frame and the bit mask for each packet caught is the **packetsMask**. The packetsMask is a total of 512 bits due to the largest number of packets per frame among our detectors.

| For eg. Jungfrau has 128 packets per frame. If **packetNumeber** is 128, then this frame is complete. If it is 127 or less, it is a partial frame due to missing packets. If one would still like to use it, the **packetsMask** will specify which packet has been received or is missing.
