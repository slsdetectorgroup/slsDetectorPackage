#include "multiSlsDetector.h"
#include "SharedMemory.h"
#include "ZmqSocket.h"
#include "detectorData.h"
#include "file_utils.h"
#include "logger.h"
#include "multiSlsDetectorClient.h"
#include "slsDetector.h"
#include "slsDetectorCommand.h"
#include "sls_detector_exceptions.h"
#include "versionAPI.h"

#include "container_utils.h"
#include "network_utils.h"
#include "string_utils.h"
#include "ToString.h"

#include <cstring>
#include <iomanip>
#include <iostream>
#include <rapidjson/document.h> //json header in zmq stream
#include <sstream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#include <chrono>
#include <future>
#include <vector>

using namespace sls;

multiSlsDetector::multiSlsDetector(int multi_id, bool verify, bool update)
    : multiId(multi_id), multi_shm(multi_id, -1) {
    setupMultiDetector(verify, update);
}

multiSlsDetector::~multiSlsDetector() = default;

void multiSlsDetector::setupMultiDetector(bool verify, bool update) {
    initSharedMemory(verify);
    initializeMembers(verify);
    if (update) {
        updateUserdetails();
    }
}

template <typename RT, typename... CT>
std::vector<RT>
multiSlsDetector::serialCall(RT (slsDetector::*somefunc)(CT...),
                             typename NonDeduced<CT>::type... Args) {
    std::vector<RT> result;
    result.reserve(detectors.size());
    for (auto &d : detectors) {
        result.push_back((d.get()->*somefunc)(Args...));
    }
    return result;
}

template <typename RT, typename... CT>
std::vector<RT>
multiSlsDetector::serialCall(RT (slsDetector::*somefunc)(CT...) const,
                             typename NonDeduced<CT>::type... Args) const {
    std::vector<RT> result;
    result.reserve(detectors.size());
    for (auto &d : detectors) {
        result.push_back((d.get()->*somefunc)(Args...));
    }
    return result;
}

template <typename RT, typename... CT>
std::vector<RT>
multiSlsDetector::parallelCall(RT (slsDetector::*somefunc)(CT...),
                               typename NonDeduced<CT>::type... Args) {
    std::vector<std::future<RT>> futures;
    for (auto &d : detectors) {
        futures.push_back(
            std::async(std::launch::async, somefunc, d.get(), Args...));
    }
    std::vector<RT> result;
    result.reserve(detectors.size());
    for (auto &i : futures) {
        result.push_back(i.get());
    }
    return result;
}

template <typename RT, typename... CT>
std::vector<RT>
multiSlsDetector::parallelCall(RT (slsDetector::*somefunc)(CT...) const,
                               typename NonDeduced<CT>::type... Args) const {
    std::vector<std::future<RT>> futures;
    for (auto &d : detectors) {
        futures.push_back(
            std::async(std::launch::async, somefunc, d.get(), Args...));
    }
    std::vector<RT> result;
    result.reserve(detectors.size());
    for (auto &i : futures) {
        result.push_back(i.get());
    }
    return result;
}

template <typename... CT>
void multiSlsDetector::parallelCall(void (slsDetector::*somefunc)(CT...),
                                    typename NonDeduced<CT>::type... Args) {
    std::vector<std::future<void>> futures;
    for (auto &d : detectors) {
        futures.push_back(
            std::async(std::launch::async, somefunc, d.get(), Args...));
    }
    for (auto &i : futures) {
        i.get();
    }
    return;
}

template <typename... CT>
void multiSlsDetector::parallelCall(
    void (slsDetector::*somefunc)(CT...) const,
    typename NonDeduced<CT>::type... Args) const {
    std::vector<std::future<void>> futures;
    for (auto &d : detectors) {
        futures.push_back(
            std::async(std::launch::async, somefunc, d.get(), Args...));
    }
    for (auto &i : futures) {
        i.get();
    }
    return;
}

void multiSlsDetector::setAcquiringFlag(bool flag) {
    multi_shm()->acquiringFlag = flag;
}

int multiSlsDetector::getMultiId() const { return multiId; }

std::string multiSlsDetector::getPackageVersion() const { return GITBRANCH; }

int64_t multiSlsDetector::getClientSoftwareVersion() const { return APILIB; }

void multiSlsDetector::freeSharedMemory(int multiId, int detPos) {
    // single
    if (detPos >= 0) {
        SharedMemory<sharedSlsDetector> temp_shm(multiId, detPos);
        if (temp_shm.IsExisting()) {
            temp_shm.RemoveSharedMemory();
        }
        return;
    }

    // multi - get number of detectors from shm
    SharedMemory<sharedMultiSlsDetector> multiShm(multiId, -1);
    int numDetectors = 0;

    if (multiShm.IsExisting()) {
        multiShm.OpenSharedMemory();
        numDetectors = multiShm()->numberOfDetectors;
        multiShm.RemoveSharedMemory();
    }

    for (int i = 0; i < numDetectors; ++i) {
        SharedMemory<sharedSlsDetector> shm(multiId, i);
        shm.RemoveSharedMemory();
    }
}

void multiSlsDetector::freeSharedMemory() {
    zmqSocket.clear();
    for (auto &d : detectors) {
        d->freeSharedMemory();
    }
    detectors.clear();

    // clear multi detector shm
    multi_shm.RemoveSharedMemory();
    client_downstream = false;
}

