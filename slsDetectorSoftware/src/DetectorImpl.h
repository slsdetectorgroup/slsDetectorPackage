// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "CtbConfig.h"
#include "SharedMemory.h"
#include "sls/Result.h"
#include "sls/ZmqSocket.h"
#include "sls/logger.h"
#include "sls/sls_detector_defs.h"

#include <future>
#include <memory>
#include <mutex>
#include <numeric>
#include <semaphore.h>
#include <string>
#include <thread>
#include <vector>

namespace sls {

class detectorData;
class Module;

#define DETECTOR_SHMAPIVERSION 0x190809
#define DETECTOR_SHMVERSION    0x220505
#define SHORT_STRING_LENGTH    50

/**
 * @short structure allocated in shared memory to store detector settings
 * for IPC and cache
 */
struct sharedDetector {

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

    int totalNumberOfModules;
    slsDetectorDefs::detectorType detType;

    /** END OF FIXED PATTERN
     * -----------------------------------------------*/

    /** Number of modules operated at once */
    slsDetectorDefs::xy numberOfModules;

    /**  max number of channels for complete detector*/
    slsDetectorDefs::xy numberOfChannels;

    bool acquiringFlag;
    bool initialChecks;
    bool gapPixels;
    /** high water mark of listening tcp port (only data) */
    int zmqHwm;
    /** in shm for gui purposes */
    defs::ROI rx_roi{};
};

class DetectorImpl : public virtual slsDetectorDefs {
  public:
    /**
     * @param verify true to verify if shared memory version matches existing
     * one
     * @param update true to update last user pid, date etc
     */
    explicit DetectorImpl(int detector_index = 0, bool verify = true,
                          bool update = true);

    template <class CT> struct NonDeduced {
        using type = CT;
    };
    template <typename RT, typename... CT>
    Result<RT> Parallel(RT (Module::*somefunc)(CT...),
                        std::vector<int> positions,
                        typename NonDeduced<CT>::type... Args) {

        if (modules.empty())
            throw RuntimeError("No modules added");
        if (positions.empty() ||
            (positions.size() == 1 && positions[0] == -1)) {
            positions.resize(modules.size());
            std::iota(begin(positions), end(positions), 0);
        }
        std::vector<std::future<RT>> futures;
        futures.reserve(positions.size());
        for (size_t i : positions) {
            if (i >= modules.size())
                throw RuntimeError("Module out of range");
            futures.push_back(std::async(std::launch::async, somefunc,
                                         modules[i].get(), Args...));
        }
        Result<RT> result;
        result.reserve(positions.size());
        for (auto &i : futures) {
            result.push_back(i.get());
        }
        return result;
    }

    template <typename RT, typename... CT>
    Result<RT> Parallel(RT (Module::*somefunc)(CT...) const,
                        std::vector<int> positions,
                        typename NonDeduced<CT>::type... Args) const {

        if (modules.empty())
            throw RuntimeError("No modules added");
        if (positions.empty() ||
            (positions.size() == 1 && positions[0] == -1)) {
            positions.resize(modules.size());
            std::iota(begin(positions), end(positions), 0);
        }
        std::vector<std::future<RT>> futures;
        futures.reserve(positions.size());
        for (size_t i : positions) {
            if (i >= modules.size())
                throw RuntimeError("Module out of range");
            futures.push_back(std::async(std::launch::async, somefunc,
                                         modules[i].get(), Args...));
        }
        Result<RT> result;
        result.reserve(positions.size());
        for (auto &i : futures) {
            result.push_back(i.get());
        }
        return result;
    }

    template <typename... CT>
    void Parallel(void (Module::*somefunc)(CT...), std::vector<int> positions,
                  typename NonDeduced<CT>::type... Args) {

        if (modules.empty())
            throw RuntimeError("No modules added");
        if (positions.empty() ||
            (positions.size() == 1 && positions[0] == -1)) {
            positions.resize(modules.size());
            std::iota(begin(positions), end(positions), 0);
        }
        std::vector<std::future<void>> futures;
        futures.reserve(positions.size());
        for (size_t i : positions) {
            if (i >= modules.size())
                throw RuntimeError("Module out of range");
            futures.push_back(std::async(std::launch::async, somefunc,
                                         modules[i].get(), Args...));
        }
        for (auto &i : futures) {
            i.get();
        }
    }

