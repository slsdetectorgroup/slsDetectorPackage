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

#include <map>
#include <mutex>

namespace sls {

class GeneralData;
class Fifo;
class DataStreamer;
class ZmqSocket;

class DataStreamer : private virtual slsDetectorDefs, public ThreadObject {

  public:
    DataStreamer(int index);
    ~DataStreamer();

    void SetFifo(Fifo *f);
    void SetGeneralData(GeneralData *g);

    void SetFileIndex(uint64_t value);
    void SetNumberofPorts(xy np);
    void SetFlipRows(bool fd);
    void SetQuadEnable(bool value);
    void SetNumberofTotalFrames(uint64_t value);
    void
    SetAdditionalJsonHeader(const std::map<std::string, std::string> &json);
    void SetReceiverROI(ROI roi);

    void ResetParametersforNewAcquisition(const std::string &fname);
    /**
     * Creates Zmq Sockets
     * (throws an exception if it couldnt create zmq sockets)
     * @param port streaming port start index
     * @param hwm streaming high water mark
     */
    void CreateZmqSockets(uint16_t port, int hwm);
    void CloseZmqSocket();
    void RestreamStop();

  private:
    /**
     * Record First Index
     */
    void RecordFirstIndex(uint64_t fnum, size_t firstImageIndex);

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
    void ProcessAnImage(sls_detector_header header, size_t size, char *data);

    int SendDummyHeader();

    /**
     * Create and send Json Header
     * @param rheader header of image
     * @param size data size (could have been modified in call back)
     * @param nx number of pixels in x dim
     * @param ny number of pixels in y dim
     * @returns 0 if error, else 1
     */
    int SendDataHeader(sls_detector_header header, uint32_t size = 0,
                       uint32_t nx = 0, uint32_t ny = 0);

    static const std::string TypeName;
    const GeneralData *generalData{nullptr};
    Fifo *fifo{nullptr};
    ZmqSocket *zmqSocket{nullptr};
    int adcConfigured{-1};
    uint64_t fileIndex{0};
    bool flipRows{false};
    std::map<std::string, std::string> additionalJsonHeader;
    ROI receiverRoi{};

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
    bool quadEnable{false};
    uint64_t nTotalFrames{0};
};

} // namespace sls
