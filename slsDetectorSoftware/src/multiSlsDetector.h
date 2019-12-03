#pragma once

#include "Result.h"
#include "SharedMemory.h"
#include "logger.h"
#include "sls_detector_defs.h"

class slsDetector;
class ZmqSocket;
class detectorData;

#include <memory>
#include <mutex>
#include <semaphore.h>
#include <string>
#include <thread>
#include <vector>

#define MULTI_SHMAPIVERSION 0x190809
#define MULTI_SHMVERSION 0x190819
#define SHORT_STRING_LENGTH 50
#define DATE_LENGTH 30

#include <future>
#include <numeric>
/**
 * @short structure allocated in shared memory to store detector settings
 * for IPC and cache
 */
struct sharedMultiSlsDetector {

    /* FIXED PATTERN FOR STATIC FUNCTIONS. DO NOT CHANGE, ONLY APPEND
     * ------*/

    /** shared memory version */
    int shmversion;

    /** last process id accessing the shared memory */
    pid_t lastPID;

    /** last user name accessing the shared memory */
    char lastUser[SHORT_STRING_LENGTH];

    /** last time stamp when accessing the shared memory */
    char lastDate[SHORT_STRING_LENGTH];

    /** number of sls detectors in shared memory */
    int numberOfDetectors;

    /** multi detector type */
    slsDetectorDefs::detectorType multiDetectorType;

    /** END OF FIXED PATTERN
     * -----------------------------------------------*/

    /** Number of detectors operated at once */
    slsDetectorDefs::xy numberOfDetector;

    /**  max number of channels for complete detector*/
    slsDetectorDefs::xy numberOfChannels;

    /** flag for acquiring */
    bool acquiringFlag;

    /** data streaming (up stream) enable in receiver */
    bool receiver_upstream;
};

class multiSlsDetector : public virtual slsDetectorDefs {
  public:
    /**
     * Constructor
     * @param id multi detector id
     * @param verify true to verify if shared memory version matches existing
     * one
     * @param update true to update last user pid, date etc
     */
    explicit multiSlsDetector(int multi_id = 0, bool verify = true,
                              bool update = true);

    /**
     * Destructor
     */
    virtual ~multiSlsDetector();

    template <class CT> struct NonDeduced { using type = CT; };
    template <typename RT, typename... CT>
    sls::Result<RT> Parallel(RT (slsDetector::*somefunc)(CT...),
                             std::vector<int> positions,
                             typename NonDeduced<CT>::type... Args) {

        if (detectors.size() == 0) 
            throw sls::RuntimeError("No detectors added");                      
        if (positions.empty() ||
            (positions.size() == 1 && positions[0] == -1)) {
            positions.resize(detectors.size());
            std::iota(begin(positions), end(positions), 0);
        }
        std::vector<std::future<RT>> futures;
        futures.reserve(positions.size());
        for (size_t i : positions) {
            if (i >= detectors.size())
                throw sls::RuntimeError("Detector out of range");
            futures.push_back(std::async(std::launch::async, somefunc,
                                         detectors[i].get(), Args...));
        }
        sls::Result<RT> result;
        result.reserve(positions.size());
        for (auto &i : futures) {
            result.push_back(i.get());
        }
        return result;
    }

    template <typename RT, typename... CT>
    sls::Result<RT> Parallel(RT (slsDetector::*somefunc)(CT...) const,
                             std::vector<int> positions,
                             typename NonDeduced<CT>::type... Args) const {

        if (detectors.size() == 0) 
            throw sls::RuntimeError("No detectors added");  
        if (positions.empty() ||
            (positions.size() == 1 && positions[0] == -1)) {
            positions.resize(detectors.size());
            std::iota(begin(positions), end(positions), 0);
        }
        std::vector<std::future<RT>> futures;
        futures.reserve(positions.size());
        for (size_t i : positions) {
            if (i >= detectors.size())
                throw sls::RuntimeError("Detector out of range");
            futures.push_back(std::async(std::launch::async, somefunc,
                                         detectors[i].get(), Args...));
        }
        sls::Result<RT> result;
        result.reserve(positions.size());
        for (auto &i : futures) {
            result.push_back(i.get());
        }
        return result;
    }

    template <typename... CT>
    void Parallel(void (slsDetector::*somefunc)(CT...),
                  std::vector<int> positions,
                  typename NonDeduced<CT>::type... Args) {

        if (detectors.size() == 0) 
            throw sls::RuntimeError("No detectors added");  
        if (positions.empty() ||
            (positions.size() == 1 && positions[0] == -1)) {
            positions.resize(detectors.size());
            std::iota(begin(positions), end(positions), 0);
        }
        std::vector<std::future<void>> futures;
        futures.reserve(positions.size());
        for (size_t i : positions) {
            if (i >= detectors.size())
                throw sls::RuntimeError("Detector out of range");
            futures.push_back(std::async(std::launch::async, somefunc,
                                         detectors[i].get(), Args...));
        }
        for (auto &i : futures) {
            i.get();
        }
    }