    template <typename... CT>
    void Parallel(void (Module::*somefunc)(CT...) const,
                  std::vector<int> positions,
                  typename NonDeduced<CT>::type... Args) const {

        if (modules.empty())
            throw RuntimeError("No modules added");
        if (positions.empty() ||
            (positions.size() == 1 && positions[0] == -1)) {
            positions.resize(modules.size());
            std::iota(begin(positions), end(positions), 0);
        }
        std::vector<std::future<void>> futures;
        futures.reserve(positions.size());
        for (size_t i : positions) {
            if (i >= modules.size())
                throw RuntimeError("Module out of range");
            futures.push_back(std::async(std::launch::async, somefunc,
                                         modules[i].get(), Args...));
        }
        for (auto &i : futures) {
            i.get();
        }
    }

    bool isAllPositions(Positions pos) const;

    /** set acquiring flag in shared memory */
    void setAcquiringFlag(bool flag);

    /** return detector index in shared memory */
    int getDetectorIndex() const;

    /** Get user details of shared memory */
    std::string getUserDetails();

    bool getInitialChecks() const;

    /** initial compaibility and other server start up checks
     * default enabled */
    void setInitialChecks(const bool value);

    bool hasModulesInSharedMemory();

    /** Sets the hostname of all sls modules in shared memory and updates
     * local cache */
    void setHostname(const std::vector<std::string> &name);

    /** Gets the total number of modules */
    int size() const;

    slsDetectorDefs::xy getNumberOfModules() const;

    slsDetectorDefs::xy getNumberOfChannels() const;

    /** Must be set before setting hostname
     * Sets maximum number of channels of all sls modules */
    void setNumberOfChannels(const slsDetectorDefs::xy c);

    bool getGapPixelsinCallback() const;
    void setGapPixelsinCallback(const bool enable);
    int getTransmissionDelay() const;
    void setTransmissionDelay(int step);
    bool getDataStreamingToClient();
    void setDataStreamingToClient(bool enable);
    int getClientStreamingHwm() const;
    void setClientStreamingHwm(const int limit);

    /**
     * register callback for accessing acquisition final data
     * @param func function to be called at the end of the acquisition.
     * gets module status and progress index as arguments
     * @param pArg argument
     */
    void registerAcquisitionFinishedCallback(void (*func)(double, int, void *),
                                             void *pArg);

    /**
     * register calbback for accessing module final data,
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

    /** also takes care of master and slave for multi module mythen */
    void startAcquisition(const bool blocking, Positions pos);

    /** also takes care of master and slave for multi module mythen */
    void sendSoftwareTrigger(const bool block, Positions pos);

    /** also takes care of master and slave for multi module mythen */
    void stopDetector(Positions pos);

    /**
     * Combines data from all readouts and gives it to the gui
     * or just gives progress of acquisition by polling receivers
     */
    void processData(bool receiver);

    /**
     * Convert raw file
     * [Jungfrau][Ctb][Moench] from pof file
     * [Mythen3][Gotthard2] from rbf file
     * @param fname name of pof/rbf file
     * @returns binary of the program
     */
    std::vector<char> readProgrammingFile(const std::string &fname);

    void setNumberofUDPInterfaces(int n, Positions pos);
    Result<int> getDefaultDac(defs::dacIndex index, defs::detectorSettings sett,
                              Positions pos = {});
    void setDefaultDac(defs::dacIndex index, int defaultValue,
                       defs::detectorSettings sett, Positions pos);

    void verifyUniqueDetHost(const uint16_t port,
                             std::vector<int> positions) const;
    void verifyUniqueRxHost(const uint16_t port, const int moduleId) const;

    std::pair<std::string, uint16_t>
    verifyUniqueDetHost(const std::string &name);
    std::pair<std::string, uint16_t>
    verifyUniqueRxHost(const std::string &name,
                       std::vector<int> positions) const;
    std::vector<std::pair<std::string, uint16_t>>
    verifyUniqueRxHost(const std::vector<std::string> &names) const;

    defs::ROI getRxROI() const;
    void setRxROI(const defs::ROI arg);
    void clearRxROI();

    void getBadChannels(const std::string &fname, Positions pos) const;
    void setBadChannels(const std::string &fname, Positions pos);
    void setBadChannels(const std::vector<int> list, Positions pos);

