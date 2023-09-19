// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
/************************************************
 * @file Listener.h
 * @short creates the listener thread that
 * listens to udp sockets, writes data to memory
 * & puts pointers to their memory addresses into fifos
 ***********************************************/
/**
 *@short creates & manages a listener thread each
 */

#include "ThreadObject.h"
#include "sls/UdpRxSocket.h"
#include <atomic>
#include <memory>

namespace sls {

class GeneralData;
class Fifo;

class Listener : private virtual slsDetectorDefs, public ThreadObject {

  public:
    Listener(int index, std::atomic<runStatus> *status);
    ~Listener();

    bool isPortDisabled() const;
    uint64_t GetPacketsCaught() const;
    uint64_t GetNumCompleteFramesCaught() const;
    uint64_t GetLastFrameIndexCaught() const;
    /**  negative values in case of extra packets */
    int64_t GetNumMissingPacket(bool stoppedFlag, uint64_t numPackets) const;
    bool GetStartedFlag() const;
    uint64_t GetCurrentFrameIndex() const;
    uint64_t GetListenedIndex() const;

    void SetFifo(Fifo *f);
    void SetGeneralData(GeneralData *g);
    void SetUdpPortNumber(const uint16_t portNumber);
    void SetEthernetInterface(const std::string e);
    void SetActivate(bool enable);
    void SetDetectorDatastream(bool enable);
    void SetNoRoi(bool enable);
    void SetSilentMode(bool enable);

    void ResetParametersforNewAcquisition();
    void CreateUDPSocket(int &actualSize);
    void ShutDownUDPSocket();
    void DeleteUDPSocket();
    /** to set & get actual buffer size */
    void CreateDummySocketForUDPSocketBufferSize(int s, int &actualSize);

    /**
     * Set hard coded (calculated but not from detector) row and column
     * r is in row index if detector has not send them yet in firmware,
     * c is in col index for jungfrau and eiger (for missing
     * packets/deactivated) c when used is in 2d
     */
    void SetHardCodedPosition(uint16_t r, uint16_t c);
    std::pair<uint16_t, uint16_t> GetHardCodedPosition();

  private:
    void RecordFirstIndex(uint64_t fnum);

    /**
     * Thread Execution for Listener Class
     * Pop free addresses, listen to udp socket,
     * write to memory & push the address into fifo
     */
    void ThreadExecution() override;

    /**
     * Pushes non empty buffers into fifo/ frees empty buffer,
     * pushes dummy buffer into fifo
     * and reset running mask by calling StopRunning()
     */
    void StopListening(char *buf, size_t &size);

    /**
     * Listen to the UDP Socket for an image,
     * place them in the right order
     * @returns number of bytes of relevant data, can be image size or 0 (stop
     * acquisition) or -1 to discard image
     */
    uint32_t ListenToAnImage(sls_receiver_header &dstHeader, char *dstData);

    size_t HandleFuturePacket(bool EOA, uint32_t numpackets, uint64_t fnum,
                              bool isHeaderEmpty, size_t imageSize,
                              sls_receiver_header &rxHeader);

    void CopyPacket(char *dst, char *src, uint32_t dataSize,
                    uint32_t detHeaderSize, uint32_t correctedDataSize,
                    uint32_t &numpackets, bool &isHeaderEmpty,
                    bool standardHeader, sls_receiver_header &rxHeader,
                    sls_detector_header *detHeader, uint32_t pnum,
                    uint64_t bnum);

    void GetPacketIndices(uint64_t &fnum, uint32_t &pnum, uint64_t &bnum,
                          bool standardHeader, char *packet,
                          sls_detector_header *&header);

    void PrintFifoStatistics();

    static const std::string TypeName;
    GeneralData *generalData{nullptr};
    Fifo *fifo;

    // individual members
    std::atomic<runStatus> *status;
    std::unique_ptr<UdpRxSocket> udpSocket{nullptr};

    uint16_t udpPortNumber{0};
    std::string eth;
    bool activated{false};
    bool detectorDataStream{true};
    bool noRoi{false};
    bool silentMode;
    bool disabledPort{false};

    /** row hardcoded as 1D or 2d,
     * if detector does not send them yet or
     * missing packets/deactivated (eiger/jungfrau sends 2d pos) **/
    uint16_t row{0};

    /** column hardcoded as 2D,
     * deactivated/missing packets (eiger/jungfrau sends 2d pos) **/
    uint16_t column{0};

    // acquisition start
    /** Aquisition Started flag */
    std::atomic<bool> startedFlag{false};
    /** Frame Number of First Frame  */
    uint64_t firstIndex{0};

    // for acquisition summary
    std::atomic<uint64_t> numPacketsCaught{0};
    std::atomic<uint64_t> numCompleteFramesCaught{0};
    std::atomic<uint64_t> lastCaughtFrameIndex{0};

    // parameters to acquire image
    /** Current Frame Index, default value is 0
     * ( always check startedFlag for validity first)
     */
    uint64_t currentFrameIndex{0};
    /** True if there is a packet carry over from previous Image */
    bool carryOverFlag{false};
    std::unique_ptr<char[]> carryOverPacket;
    /** Listening buffer for one packet - might be removed when we can peek and
     * eiger fnum is in header */
    std::unique_ptr<char[]> listeningPacket;
    std::atomic<bool> udpSocketAlive{false};

    // for print progress during acquisition*/
    uint32_t numPacketsStatistic{0};
    uint32_t numFramesStatistic{0};

    /**
     * starting packet number is odd or even, accordingly increment frame number
     * to get first packet number as 0
     * (pecific to gotthard, can vary between modules, hence defined here) */
    bool oddStartingPacket{true};
};

} // namespace sls