    template <typename... CT>
    void Parallel(void (slsDetector::*somefunc)(CT...) const,
                  std::vector<int> positions,
                  typename NonDeduced<CT>::type... Args) const {

        if (detectors.size() == 0) 
            throw sls::RuntimeError("No detectors added");  
        if (positions.empty() ||
            (positions.size() == 1 && positions[0] == -1)) {
            positions.resize(detectors.size());
            std::iota(begin(positions), end(positions), 0);
        }
        std::vector<std::future<void>> futures;
        futures.reserve(positions.size());
        for (size_t i : positions) {
            if (i >= detectors.size())
                throw sls::RuntimeError("Detector out of range");
            futures.push_back(std::async(std::launch::async, somefunc,
                                         detectors[i].get(), Args...));
        }
        for (auto &i : futures) {
            i.get();
        }
    }

    /**
     * Loop through the detectors serially and return the result as a vector
     */

    template <typename RT, typename... CT>
    std::vector<RT> serialCall(RT (slsDetector::*somefunc)(CT...),
                               typename NonDeduced<CT>::type... Args);

    /**
     * Loop through the detectors serially and return the result as a vector
     * Const qualified version
     */
    template <typename RT, typename... CT>
    std::vector<RT> serialCall(RT (slsDetector::*somefunc)(CT...) const,
                               typename NonDeduced<CT>::type... Args) const;

    /**
     * Loop through the detectors in parallel and return the result as a vector
     */
    template <typename RT, typename... CT>
    std::vector<RT> parallelCall(RT (slsDetector::*somefunc)(CT...),
                                 typename NonDeduced<CT>::type... Args);

    /**
     * Loop through the detectors in parallel and return the result as a vector
     * Const qualified version
     */
    template <typename RT, typename... CT>
    std::vector<RT> parallelCall(RT (slsDetector::*somefunc)(CT...) const,
                                 typename NonDeduced<CT>::type... Args) const;

    template <typename... CT>
    void parallelCall(void (slsDetector::*somefunc)(CT...),
                      typename NonDeduced<CT>::type... Args);

    template <typename... CT>
    void parallelCall(void (slsDetector::*somefunc)(CT...) const,
                      typename NonDeduced<CT>::type... Args) const;

    /** set acquiring flag in shared memory */
    void setAcquiringFlag(bool flag); 

    /** return multi detector shared memory ID */
    int getMultiId() const;

    std::string getPackageVersion() const;
    
    int64_t getClientSoftwareVersion() const; 

    /** Free specific shared memory from the command line without creating object */
    static void freeSharedMemory(int multiId, int detPos = -1);

    /** Free all modules from current multi Id shared memory and delete members */
    void freeSharedMemory(); 

    /** Get user details of shared memory */
    std::string getUserDetails(); 

    /**
     * Connect to Virtual Detector Servers at local host
     * @param ndet number of detectors
     * @param port starting port number
     */
    void setVirtualDetectorServers(const int numdet, const int port);

    /** Sets the hostname of all sls detectors in shared memory and updates local cache */
    void setHostname(const std::vector<std::string> &name); 

    /** Gets the total number of detectors */
    int size() const;

    slsDetectorDefs::xy getNumberOfDetectors() const; 

    slsDetectorDefs::xy getNumberOfChannels() const; 

    /** Must be set before setting hostname
     * Sets maximum number of channels of all sls detectors */
    void setNumberOfChannels(const slsDetectorDefs::xy c); 

    /**
     * Enable gap pixels, only for Eiger and for 8,16 and 32 bit mode. (Eiger)
     * 4 bit mode gap pixels only in gui call back
     */
    void setGapPixelsinReceiver(bool enable);

    /**
     * Enable data streaming to client
     * @param enable 0 to disable, 1 to enable, -1 to get the value
     * @returns data streaming to client enable
     */
    bool enableDataStreamingToClient(int enable = -1);

    void savePattern(const std::string &fname); 

    /**
     * register callback for accessing acquisition final data
     * @param func function to be called at the end of the acquisition.
     * gets detector status and progress index as arguments
     * @param pArg argument
     */
    void registerAcquisitionFinishedCallback(void (*func)(double, int, void *),
                                             void *pArg);

    /**
     * register calbback for accessing detector final data,
     * also enables data streaming in client and receiver
     * @param userCallback function for plotting/analyzing the data.
     * Its arguments are
     * the data structure d and the frame number f,
     * s is for subframe number for eiger for 32 bit mode
     * @param pArg argument
     */
    void registerDataCallback(void (*userCallback)(detectorData *, uint64_t,
                                                   uint32_t, void *),
                              void *pArg);

