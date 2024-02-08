// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
/************************************************
 * @file GeneralData.h
 * @short abstract for setting/getting properties of detector data
 ***********************************************/
/**
 *@short abstract for setting/getting properties of detector data
 */

#include "receiver_defs.h"
#include "sls/ToString.h"
#include "sls/logger.h"
#include "sls/sls_detector_defs.h"
#include <cmath> //ceil
#include <vector>

namespace sls {

class GeneralData {

  public:
    slsDetectorDefs::detectorType detType{slsDetectorDefs::GENERIC};
    uint32_t nPixelsX{0};
    uint32_t nPixelsY{0};
    uint32_t headerSizeinPacket{0};
    /** Size of just data in 1 packet (in bytes) */
    uint32_t dataSize{0};
    uint32_t packetSize{0};
    /** Number of packets in an image (for each listening UDP port) */
    uint32_t packetsPerFrame{0};
    /** Image size (in bytes, for each listening UDP port) */
    uint32_t imageSize{0};
    uint64_t frameIndexMask{0};
    uint32_t frameIndexOffset{0};
    uint32_t packetIndexMask{0};
    uint32_t packetIndexOffset{0};
    uint32_t framesPerFile{0};
    uint32_t fifoDepth{0};
    int numUDPInterfaces{1};
    uint32_t headerPacketSize{0};
    /** Streaming (for ROI - mainly short Gotthard)  */
    uint32_t nPixelsXComplete{0};
    /** Streaming (for ROI - mainly short Gotthard)  */
    uint32_t nPixelsYComplete{0};
    /** Streaming (for ROI - mainly short Gotthard) - Image size (in bytes) */
    uint32_t imageSizeComplete{0};
    /** if standard header implemented in firmware */
    bool standardheader{false};
    uint32_t udpSocketBufferSize{RECEIVE_SOCKET_BUFFER_SIZE};
    uint32_t vetoDataSize{0};
    uint32_t vetoPacketSize{0};
    uint32_t vetoImageSize{0};
    uint32_t vetoHsize{0};
    uint32_t maxRowsPerReadout{0};
    uint32_t dynamicRange{16};
    bool tengigaEnable{false};
    uint32_t nAnalogSamples{0};
    uint32_t nDigitalSamples{0};
    uint32_t nTransceiverSamples{0};
    slsDetectorDefs::readoutMode readoutType{slsDetectorDefs::ANALOG_ONLY};
    uint32_t adcEnableMaskOneGiga{BIT32_MASK};
    uint32_t adcEnableMaskTenGiga{BIT32_MASK};
    slsDetectorDefs::ROI detectorRoi{};
    uint32_t counterMask{0};
    uint32_t transceiverMask{0};
    slsDetectorDefs::frameDiscardPolicy frameDiscardMode{
        slsDetectorDefs::NO_DISCARD};

    GeneralData(){};
    virtual ~GeneralData(){};

    // Returns the pixel depth in byte, 4 bits being 0.5 byte
    float GetPixelDepth() { return float(dynamicRange) / 8; }

    void ThrowGenericError(std::string msg) const {
        throw RuntimeError(
            msg + std::string("This is a generic function that should be "
                              "overloaded by a derived class"));
    }

    /**
     * Get Header Infomation (frame number, packet number)
     * @param index thread index for debugging purposes
     * @param packetData pointer to data
     * @param oddStartingPacket odd starting packet (gotthard)
     * @param frameNumber frame number
     * @param packetNumber packet number
     * @param bunchId bunch Id
     */
    virtual void GetHeaderInfo(int index, char *packetData,
                               bool oddStartingPacket, uint64_t &frameNumber,
                               uint32_t &packetNumber,
                               uint64_t &bunchId) const {
        frameNumber = ((uint32_t)(*((uint32_t *)(packetData))));
        frameNumber++;
        packetNumber = frameNumber & packetIndexMask;
        frameNumber = (frameNumber & frameIndexMask) >> frameIndexOffset;
        bunchId = -1;
    }

