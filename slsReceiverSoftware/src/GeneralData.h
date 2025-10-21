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

struct CtbImageInputs {
    slsDetectorDefs::readoutMode mode{slsDetectorDefs::ANALOG_ONLY};
    int nAnalogSamples{};
    uint32_t adcMask{};
    int nTransceiverSamples{};
    uint32_t transceiverMask{};
    int nDigitalSamples{};
    int dbitOffset{};
    bool dbitReorder{};
    std::vector<int> dbitList{};

    inline void print() const {
        LOG(logINFO) << "CTB Image Inputs: "
                     << "Readout Mode:" << ToString(mode)
                     << "\n\tNumber of Analog Samples:" << nAnalogSamples
                     << "\n\tADC Enable 1G:" << std::hex << adcMask << std::dec
                     << "\n\tNumber of Transceiver Samples:"
                     << nTransceiverSamples
                     << "\n\tTransceiver Mask:" << std::hex << transceiverMask
                     << std::dec
                     << "\n\tNumber of Digital Samples:" << nDigitalSamples
                     << "\n\tDBIT Offset:" << dbitOffset
                     << "\n\tDBIT Reorder:" << dbitReorder
                     << "\n\tDBIT List:" << ToString(dbitList);
    }
};

struct CtbImageOutputs {
    int nAnalogBytes{};
    int nDigitalBytes{};
    int nDigitalBytesReserved{}; // including dbit offset and for 64 bits
    int nTransceiverBytes{};
    int nPixelsX{};

    inline void print() const {
        LOG(logINFO) << "CTB Image Outputs: "
                     << "\n\tNumber of Analog Bytes:" << nAnalogBytes
                     << "\n\tNumber of Actual Digital Bytes:" << nDigitalBytes
                     << "\n\tNumber of Digital Bytes Reserved:"
                     << nDigitalBytesReserved
                     << "\n\tNumber of Transceiver Bytes:" << nTransceiverBytes
                     << "\n\tNumber of Pixels in X:" << nPixelsX;
    }
};