    /**
     * Performs a complete acquisition
     * resets frames caught in receiver, starts receiver, starts detector,
     * blocks till detector finished acquisition, stop receiver, increments file
     * index, loops for measurements, calls required call backs.
     * @returns OK or FAIL depending on if it already started
     */
    int acquire(); 

    /**
     * Combines data from all readouts and gives it to the gui
     * or just gives progress of acquisition by polling receivers
     */
    void processData();

    /**
     * Convert raw file
     * @param fname name of pof file
     * @param fpgasrc pointer in memory to read pof to
     * @returns file size
     */
    std::vector<char> readPofFile(const std::string &fname);

  private:
    /**
     * Creates/open shared memory, initializes detector structure and members
     * Called by constructor/ set hostname / read config file
     * @param verify true to verify if shared memory version matches existing
     * one
     * @param update true to update last user pid, date etc
     */
    void setupMultiDetector(bool verify = true, bool update = true);

    /**
     * Creates shm and initializes shm structure OR
     * Open shm and maps to structure
     * @param verify true to verify if shm size matches existing one
     * @param update true to update last user pid, date etc
     */
    void initSharedMemory(bool verify = true);

    /** Initialize detector structure for the shared memory just created */
    void initializeDetectorStructure();

    /** Initialize members (eg. slsDetectors from shm, zmqsockets)
     * @param verify true to verify if shm size matches existing one
     */
    void initializeMembers(bool verify = true);

    /** Update in shm */
    void updateUserdetails();

    bool isAcquireReady();

    /** Execute command in terminal and return result */
    std::string exec(const char *cmd);

    void addSlsDetector(const std::string &hostname);

    void updateDetectorSize();

    /**
     * Create Receiving Data Sockets
     * @param destroy is true to destroy all the sockets
     * @returns OK or FAIL
     */
    int createReceivingDataSockets(const bool destroy = false);

    /**
     * Reads frames from receiver through a constant socket
     * Called during acquire() when call back registered or when using gui
     */
    void readFrameFromReceiver();

    /**
     * add gap pixels to the image (only for Eiger in 4 bit mode)
     * @param image pointer to image without gap pixels
     * @param gpImage poiner to image with gap pixels, if NULL, allocated
     * inside function
     * quadEnable quad enabled
     * @returns number of data bytes of image with gap pixels
     */
    int processImageWithGapPixels(char *image, char *&gpImage, bool quadEnable);

    double setTotalProgress();

    double getCurrentProgress();

    void incrementProgress();

    void setCurrentProgress(int64_t i = 0);

    void startProcessingThread();

    /**
     * Check if processing thread is ready to join main thread
     * @returns true if ready, else false
     */
    bool getJoinThreadFlag() const;

    /**
     * Main thread sets if the processing thread should join it
     * @param v true if it should join, else false
     */
    void setJoinThreadFlag(bool value);

    /**
     * Listen to key event to stop acquiring
     * when using acquire command
     */
    int kbhit();

    /**
     * Convert a double holding time in seconds to an int64_t with nano seconds
     * Used for conversion when sending time to detector
     * @param t time in seconds
     * @returns time in nano seconds
     */
    int64_t secondsToNanoSeconds(double t);

    /** Multi detector Id */
    const int multiId{0};

    /** Shared Memory object */
    sls::SharedMemory<sharedMultiSlsDetector> multi_shm{0, -1};

    /** pointers to the slsDetector structures */
    std::vector<std::unique_ptr<slsDetector>> detectors;

    /** data streaming (down stream) enabled in client (zmq sckets created) */
    bool client_downstream{false};

    /** ZMQ Socket - Receiver to Client */
    std::vector<std::unique_ptr<ZmqSocket>> zmqSocket;

    /** semaphore to let postprocessing thread continue for next
     * scan/measurement */
    sem_t sem_newRTAcquisition;

    /** semaphore to let main thread know it got all the dummy packets (also
     * from ext. process) */
    sem_t sem_endRTAcquisition;

    /** Total number of frames/images for next acquisition */
    double totalProgress{0};

    /** Current progress or frames/images processed in current acquisition */
    double progressIndex{0};

    /** mutex to synchronize main and data processing threads */
    mutable std::mutex mp;

    /** sets when the acquisition is finished */
    bool jointhread{false};

    /** the data processing thread */
    std::thread dataProcessingThread;

    /** detector data packed for the gui */
    detectorData *thisData{nullptr};

    void (*acquisition_finished)(double, int, void *){nullptr};
    void *acqFinished_p{nullptr};

    void (*dataReady)(detectorData *, uint64_t, uint32_t, void *){nullptr};
    void *pCallbackArg{nullptr};
};