    virtual void SetDetectorROI(slsDetectorDefs::ROI i) {
        ThrowGenericError("SetDetectorROI");
    };

    /**@returns adc configured */
    virtual int GetAdcConfigured(int index, slsDetectorDefs::ROI i) const {
        ThrowGenericError("GetAdcConfigured");
        return 0;
    };

    virtual void SetDynamicRange(int dr) {
        ThrowGenericError("SetDynamicRange");
    };

    virtual void SetTenGigaEnable(bool tgEnable) {
        ThrowGenericError("SetTenGigaEnable");
    };

    virtual bool SetOddStartingPacket(int index, char *packetData) {
        ThrowGenericError("SetOddStartingPacket");
        return false;
    };

    virtual void SetNumberofInterfaces(const int n) {
        ThrowGenericError("SetNumberofInterfaces");
    };

    virtual void SetCounterMask(const int n) {
        ThrowGenericError("setCounterMask");
    };

    virtual int GetNumberOfAnalogDatabytes() {
        ThrowGenericError("GetNumberOfAnalogDatabytes");
        return 0;
    };

    virtual int GetNumberOfDigitalDatabytes() {
        ThrowGenericError("GetNumberOfDigitalDatabytes");
        return 0;
    };

    virtual int GetNumberOfTransceiverDatabytes() {
        ThrowGenericError("GetNumberOfTransceiverDatabytes");
        return 0;
    };

    virtual void SetNumberOfAnalogSamples(int n) {
        ThrowGenericError("SetNumberOfAnalogSamples");
    };

    virtual void SetNumberOfDigitalSamples(int n) {
        ThrowGenericError("SetNumberOfDigitalSamples");
    };

    virtual void SetNumberOfTransceiverSamples(int n) {
        ThrowGenericError("SetNumberOfTransceiverSamples");
    };

    virtual void SetOneGigaAdcEnableMask(int n) {
        ThrowGenericError("SetOneGigaAdcEnableMask");
    };

    virtual void SetTenGigaAdcEnableMask(int n) {
        ThrowGenericError("SetTenGigaAdcEnableMask");
    };

    virtual void SetReadoutMode(slsDetectorDefs::readoutMode r) {
        ThrowGenericError("SetReadoutMode");
    };

    virtual void SetTransceiverEnableMask(int n) {
        ThrowGenericError("SetTransceiverEnableMask");
    };
};

class GotthardData : public GeneralData {

  private:
    const int nChan = 128;
    const int nChipsPerAdc = 2;

  public:
    GotthardData() {
        detType = slsDetectorDefs::GOTTHARD;
        nPixelsY = 1;
        headerSizeinPacket = 6;
        framesPerFile = MAX_FRAMES_PER_FILE;
        UpdateImageSize();
    };

    /**
     * Get Header Infomation (frame number, packet number)
     * @param index thread index for debugging purposes
     * @param packetData pointer to data
     * @param oddStartingPacket odd starting packet (gotthard)
     * @param frameNumber frame number
     * @param packetNumber packet number
     * @param bunchId bunch Id
     */
    void GetHeaderInfo(int index, char *packetData, bool oddStartingPacket,
                       uint64_t &frameNumber, uint32_t &packetNumber,
                       uint64_t &bunchId) const {
        if (nPixelsX == 1280) {
            frameNumber = *reinterpret_cast<uint32_t *>(packetData);
            if (oddStartingPacket)
                frameNumber++;
            packetNumber = frameNumber & packetIndexMask;
            frameNumber = (frameNumber & frameIndexMask) >> frameIndexOffset;
        } else {
            frameNumber = *reinterpret_cast<uint32_t *>(packetData);
            packetNumber = 0;
        }
        bunchId = -1;
    }

