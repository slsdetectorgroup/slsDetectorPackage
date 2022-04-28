// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
/************************************************
 * @file DataStreamer.h
 * @short streams data from receiver via ZMQ
 ***********************************************/
/**
 *@short creates & manages a data streamer thread each
 */

#include "ThreadObject.h"
#include "sls/network_utils.h"

class GeneralData;
class Fifo;
class DataStreamer;
class ZmqSocket;

#include <map>
#include <mutex>

class DataStreamer : private virtual slsDetectorDefs, public ThreadObject {

  public:
    /**
     * Constructor
     * Calls Base Class CreateThread(), sets ErrorMask if error and increments
     * NumberofDataStreamers
     * @param ind self index
     * @param f address of Fifo pointer
     * @param dr pointer to dynamic range
     * @param r detectorRoi
     * @param fi pointer to file index
     * @param fr flip rows
     * @param nm number of ports in each dimension
     * @param qe pointer to quad Enable
     * @param tot pointer to total number of frames
     */
    DataStreamer(int ind, Fifo *f, uint32_t *dr, ROI *r, uint64_t *fi, bool fr,
                 xy np, bool *qe, uint64_t *tot);

    /**
     * Destructor
     * Calls Base Class DestroyThread() and decrements NumberofDataStreamers
     */
    ~DataStreamer();

    void SetFifo(Fifo *f);
    void SetReceiverROI(ROI roi);
    void ResetParametersforNewAcquisition(const std::string &fname);
    void SetGeneralData(GeneralData *g);
    void SetNumberofPorts(xy np);
    void SetFlipRows(bool fd);
    void
    SetAdditionalJsonHeader(const std::map<std::string, std::string> &json);

    /**
     * Creates Zmq Sockets
     * (throws an exception if it couldnt create zmq sockets)
     * @param nunits pointer to number of theads/ units per detector
     * @param port streaming port start index
     * @param ip streaming source ip
     * @param hwm streaming high water mark
     */
    void CreateZmqSockets(int *nunits, uint32_t port, const sls::IpAddr ip,
                          int hwm);
    void CloseZmqSocket();
    void RestreamStop();

  private:
    /**
     * Record First Index
     * @param fnum current frame number
     * @param buf get frame index from buffer to calculate first index to record
     */
    void RecordFirstIndex(uint64_t fnum, char *buf);
    void ThreadExecution();

    /**
     * Frees dummy buffer,
     * reset running mask by calling StopRunning()
     */
    void StopProcessing(char *buf);

    /**
     * Process an image popped from fifo,
     * write to file if fw enabled & update parameters
     */
    void ProcessAnImage(char *buf);

    /**
     * Create and send Json Header
     * @param rheader header of image
     * @param size data size (could have been modified in call back)
     * @param nx number of pixels in x dim
     * @param ny number of pixels in y dim
     * @param dummy true if its a dummy header
     * @returns 0 if error, else 1
     */
    int SendHeader(sls_receiver_header *rheader, uint32_t size = 0,
                   uint32_t nx = 0, uint32_t ny = 0, bool dummy = true);

    static const std::string TypeName;
    const GeneralData *generalData{nullptr};
    Fifo *fifo;
    ZmqSocket *zmqSocket{nullptr};
    uint32_t *dynamicRange;
    ROI *detectorRoi;
    int adcConfigured{-1};
    ROI receiverRoi{};
    uint64_t *fileIndex;
    bool flipRows;
    std::map<std::string, std::string> additionalJsonHeader;

    /** Used by streamer thread to update local copy (reduce number of locks
     * during streaming) */
    std::atomic<bool> isAdditionalJsonUpdated{false};

    /** mutex to update json and to read and update local copy */
    mutable std::mutex additionalJsonMutex;

    /** local copy of additional json header  (it can be update on the fly) */
    std::map<std::string, std::string> localAdditionalJsonHeader;

    bool startedFlag{false};
    uint64_t firstIndex{0};
    std::string fileNametoStream;
    /** Complete buffer used for detectorRoi, eg. shortGotthard */
    char *completeBuffer{nullptr};

    xy numPorts{1, 1};
    bool *quadEnable;
    uint64_t *totalNumFrames;
};
