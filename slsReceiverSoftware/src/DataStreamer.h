#pragma once
/************************************************
 * @file DataStreamer.h
 * @short streams data from receiver via ZMQ
 ***********************************************/
/**
 *@short creates & manages a data streamer thread each
 */

#include "ThreadObject.h"
#include "network_utils.h"

class GeneralData;
class Fifo;
class DataStreamer;
class ZmqSocket;

#include <map>

class DataStreamer : private virtual slsDetectorDefs, public ThreadObject {

  public:
    /**
     * Constructor
     * Calls Base Class CreateThread(), sets ErrorMask if error and increments
     * NumberofDataStreamers
     * @param ind self index
     * @param f address of Fifo pointer
     * @param dr pointer to dynamic range
     * @param r roi
     * @param fi pointer to file index
     * @param fd flipped data enable for x dimension
     * @param nd pointer to number of detectors in each dimension
     * @param qe pointer to quad Enable
     * @param tot pointer to total number of frames
     */
    DataStreamer(int ind, Fifo *f, uint32_t *dr, ROI *r, uint64_t *fi, int fd,
                 int *nd, bool *qe, uint64_t *tot);

    /**
     * Destructor
     * Calls Base Class DestroyThread() and decrements NumberofDataStreamers
     */
    ~DataStreamer();

    /**
     * Set Fifo pointer to the one given
     * @param f address of Fifo pointer
     */
    void SetFifo(Fifo *f);

    /**
     * Reset parameters for new acquisition
     */
    void ResetParametersforNewAcquisition(const std::string &fname);

    /**
     * Set GeneralData pointer to the one given
     * @param g address of GeneralData (Detector Data) pointer
     */
    void SetGeneralData(GeneralData *g);

    /**
     * Set number of detectors
     * @param nd number of detectors in both dimensions
     */
    void SetNumberofDetectors(int *nd);

    /**
     * Set Flipped data enable across x dimension
     * @param fd data enable in x dimension
     */
    void SetFlippedDataX(int fd);

    /**
     * Set additional json header
     * @param json additional json header
     */
    void
    SetAdditionalJsonHeader(const std::map<std::string, std::string> &json);

    /**
     * Creates Zmq Sockets
     * (throws an exception if it couldnt create zmq sockets)
     * @param nunits pointer to number of theads/ units per detector
     * @param port streaming port start index
     * @param ip streaming source ip
     */
    void CreateZmqSockets(int *nunits, uint32_t port, const sls::IpAddr ip);

    /**
     * Shuts down and deletes Zmq Sockets
     */
    void CloseZmqSocket();

    /**
     * Restream stop dummy packet
     */
    void RestreamStop();

  private:
    /**
     * Record First Index
     * @param fnum current frame number
     * @param buf get frame index from buffer to calculate first index to record
     */
    void RecordFirstIndex(uint64_t fnum, char *buf);

    /**
     * Thread Exeution for DataStreamer Class
     * Stream an image via zmq
     */
    void ThreadExecution();

    /**
     * Frees dummy buffer,
     * reset running mask by calling StopRunning()
     * @param buf address of pointer
     */
    void StopProcessing(char *buf);

    /**
     * Process an image popped from fifo,
     * write to file if fw enabled & update parameters
     * @param buf address of pointer
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

    /** type of thread */
    static const std::string TypeName;

    /** GeneralData (Detector Data) object */
    const GeneralData *generalData{nullptr};

    /** Fifo structure */
    Fifo *fifo;

    /** ZMQ Socket - Receiver to Client */
    ZmqSocket *zmqSocket{nullptr};

    /** Pointer to dynamic range */
    uint32_t *dynamicRange;

    /** ROI */
    ROI *roi;

    /** adc Configured */
    int adcConfigured{-1};

    /** Pointer to file index */
    uint64_t *fileIndex;

    /** flipped data across x axis */
    int flippedDataX;

    /** additional json header */
    std::map<std::string, std::string> additionalJsonHeader;

    /** Used by streamer thread to update local copy (reduce number of locks
     * during streaming) */
    std::atomic<bool>{false};

    /** mutex to update json and to read and update local copy */
    std::mutex additionalJsonMutex;

    /** local copy of additional json header  (it can be update on the fly) */
    std::map<std::string, std::string> localAdditionalJsonHeader;

    /** Aquisition Started flag */
    bool startedFlag{nullptr};

    /** Frame Number of First Frame */
    uint64_t firstIndex{0};

    /* File name to stream */
    std::string fileNametoStream;

    /** Complete buffer used for roi, eg. shortGotthard */
    char *completeBuffer{nullptr};

    /** Number of Detectors in X and Y dimension */
    int numDet[2];

    /** Quad Enable */
    bool *quadEnable;

    /** Total number of frames */
    uint64_t *totalNumFrames;
};