    /** @returns adc configured */
    int GetAdcConfigured(int index, slsDetectorDefs::ROI i) const {
        int adc = -1;
        // single adc
        if (i.xmin != -1) {
            // gotthard can have only one adc per detector enabled (or all)
            // adc = mid value/numchans also for only 1 roi
            adc = ((((i.xmax) + (i.xmin)) / 2) / (nChan * nChipsPerAdc));
            if ((adc < 0) || (adc > 4)) {
                LOG(logWARNING) << index
                                << ": Deleting ROI. "
                                   "Adc value should be between 0 and 4";
                adc = -1;
            }
        }
        LOG(logINFO) << "Adc Configured: " << adc;
        return adc;
    };

    /**
     * Set odd starting packet (gotthard)
     * @param index thread index for debugging purposes
     * @param packetData pointer to data
     * @returns true or false for odd starting packet number
     */
    bool SetOddStartingPacket(int index, char *packetData) {
        bool oddStartingPacket = true;
        // care only if no roi
        if (nPixelsX == 1280) {
            uint32_t fnum = ((uint32_t)(*((uint32_t *)(packetData))));
            uint32_t firstData = ((uint32_t)(*((uint32_t *)(packetData + 4))));
            // first packet
            if (firstData == 0xCACACACA) {
                // packet number should be 0, but is 1 => so odd starting packet
                if (fnum & packetIndexMask) {
                    oddStartingPacket = true;
                } else {
                    oddStartingPacket = false;
                }
            }
            // second packet
            else {
                // packet number should be 1, but is 0 => so odd starting packet
                if (!(fnum & packetIndexMask)) {
                    oddStartingPacket = true;
                } else {
                    oddStartingPacket = false;
                }
            }
        }
        return oddStartingPacket;
    };

    void SetDetectorROI(slsDetectorDefs::ROI i) {
        detectorRoi = i;
        UpdateImageSize();
    };

  private:
    void UpdateImageSize() {

        // all adcs
        if (detectorRoi.xmin == -1) {
            nPixelsX = 1280;
            dataSize = 1280;
            packetsPerFrame = 2;
            frameIndexMask = 0xFFFFFFFE;
            frameIndexOffset = 1;
            packetIndexMask = 1;
            framesPerFile = MAX_FRAMES_PER_FILE;
            nPixelsXComplete = 0;
            nPixelsYComplete = 0;
            imageSizeComplete = 0;
            fifoDepth = 50000;
        } else {
            nPixelsX = 256;
            dataSize = 512;
            packetsPerFrame = 1;
            frameIndexMask = 0xFFFFFFFF;
            frameIndexOffset = 0;
            packetIndexMask = 0;
            framesPerFile = SHORT_MAX_FRAMES_PER_FILE;
            nPixelsXComplete = 1280;
            nPixelsYComplete = 1;
            imageSizeComplete = 1280 * 2;
            fifoDepth = 75000;
        }
        imageSize = int(nPixelsX * nPixelsY * GetPixelDepth());
        packetSize = headerSizeinPacket + dataSize;
        packetsPerFrame = imageSize / dataSize;
    };
};

class EigerData : public GeneralData {

  public:
    EigerData() {
        detType = slsDetectorDefs::EIGER;
        headerSizeinPacket = sizeof(slsDetectorDefs::sls_detector_header);
        framesPerFile = EIGER_MAX_FRAMES_PER_FILE;
        numUDPInterfaces = 2;
        headerPacketSize = 40;
        standardheader = true;
        maxRowsPerReadout = 256;
        UpdateImageSize();
    };

    void SetDynamicRange(int dr) {
        dynamicRange = dr;
        UpdateImageSize();
    }

    void SetTenGigaEnable(bool tgEnable) {
        tengigaEnable = tgEnable;
        UpdateImageSize();
    };

  private:
    void UpdateImageSize() {
        nPixelsX = (256 * 4) / numUDPInterfaces;
        nPixelsY = 256;
        dataSize = (tengigaEnable ? 4096 : 1024);
        packetSize = headerSizeinPacket + dataSize;
        imageSize = int(nPixelsX * nPixelsY * GetPixelDepth());
        packetsPerFrame = imageSize / dataSize;
        fifoDepth = (dynamicRange == 32 ? 100 : 1000);
    };
};

class JungfrauData : public GeneralData {

