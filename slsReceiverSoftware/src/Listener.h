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

class GeneralData;
class Fifo;

class Listener : private virtual slsDetectorDefs, public ThreadObject {

  public:
    /**
     * Constructor
     * Calls Base Class CreateThread(), sets ErrorMask if error and increments
     * NumberofListerners
     * @param ind self index
     * @param dtype detector type
     * @param f address of Fifo pointer
     * @param s pointer to receiver status
     * @param portno pointer to udp port number
     * @param e ethernet interface
     * @param dr pointer to dynamic range
     * @param us pointer to udp socket buffer size
     * @param as pointer to actual udp socket buffer size
     * @param fpf pointer to frames per file
     * @param fdp frame discard policy
     * @param act pointer to activated
     * @param detds pointer to detector data stream
     * @param sm pointer to silent mode
     */
    Listener(int ind, detectorType dtype, Fifo *f, std::atomic<runStatus> *s,
             uint32_t *portno, std::string *e, int *us, int *as, uint32_t *fpf,
             frameDiscardPolicy *fdp, bool *act, bool *detds, bool *sm);

    /**
     * Destructor
     * Calls Base Class DestroyThread() and decrements NumberofListerners
     */
    ~Listener();

    uint64_t GetPacketsCaught() const;
    uint64_t GetNumCompleteFramesCaught() const;
    uint64_t GetLastFrameIndexCaught() const;
    /**  negative values in case of extra packets */
    int64_t GetNumMissingPacket(bool stoppedFlag, uint64_t numPackets) const;
    bool GetStartedFlag() const;
    uint64_t GetCurrentFrameIndex() const;
    uint64_t GetListenedIndex() const;

    void SetFifo(Fifo *f);
    void ResetParametersforNewAcquisition();
    void SetGeneralData(GeneralData *g);
    void CreateUDPSockets();
    void ShutDownUDPSocket();

    /**
     * Create & closes a dummy UDP socket
     * to set & get actual buffer size
     * @param s UDP socket buffer size to be set
     */
    void CreateDummySocketForUDPSocketBufferSize(int s);

    /**
     * Set hard coded (calculated but not from detector) row and column
     * r is in row index if detector has not send them yet in firmware,
     * c is in col index for jungfrau and eiger (for missing packets/deactivated
     * eiger) c when used is in 2d
     */
    void SetHardCodedPosition(uint16_t r, uint16_t c);

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
     * @param buf address of buffer
     */
    void StopListening(char *buf);

    /**
     * Listen to the UDP Socket for an image,
     * place them in the right order
     * @param buf address of buffer
     * @returns number of bytes of relevant data, can be image size or 0 (stop
     * acquisition) or -1 to discard image
     */
    uint32_t ListenToAnImage(char *buf);

    void PrintFifoStatistics();

    static const std::string TypeName;
    GeneralData *generalData{nullptr};
    Fifo *fifo;

    // individual members
    detectorType myDetectorType;
    std::atomic<runStatus> *status;
    std::unique_ptr<sls::UdpRxSocket> udpSocket{nullptr};
    uint32_t *udpPortNumber;
    std::string *eth;
    int *udpSocketBufferSize;
    /** double due to kernel bookkeeping */
    int *actualUDPSocketBufferSize;
    uint32_t *framesPerFile;
    frameDiscardPolicy *frameDiscardMode;
    bool *activated;
    bool *detectorDataStream;
    bool *silentMode;

    /** row hardcoded as 1D or 2d,
     * if detector does not send them yet or
     * missing packets/deactivated (eiger/jungfrau sends 2d pos) **/
    uint16_t row{0};

    /** column hardcoded as 2D,
     * deactivated eiger/missing packets (eiger/jungfrau sends 2d pos) **/
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