inline CtbImageOutputs computeCtbImageSize(const CtbImageInputs &in) {
    CtbImageOutputs out{};
    constexpr int num_bytes_per_analog_channel = 2;
    constexpr int num_bytes_per_transceiver_channel = 8;
    constexpr int max_digital_channels = 64;

    // in.print(); // for debugging

    // analog channels (normal, analog/digital readout)
    if (in.mode == slsDetectorDefs::ANALOG_ONLY ||
        in.mode == slsDetectorDefs::ANALOG_AND_DIGITAL) {
        int nAnalogChans = __builtin_popcount(in.adcMask);

        out.nPixelsX += nAnalogChans;
        out.nAnalogBytes =
            nAnalogChans * num_bytes_per_analog_channel * in.nAnalogSamples;
        LOG(logDEBUG1) << " Number of Analog Channels:" << nAnalogChans
                       << " Databytes: " << out.nAnalogBytes;
    }

    // digital channels
    if (in.mode == slsDetectorDefs::DIGITAL_ONLY ||
        in.mode == slsDetectorDefs::ANALOG_AND_DIGITAL ||
        in.mode == slsDetectorDefs::DIGITAL_AND_TRANSCEIVER) {

        int nSamples = in.nDigitalSamples;
        {
            // allocate enought for 64 bits and dbit offset for now
            // TODO: to be replaced in the future with the actual reserved and
            // used
            int32_t num_bytes_per_bit =
                (nSamples % 8 == 0) ? (nSamples / 8) : (nSamples / 8 + 1);
            out.nDigitalBytesReserved =
                max_digital_channels * num_bytes_per_bit;
            LOG(logDEBUG1) << "Number of Digital Channels:"
                           << max_digital_channels << " Databytes reserved: "
                           << out.nDigitalBytesReserved;
        }

        // remove offset
        if (in.dbitOffset > 0) {
            int nBytesReserved = out.nDigitalBytesReserved - in.dbitOffset;
            nSamples = nBytesReserved / sizeof(uint64_t);
        }
        // calculate channels
        int nChans = in.dbitList.size();
        if (nChans == 0) {
            nChans = max_digital_channels;
        }
        out.nPixelsX += nChans;

        // calculate actual bytes
        if (!in.dbitReorder) {
            uint32_t nBitsPerSample = nChans;
            if (nBitsPerSample % 8 != 0) {
                nBitsPerSample += (8 - (nBitsPerSample % 8));
            }
            out.nDigitalBytes = (nBitsPerSample / 8) * nSamples;
        } else {
            uint32_t nBitsPerSignal = nSamples;
            if (nBitsPerSignal % 8 != 0) {
                nBitsPerSignal += (8 - (nBitsPerSignal % 8));
            }
            out.nDigitalBytes = nChans * (nBitsPerSignal / 8);
        }
        LOG(logDEBUG1) << "Number of Actual Digital Channels:" << nChans
                       << " Databytes: " << out.nDigitalBytes;
    }

    // transceiver channels
    if (in.mode == slsDetectorDefs::TRANSCEIVER_ONLY ||
        in.mode == slsDetectorDefs::DIGITAL_AND_TRANSCEIVER) {
        int nTransceiverChans = __builtin_popcount(in.transceiverMask);

        out.nPixelsX += nTransceiverChans;
        out.nTransceiverBytes = nTransceiverChans *
                                num_bytes_per_transceiver_channel *
                                in.nTransceiverSamples;
        LOG(logDEBUG1) << "Number of Transceiver Channels:" << nTransceiverChans
                       << " Databytes: " << out.nTransceiverBytes;
    }
    return out;
}

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
    std::vector<int> ctbDbitList{};
    int ctbDbitOffset{0};
    bool ctbDbitReorder{false};
    slsDetectorDefs::readoutMode readoutType{slsDetectorDefs::ANALOG_ONLY};
    uint32_t adcEnableMaskOneGiga{BIT32_MASK};
    uint32_t adcEnableMaskTenGiga{BIT32_MASK};
    uint32_t counterMask{0};
    uint32_t transceiverMask{0};
    slsDetectorDefs::frameDiscardPolicy frameDiscardMode{
        slsDetectorDefs::NO_DISCARD};
    /* actual image size after ctboffset and ctbreorder */
    uint32_t actualImageSize{0};
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
     * @param frameNumber frame number
     * @param packetNumber packet number
     * @param bunchId bunch Id
     */
    virtual void GetHeaderInfo(int index, char *packetData,
                               uint64_t &frameNumber, uint32_t &packetNumber,
                               uint64_t &bunchId) const {
        frameNumber = ((uint32_t)(*((uint32_t *)(packetData))));
        frameNumber++;
        packetNumber = frameNumber & packetIndexMask;
        frameNumber = (frameNumber & frameIndexMask) >> frameIndexOffset;
        bunchId = -1;
    }

    virtual void SetDynamicRange(int dr) {
        ThrowGenericError("SetDynamicRange");
    };

    virtual void SetTenGigaEnable(bool tgEnable) {
        ThrowGenericError("SetTenGigaEnable");
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

    virtual void SetctbDbitOffset(const int n) {
        ThrowGenericError("SetctbDbitOffset");
    };

    virtual void SetctbDbitList(const std::vector<int> &value) {
        ThrowGenericError("SetctbDbitList");
    };

    virtual void SetctbDbitReorder(const bool reorder) {
        ThrowGenericError("SetctbDbitReorder");
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
        actualImageSize = imageSize;
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
        actualImageSize = imageSize;
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
        actualImageSize = imageSize;
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
        actualImageSize = imageSize;

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
     * @param frameNumber frame number
     * @param packetNumber packet number
     * @param bunchId bunch Id
     */
    void GetHeaderInfo(int index, char *packetData, uint64_t &frameNumber,
                       uint32_t &packetNumber, uint64_t &bunchId) const {
        frameNumber = *reinterpret_cast<uint64_t *>(packetData);
        bunchId = *reinterpret_cast<uint64_t *>(packetData + 8);
        packetNumber = 0;
    };

  private:
    void UpdateImageSize() {
        packetSize = headerSizeinPacket + dataSize;
        imageSize = int(nPixelsX * nPixelsY * GetPixelDepth());
        actualImageSize = imageSize;
        packetsPerFrame = imageSize / dataSize;
        vetoPacketSize = vetoHsize + vetoDataSize;
        vetoImageSize = vetoDataSize * packetsPerFrame;
        udpSocketBufferSize = (1000 * 1024 * 1024) / numUDPInterfaces;
    };
};

class ChipTestBoardData : public GeneralData {
  private:
    const int NCHAN_DIGITAL = 64;
    int nAnalogBytes = 0;
    int nDigitalBytes = 0;
    int nTransceiverBytes = 0;

  public:
    /** Constructor */
    ChipTestBoardData() {
        detType = slsDetectorDefs::CHIPTESTBOARD;
        nPixelsY = 1;
        headerSizeinPacket = sizeof(slsDetectorDefs::sls_detector_header);
        frameIndexMask = 0xFFFFFF; // 10g
        frameIndexOffset = 8;      // 10g
        packetIndexMask = 0xFF;    // 10g
        framesPerFile = CTB_MAX_FRAMES_PER_FILE;
        fifoDepth = 2500;
        standardheader = true;
        ctbDbitReorder = true;
        UpdateImageSize();
    }

  public:
    int GetNumberOfAnalogDatabytes() { return nAnalogBytes; }

    int GetNumberOfDigitalDatabytes() { return nDigitalBytes; }

    int GetNumberOfTransceiverDatabytes() { return nTransceiverBytes; }

    void SetNumberOfAnalogSamples(int n) {
        nAnalogSamples = n;
        UpdateImageSize();
    }

    void SetNumberOfDigitalSamples(int n) {
        nDigitalSamples = n;
        UpdateImageSize();
    }

    void SetNumberOfTransceiverSamples(int n) {
        nTransceiverSamples = n;
        UpdateImageSize();
    }

    void SetctbDbitOffset(const int value) {
        ctbDbitOffset = value;
        UpdateImageSize();
    }

    void SetctbDbitList(const std::vector<int> &value) {
        ctbDbitList = std::move(value);
        UpdateImageSize();
    }

    void SetctbDbitReorder(const bool value) {
        ctbDbitReorder = value;
        UpdateImageSize();
    }

    void SetOneGigaAdcEnableMask(int n) {
        adcEnableMaskOneGiga = n;
        UpdateImageSize();
    }

    void SetTenGigaAdcEnableMask(int n) {
        adcEnableMaskTenGiga = n;
        UpdateImageSize();
    }

    void SetTransceiverEnableMask(int n) {
        transceiverMask = n;
        UpdateImageSize();
    }

    void SetReadoutMode(slsDetectorDefs::readoutMode r) {
        readoutType = r;
        UpdateImageSize();
    }

    void SetTenGigaEnable(bool tg) {
        tengigaEnable = tg;
        UpdateImageSize();
    }

  private:
    void UpdateImageSize() {
        // used in calculations so cant remove now - TODO: remove later
        nDigitalBytes = sizeof(uint64_t) * nDigitalSamples;

        // calculate image size
        CtbImageInputs inputs{};
        inputs.mode = readoutType;
        inputs.nAnalogSamples = nAnalogSamples;
        inputs.adcMask =
            tengigaEnable ? adcEnableMaskTenGiga : adcEnableMaskOneGiga;
        inputs.nTransceiverSamples = nTransceiverSamples;
        inputs.transceiverMask = transceiverMask;
        inputs.nDigitalSamples = nDigitalSamples;
        inputs.dbitOffset = ctbDbitOffset;
        inputs.dbitList = ctbDbitList;
        inputs.dbitReorder = ctbDbitReorder;

        auto out = computeCtbImageSize(inputs);

        nPixelsX = out.nPixelsX;
        nAnalogBytes = out.nAnalogBytes;
        nTransceiverBytes = out.nTransceiverBytes;

        imageSize = out.nAnalogBytes + out.nDigitalBytesReserved +
                    out.nTransceiverBytes;
        // to write to file: after ctb offset and reorder
        actualImageSize =
            out.nAnalogBytes + out.nDigitalBytes + out.nTransceiverBytes;
        LOG(logDEBUG1) << "Actual image size: " << actualImageSize;

        // calculate network parameters
        dataSize = tengigaEnable ? 8144 : UDP_PACKET_DATA_BYTES;
        packetSize = headerSizeinPacket + dataSize;
        packetsPerFrame = ceil((double)imageSize / (double)dataSize);
        LOG(logDEBUG1) << "Total Number of Channels:" << nPixelsX
                       << " Databytes: " << imageSize;
    }
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
        ctbDbitReorder = true;
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

    void SetctbDbitOffset(const int value) {
        ctbDbitOffset = value;
        UpdateImageSize();
    }

    void SetctbDbitList(const std::vector<int> &value) {
        ctbDbitList = std::move(value);
        UpdateImageSize();
    }

    void SetctbDbitReorder(const bool value) {
        ctbDbitReorder = value;
        UpdateImageSize();
    }

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
        // used in calculations so cant remove now - TODO: remove later
        nDigitalBytes = sizeof(uint64_t) * nDigitalSamples;

        // calculate image size
        CtbImageInputs inputs{};
        inputs.mode = readoutType;
        inputs.nAnalogSamples = nAnalogSamples;
        inputs.adcMask = adcEnableMaskTenGiga;
        inputs.nTransceiverSamples = nTransceiverSamples;
        inputs.transceiverMask = transceiverMask;
        inputs.nDigitalSamples = nDigitalSamples;
        inputs.dbitOffset = ctbDbitOffset;
        inputs.dbitList = ctbDbitList;
        inputs.dbitReorder = ctbDbitReorder;

        auto out = computeCtbImageSize(inputs);

        nPixelsX = out.nPixelsX;
        nAnalogBytes = out.nAnalogBytes;
        nTransceiverBytes = out.nTransceiverBytes;

        imageSize = out.nAnalogBytes + out.nDigitalBytesReserved +
                    out.nTransceiverBytes;
        // to write to file: after ctb offset and reorder
        actualImageSize =
            out.nAnalogBytes + out.nDigitalBytes + out.nTransceiverBytes;
        LOG(logDEBUG1) << "Actual image size: " << actualImageSize;

        // calculate network parameters
        packetsPerFrame = ceil((double)imageSize / (double)dataSize);
        LOG(logDEBUG1) << "Total Number of Channels:" << nPixelsX
                       << " Databytes: " << imageSize;
    };
};

} // namespace sls