  public:
    JungfrauData() {
        detType = slsDetectorDefs::JUNGFRAU;
        headerSizeinPacket = sizeof(slsDetectorDefs::sls_detector_header);
        dataSize = 8192;
        packetSize = headerSizeinPacket + dataSize;
        framesPerFile = JFRAU_MAX_FRAMES_PER_FILE;
        fifoDepth = 2500;
        standardheader = true;
        maxRowsPerReadout = 512;
        UpdateImageSize();
    };

    void SetNumberofInterfaces(const int n) {
        numUDPInterfaces = n;
        UpdateImageSize();
    };

  private:
    void UpdateImageSize() {
        nPixelsX = (256 * 4);
        nPixelsY = (256 * 2) / numUDPInterfaces;
        imageSize = int(nPixelsX * nPixelsY * GetPixelDepth());
        packetsPerFrame = imageSize / dataSize;
        udpSocketBufferSize = (1000 * 1024 * 1024) / numUDPInterfaces;
    };
};

class MoenchData : public GeneralData {

  public:
    MoenchData() {
        detType = slsDetectorDefs::MOENCH;
        headerSizeinPacket = sizeof(slsDetectorDefs::sls_detector_header);
        dataSize = 6400;
        packetSize = headerSizeinPacket + dataSize;
        framesPerFile = MOENCH_MAX_FRAMES_PER_FILE;
        fifoDepth = 1000;
        standardheader = true;
        maxRowsPerReadout = 400;
        frameDiscardMode = slsDetectorDefs::DISCARD_PARTIAL_FRAMES;
        UpdateImageSize();
    };

    void SetNumberofInterfaces(const int n) {
        numUDPInterfaces = n;
        UpdateImageSize();
    };

  private:
    void UpdateImageSize() {
        nPixelsX = (400);
        nPixelsY = (400) / numUDPInterfaces;
        imageSize = int(nPixelsX * nPixelsY * GetPixelDepth());
        packetsPerFrame = imageSize / dataSize;
        udpSocketBufferSize = (1000 * 1024 * 1024) / numUDPInterfaces;
    };
};

class Mythen3Data : public GeneralData {
  private:
    int ncounters{0};
    const int NCHAN = 1280;

  public:
    Mythen3Data() {
        detType = slsDetectorDefs::MYTHEN3;
        nPixelsY = 1;
        headerSizeinPacket = sizeof(slsDetectorDefs::sls_detector_header);
        framesPerFile = MYTHEN3_MAX_FRAMES_PER_FILE;
        fifoDepth = 50000;
        standardheader = true;
        udpSocketBufferSize = (1000 * 1024 * 1024);
        dynamicRange = 32;
        tengigaEnable = true;
        SetCounterMask(0x7);
        UpdateImageSize();
    };

    void SetDynamicRange(int dr) {
        dynamicRange = dr;
        UpdateImageSize();
    };

    void SetTenGigaEnable(bool tg) {
        tengigaEnable = tg;
        UpdateImageSize();
    };

    virtual void SetCounterMask(const int mask) {
        int n = __builtin_popcount(mask);
        if (n < 1 || n > 3) {
            throw RuntimeError("Invalid number of counters " +
                               std::to_string(n) + ". Expected 1-3.");
        }
        counterMask = mask;
        ncounters = n;
        UpdateImageSize();
    };

