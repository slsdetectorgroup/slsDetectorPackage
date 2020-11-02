#include "DetectorImpl.h"
#include "Module.h"
#include "SharedMemory.h"
#include "sls/ZmqSocket.h"
#include "sls/detectorData.h"
#include "sls/file_utils.h"
#include "sls/logger.h"
#include "sls/sls_detector_exceptions.h"
#include "sls/versionAPI.h"

#include "sls/ToString.h"
#include "sls/container_utils.h"
#include "sls/network_utils.h"
#include "sls/string_utils.h"

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

namespace sls {

DetectorImpl::DetectorImpl(int multi_id, bool verify, bool update)
    : multiId(multi_id), multi_shm(multi_id, -1) {
    setupMultiDetector(verify, update);
}

DetectorImpl::~DetectorImpl() = default;

void DetectorImpl::setupMultiDetector(bool verify, bool update) {
    initSharedMemory(verify);
    initializeMembers(verify);
    if (update) {
        updateUserdetails();
    }
}

void DetectorImpl::setAcquiringFlag(bool flag) {
    multi_shm()->acquiringFlag = flag;
}

int DetectorImpl::getMultiId() const { return multiId; }

void DetectorImpl::freeSharedMemory(int multiId, int detPos) {
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

void DetectorImpl::freeSharedMemory() {
    zmqSocket.clear();
    for (auto &d : detectors) {
        d->freeSharedMemory();
    }
    detectors.clear();

    // clear multi detector shm
    multi_shm.RemoveSharedMemory();
    client_downstream = false;
}

std::string DetectorImpl::getUserDetails() {
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

bool DetectorImpl::getInitialChecks() const {
    return multi_shm()->initialChecks;
}

void DetectorImpl::setInitialChecks(const bool value) {
    multi_shm()->initialChecks = value;
}

void DetectorImpl::initSharedMemory(bool verify) {
    if (!multi_shm.IsExisting()) {
        multi_shm.CreateSharedMemory();
        initializeDetectorStructure();
    } else {
        multi_shm.OpenSharedMemory();
        if (verify && multi_shm()->shmversion != MULTI_SHMVERSION) {
            LOG(logERROR) << "Multi shared memory (" << multiId
                          << ") version mismatch "
                             "(expected 0x"
                          << std::hex << MULTI_SHMVERSION << " but got 0x"
                          << multi_shm()->shmversion << std::dec
                          << ". Clear Shared memory to continue.";
            throw SharedMemoryError("Shared memory version mismatch!");
        }
    }
}

void DetectorImpl::initializeDetectorStructure() {
    multi_shm()->shmversion = MULTI_SHMVERSION;
    multi_shm()->numberOfDetectors = 0;
    multi_shm()->multiDetectorType = GENERIC;
    multi_shm()->numberOfDetector.x = 0;
    multi_shm()->numberOfDetector.y = 0;
    multi_shm()->numberOfChannels.x = 0;
    multi_shm()->numberOfChannels.y = 0;
    multi_shm()->acquiringFlag = false;
    multi_shm()->initialChecks = true;
    multi_shm()->gapPixels = false;
    // zmqlib default
    multi_shm()->zmqHwm = -1;
}

void DetectorImpl::initializeMembers(bool verify) {
    // DetectorImpl
    zmqSocket.clear();

    // get objects from single det shared memory (open)
    for (int i = 0; i < multi_shm()->numberOfDetectors; i++) {
        try {
            detectors.push_back(sls::make_unique<Module>(multiId, i, verify));
        } catch (...) {
            detectors.clear();
            throw;
        }
    }
}

void DetectorImpl::updateUserdetails() {
    multi_shm()->lastPID = getpid();
    memset(multi_shm()->lastUser, 0, sizeof(multi_shm()->lastUser));
    memset(multi_shm()->lastDate, 0, sizeof(multi_shm()->lastDate));
    try {
        sls::strcpy_safe(multi_shm()->lastUser, exec("whoami").c_str());
        sls::strcpy_safe(multi_shm()->lastDate, exec("date").c_str());
    } catch (...) {
        sls::strcpy_safe(multi_shm()->lastUser, "errorreading");
        sls::strcpy_safe(multi_shm()->lastDate, "errorreading");
    }
}

bool DetectorImpl::isAcquireReady() {
    if (multi_shm()->acquiringFlag) {
        LOG(logWARNING)
            << "Acquire has already started. "
               "If previous acquisition terminated unexpectedly, "
               "reset busy flag to restart.(sls_detector_put clearbusy)";
        return false;
    }
    multi_shm()->acquiringFlag = true;
    return true;
}

std::string DetectorImpl::exec(const char *cmd) {
    char buffer[128];
    std::string result;
    FILE *pipe = popen(cmd, "r");
    if (pipe == nullptr) {
        throw RuntimeError("Could not open pipe");
    }

    while (feof(pipe) == 0) {
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
    }

    pclose(pipe);
    result.erase(result.find_last_not_of(" \t\n\r") + 1);
    return result;
}

void DetectorImpl::setVirtualDetectorServers(const int numdet, const int port) {
    std::vector<std::string> hostnames;
    for (int i = 0; i < numdet; ++i) {
        // * 2 is for control and stop port
        hostnames.push_back(std::string("localhost:") +
                            std::to_string(port + i * 2));
    }
    setHostname(hostnames);
}

void DetectorImpl::setHostname(const std::vector<std::string> &name) {
    // this check is there only to allow the previous detsizechan command
    if (multi_shm()->numberOfDetectors != 0) {
        LOG(logWARNING) << "There are already detector(s) in shared memory."
                           "Freeing Shared memory now.";
        bool initialChecks = multi_shm()->initialChecks;
        freeSharedMemory();
        setupMultiDetector();
        multi_shm()->initialChecks = initialChecks;
    }
    for (const auto &hostname : name) {
        addSlsDetector(hostname);
    }
    updateDetectorSize();
}

void DetectorImpl::addSlsDetector(const std::string &hostname) {
    LOG(logINFO) << "Adding detector " << hostname;

    int port = DEFAULT_PORTNO;
    std::string host = hostname;
    auto res = sls::split(hostname, ':');
    if (res.size() > 1) {
        host = res[0];
        port = StringTo<int>(res[1]);
    }

    if (host != "localhost") {
        for (auto &d : detectors) {
            if (d->getHostname() == host) {
                LOG(logWARNING)
                    << "Detector " << host
                    << "already part of the multiDetector!" << std::endl
                    << "Remove it before adding it back in a new position!";
                return;
            }
        }
    }

    // get type by connecting
    detectorType type = Module::getTypeFromDetector(host, port);
    auto pos = detectors.size();
    detectors.emplace_back(sls::make_unique<Module>(type, multiId, pos, false));
    multi_shm()->numberOfDetectors = detectors.size();
    detectors[pos]->setControlPort(port);
    detectors[pos]->setStopPort(port + 1);
    detectors[pos]->setHostname(host, multi_shm()->initialChecks);
    // detector type updated by now
    multi_shm()->multiDetectorType =
        Parallel(&Module::getDetectorType, {})
            .tsquash("Inconsistent detector types.");
    // for moench and ctb
    detectors[pos]->updateNumberOfChannels();
}

void DetectorImpl::updateDetectorSize() {
    LOG(logDEBUG) << "Updating Multi-Detector Size: " << size();

    const slsDetectorDefs::xy det_size = detectors[0]->getNumberOfChannels();
    int maxx = multi_shm()->numberOfChannels.x;
    int maxy = multi_shm()->numberOfChannels.y;
    int ndetx = 0, ndety = 0;
    // 1d, add detectors along x axis
    if (det_size.y == 1) {
        if (maxx == 0) {
            maxx = det_size.x * size();
        }
        ndetx = maxx / det_size.x;
        ndety = size() / ndetx;
        if ((maxx % det_size.x) > 0) {
            ++ndety;
        }
    }
    // 2d, add detectors along y axis (due to eiger top/bottom)
    else {
        if (maxy == 0) {
            maxy = det_size.y * size();
        }
        ndety = maxy / det_size.y;
        ndetx = size() / ndety;
        if ((maxy % det_size.y) > 0) {
            ++ndetx;
        }
    }

    multi_shm()->numberOfDetector.x = ndetx;
    multi_shm()->numberOfDetector.y = ndety;
    multi_shm()->numberOfChannels.x = det_size.x * ndetx;
    multi_shm()->numberOfChannels.y = det_size.y * ndety;

    LOG(logDEBUG) << "\n\tNumber of Detectors in X direction:"
                  << multi_shm()->numberOfDetector.x
                  << "\n\tNumber of Detectors in Y direction:"
                  << multi_shm()->numberOfDetector.y
                  << "\n\tNumber of Channels in X direction:"
                  << multi_shm()->numberOfChannels.x
                  << "\n\tNumber of Channels in Y direction:"
                  << multi_shm()->numberOfChannels.y;

    for (auto &d : detectors) {
        d->updateNumberOfDetector(multi_shm()->numberOfDetector);
    }
}

int DetectorImpl::size() const { return detectors.size(); }

slsDetectorDefs::xy DetectorImpl::getNumberOfDetectors() const {
    return multi_shm()->numberOfDetector;
}

slsDetectorDefs::xy DetectorImpl::getNumberOfChannels() const {
    return multi_shm()->numberOfChannels;
}

void DetectorImpl::setNumberOfChannels(const slsDetectorDefs::xy c) {
    if (size() > 1) {
        throw RuntimeError(
            "Set the number of channels before setting hostname.");
    }
    multi_shm()->numberOfChannels = c;
}

bool DetectorImpl::getGapPixelsinCallback() const {
    return multi_shm()->gapPixels;
}

void DetectorImpl::setGapPixelsinCallback(const bool enable) {
    if (enable) {
        switch (multi_shm()->multiDetectorType) {
        case JUNGFRAU:
            break;
        case EIGER:
            if (size() && detectors[0]->getQuad()) {
                break;
            }
            if (multi_shm()->numberOfDetector.y % 2 != 0) {
                throw RuntimeError("Gap pixels can only be used "
                                   "for full modules.");
            }
            break;
        default:
            throw RuntimeError("Gap Pixels is not implemented for " +
                               ToString(multi_shm()->multiDetectorType));
        }
    }
    multi_shm()->gapPixels = enable;
}

int DetectorImpl::destroyReceivingDataSockets() {
    LOG(logINFO) << "Going to destroy data sockets";
    // close socket
    zmqSocket.clear();

    client_downstream = false;
    LOG(logINFO) << "Destroyed Receiving Data Socket(s)";
    return OK;
}

int DetectorImpl::createReceivingDataSockets() {
    if (client_downstream) {
        return OK;
    }
    LOG(logINFO) << "Going to create data sockets";

    size_t numSockets = detectors.size();
    size_t numSocketsPerDetector = 1;
    if (multi_shm()->multiDetectorType == EIGER) {
        numSocketsPerDetector = 2;
    }
    // gotthard2 second interface is only for veto debugging
    else if (multi_shm()->multiDetectorType != GOTTHARD2) {
        if (Parallel(&Module::getNumberofUDPInterfacesFromShm, {}).squash() ==
            2) {
            numSocketsPerDetector = 2;
        }
    }
    numSockets *= numSocketsPerDetector;

    for (size_t iSocket = 0; iSocket < numSockets; ++iSocket) {
        uint32_t portnum = (detectors[iSocket / numSocketsPerDetector]
                                ->getClientStreamingPort());
        portnum += (iSocket % numSocketsPerDetector);
        try {
            zmqSocket.push_back(sls::make_unique<ZmqSocket>(
                detectors[iSocket / numSocketsPerDetector]
                    ->getClientStreamingIP()
                    .str()
                    .c_str(),
                portnum));
            // set high water mark
            int hwm = multi_shm()->zmqHwm;
            if (hwm >= 0) {
                zmqSocket[iSocket]->SetReceiveHighWaterMark(hwm);
                if (zmqSocket[iSocket]->GetReceiveHighWaterMark() != hwm) {
                    throw sls::ZmqSocketError("Could not set zmq rcv hwm to " +
                                              std::to_string(hwm));
                }
            }
            LOG(logINFO) << "Zmq Client[" << iSocket << "] at "
                         << zmqSocket.back()->GetZmqServerAddress() << "[hwm: "
                         << zmqSocket.back()->GetReceiveHighWaterMark() << "]";
        } catch (...) {
            LOG(logERROR) << "Could not create Zmq socket on port " << portnum;
            destroyReceivingDataSockets();
            return FAIL;
        }
    }

    client_downstream = true;
    LOG(logINFO) << "Receiving Data Socket(s) created";
    return OK;
}

void DetectorImpl::readFrameFromReceiver() {

    bool gapPixels = multi_shm()->gapPixels;
    LOG(logDEBUG) << "Gap pixels: " << gapPixels;
    int nX = 0;
    int nY = 0;
    int nDetPixelsX = 0;
    int nDetPixelsY = 0;
    bool quadEnable = false;
    bool eiger = false;
    bool numInterfaces = 1;
    // gotthard2 second interface is veto debugging
    if (multi_shm()->multiDetectorType != GOTTHARD2) {
        numInterfaces = Parallel(&Module::getNumberofUDPInterfacesFromShm, {})
                            .squash(); // cannot pick up from zmq
    }
    std::vector<bool> runningList(zmqSocket.size());
    std::vector<bool> connectList(zmqSocket.size());
    numZmqRunning = 0;
    for (size_t i = 0; i < zmqSocket.size(); ++i) {
        if (zmqSocket[i]->Connect() == 0) {
            connectList[i] = true;
            runningList[i] = true;
            ++numZmqRunning;
        } else {
            // to remember the list it connected to, to disconnect later
            connectList[i] = false;
            LOG(logERROR) << "Could not connect to socket  "
                          << zmqSocket[i]->GetZmqServerAddress();
            runningList[i] = false;
        }
    }
    bool data = false;
    bool completeImage = false;
    std::unique_ptr<char[]> image{nullptr};
    std::unique_ptr<char[]> multiframe{nullptr};
    char *multigappixels = nullptr;
    int multisize = 0;
    // only first message header
    uint32_t size = 0, nPixelsX = 0, nPixelsY = 0, dynamicRange = 0;
    float bytesPerPixel = 0;
    // header info every header
    std::string currentFileName;
    uint64_t currentAcquisitionIndex = -1, currentFrameIndex = -1,
             currentFileIndex = -1;
    double currentProgress = 0.00;
    uint32_t currentSubFrameIndex = -1, coordX = -1, coordY = -1,
             flippedDataX = -1;

    while (numZmqRunning != 0) {
        // reset data
        data = false;
        if (multiframe != nullptr) {
            memset(multiframe.get(), 0xFF, multisize);
        }

        completeImage = (numZmqRunning == (int)zmqSocket.size());

        // get each frame
        for (unsigned int isocket = 0; isocket < zmqSocket.size(); ++isocket) {

            // if running
            if (runningList[isocket]) {

                // HEADER
                {
                    zmqHeader zHeader;
                    if (zmqSocket[isocket]->ReceiveHeader(
                            isocket, zHeader,
                            SLS_DETECTOR_JSON_HEADER_VERSION) == 0) {
                        // parse error, version error or end of acquisition for
                        // socket
                        runningList[isocket] = false;
                        completeImage = false;
                        --numZmqRunning;
                        continue;
                    }

                    // if first message, allocate (all one time stuff)
                    if (image == nullptr) {
                        // allocate
                        size = zHeader.imageSize;
                        multisize = size * zmqSocket.size();
                        image = sls::make_unique<char[]>(size);
                        multiframe = sls::make_unique<char[]>(multisize);
                        memset(multiframe.get(), 0xFF, multisize);
                        // dynamic range
                        dynamicRange = zHeader.dynamicRange;
                        bytesPerPixel = (float)dynamicRange / 8;
                        // shape
                        nPixelsX = zHeader.npixelsx;
                        nPixelsY = zHeader.npixelsy;
                        // detector shape
                        nX = zHeader.ndetx;
                        nY = zHeader.ndety;
                        nY *= numInterfaces;
                        nDetPixelsX = nX * nPixelsX;
                        nDetPixelsY = nY * nPixelsY;
                        // det type
                        eiger = (zHeader.detType == EIGER)
                                    ? true
                                    : false; // to be changed to EIGER when
                                             // firmware updates its header data
                        quadEnable = (zHeader.quad == 0) ? false : true;
                        LOG(logDEBUG1)
                            << "One Time Header Info:"
                               "\n\tsize: "
                            << size << "\n\tmultisize: " << multisize
                            << "\n\tdynamicRange: " << dynamicRange
                            << "\n\tbytesPerPixel: " << bytesPerPixel
                            << "\n\tnPixelsX: " << nPixelsX
                            << "\n\tnPixelsY: " << nPixelsY << "\n\tnX: " << nX
                            << "\n\tnY: " << nY << "\n\teiger: " << eiger
                            << "\n\tquadEnable: " << quadEnable;
                    }
                    // each time, parse rest of header
                    currentFileName = zHeader.fname;
                    currentAcquisitionIndex = zHeader.acqIndex;
                    currentFrameIndex = zHeader.frameIndex;
                    currentProgress = zHeader.progress;
                    currentFileIndex = zHeader.fileIndex;
                    currentSubFrameIndex = zHeader.expLength;
                    coordY = zHeader.row;
                    coordX = zHeader.column;
                    if (eiger) {
                        coordY = (nY - 1) - coordY;
                    }
                    flippedDataX = zHeader.flippedDataX;
                    if (zHeader.completeImage == 0) {
                        completeImage = false;
                    }
                    LOG(logDEBUG1)
                        << "Header Info:"
                           "\n\tcurrentFileName: "
                        << currentFileName << "\n\tcurrentAcquisitionIndex: "
                        << currentAcquisitionIndex
                        << "\n\tcurrentFrameIndex: " << currentFrameIndex
                        << "\n\tcurrentFileIndex: " << currentFileIndex
                        << "\n\tcurrentSubFrameIndex: " << currentSubFrameIndex
                        << "\n\tcurrentProgress: " << currentProgress
                        << "\n\tcoordX: " << coordX << "\n\tcoordY: " << coordY
                        << "\n\tflippedDataX: " << flippedDataX
                        << "\n\tcompleteImage: " << completeImage;
                }

                // DATA
                data = true;
                zmqSocket[isocket]->ReceiveData(isocket, image.get(), size);

                // creating multi image
                {
                    uint32_t xoffset = coordX * nPixelsX * bytesPerPixel;
                    uint32_t yoffset = coordY * nPixelsY;
                    uint32_t singledetrowoffset = nPixelsX * bytesPerPixel;
                    uint32_t rowoffset = nX * singledetrowoffset;
                    if (multi_shm()->multiDetectorType == CHIPTESTBOARD) {
                        singledetrowoffset = size;
                    }
                    LOG(logDEBUG1)
                        << "Multi Image Info:"
                           "\n\txoffset: "
                        << xoffset << "\n\tyoffset: " << yoffset
                        << "\n\tsingledetrowoffset: " << singledetrowoffset
                        << "\n\trowoffset: " << rowoffset;

                    if (eiger && (flippedDataX != 0U)) {
                        for (uint32_t i = 0; i < nPixelsY; ++i) {
                            memcpy((multiframe.get()) +
                                       ((yoffset + (nPixelsY - 1 - i)) *
                                        rowoffset) +
                                       xoffset,
                                   image.get() + (i * singledetrowoffset),
                                   singledetrowoffset);
                        }
                    } else {
                        for (uint32_t i = 0; i < nPixelsY; ++i) {
                            memcpy((multiframe.get()) +
                                       ((yoffset + i) * rowoffset) + xoffset,
                                   image.get() + (i * singledetrowoffset),
                                   singledetrowoffset);
                        }
                    }
                }
            }
        }

        LOG(logDEBUG) << "Call Back Info:"
                      << "\n\t nDetPixelsX: " << nDetPixelsX
                      << "\n\t nDetPixelsY: " << nDetPixelsY
                      << "\n\t databytes: " << multisize
                      << "\n\t dynamicRange: " << dynamicRange;

        // send data to callback
        if (data) {
            char *callbackImage = multiframe.get();
            int imagesize = multisize;
            int nDetActualPixelsX = nDetPixelsX;
            int nDetActualPixelsY = nDetPixelsY;

            if (gapPixels) {
                int n = InsertGapPixels(multiframe.get(), multigappixels,
                                        quadEnable, dynamicRange,
                                        nDetActualPixelsX, nDetActualPixelsY);
                callbackImage = multigappixels;
                imagesize = n;
            }
            LOG(logDEBUG) << "Image Info:"
                          << "\n\tnDetActualPixelsX: " << nDetActualPixelsX
                          << "\n\tnDetActualPixelsY: " << nDetActualPixelsY
                          << "\n\timagesize: " << imagesize
                          << "\n\tdynamicRange: " << dynamicRange;

            thisData = new detectorData(currentProgress, currentFileName,
                                        nDetActualPixelsX, nDetActualPixelsY,
                                        callbackImage, imagesize, dynamicRange,
                                        currentFileIndex, completeImage);

            dataReady(
                thisData, currentFrameIndex,
                ((dynamicRange == 32 && eiger) ? currentSubFrameIndex : -1),
                pCallbackArg);
            delete thisData;
        }
    }

    // Disconnect resources
    for (size_t i = 0; i < zmqSocket.size(); ++i) {
        if (connectList[i]) {
            zmqSocket[i]->Disconnect();
        }
    }

    // free resources
    delete[] multigappixels;
}

int DetectorImpl::InsertGapPixels(char *image, char *&gpImage, bool quadEnable,
                                  int dr, int &nPixelsx, int &nPixelsy) {

    LOG(logDEBUG) << "Insert Gap pixels:"
                  << "\n\t nPixelsx: " << nPixelsx
                  << "\n\t nPixelsy: " << nPixelsy
                  << "\n\t quadEnable: " << quadEnable << "\n\t dr: " << dr;

    // inter module gap pixels
    int modGapPixelsx = 8;
    int modGapPixelsy = 36;
    // inter chip gap pixels
    int chipGapPixelsx = 2;
    int chipGapPixelsy = 2;
    // number of pixels in a chip
    int nChipPixelsx = 256;
    int nChipPixelsy = 256;
    // 1 module
    // number of chips in a module
    int nMod1Chipx = 4;
    int nMod1Chipy = 2;
    if (quadEnable) {
        nMod1Chipx = 2;
    }
    // number of pixels in a module
    int nMod1Pixelsx = nChipPixelsx * nMod1Chipx;
    int nMod1Pixelsy = nChipPixelsy * nMod1Chipy;
    // number of gap pixels in a module
    int nMod1GapPixelsx = (nMod1Chipx - 1) * chipGapPixelsx;
    int nMod1GapPixelsy = (nMod1Chipy - 1) * chipGapPixelsy;
    // total number of modules
    int nModx = nPixelsx / nMod1Pixelsx;
    int nMody = nPixelsy / nMod1Pixelsy;

    // check if not full modules
    // (setting gap pixels and then adding half module or disabling quad)
    if (nPixelsy / nMod1Pixelsy == 0) {
        LOG(logERROR) << "Gap pixels can only be enabled with full modules. "
                         "Sending dummy data without gap pixels.\n";
        double bytesPerPixel = (double)dr / 8.00;
        int imagesize = nPixelsy * nPixelsx * bytesPerPixel;
        if (gpImage == nullptr) {
            gpImage = new char[imagesize];
        }
        memset(gpImage, 0xFF, imagesize);
        return imagesize;
    }

    // total number of pixels
    int nTotx =
        nPixelsx + (nMod1GapPixelsx * nModx) + (modGapPixelsx * (nModx - 1));
    int nToty =
        nPixelsy + (nMod1GapPixelsy * nMody) + (modGapPixelsy * (nMody - 1));
    // total number of chips
    int nChipx = nPixelsx / nChipPixelsx;
    int nChipy = nPixelsy / nChipPixelsy;

    double bytesPerPixel = (double)dr / 8.00;
    int imagesize = nTotx * nToty * bytesPerPixel;

    int nChipBytesx = nChipPixelsx * bytesPerPixel;         // 1 chip bytes in x
    int nChipGapBytesx = chipGapPixelsx * bytesPerPixel;    // 2 pixel bytes
    int nModGapBytesx = modGapPixelsx * bytesPerPixel;      // 8 pixel bytes
    int nChipBytesy = nChipPixelsy * nTotx * bytesPerPixel; // 1 chip bytes in y
    int nChipGapBytesy = chipGapPixelsy * nTotx * bytesPerPixel; // 2 lines
    int nModGapBytesy = modGapPixelsy * nTotx *
                        bytesPerPixel; // 36 lines
                                       // 4 bit mode, its 1 byte (because for 4
                                       // bit mode, we handle 1 byte at a time)
    int pixel1 = (int)(ceil(bytesPerPixel));
    int row1Bytes = nTotx * bytesPerPixel;
    int nMod1TotPixelsx = nMod1Pixelsx + nMod1GapPixelsx;
    if (dr == 4) {
        nMod1TotPixelsx /= 2;
    }
    // eiger requires inter chip gap pixels are halved
    // jungfrau prefers same inter chip gap pixels as the boundary pixels
    int divisionValue = 2;
    slsDetectorDefs::detectorType detType = multi_shm()->multiDetectorType;
    if (detType == JUNGFRAU) {
        divisionValue = 1;
    }
    LOG(logDEBUG) << "Insert Gap pixels Calculations:\n\t"
                  << "nPixelsx: " << nPixelsx << "\n\t"
                  << "nPixelsy: " << nPixelsy << "\n\t"
                  << "nMod1Pixelsx: " << nMod1Pixelsx << "\n\t"
                  << "nMod1Pixelsy: " << nMod1Pixelsy << "\n\t"
                  << "nMod1GapPixelsx: " << nMod1GapPixelsx << "\n\t"
                  << "nMod1GapPixelsy: " << nMod1GapPixelsy << "\n\t"
                  << "nChipy: " << nChipy << "\n\t"
                  << "nChipx: " << nChipx << "\n\t"
                  << "nModx: " << nModx << "\n\t"
                  << "nMody: " << nMody << "\n\t"
                  << "nTotx: " << nTotx << "\n\t"
                  << "nToty: " << nToty << "\n\t"
                  << "bytesPerPixel: " << bytesPerPixel << "\n\t"
                  << "imagesize: " << imagesize << "\n\t"
                  << "nChipBytesx: " << nChipBytesx << "\n\t"
                  << "nChipGapBytesx: " << nChipGapBytesx << "\n\t"
                  << "nModGapBytesx: " << nModGapBytesx << "\n\t"
                  << "nChipBytesy: " << nChipBytesy << "\n\t"
                  << "nChipGapBytesy: " << nChipGapBytesy << "\n\t"
                  << "nModGapBytesy: " << nModGapBytesy << "\n\t"
                  << "pixel1: " << pixel1 << "\n\t"
                  << "row1Bytes: " << row1Bytes << "\n\t"
                  << "nMod1TotPixelsx: " << nMod1TotPixelsx << "\n\t"
                  << "divisionValue: " << divisionValue << "\n\n";

    if (gpImage == nullptr) {
        gpImage = new char[imagesize];
    }
    memset(gpImage, 0xFF, imagesize);
    // memcpy(gpImage, image, imagesize);
    char *src = nullptr;
    char *dst = nullptr;

    // copying line by line
    src = image;
    dst = gpImage;
    // for each chip row in y
    for (int iChipy = 0; iChipy < nChipy; ++iChipy) {
        // for each row
        for (int iy = 0; iy < nChipPixelsy; ++iy) {
            // in each row, for every chip
            for (int iChipx = 0; iChipx < nChipx; ++iChipx) {
                // copy 1 chip line
                memcpy(dst, src, nChipBytesx);
                src += nChipBytesx;
                dst += nChipBytesx;
                // skip inter chip gap pixels in x
                if (((iChipx + 1) % nMod1Chipx) != 0) {
                    dst += nChipGapBytesx;
                }
                // skip inter module gap pixels in x
                else if (iChipx + 1 != nChipx) {
                    dst += nModGapBytesx;
                }
            }
        }
        // skip inter chip gap pixels in y
        if (((iChipy + 1) % nMod1Chipy) != 0) {
            dst += nChipGapBytesy;
        }
        // skip inter module gap pixels in y
        else if (iChipy + 1 != nChipy) {
            dst += nModGapBytesy;
        }
    }

    // iner chip gap pixel values is half of neighboring one
    // (corners becomes divide by 4 automatically after horizontal filling)

    // vertical filling of inter chip gap pixels
    dst = gpImage;
    // for each chip row in y
    for (int iChipy = 0; iChipy < nChipy; ++iChipy) {
        // for each row
        for (int iy = 0; iy < nChipPixelsy; ++iy) {
            // in each row, for every chip
            for (int iChipx = 0; iChipx < nChipx; ++iChipx) {
                // go to gap pixels
                dst += nChipBytesx;
                // fix inter chip gap pixels in x
                if (((iChipx + 1) % nMod1Chipx) != 0) {
                    uint8_t temp8 = 0;
                    uint16_t temp16 = 0;
                    uint32_t temp32 = 0;
                    uint8_t g1 = 0;
                    uint8_t g2 = 0;
                    switch (dr) {
                    case 4:
                        // neighbouring gap pixels to left
                        temp8 = (*((uint8_t *)(dst - 1)));
                        g1 = ((temp8 & 0xF) / 2);
                        (*((uint8_t *)(dst - 1))) = (temp8 & 0xF0) + g1;
                        // neighbouring gap pixels to right
                        temp8 = (*((uint8_t *)(dst + 1)));
                        g2 = ((temp8 >> 4) / 2);
                        (*((uint8_t *)(dst + 1))) = (g2 << 4) + (temp8 & 0x0F);
                        // gap pixels
                        (*((uint8_t *)dst)) = (g1 << 4) + g2;
                        break;
                    case 8:
                        // neighbouring gap pixels to left
                        temp8 = (*((uint8_t *)(dst - pixel1))) / 2;
                        (*((uint8_t *)dst)) = temp8;
                        (*((uint8_t *)(dst - pixel1))) = temp8;
                        // neighbouring gap pixels to right
                        temp8 = (*((uint8_t *)(dst + 2 * pixel1))) / 2;
                        (*((uint8_t *)(dst + pixel1))) = temp8;
                        (*((uint8_t *)(dst + 2 * pixel1))) = temp8;
                        break;
                    case 16:
                        // neighbouring gap pixels to left
                        temp16 =
                            (*((uint16_t *)(dst - pixel1))) / divisionValue;
                        (*((uint16_t *)dst)) = temp16;
                        (*((uint16_t *)(dst - pixel1))) = temp16;
                        // neighbouring gap pixels to right
                        temp16 =
                            (*((uint16_t *)(dst + 2 * pixel1))) / divisionValue;
                        (*((uint16_t *)(dst + pixel1))) = temp16;
                        (*((uint16_t *)(dst + 2 * pixel1))) = temp16;
                        break;
                    default:
                        // neighbouring gap pixels to left
                        temp32 = (*((uint32_t *)(dst - pixel1))) / 2;
                        (*((uint32_t *)dst)) = temp32;
                        (*((uint32_t *)(dst - pixel1))) = temp32;
                        // neighbouring gap pixels to right
                        temp32 = (*((uint32_t *)(dst + 2 * pixel1))) / 2;
                        (*((uint32_t *)(dst + pixel1))) = temp32;
                        (*((uint32_t *)(dst + 2 * pixel1))) = temp32;
                        break;
                    }
                    dst += nChipGapBytesx;
                }
                // skip inter module gap pixels in x
                else if (iChipx + 1 != nChipx) {
                    dst += nModGapBytesx;
                }
            }
        }
        // skip inter chip gap pixels in y
        if (((iChipy + 1) % nMod1Chipy) != 0) {
            dst += nChipGapBytesy;
        }
        // skip inter module gap pixels in y
        else if (iChipy + 1 != nChipy) {
            dst += nModGapBytesy;
        }
    }

    // horizontal filling of inter chip gap pixels
    // starting at bottom part (1 line below to copy from)
    src = gpImage + (nChipBytesy - row1Bytes);
    dst = gpImage + nChipBytesy;
    // for each chip row in y
    for (int iChipy = 0; iChipy < nChipy; ++iChipy) {
        // for each module in x
        for (int iModx = 0; iModx < nModx; ++iModx) {
            // in each module, for every pixel in x
            for (int iPixel = 0; iPixel < nMod1TotPixelsx; ++iPixel) {
                uint8_t temp8 = 0, g1 = 0, g2 = 0;
                uint16_t temp16 = 0;
                uint32_t temp32 = 0;
                switch (dr) {
                case 4:
                    temp8 = (*((uint8_t *)src));
                    g1 = ((temp8 >> 4) / 2);
                    g2 = ((temp8 & 0xF) / 2);
                    temp8 = (g1 << 4) + g2;
                    (*((uint8_t *)dst)) = temp8;
                    (*((uint8_t *)src)) = temp8;
                    break;
                case 8:
                    temp8 = (*((uint8_t *)src)) / divisionValue;
                    (*((uint8_t *)dst)) = temp8;
                    (*((uint8_t *)src)) = temp8;
                    break;
                case 16:
                    temp16 = (*((uint16_t *)src)) / divisionValue;
                    (*((uint16_t *)dst)) = temp16;
                    (*((uint16_t *)src)) = temp16;
                    break;
                default:
                    temp32 = (*((uint32_t *)src)) / 2;
                    (*((uint32_t *)dst)) = temp32;
                    (*((uint32_t *)src)) = temp32;
                    break;
                }
                // every pixel (but 4 bit mode, every byte)
                src += pixel1;
                dst += pixel1;
            }
            // skip inter module gap pixels in x
            if (iModx + 1 < nModx) {
                src += nModGapBytesx;
                dst += nModGapBytesx;
            }
        }
        // bottom parts, skip inter chip gap pixels
        if ((iChipy % nMod1Chipy) == 0) {
            src += nChipGapBytesy;
        }
        // top parts, skip inter module gap pixels and two chips
        else {
            src += (nModGapBytesy + 2 * nChipBytesy - 2 * row1Bytes);
            dst += (nModGapBytesy + 2 * nChipBytesy);
        }
    }

    nPixelsx = nTotx;
    nPixelsy = nToty;
    return imagesize;
}

bool DetectorImpl::getDataStreamingToClient() { return client_downstream; }

void DetectorImpl::setDataStreamingToClient(bool enable) {
    // destroy data threads
    if (!enable) {
        destroyReceivingDataSockets();
        // create data threads
    } else {
        if (createReceivingDataSockets() == FAIL) {
            throw RuntimeError("Could not create data threads in client.");
        }
    }
}

int DetectorImpl::getClientStreamingHwm() const {
    // disabled
    if (!client_downstream) {
        return multi_shm()->zmqHwm;
    }
    // enabled
    sls::Result<int> result;
    result.reserve(zmqSocket.size());
    for (auto &it : zmqSocket) {
        result.push_back(it->GetReceiveHighWaterMark());
    }
    int res = result.tsquash("Inconsistent zmq receive hwm values");
    return res;
}

void DetectorImpl::setClientStreamingHwm(const int limit) {
    if (limit < -1) {
        throw sls::RuntimeError(
            "Cannot set hwm to less than -1 (-1 is lib default).");
    }
    // update shm
    multi_shm()->zmqHwm = limit;

    // streaming enabled
    if (client_downstream) {
        // custom limit, set it directly
        if (limit >= 0) {
            for (auto &it : zmqSocket) {
                it->SetReceiveHighWaterMark(limit);
                if (it->GetReceiveHighWaterMark() != limit) {
                    multi_shm()->zmqHwm = -1;
                    throw sls::ZmqSocketError("Could not set zmq rcv hwm to " +
                                              std::to_string(limit));
                }
            }
            LOG(logINFO) << "Setting Client Zmq socket rcv hwm to " << limit;
        }
        // default, disable and enable to get default
        else {
            setDataStreamingToClient(false);
            setDataStreamingToClient(true);
        }
    }
}

void DetectorImpl::registerAcquisitionFinishedCallback(void (*func)(double, int,
                                                                    void *),
                                                       void *pArg) {
    acquisition_finished = func;
    acqFinished_p = pArg;
}

void DetectorImpl::registerDataCallback(void (*userCallback)(detectorData *,
                                                             uint64_t, uint32_t,
                                                             void *),
                                        void *pArg) {
    dataReady = userCallback;
    pCallbackArg = pArg;
    setDataStreamingToClient(dataReady == nullptr ? false : true);
}

int DetectorImpl::acquire() {
    // ensure acquire isnt started multiple times by same client
    if (!isAcquireReady()) {
        return FAIL;
    }

    try {
        struct timespec begin, end;
        clock_gettime(CLOCK_REALTIME, &begin);

        bool receiver = Parallel(&Module::getUseReceiverFlag, {}).squash(false);

        if (dataReady == nullptr) {
            setJoinThreadFlag(false);
        }

        // verify receiver is idle
        if (receiver) {
            if (Parallel(&Module::getReceiverStatus, {}).squash(ERROR) !=
                IDLE) {
                Parallel(&Module::stopReceiver, {});
            }
        }

        startProcessingThread(receiver);

        // start receiver
        if (receiver) {
            Parallel(&Module::startReceiver, {});
        }

        // start and read all
        try {
            Parallel(&Module::startAndReadAll, {});
        } catch (...) {
            if (receiver)
                Parallel(&Module::stopReceiver, {});
            throw;
        }

        // stop receiver
        if (receiver) {
            Parallel(&Module::stopReceiver, {});
            Parallel(&Module::incrementFileIndex, {});
        }

        // let the progress thread (no callback) know acquisition is done
        if (dataReady == nullptr) {
            setJoinThreadFlag(true);
        }
        if (receiver) {
            while (numZmqRunning != 0) {
                Parallel(&Module::restreamStopFromReceiver, {});
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        }
        dataProcessingThread.join();

        if (acquisition_finished != nullptr) {
            int status = Parallel(&Module::getRunStatus, {}).squash(ERROR);
            auto a = Parallel(&Module::getReceiverProgress, {});
            double progress = (*std::min_element(a.begin(), a.end()));
            acquisition_finished(progress, status, acqFinished_p);
        }

        clock_gettime(CLOCK_REALTIME, &end);
        LOG(logDEBUG1) << "Elapsed time for acquisition:"
                       << ((end.tv_sec - begin.tv_sec) +
                           (end.tv_nsec - begin.tv_nsec) / 1000000000.0)
                       << " seconds";
    } catch (...) {
        setAcquiringFlag(false);
        throw;
    }
    setAcquiringFlag(false);
    return OK;
}

void DetectorImpl::printProgress(double progress) {
    // spaces for python printout
    std::cout << "    " << std::fixed << std::setprecision(2) << std::setw(6)
              << progress << " \%";
    std::cout << '\r' << std::flush;
}

void DetectorImpl::startProcessingThread(bool receiver) {
    dataProcessingThread =
        std::thread(&DetectorImpl::processData, this, receiver);
}

void DetectorImpl::processData(bool receiver) {
    if (receiver) {
        if (dataReady != nullptr) {
            readFrameFromReceiver();
        }
        // only update progress
        else {
            double progress = 0;
            printProgress(progress);

            while (true) {
                // to exit acquire by typing q
                if (kbhit() != 0) {
                    if (fgetc(stdin) == 'q') {
                        LOG(logINFO)
                            << "Caught the command to stop acquisition";
                        Parallel(&Module::stopAcquisition, {});
                    }
                }
                // get and print progress
                double temp =
                    (double)Parallel(&Module::getReceiverProgress, {0})
                        .squash();
                if (temp != progress) {
                    printProgress(progress);
                    progress = temp;
                }

                // exiting loop
                if (getJoinThreadFlag()) {
                    // print progress one final time before exiting
                    progress =
                        (double)Parallel(&Module::getReceiverProgress, {0})
                            .squash();
                    printProgress(progress);
                    break;
                }
                // otherwise error when connecting to the receiver too fast
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }
}

bool DetectorImpl::getJoinThreadFlag() const {
    std::lock_guard<std::mutex> lock(mp);
    return jointhread;
}

void DetectorImpl::setJoinThreadFlag(bool value) {
    std::lock_guard<std::mutex> lock(mp);
    jointhread = value;
}

int DetectorImpl::kbhit() {
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); // STDIN_FILENO is 0
    select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

std::vector<char> DetectorImpl::readProgrammingFile(const std::string &fname) {
    // validate type of file
    bool isPof = false;
    switch (multi_shm()->multiDetectorType) {
    case JUNGFRAU:
    case CHIPTESTBOARD:
    case MOENCH:
        if (fname.find(".pof") == std::string::npos) {
            throw RuntimeError("Programming file must be a pof file.");
        }
        isPof = true;
        break;
    case MYTHEN3:
    case GOTTHARD2:
        if (fname.find(".rbf") == std::string::npos) {
            throw RuntimeError("Programming file must be an rbf file.");
        }
        break;
    default:
        throw RuntimeError("programfpga not implemented for this detector");
    }

    LOG(logINFO)
        << "Updating Firmware. This can take awhile. Please be patient...";
    LOG(logDEBUG1) << "Programming FPGA with file name:" << fname;

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
    LOG(logDEBUG1) << "Converting " << fname << " to " << destfname;
    {
        constexpr int pofNumHeaderBytes = 0x11C;
        constexpr int pofFooterOfst = 0x1000000;
        int dstFilePos = 0;
        if (isPof) {
            // Read header and discard
            for (int i = 0; i < pofNumHeaderBytes; ++i) {
                fgetc(src);
            }
            // Write 0xFF to destination 0x80 times (padding)
            constexpr int pofNumPadding{0x80};
            constexpr uint8_t c{0xFF};
            while (dstFilePos < pofNumPadding) {
                write(dst, &c, sizeof(c));
                ++dstFilePos;
            }
        }
        // Swap bits from source and write to dest
        while (!feof(src)) {
            // pof: exit early to discard footer
            if (isPof && dstFilePos >= pofFooterOfst) {
                break;
            }
            // read source
            int s = fgetc(src);
            if (s < 0) {
                break;
            }
            // swap bits
            int d = 0;
            for (int i = 0; i < 8; ++i) {
                d = d | (((s & (1 << i)) >> i) << (7 - i));
            }
            write(dst, &d, 1);
            ++dstFilePos;
        }
        // validate pof: read less than footer offset
        if (isPof && dstFilePos < pofFooterOfst) {
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
    LOG(logDEBUG1) << "File has been converted to " << destfname;

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
    LOG(logDEBUG1) << "Successfully loaded the rawbin file to program memory";
    LOG(logINFO) << "Read file into memory";
    return buffer;
}

} // namespace sls