std::string multiSlsDetector::getUserDetails() {
    if (detectors.empty()) {
        return std::string("none");
    }

    std::ostringstream sstream;
    sstream << "\nHostname: ";
    for (auto &d : detectors) {
        sstream << (d->isFixedPatternSharedMemoryCompatible() ? d->getHostname()
                                                              : "Unknown")
                << "+";
    }
    sstream << "\nType: ";
    // get type from multi shm
    if (multi_shm()->shmversion >= MULTI_SHMAPIVERSION) {
        sstream << ToString(multi_shm()->multiDetectorType);
    }
    // get type from slsdet shm
    else {
        for (auto &d : detectors) {
            sstream << (d->isFixedPatternSharedMemoryCompatible()
                            ? ToString(d->getDetectorType())
                            : "Unknown")
                    << "+";
        }
    }

    sstream << "\nPID: " << multi_shm()->lastPID
            << "\nUser: " << multi_shm()->lastUser
            << "\nDate: " << multi_shm()->lastDate << std::endl;

    return sstream.str();
}

void multiSlsDetector::initSharedMemory(bool verify) {
    if (!multi_shm.IsExisting()) {
        multi_shm.CreateSharedMemory();
        initializeDetectorStructure();
    } else {
        multi_shm.OpenSharedMemory();
        if (verify && multi_shm()->shmversion != MULTI_SHMVERSION) {
            FILE_LOG(logERROR) << "Multi shared memory (" << multiId
                               << ") version mismatch "
                                  "(expected 0x"
                               << std::hex << MULTI_SHMVERSION << " but got 0x"
                               << multi_shm()->shmversion << std::dec
                               << ". Clear Shared memory to continue.";
            throw SharedMemoryError("Shared memory version mismatch!");
        }
    }
}

void multiSlsDetector::initializeDetectorStructure() {
    multi_shm()->shmversion = MULTI_SHMVERSION;
    multi_shm()->numberOfDetectors = 0;
    multi_shm()->multiDetectorType = GENERIC;
    multi_shm()->numberOfDetector.x = 0;
    multi_shm()->numberOfDetector.y = 0;
    multi_shm()->numberOfChannels.x = 0;
    multi_shm()->numberOfChannels.y = 0;
    multi_shm()->acquiringFlag = false;
    multi_shm()->receiver_upstream = false;
}

void multiSlsDetector::initializeMembers(bool verify) {
    // multiSlsDetector
    zmqSocket.clear();

    // get objects from single det shared memory (open)
    for (int i = 0; i < multi_shm()->numberOfDetectors; i++) {
        try {
            detectors.push_back(
                sls::make_unique<slsDetector>(multiId, i, verify));
        } catch (...) {
            detectors.clear();
            throw;
        }
    }
}

void multiSlsDetector::updateUserdetails() {
    multi_shm()->lastPID = getpid();
    memset(multi_shm()->lastUser, 0, SHORT_STRING_LENGTH);
    memset(multi_shm()->lastDate, 0, SHORT_STRING_LENGTH);
    try {
        sls::strcpy_safe(multi_shm()->lastUser, exec("whoami").c_str());
        sls::strcpy_safe(multi_shm()->lastDate, exec("date").c_str());
    } catch (...) {
        sls::strcpy_safe(multi_shm()->lastUser, "errorreading");
        sls::strcpy_safe(multi_shm()->lastDate, "errorreading");
    }
}

bool multiSlsDetector::isAcquireReady() {
    if (multi_shm()->acquiringFlag) {
        FILE_LOG(logWARNING)
            << "Acquire has already started. "
               "If previous acquisition terminated unexpectedly, "
               "reset busy flag to restart.(sls_detector_put busy 0)";
        return false;
    }
    multi_shm()->acquiringFlag = true;
    return true;
}