  private:
    void UpdateImageSize() {
        nPixelsX = (NCHAN * ncounters); // max 1280 channels x 3 counters
        LOG(logINFO) << "nPixelsX: " << nPixelsX;
        imageSize = nPixelsX * nPixelsY * GetPixelDepth();

        // 10g
        if (tengigaEnable) {
            if (dynamicRange == 32 && ncounters > 1) {
                packetsPerFrame = 2;
            } else {
                packetsPerFrame = 1;
            }
            dataSize = imageSize / packetsPerFrame;
        }
        // 1g
        else {
            if (ncounters == 3) {
                dataSize = 768;
            } else {
                dataSize = 1280;
            }
            packetsPerFrame = imageSize / dataSize;
        }

        LOG(logINFO) << "Packets Per Frame: " << packetsPerFrame;
        packetSize = headerSizeinPacket + dataSize;
        LOG(logINFO) << "PacketSize: " << packetSize;
    };
};

class Gotthard2Data : public GeneralData {
  public:
    Gotthard2Data() {
        detType = slsDetectorDefs::GOTTHARD2;
        nPixelsX = 128 * 10;
        nPixelsY = 1;
        headerSizeinPacket = sizeof(slsDetectorDefs::sls_detector_header);
        dataSize = 2560; // 1280 channels * 2 bytes
        framesPerFile = GOTTHARD2_MAX_FRAMES_PER_FILE;
        fifoDepth = 50000;
        standardheader = true;
        vetoDataSize = 160;
        vetoHsize = 16;
        UpdateImageSize();
    };

    void SetNumberofInterfaces(const int n) {
        numUDPInterfaces = n;
        UpdateImageSize();
    };

    /**
     * Get Header Infomation (frame number, packet number) for veto packets
     * @param index thread index for debugging purposes
     * @param packetData pointer to data
     * @param oddStartingPacket odd starting packet (gotthard)
     * @param frameNumber frame number
     * @param packetNumber packet number
     * @param bunchId bunch Id
     */
    void GetHeaderInfo(int index, char *packetData, bool oddStartingPacket,
                       uint64_t &frameNumber, uint32_t &packetNumber,
                       uint64_t &bunchId) const {
        frameNumber = *reinterpret_cast<uint64_t *>(packetData);
        bunchId = *reinterpret_cast<uint64_t *>(packetData + 8);
        packetNumber = 0;
    };

  private:
    void UpdateImageSize() {
        packetSize = headerSizeinPacket + dataSize;
        imageSize = int(nPixelsX * nPixelsY * GetPixelDepth());
        packetsPerFrame = imageSize / dataSize;
        vetoPacketSize = vetoHsize + vetoDataSize;
        vetoImageSize = vetoDataSize * packetsPerFrame;
        udpSocketBufferSize = (1000 * 1024 * 1024) / numUDPInterfaces;
    };
};

class ChipTestBoardData : public GeneralData {
  private:
    const int NCHAN_DIGITAL = 64;
    const int NUM_BYTES_PER_ANALOG_CHANNEL = 2;
    const int NUM_BYTES_PER_TRANSCEIVER_CHANNEL = 8;
    int nAnalogBytes = 0;
    int nDigitalBytes = 0;
    int nTransceiverBytes = 0;

  public:
    /** Constructor */
    ChipTestBoardData() {
        detType = slsDetectorDefs::CHIPTESTBOARD;
        nPixelsY = 1; // number of samples
        headerSizeinPacket = sizeof(slsDetectorDefs::sls_detector_header);
        frameIndexMask = 0xFFFFFF; // 10g
        frameIndexOffset = 8;      // 10g
        packetIndexMask = 0xFF;    // 10g
        framesPerFile = CTB_MAX_FRAMES_PER_FILE;
        fifoDepth = 2500;
        standardheader = true;
        UpdateImageSize();
    };

  public:
    int GetNumberOfAnalogDatabytes() { return nAnalogBytes; };

    int GetNumberOfDigitalDatabytes() { return nDigitalBytes; };

    int GetNumberOfTransceiverDatabytes() { return nTransceiverBytes; };

    void SetNumberOfAnalogSamples(int n) {
        nAnalogSamples = n;
        UpdateImageSize();
    };

    void SetNumberOfDigitalSamples(int n) {
        nDigitalSamples = n;
        UpdateImageSize();
    };

    void SetNumberOfTransceiverSamples(int n) {
        nTransceiverSamples = n;
        UpdateImageSize();
    };

    void SetOneGigaAdcEnableMask(int n) {
        adcEnableMaskOneGiga = n;
        UpdateImageSize();
    };