    std::vector<std::string> getCtbDacNames() const;
    std::string getCtbDacName(const defs::dacIndex i) const;
    void setCtbDacNames(const std::vector<std::string> &names);
    void setCtbDacName(const defs::dacIndex index, const std::string &name);

    std::vector<std::string> getCtbAdcNames() const;
    std::string getCtbAdcName(const int i) const;
    void setCtbAdcNames(const std::vector<std::string> &names);
    void setCtbAdcName(const int index, const std::string &name);

    std::vector<std::string> getCtbSignalNames() const;
    std::string getCtbSignalName(const int i) const;
    void setCtbSignalNames(const std::vector<std::string> &names);
    void setCtbSignalName(const int index, const std::string &name);

    std::vector<std::string> getCtbPowerNames() const;
    std::string getCtbPowerName(const defs::dacIndex i) const;
    void setCtbPowerNames(const std::vector<std::string> &names);
    void setCtbPowerName(const defs::dacIndex index, const std::string &name);

    std::vector<std::string> getCtbSlowADCNames() const;
    std::string getCtbSlowADCName(const defs::dacIndex i) const;
    void setCtbSlowADCNames(const std::vector<std::string> &names);
    void setCtbSlowADCName(const defs::dacIndex index, const std::string &name);

  private:
    /**
     * Creates/open shared memory, initializes detector structure and members
     * Called by constructor/ set hostname / read config file
     * @param verify true to verify if shared memory version matches existing
     * one
     * @param update true to update last user pid, date etc
     */
    void setupDetector(bool verify = true, bool update = true);

    /**
     * Creates shm and initializes shm structure OR
     * Open shm and maps to structure
     * @param verify true to verify if shm size matches existing one
     */
    void initSharedMemory(bool verify = true);

    /** Initialize detector structure for the shared memory just created */
    void initializeDetectorStructure();

    /** Initialize members (eg. modules from shm, zmqsockets)
     * @param verify true to verify if shm size matches existing one
     */
    void initializeMembers(bool verify = true);

    /** Update in shm */
    void updateUserdetails();

    bool isAcquireReady();

    /** Execute command in terminal and return result */
    std::string exec(const char *cmd);

    void addModule(const std::string &hostname);

    void updateDetectorSize();

    void destroyReceivingDataSockets();
    void createReceivingDataSockets();

    /**
     * Reads frames from receiver through a constant socket
     * Called during acquire() when call back registered or when using gui
     */
    void readFrameFromReceiver();

    /** [Eiger][Jungfrau][Moench]
     * add gap pixels to the imag
     * @param image pointer to image without gap pixels
     * @param gpImage poiner to image with gap pixels, if NULL, allocated
     * @param quadEnable quad enabled
     * @param dr dynamic range
     * @param nPixelsx number of pixels in X axis (updated)
     * @param nPixelsy number of pixels in Y axis (updated)
     * @returns total data bytes for updated image
     */
    int insertGapPixels(char *image, char *&gpImage, bool quadEnable, int dr,
                        int &nPixelsx, int &nPixelsy);

    bool handleSynchronization(Positions pos);
    void getMasterSlaveList(std::vector<int> positions,
                            std::vector<int> &masters,
                            std::vector<int> &slaves);

    void printProgress(double progress);

    void startProcessingThread(bool receiver);

    /**
     * Check if processing thread is ready to join main thread
     * @returns true if ready, else false
     */
    bool getJoinThreadFlag() const;

    /**
     * Main thread sets if the processing thread should join it
     * @param value true if it should join, else false
     */
    void setJoinThreadFlag(bool value);

    /**
     * Listen to key event to stop acquiring
     * when using acquire command
     */
    int kbhit();

    defs::xy getPortGeometry() const;
    defs::xy calculatePosition(int moduleIndex, defs::xy geometry) const;

    void verifyUniqueHost(
        bool isDet, std::vector<std::pair<std::string, uint16_t>> &hosts) const;

    const int detectorIndex{0};
    SharedMemory<sharedDetector> shm{0, -1};
    SharedMemory<CtbConfig> ctb_shm{0, -1, CtbConfig::shm_tag()};
    std::vector<std::unique_ptr<Module>> modules;

    /** data streaming (down stream) enabled in client (zmq sckets created) */
    bool client_downstream{false};
    std::vector<std::unique_ptr<ZmqSocket>> zmqSocket;
    std::atomic<int> numZmqRunning{0};

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

} // namespace sls