std::string multiSlsDetector::exec(const char *cmd) {
    int bufsize = 128;
    char buffer[bufsize];
    std::string result = "";
    FILE *pipe = popen(cmd, "r");
    if (pipe == nullptr) {
        throw RuntimeError("Could not open pipe");
    }
    try {
        while (feof(pipe) == 0) {
            if (fgets(buffer, bufsize, pipe) != nullptr) {
                result += buffer;
            }
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    result.erase(result.find_last_not_of(" \t\n\r") + 1);
    return result;
}

void multiSlsDetector::setVirtualDetectorServers(const int numdet,
                                                 const int port) {
    std::vector<std::string> hostnames;
    for (int i = 0; i < numdet; ++i) {
        // * 2 is for control and stop port
        hostnames.push_back(std::string("localhost:") +
                            std::to_string(port + i * 2));
    }
    setHostname(hostnames);
}

void multiSlsDetector::setHostname(const std::vector<std::string> &name) {
    // this check is there only to allow the previous detsizechan command
    if (multi_shm()->numberOfDetectors != 0) {
        FILE_LOG(logWARNING)
            << "There are already detector(s) in shared memory."
               "Freeing Shared memory now.";
        freeSharedMemory();
        setupMultiDetector();
    }
    for (const auto &hostname : name) {
        addSlsDetector(hostname);
    }
    updateDetectorSize();
}

void multiSlsDetector::addSlsDetector(const std::string &hostname) {
    FILE_LOG(logINFO) << "Adding detector " << hostname;

    int port = DEFAULT_PORTNO;
    std::string host = hostname;
    auto res = sls::split(hostname, ':');
    if (res.size() > 1) {
        host = res[0];
        port = std::stoi(res[1]);
    }

    if (host != "localhost") {
        for (auto &d : detectors) {
            if (d->getHostname() == host) {
                FILE_LOG(logWARNING)
                    << "Detector " << host
                    << "already part of the multiDetector!" << std::endl
                    << "Remove it before adding it back in a new position!";
                return;
            }
        }
    }

    // get type by connecting
    detectorType type = slsDetector::getTypeFromDetector(host, port);
    int pos = (int)detectors.size();
    detectors.push_back(
        sls::make_unique<slsDetector>(type, multiId, pos, false));
    multi_shm()->numberOfDetectors = detectors.size();
    detectors[pos]->setControlPort(port);
    detectors[pos]->setStopPort(port + 1);
    detectors[pos]->setHostname(host);
    // detector type updated by now
    multi_shm()->multiDetectorType = Parallel(&slsDetector::getDetectorType, {}).tsquash("Inconsistent detector types.");
}

void multiSlsDetector::updateDetectorSize() {
    FILE_LOG(logDEBUG) << "Updating Multi-Detector Size: " << size();

    const slsDetectorDefs::xy det_size = detectors[0]->getNumberOfChannels();

    int maxy = multi_shm()->numberOfChannels.y;
    if (maxy == 0) {
        maxy = det_size.y * size();
    }

    int ndety = maxy / det_size.y;
    int ndetx = size() / ndety;
    if ((maxy % det_size.y) > 0) {
        ++ndetx;
    }

    multi_shm()->numberOfDetector.x = ndetx;
    multi_shm()->numberOfDetector.y = ndety;
    multi_shm()->numberOfChannels.x = det_size.x * ndetx;
    multi_shm()->numberOfChannels.y = det_size.y * ndety;

    FILE_LOG(logDEBUG) << "\n\tNumber of Detectors in X direction:"
                       << multi_shm()->numberOfDetector.x
                       << "\n\tNumber of Detectors in Y direction:"
                       << multi_shm()->numberOfDetector.y
                       << "\n\tNumber of Channels in X direction:"
                       << multi_shm()->numberOfChannels.x
                       << "\n\tNumber of Channels in Y direction:"
                       << multi_shm()->numberOfChannels.y;

    for (auto &d : detectors) {
        d->updateMultiSize(multi_shm()->numberOfDetector);
    }
}

int multiSlsDetector::size() const { return detectors.size(); }

slsDetectorDefs::xy multiSlsDetector::getNumberOfDetectors() const {
    return multi_shm()->numberOfDetector;
}

slsDetectorDefs::xy multiSlsDetector::getNumberOfChannels() const {
    return multi_shm()->numberOfChannels;
}

void multiSlsDetector::setNumberOfChannels(const slsDetectorDefs::xy c) {
    if (size() > 1) {
        throw RuntimeError(
            "Set the number of channels before setting hostname.");
    }
    multi_shm()->numberOfChannels = c;
}

void multiSlsDetector::readConfigurationFile(const std::string &fname) {
    freeSharedMemory();
    setupMultiDetector();
    FILE_LOG(logINFO) << "Loading configuration file: " << fname;

    std::ifstream input_file;
    input_file.open(fname.c_str(), std::ios_base::in);
    if (!input_file.is_open()) {
        throw RuntimeError("Could not open configuration file " + fname +
                           " for reading");
    }
    std::string current_line;
    while (input_file.good()) {
        getline(input_file, current_line);
        if (current_line.find('#') != std::string::npos) {
            current_line.erase(current_line.find('#'));
        }
        FILE_LOG(logDEBUG1)
            << "current_line after removing comments:\n\t" << current_line;
        if (current_line.length() > 1) {
            multiSlsDetectorClient(current_line, PUT_ACTION, nullptr);
        }
    }
    input_file.close();
}

void multiSlsDetector::setGapPixelsinReceiver(bool enable) {
    Parallel(&slsDetector::enableGapPixels, {}, static_cast<int>(enable));
    // update number of channels
    Result<slsDetectorDefs::xy> res =
        Parallel(&slsDetector::getNumberOfChannels, {});
    multi_shm()->numberOfChannels.x = 0;
    multi_shm()->numberOfChannels.y = 0;
    for (auto &it : res) {
        multi_shm()->numberOfChannels.x += it.x;
        multi_shm()->numberOfChannels.y += it.y;
    }
}

int multiSlsDetector::createReceivingDataSockets(const bool destroy) {
    if (destroy) {
        FILE_LOG(logINFO) << "Going to destroy data sockets";
        // close socket
        zmqSocket.clear();

        client_downstream = false;
        FILE_LOG(logINFO) << "Destroyed Receiving Data Socket(s)";
        return OK;
    }

    FILE_LOG(logINFO) << "Going to create data sockets";

    size_t numSockets = detectors.size();
    size_t numSocketsPerDetector = 1;
    if (multi_shm()->multiDetectorType == EIGER) {
        numSocketsPerDetector = 2;
    }
    if (Parallel(&slsDetector::getNumberofUDPInterfacesFromShm, {}).squash() == 2) {
        numSocketsPerDetector = 2;
    }
    numSockets *= numSocketsPerDetector;

    for (size_t iSocket = 0; iSocket < numSockets; ++iSocket) {
        uint32_t portnum = (detectors[iSocket / numSocketsPerDetector]
                                ->getClientStreamingPort());
        portnum += (iSocket % numSocketsPerDetector);
        try {
            zmqSocket.push_back(sls::make_unique<ZmqSocket>(
                detectors[iSocket / numSocketsPerDetector]
                    ->getClientStreamingIP().str()
                    .c_str(),
                portnum));
            FILE_LOG(logINFO) << "Zmq Client[" << iSocket << "] at "
                              << zmqSocket.back()->GetZmqServerAddress();
        } catch (...) {
            FILE_LOG(logERROR)
                << "Could not create Zmq socket on port " << portnum;
            createReceivingDataSockets(true);
            return FAIL;
        }
    }

    client_downstream = true;
    FILE_LOG(logINFO) << "Receiving Data Socket(s) created";
    return OK;
}

void multiSlsDetector::readFrameFromReceiver() {

    int nX = 0;
    int nY = 0;
    int nDetPixelsX = 0;
    int nDetPixelsY = 0;
    bool gappixelsenable = false;
    bool quadEnable = false;
    bool eiger = false;
    bool numInterfaces = Parallel(&slsDetector::getNumberofUDPInterfacesFromShm, {}).squash(); // cannot pick up from zmq

    bool runningList[zmqSocket.size()], connectList[zmqSocket.size()];
    int numRunning = 0;
    for (size_t i = 0; i < zmqSocket.size(); ++i) {
        if (zmqSocket[i]->Connect() == 0) {
            connectList[i] = true;
            runningList[i] = true;
            ++numRunning;
        } else {
            // to remember the list it connected to, to disconnect later
            connectList[i] = false;
            FILE_LOG(logERROR) << "Could not connect to socket  "
                               << zmqSocket[i]->GetZmqServerAddress();
            runningList[i] = false;
        }
    }
    int numConnected = numRunning;
    bool data = false;
    char *image = nullptr;
    char *multiframe = nullptr;
    char *multigappixels = nullptr;
    int multisize = 0;
    // only first message header
    uint32_t size = 0, nPixelsX = 0, nPixelsY = 0, dynamicRange = 0;
    float bytesPerPixel = 0;
    // header info every header
    std::string currentFileName = "";
    uint64_t currentAcquisitionIndex = -1, currentFrameIndex = -1,
             currentFileIndex = -1;
    uint32_t currentSubFrameIndex = -1, coordX = -1, coordY = -1,
             flippedDataX = -1;

    // wait for real time acquisition to start
    bool running = true;
    sem_wait(&sem_newRTAcquisition);
    if (getJoinThreadFlag()) {
        running = false;
    }

    while (running) {
        // reset data
        data = false;
        if (multiframe != nullptr) {
            memset(multiframe, 0xFF, multisize);
        }

        // get each frame
        for (unsigned int isocket = 0; isocket < zmqSocket.size(); ++isocket) {

            // if running
            if (runningList[isocket]) {

                // HEADER
                {
                    rapidjson::Document doc;
                    if (zmqSocket[isocket]->ReceiveHeader(
                            isocket, doc, SLS_DETECTOR_JSON_HEADER_VERSION) ==
                        0) {
                        // parse error, version error or end of acquisition for
                        // socket
                        runningList[isocket] = false;
                        --numRunning;
                        continue;
                    }

                    // if first message, allocate (all one time stuff)
                    if (image == nullptr) {
                        // allocate
                        size = doc["size"].GetUint();
                        multisize = size * zmqSocket.size();
                        image = new char[size];
                        multiframe = new char[multisize];
                        memset(multiframe, 0xFF, multisize);
                        // dynamic range
                        dynamicRange = doc["bitmode"].GetUint();
                        bytesPerPixel = (float)dynamicRange / 8;
                        // shape
                        nPixelsX = doc["shape"][0].GetUint();
                        nPixelsY = doc["shape"][1].GetUint();
                        // detector shape
                        nX = doc["detshape"][0].GetUint();
                        nY = doc["detshape"][1].GetUint();
                        nY *= numInterfaces;
                        nDetPixelsX = nX * nPixelsX;
                        nDetPixelsY = nY * nPixelsY;
                        // det type
                        eiger =
                            (doc["detType"].GetUint() == static_cast<int>(3))
                                ? true
                                : false; // to be changed to EIGER when firmware
                                         // updates its header data
                        gappixelsenable =
                            (doc["gappixels"].GetUint() == 0) ? false : true;
                        quadEnable =
                            (doc["quad"].GetUint() == 0) ? false : true;
                        FILE_LOG(logDEBUG1)
                            << "One Time Header Info:"
                               "\n\tsize: "
                            << size << "\n\tmultisize: " << multisize
                            << "\n\tdynamicRange: " << dynamicRange
                            << "\n\tbytesPerPixel: " << bytesPerPixel
                            << "\n\tnPixelsX: " << nPixelsX
                            << "\n\tnPixelsY: " << nPixelsY << "\n\tnX: " << nX
                            << "\n\tnY: " << nY << "\n\teiger: " << eiger
                            << "\n\tgappixelsenable: " << gappixelsenable
                            << "\n\tquadEnable: " << quadEnable;
                    }
                    // each time, parse rest of header
                    currentFileName = doc["fname"].GetString();
                    currentAcquisitionIndex = doc["acqIndex"].GetUint64();
                    currentFrameIndex = doc["fIndex"].GetUint64();
                    currentFileIndex = doc["fileIndex"].GetUint64();
                    currentSubFrameIndex = doc["expLength"].GetUint();
                    coordY = doc["row"].GetUint();
                    coordX = doc["column"].GetUint();
                    if (eiger) {
                        coordY = (nY - 1) - coordY;
                    }
                    flippedDataX = doc["flippedDataX"].GetUint();
                    FILE_LOG(logDEBUG1)
                        << "Header Info:"
                           "\n\tcurrentFileName: "
                        << currentFileName << "\n\tcurrentAcquisitionIndex: "
                        << currentAcquisitionIndex
                        << "\n\tcurrentFrameIndex: " << currentFrameIndex
                        << "\n\tcurrentFileIndex: " << currentFileIndex
                        << "\n\tcurrentSubFrameIndex: " << currentSubFrameIndex
                        << "\n\tcoordX: " << coordX << "\n\tcoordY: " << coordY
                        << "\n\tflippedDataX: " << flippedDataX;
                }

                // DATA
                data = true;
                zmqSocket[isocket]->ReceiveData(isocket, image, size);

                // creating multi image
                {
                    uint32_t xoffset = coordX * nPixelsX * bytesPerPixel;
                    uint32_t yoffset = coordY * nPixelsY;
                    uint32_t singledetrowoffset = nPixelsX * bytesPerPixel;
                    uint32_t rowoffset = nX * singledetrowoffset;
                    if (multi_shm()->multiDetectorType == CHIPTESTBOARD) {
                        singledetrowoffset = size;
                    }
                    FILE_LOG(logDEBUG1)
                        << "Multi Image Info:"
                           "\n\txoffset: "
                        << xoffset << "\n\tyoffset: " << yoffset
                        << "\n\tsingledetrowoffset: " << singledetrowoffset
                        << "\n\trowoffset: " << rowoffset;

                    if (eiger && (flippedDataX != 0u)) {
                        for (uint32_t i = 0; i < nPixelsY; ++i) {
                            memcpy(((char *)multiframe) +
                                       ((yoffset + (nPixelsY - 1 - i)) *
                                        rowoffset) +
                                       xoffset,
                                   (char *)image + (i * singledetrowoffset),
                                   singledetrowoffset);
                        }
                    } else {
                        for (uint32_t i = 0; i < nPixelsY; ++i) {
                            memcpy(((char *)multiframe) +
                                       ((yoffset + i) * rowoffset) + xoffset,
                                   (char *)image + (i * singledetrowoffset),
                                   singledetrowoffset);
                        }
                    }
                }
            }
        }
        FILE_LOG(logDEBUG) << "Call Back Info:"
                           << "\n\t nDetPixelsX: " << nDetPixelsX
                           << "\n\t nDetPixelsY: " << nDetPixelsY
                           << "\n\t databytes: " << multisize
                           << "\n\t dynamicRange: " << dynamicRange;

        // send data to callback
        if (data) {
            setCurrentProgress(currentFrameIndex + 1);
            // 4bit gap pixels
            if (dynamicRange == 4 && gappixelsenable) {
                if (quadEnable) {
                    nDetPixelsX += 2;
                    nDetPixelsY += 2;
                } else {
                    nDetPixelsX = nX * (nPixelsX + 3);
                    nDetPixelsY = nY * (nPixelsY + 1);
                }
                int n = processImageWithGapPixels(multiframe, multigappixels,
                                                  quadEnable);
                FILE_LOG(logDEBUG) << "Call Back Info Recalculated:"
                                   << "\n\t nDetPixelsX: " << nDetPixelsX
                                   << "\n\t nDetPixelsY: " << nDetPixelsY
                                   << "\n\t databytes: " << n;
                thisData = new detectorData(
                    getCurrentProgress(), currentFileName.c_str(), nDetPixelsX,
                    nDetPixelsY, multigappixels, n, dynamicRange,
                    currentFileIndex);
            }
            // normal pixels
            else {
                thisData = new detectorData(
                    getCurrentProgress(), currentFileName.c_str(), nDetPixelsX,
                    nDetPixelsY, multiframe, multisize, dynamicRange,
                    currentFileIndex);
            }
            dataReady(thisData, currentFrameIndex,
                      ((dynamicRange == 32) ? currentSubFrameIndex : -1),
                      pCallbackArg);
            delete thisData;
        }

        // all done
        if (numRunning == 0) {
            // let main thread know that all dummy packets have been received
            //(also from external process),
            // main thread can now proceed to measurement finished call back
            sem_post(&sem_endRTAcquisition);
            // wait for next scan/measurement, else join thread
            sem_wait(&sem_newRTAcquisition);
            // done with complete acquisition
            if (getJoinThreadFlag()) {
                running = false;
            } else {
                // starting a new scan/measurement (got dummy data)
                for (size_t i = 0; i < zmqSocket.size(); ++i) {
                    runningList[i] = connectList[i];
                }
                numRunning = numConnected;
            }
        }
    }

    // Disconnect resources
    for (size_t i = 0; i < zmqSocket.size(); ++i) {
        if (connectList[i]) {
            zmqSocket[i]->Disconnect();
        }
    }

    // free resources
    if (image != nullptr)
        delete[] image;
    if (multiframe)
        delete[] multiframe;
    if (multigappixels)
        delete[] multigappixels;
}

int multiSlsDetector::processImageWithGapPixels(char *image, char *&gpImage,
                                                bool quadEnable) {
    // eiger 4 bit mode
    int nxb =
        multi_shm()->numberOfDetector.x * (512 + 3); //(divided by 2 already)
    int nyb = multi_shm()->numberOfDetector.y * (256 + 1);
    int nchipInRow = 4;
    int nxchip = multi_shm()->numberOfDetector.x * 4;
    int nychip = multi_shm()->numberOfDetector.y * 1;
    if (quadEnable) {
        nxb = multi_shm()->numberOfDetector.x *
              (256 + 1); //(divided by 2 already)
        nyb = multi_shm()->numberOfDetector.y * (512 + 2);
        nxchip /= 2;
        nychip *= 2;
        nchipInRow /= 2;
    }
    int gapdatabytes = nxb * nyb;

    // allocate
    if (gpImage == nullptr) {
        gpImage = new char[gapdatabytes];
    }
    // fill value
    memset(gpImage, 0xFF, gapdatabytes);

    const int b1chipx = 128;
    const int b1chipy = 256;
    char *src = nullptr;
    char *dst = nullptr;

    // copying line by line
    src = image;
    dst = gpImage;
    for (int row = 0; row < nychip; ++row) { // for each chip row
        for (int ichipy = 0; ichipy < b1chipy;
             ++ichipy) {                             // for each row in a chip
            for (int col = 0; col < nxchip; ++col) { // for each chip in a row
                memcpy(dst, src, b1chipx);
                src += b1chipx;
                dst += b1chipx;
                if (((col + 1) % nchipInRow) != 0) { // skip gap pixels
                    ++dst;
                }
            }
        }

        dst += (2 * nxb);
    }

    // vertical filling of values
    {
        uint8_t temp, g1, g2;
        int mod;
        dst = gpImage;
        for (int row = 0; row < nychip; ++row) { // for each chip row
            for (int ichipy = 0; ichipy < b1chipy;
                 ++ichipy) { // for each row in a chip
                for (int col = 0; col < nxchip;
                     ++col) { // for each chip in a row
                    dst += b1chipx;
                    mod = (col + 1) % nchipInRow; // get gap pixels
                    // copy gap pixel(chip 0, 1, 2)
                    if (mod != 0) {
                        // neighbouring gap pixels to left
                        temp = (*((uint8_t *)(dst - 1)));
                        g1 = ((temp & 0xF) / 2);
                        (*((uint8_t *)(dst - 1))) = (temp & 0xF0) + g1;

                        // neighbouring gap pixels to right
                        temp = (*((uint8_t *)(dst + 1)));
                        g2 = ((temp >> 4) / 2);
                        (*((uint8_t *)(dst + 1))) = (g2 << 4) + (temp & 0x0F);

                        // gap pixels
                        (*((uint8_t *)dst)) = (g1 << 4) + g2;

                        // increment to point to proper chip destination
                        ++dst;
                    }
                }
            }

            dst += (2 * nxb);
        }
    }

    // return gapdatabytes;
    // horizontal filling
    {
        uint8_t temp, g1, g2;
        char *dst_prevline = nullptr;
        dst = gpImage;
        for (int row = 0; row < nychip; ++row) { // for each chip row
            dst += (b1chipy * nxb);
            // horizontal copying of gap pixels from neighboring past line
            // (bottom parts)
            if (row < nychip - 1) {
                dst_prevline = dst - nxb;
                for (int gapline = 0; gapline < nxb; ++gapline) {
                    temp = (*((uint8_t *)dst_prevline));
                    g1 = ((temp >> 4) / 2);
                    g2 = ((temp & 0xF) / 2);
                    (*((uint8_t *)dst_prevline)) = (g1 << 4) + g2;
                    (*((uint8_t *)dst)) = (*((uint8_t *)dst_prevline));
                    ++dst;
                    ++dst_prevline;
                }
            }

            // horizontal copying of gap pixels from neihboring future line (top
            // part)
            if (row > 0) {
                dst -= ((b1chipy + 1) * nxb);
                dst_prevline = dst + nxb;
                for (int gapline = 0; gapline < nxb; ++gapline) {
                    temp = (*((uint8_t *)dst_prevline));
                    g1 = ((temp >> 4) / 2);
                    g2 = ((temp & 0xF) / 2);
                    temp = (g1 << 4) + g2;
                    (*((uint8_t *)dst_prevline)) = temp;
                    (*((uint8_t *)dst)) = temp;
                    ++dst;
                    ++dst_prevline;
                }
                dst += ((b1chipy + 1) * nxb);
            }

            dst += nxb;
        }
    }

    return gapdatabytes;
}

bool multiSlsDetector::enableDataStreamingToClient(int enable) {
    if (enable >= 0) {
        // destroy data threads
        if (enable == 0) {
            createReceivingDataSockets(true);
            // create data threads
        } else {
            if (createReceivingDataSockets() == FAIL) {
                throw RuntimeError("Could not create data threads in client.");
            }
        }
    }
    return client_downstream;
}


void multiSlsDetector::savePattern(const std::string &fname) {
    std::ofstream outfile;
    outfile.open(fname.c_str(), std::ios_base::out);
    if (!outfile.is_open()) {
        throw RuntimeError("Could not create file to save pattern");
    }
    // get pattern limits
    auto r = Parallel(&slsDetector::setPatternLoopAddresses, {}, -1, -1, -1)
                 .tsquash("Inconsistent pattern limits");
    // pattern words
    for (int i = r[0]; i <= r[1]; ++i) {
        std::ostringstream os;
        os << "patword 0x" << std::hex << i;
        std::string cmd = os.str();
        multiSlsDetectorClient(cmd, GET_ACTION, this, outfile);
    }
    // rest of pattern file
    const std::vector<std::string> commands{
        "patioctrl", 
        "patclkctrl",
        "patlimits",
        "patloop0",
        "patnloop0",
        "patloop1",
        "patnloop1",
        "patloop2",
        "patnloop2",
        "patwait0",
        "patwaittime0",
        "patwait1",
        "patwaittime1",
        "patwait2",
        "patwaittime2",
        "patmask",
        "patsetbit",
    };
    for (const auto &cmd : commands)
        multiSlsDetectorClient(cmd, GET_ACTION, this, outfile);
}

void multiSlsDetector::loadParameters(const std::string &fname) {
    std::ifstream input_file;
    input_file.open(fname.c_str(), std::ios_base::in);
    if (!input_file.is_open()) {
        throw RuntimeError("Could not open parameter file " + fname +
                           " for reading");
    }
    std::string current_line;
    while (input_file.good()) {
        getline(input_file, current_line);
        if (current_line.find('#') != std::string::npos) {
            current_line.erase(current_line.find('#'));
        }
        FILE_LOG(logDEBUG1)
            << "current_line after removing comments:\n\t" << current_line;
        if (current_line.length() > 1) {
            multiSlsDetectorClient(current_line, PUT_ACTION, this);
        }    
    }
    input_file.close();
}


void multiSlsDetector::registerAcquisitionFinishedCallback(
    void (*func)(double, int, void *), void *pArg) {
    acquisition_finished = func;
    acqFinished_p = pArg;
}

void multiSlsDetector::registerDataCallback(
    void (*userCallback)(detectorData *, uint64_t, uint32_t, void *),
    void *pArg) {
    dataReady = userCallback;
    pCallbackArg = pArg;
    if (Parallel(&slsDetector::getUseReceiverFlag, {}).squash(false)) {
        if (dataReady == nullptr) {
            enableDataStreamingToClient(0);
            Parallel(&slsDetector::enableDataStreamingFromReceiver, {}, 0);
        } else {
            enableDataStreamingToClient(1);
            Parallel(&slsDetector::enableDataStreamingFromReceiver, {}, 1);
        }
    }
}

double multiSlsDetector::setTotalProgress() {
    int64_t nf = Parallel(&slsDetector::getNumberOfFramesFromShm, {})
                 .tsquash("Inconsistent number of frames");
    int64_t nc = Parallel(&slsDetector::getNumberOfTriggersFromShm, {})
                 .tsquash("Inconsistent number of triggers");
    if (nf == 0 || nc == 0) {
        throw RuntimeError("Number of frames or triggers is 0");
    }

    int ns = 1;
    if (multi_shm()->multiDetectorType == JUNGFRAU) {
        ns = Parallel(&slsDetector::getNumberOfAdditionalStorageCellsFromShm, {})
                 .tsquash("Inconsistent number of additional storage cells");
        ++ns;
    }

    totalProgress = nf * nc * ns;
    FILE_LOG(logDEBUG1) << "nf " << nf << " nc " << nc << " ns " << ns;
    FILE_LOG(logDEBUG1) << "Set total progress " << totalProgress << std::endl;
    return totalProgress;
}

double multiSlsDetector::getCurrentProgress() {
    std::lock_guard<std::mutex> lock(mp);
    return 100. * progressIndex / totalProgress;
}

void multiSlsDetector::incrementProgress() {
    std::lock_guard<std::mutex> lock(mp);
    progressIndex += 1;
    std::cout << std::fixed << std::setprecision(2) << std::setw(6)
              << 100. * progressIndex / totalProgress
              << " \%";
    std::cout << '\r' << std::flush;
}

void multiSlsDetector::setCurrentProgress(int64_t i) {
    std::lock_guard<std::mutex> lock(mp);
    progressIndex = (double)i;
    std::cout << std::fixed << std::setprecision(2) << std::setw(6)
              << 100. * progressIndex / totalProgress
              << " \%";
    std::cout << '\r' << std::flush;
}

int multiSlsDetector::acquire() {
    // ensure acquire isnt started multiple times by same client
    if (!isAcquireReady()) {
        return FAIL;
    }

    struct timespec begin, end;
    clock_gettime(CLOCK_REALTIME, &begin);

    // in the real time acquisition loop, processing thread will wait for a post
    // each time
    sem_init(&sem_newRTAcquisition, 1, 0);
    // in the real time acquistion loop, main thread will wait for processing
    // thread to be done each time (which in turn waits for receiver/ext
    // process)
    sem_init(&sem_endRTAcquisition, 1, 0);

    bool receiver = Parallel(&slsDetector::getUseReceiverFlag, {}).squash(false);
    progressIndex = 0;
    setJoinThreadFlag(false);

    // verify receiver is idle
    if (receiver) {
        if (Parallel(&slsDetector::getReceiverStatus, {}).squash(ERROR) != IDLE) {
            Parallel(&slsDetector::stopReceiver, {});
        }
    }
    setTotalProgress();

    startProcessingThread();

    // start receiver
    if (receiver) {
        Parallel(&slsDetector::startReceiver, {});
        // let processing thread listen to these packets
        sem_post(&sem_newRTAcquisition);
    }

    // start and read all
    if (multi_shm()->multiDetectorType == EIGER) {
        Parallel(&slsDetector::prepareAcquisition, {});
    }
    Parallel(&slsDetector::startAndReadAll, {});

    // stop receiver
    if (receiver) {
        Parallel(&slsDetector::stopReceiver, {});
        if (dataReady != nullptr) {
            sem_wait(&sem_endRTAcquisition); // waits for receiver's
        }
        // external process to be
        // done sending data to gui

        Parallel(&slsDetector::incrementFileIndex, {});
    }

    // waiting for the data processing thread to finish!
    setJoinThreadFlag(true);
    sem_post(&sem_newRTAcquisition);
    dataProcessingThread.join();

    if (acquisition_finished != nullptr) {
        // same status for all, else error
        int status = static_cast<int>(ERROR);
        auto t = Parallel(&slsDetector::getRunStatus, {});
        if (t.equal())
            status = t.front();
        acquisition_finished(getCurrentProgress(), status,
                             acqFinished_p);
    }

    sem_destroy(&sem_newRTAcquisition);
    sem_destroy(&sem_endRTAcquisition);

    clock_gettime(CLOCK_REALTIME, &end);
    FILE_LOG(logDEBUG1) << "Elapsed time for acquisition:"
                        << ((end.tv_sec - begin.tv_sec) +
                            (end.tv_nsec - begin.tv_nsec) / 1000000000.0)
                        << " seconds";

    setAcquiringFlag(false);

    return OK;
}

void multiSlsDetector::startProcessingThread() {
    setTotalProgress();
    dataProcessingThread = std::thread(&multiSlsDetector::processData, this);
}

void multiSlsDetector::processData() {
    if (Parallel(&slsDetector::getUseReceiverFlag, {}).squash(false)) {
        if (dataReady != nullptr) {
            readFrameFromReceiver();
        }
        // only update progress
        else {
            int64_t caught = -1;
            while (true) {
                // to exit acquire by typing q
                if (kbhit() != 0) {
                    if (fgetc(stdin) == 'q') {
                        FILE_LOG(logINFO)
                            << "Caught the command to stop acquisition";
                        Parallel(&slsDetector::stopAcquisition, {});
                    }
                }
                // get progress
                caught = Parallel(&slsDetector::getFramesCaughtByReceiver, {0}).squash();

                // updating progress
                if (caught != -1) {
                    setCurrentProgress(caught);
                }
                // exiting loop
                if (getJoinThreadFlag()) {
                    break;
                }
                // otherwise error when connecting to the receiver too fast
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }
}

bool multiSlsDetector::getJoinThreadFlag() const {
    std::lock_guard<std::mutex> lock(mp);
    return jointhread;
}

void multiSlsDetector::setJoinThreadFlag(bool value) {
    std::lock_guard<std::mutex> lock(mp);
    jointhread = value;
}

int multiSlsDetector::kbhit() {
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); // STDIN_FILENO is 0
    select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

std::vector<char> multiSlsDetector::readPofFile(const std::string &fname) {
    FILE_LOG(logDEBUG1) << "Programming FPGA with file name:" << fname;
    size_t filesize = 0;
    // check if it exists

    struct stat st;
    if (stat(fname.c_str(), &st) != 0) {
        throw RuntimeError("Program FPGA: Programming file does not exist");
    }

    // open src
    FILE *src = fopen(fname.c_str(), "rb");
    if (src == nullptr) {
        throw RuntimeError(
            "Program FPGA: Could not open source file for programming: " +
            fname);
    }

    // create temp destination file
    char destfname[] = "/tmp/SLS_DET_MCB.XXXXXX";
    int dst = mkstemp(destfname); // create temporary file and open it in r/w
    if (dst == -1) {
        fclose(src);
        throw RuntimeError(
            std::string(
                "Could not create destination file in /tmp for programming: ") +
            destfname);
    }

    // convert src to dst rawbin
    FILE_LOG(logDEBUG1) << "Converting " << fname << " to " << destfname;
    {
        int filepos, x, y, i;
        // Remove header (0...11C)
        for (filepos = 0; filepos < 0x11C; ++filepos) {
            fgetc(src);
        }
        // Write 0x80 times 0xFF (0...7F)
        {
            char c = 0xFF;
            for (filepos = 0; filepos < 0x80; ++filepos) {
                write(dst, &c, 1);
            }
        }
        // Swap bits and write to file
        for (filepos = 0x80; filepos < 0x1000000; ++filepos) {
            x = fgetc(src);
            if (x < 0) {
                break;
            }
            y = 0;
            for (i = 0; i < 8; ++i) {
                y = y |
                    (((x & (1 << i)) >> i) << (7 - i)); // This swaps the bits
            }
            write(dst, &y, 1);
        }
        if (filepos < 0x1000000) {
            throw RuntimeError(
                "Could not convert programming file. EOF before end of flash");
        }
    }
    if (fclose(src) != 0) {
        throw RuntimeError("Program FPGA: Could not close source file");
    }
    if (close(dst) != 0) {
        throw RuntimeError("Program FPGA: Could not close destination file");
    }
    FILE_LOG(logDEBUG1) << "File has been converted to " << destfname;

    // loading dst file to memory
    FILE *fp = fopen(destfname, "r");
    if (fp == nullptr) {
        throw RuntimeError("Program FPGA: Could not open rawbin file");
    }
    if (fseek(fp, 0, SEEK_END) != 0) {
        throw RuntimeError("Program FPGA: Seek error in rawbin file");
    }
    filesize = ftell(fp);
    if (filesize <= 0) {
        throw RuntimeError("Program FPGA: Could not get length of rawbin file");
    }
    rewind(fp);

    std::vector<char> buffer(filesize, 0);
    if (fread(buffer.data(), sizeof(char), filesize, fp) != filesize) {
        throw RuntimeError("Program FPGA: Could not read rawbin file");
    }

    if (fclose(fp) != 0) {
        throw RuntimeError(
            "Program FPGA: Could not close destination file after converting");
    }
    unlink(destfname); // delete temporary file
    FILE_LOG(logDEBUG1)
        << "Successfully loaded the rawbin file to program memory";
    FILE_LOG(logINFO) << "Read file into memory";
    return buffer;
}