    void SetTenGigaAdcEnableMask(int n) {
        adcEnableMaskTenGiga = n;
        UpdateImageSize();
    };

    void SetTransceiverEnableMask(int n) {
        transceiverMask = n;
        UpdateImageSize();
    };

    void SetReadoutMode(slsDetectorDefs::readoutMode r) {
        readoutType = r;
        UpdateImageSize();
    };

    void SetTenGigaEnable(bool tg) {
        tengigaEnable = tg;
        UpdateImageSize();
    };

  private:
    void UpdateImageSize() {
        nAnalogBytes = 0;
        nDigitalBytes = 0;
        nTransceiverBytes = 0;
        int nAnalogChans = 0, nDigitalChans = 0, nTransceiverChans = 0;

        // analog channels (normal, analog/digital readout)
        if (readoutType == slsDetectorDefs::ANALOG_ONLY ||
            readoutType == slsDetectorDefs::ANALOG_AND_DIGITAL) {
            uint32_t adcEnableMask =
                (tengigaEnable ? adcEnableMaskTenGiga : adcEnableMaskOneGiga);
            nAnalogChans = __builtin_popcount(adcEnableMask);

            nAnalogBytes =
                nAnalogChans * NUM_BYTES_PER_ANALOG_CHANNEL * nAnalogSamples;
            LOG(logDEBUG1) << " Number of Analog Channels:" << nAnalogChans
                           << " Databytes: " << nAnalogBytes;
        }
        // digital channels
        if (readoutType == slsDetectorDefs::DIGITAL_ONLY ||
            readoutType == slsDetectorDefs::ANALOG_AND_DIGITAL ||
            readoutType == slsDetectorDefs::DIGITAL_AND_TRANSCEIVER) {
            nDigitalChans = NCHAN_DIGITAL;
            nDigitalBytes = (sizeof(uint64_t) * nDigitalSamples);
            LOG(logDEBUG1) << "Number of Digital Channels:" << nDigitalChans
                           << " Databytes: " << nDigitalBytes;
        }
        // transceiver channels
        if (readoutType == slsDetectorDefs::TRANSCEIVER_ONLY ||
            readoutType == slsDetectorDefs::DIGITAL_AND_TRANSCEIVER) {
            nTransceiverChans = __builtin_popcount(transceiverMask);
            ;
            nTransceiverBytes = nTransceiverChans *
                                NUM_BYTES_PER_TRANSCEIVER_CHANNEL *
                                nTransceiverSamples;
            LOG(logDEBUG1) << "Number of Transceiver Channels:"
                           << nTransceiverChans
                           << " Databytes: " << nTransceiverBytes;
        }
        nPixelsX = nAnalogChans + nDigitalChans + nTransceiverChans;
        dataSize = tengigaEnable ? 8144 : UDP_PACKET_DATA_BYTES;
        packetSize = headerSizeinPacket + dataSize;
        imageSize = nAnalogBytes + nDigitalBytes + nTransceiverBytes;
        packetsPerFrame = ceil((double)imageSize / (double)dataSize);

        LOG(logDEBUG1) << "Total Number of Channels:" << nPixelsX
                       << " Databytes: " << imageSize;
    };
};

class XilinxChipTestBoardData : public GeneralData {
  private:
    const int NCHAN_DIGITAL = 64;
    const int NUM_BYTES_PER_ANALOG_CHANNEL = 2;
    const int NUM_BYTES_PER_TRANSCEIVER_CHANNEL = 8;
    int nAnalogBytes = 0;
    int nDigitalBytes = 0;
    int nTransceiverBytes = 0;

  public:
    /** Constructor */
    XilinxChipTestBoardData() {
        detType = slsDetectorDefs::XILINX_CHIPTESTBOARD;
        nPixelsY = 1; // number of samples
        headerSizeinPacket = sizeof(slsDetectorDefs::sls_detector_header);
        frameIndexMask = 0xFFFFFF; // 10g
        frameIndexOffset = 8;      // 10g
        packetIndexMask = 0xFF;    // 10g
        framesPerFile = XILINX_CTB_MAX_FRAMES_PER_FILE;
        fifoDepth = 2500;
        standardheader = true;
        dataSize = 8144;
        packetSize = headerSizeinPacket + dataSize;
        tengigaEnable = true;
        UpdateImageSize();
    };

  public:
    int GetNumberOfAnalogDatabytes() { return nAnalogBytes; };

    int GetNumberOfDigitalDatabytes() { return nDigitalBytes; };

    int GetNumberOfTransceiverDatabytes() { return nTransceiverBytes; };

    void SetNumberOfAnalogSamples(int n) {
        nAnalogSamples = n;
        UpdateImageSize();
    };

    void SetNumberOfDigitalSamples(int n) {
        nDigitalSamples = n;
        UpdateImageSize();
    };

    void SetNumberOfTransceiverSamples(int n) {
        nTransceiverSamples = n;
        UpdateImageSize();
    };

    void SetOneGigaAdcEnableMask(int n) {
        adcEnableMaskOneGiga = n;
        UpdateImageSize();
    };

    void SetTenGigaAdcEnableMask(int n) {
        adcEnableMaskTenGiga = n;
        UpdateImageSize();
    };

    void SetTransceiverEnableMask(int n) {
        transceiverMask = n;
        UpdateImageSize();
    };

    void SetReadoutMode(slsDetectorDefs::readoutMode r) {
        readoutType = r;
        UpdateImageSize();
    };

  private:
    void UpdateImageSize() {
        nAnalogBytes = 0;
        nDigitalBytes = 0;
        nTransceiverBytes = 0;
        int nAnalogChans = 0, nDigitalChans = 0, nTransceiverChans = 0;

        // analog channels (normal, analog/digital readout)
        if (readoutType == slsDetectorDefs::ANALOG_ONLY ||
            readoutType == slsDetectorDefs::ANALOG_AND_DIGITAL) {
            uint32_t adcEnableMask = adcEnableMaskTenGiga;
            nAnalogChans = __builtin_popcount(adcEnableMask);

            nAnalogBytes =
                nAnalogChans * NUM_BYTES_PER_ANALOG_CHANNEL * nAnalogSamples;
            LOG(logDEBUG1) << " Number of Analog Channels:" << nAnalogChans
                           << " Databytes: " << nAnalogBytes;
        }
        // digital channels
        if (readoutType == slsDetectorDefs::DIGITAL_ONLY ||
            readoutType == slsDetectorDefs::ANALOG_AND_DIGITAL ||
            readoutType == slsDetectorDefs::DIGITAL_AND_TRANSCEIVER) {
            nDigitalChans = NCHAN_DIGITAL;
            nDigitalBytes = (sizeof(uint64_t) * nDigitalSamples);
            LOG(logDEBUG1) << "Number of Digital Channels:" << nDigitalChans
                           << " Databytes: " << nDigitalBytes;
        }
        // transceiver channels
        if (readoutType == slsDetectorDefs::TRANSCEIVER_ONLY ||
            readoutType == slsDetectorDefs::DIGITAL_AND_TRANSCEIVER) {
            nTransceiverChans = __builtin_popcount(transceiverMask);
            ;
            nTransceiverBytes = nTransceiverChans *
                                NUM_BYTES_PER_TRANSCEIVER_CHANNEL *
                                nTransceiverSamples;
            LOG(logDEBUG1) << "Number of Transceiver Channels:"
                           << nTransceiverChans
                           << " Databytes: " << nTransceiverBytes;
        }
        nPixelsX = nAnalogChans + nDigitalChans + nTransceiverChans;

        imageSize = nAnalogBytes + nDigitalBytes + nTransceiverBytes;
        packetsPerFrame = ceil((double)imageSize / (double)dataSize);

        LOG(logDEBUG1) << "Total Number of Channels:" << nPixelsX
                       << " Databytes: " << imageSize;
    };
};

} // namespace sls
