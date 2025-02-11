// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
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
#include "sls/file_utils.h"
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

DetectorImpl::DetectorImpl(int detector_index, bool verify, bool update)
    : detectorIndex(detector_index), shm(detector_index, -1),
      ctb_shm(detector_index, -1, CtbConfig::shm_tag()) {
    setupDetector(verify, update);
}

void DetectorImpl::setupDetector(bool verify, bool update) {
    initSharedMemory(verify);
    initializeMembers(verify);
    if (update) {
        updateUserdetails();
    }

    if (ctb_shm.exists())
        ctb_shm.openSharedMemory(verify);
}

bool DetectorImpl::isAllPositions(Positions pos) const {
    return (pos.empty() || (pos.size() == 1 && pos[0] == -1) ||
            (pos.size() == modules.size()));
}

void DetectorImpl::setAcquiringFlag(bool flag) { shm()->acquiringFlag = flag; }

int DetectorImpl::getDetectorIndex() const { return detectorIndex; }

std::string DetectorImpl::getUserDetails() {
    if (modules.empty()) {
        return std::string("none");
    }

    std::ostringstream sstream;
    sstream << "\nHostname: ";
    for (auto &module : modules) {
        sstream << (module->isFixedPatternSharedMemoryCompatible()
                        ? module->getHostname()
                        : "Unknown")
                << "+";
    }
    sstream << "\nType: ";
    // get type from detector version shm
    if (shm()->shmversion >= DETECTOR_SHMAPIVERSION) {
        sstream << ToString(shm()->detType);
    }
    // get type from module shm
    else {
        for (auto &module : modules) {
            sstream << (module->isFixedPatternSharedMemoryCompatible()
                            ? ToString(module->getDetectorType())
                            : "Unknown")
                    << "+";
        }
    }

    sstream << "\nPID: " << shm()->lastPID << "\nUser: " << shm()->lastUser
            << "\nDate: " << shm()->lastDate << std::endl;

    return sstream.str();
}

bool DetectorImpl::getInitialChecks() const { return shm()->initialChecks; }

void DetectorImpl::setInitialChecks(const bool value) {
    shm()->initialChecks = value;
}

void DetectorImpl::initSharedMemory(bool verify) {
    if (!shm.exists()) {
        shm.createSharedMemory();
        initializeDetectorStructure();
    } else {
        shm.openSharedMemory(verify);
        if (verify && shm()->shmversion != DETECTOR_SHMVERSION) {
            LOG(logERROR) << "Detector shared memory (" << detectorIndex
                          << ") version mismatch "
                             "(expected 0x"
                          << std::hex << DETECTOR_SHMVERSION << " but got 0x"
                          << shm()->shmversion << std::dec
                          << ". Clear Shared memory to continue.";
            throw SharedMemoryError("Shared memory version mismatch!");
        }
    }

    // std::cout <<
}

void DetectorImpl::initializeDetectorStructure() {
    shm()->shmversion = DETECTOR_SHMVERSION;
    shm()->totalNumberOfModules = 0;
    shm()->detType = GENERIC;
    shm()->numberOfModules.x = 0;
    shm()->numberOfModules.y = 0;
    shm()->numberOfChannels.x = 0;
    shm()->numberOfChannels.y = 0;
    shm()->acquiringFlag = false;
    shm()->initialChecks = true;
    shm()->gapPixels = false;
    // zmqlib default
    shm()->zmqHwm = -1;
    shm()->rx_roi.xmin = -1;
    shm()->rx_roi.xmax = -1;
    shm()->rx_roi.ymin = -1;
    shm()->rx_roi.ymax = -1;
}

void DetectorImpl::initializeMembers(bool verify) {
    // DetectorImpl
    zmqSocket.clear();

    // get objects from single det shared memory (open)
    for (int i = 0; i < shm()->totalNumberOfModules; i++) {
        try {
            modules.push_back(make_unique<Module>(detectorIndex, i, verify));
        } catch (...) {
            modules.clear();
            throw;
        }
    }
}

void DetectorImpl::updateUserdetails() {
    shm()->lastPID = getpid();
    memset(shm()->lastUser, 0, sizeof(shm()->lastUser));
    memset(shm()->lastDate, 0, sizeof(shm()->lastDate));
    try {
        strcpy_safe(shm()->lastUser, exec("whoami").c_str());
        strcpy_safe(shm()->lastDate, exec("date").c_str());
    } catch (...) {
        strcpy_safe(shm()->lastUser, "errorreading");
        strcpy_safe(shm()->lastDate, "errorreading");
    }
}

bool DetectorImpl::isAcquireReady() {
    if (shm()->acquiringFlag) {
        LOG(logWARNING)
            << "Acquire has already started. "
               "If previous acquisition terminated unexpectedly, "
               "reset busy flag to restart.(sls_detector_put clearbusy)";
        return false;
    }
    shm()->acquiringFlag = true;
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

bool DetectorImpl::hasModulesInSharedMemory() {
    return (shm.exists() && shm()->totalNumberOfModules > 0);
}

void DetectorImpl::setHostname(const std::vector<std::string> &name) {
    // could be called after freeing shm from API
    if (!shm.exists()) {
        setupDetector();
    }
    for (const auto &hostname : name) {
        addModule(hostname);
    }
    updateDetectorSize();

    // Here we know the detector type and can add ctb shared memory
    // if needed, CTB dac names are only on detector level

    if (shm()->detType == defs::CHIPTESTBOARD ||
        shm()->detType == defs::XILINX_CHIPTESTBOARD) {
        if (ctb_shm.exists())
            ctb_shm.openSharedMemory(true);
        else
            ctb_shm.createSharedMemory();
    }
}

void DetectorImpl::addModule(const std::string &name) {
    LOG(logINFO) << "Adding module " << name;
    auto host = verifyUniqueDetHost(name);
    std::string hostname = host.first;
    uint16_t port = host.second;

    // get type by connecting
    detectorType type = Module::getTypeFromDetector(hostname, port);

    // gotthard cannot have more than 2 modules (50um=1, 25um=2
    if ((type == GOTTHARD || type == GOTTHARD2) && modules.size() > 2) {
        throw RuntimeError("Gotthard cannot have more than 2 modules. Please "
                           "free the shared memory and start again.");
    }

    auto pos = modules.size();
    modules.emplace_back(make_unique<Module>(type, detectorIndex, pos, false));
    shm()->totalNumberOfModules = modules.size();
    modules[pos]->setControlPort(port);
    modules[pos]->setStopPort(port + 1);
    modules[pos]->setHostname(hostname, shm()->initialChecks);

    // module type updated by now
    shm()->detType = Parallel(&Module::getDetectorType, {})
                         .tsquash("Inconsistent detector types.");
    // for ctb
    modules[pos]->updateNumberOfChannels();

    // for eiger, jungfrau, moench, gotthard2
    modules[pos]->updateNumberofUDPInterfaces();

    // update zmq port in case numudpinterfaces changed
    int numInterfaces = modules[pos]->getNumberofUDPInterfacesFromShm();
    modules[pos]->setClientStreamingPort(DEFAULT_ZMQ_CL_PORTNO +
                                         pos * numInterfaces);
}

void DetectorImpl::updateDetectorSize() {
    LOG(logDEBUG) << "Updating Detector Size: " << size();

    const slsDetectorDefs::xy modSize = modules[0]->getNumberOfChannels();
    if (modSize.x == 0 || modSize.y == 0) {
        throw RuntimeError(
            "Module size for x or y dimensions is 0. Unable to proceed in "
            "updating detector size. ");
    }

    int nModx = 0, nMody = 0;
    // 1d, add modules along x axis
    if (modSize.y == 1) {
        int detSizeX = shm()->numberOfChannels.x;
        int maxChanX = modSize.x * size();
        // user given detsizex used only within max value
        if (detSizeX > 1 && detSizeX <= maxChanX) {
            maxChanX = detSizeX;
        }
        if (maxChanX < modSize.x) {
            std::stringstream os;
            os << "The max det size in x dim (" << maxChanX
               << ") is less than the module size in x dim (" << modSize.x
               << "). Probably using shared memory of a different detector "
                  "type. Please free and try again.";
            throw RuntimeError(os.str());
        }
        nModx = maxChanX / modSize.x;
        if (nModx == 0) {
            throw RuntimeError(
                "number of modules in x dimension is 0. Unable to proceed.");
        }
        nMody = size() / nModx;
        if ((maxChanX % modSize.x) > 0) {
            ++nMody;
        }
    }
    // 2d, add modules along y axis (due to eiger top/bottom)
    else {
        int detSizeY = shm()->numberOfChannels.y;
        int maxChanY = modSize.y * size();
        // user given detsizey used only within max value
        if (detSizeY > 1 && detSizeY <= maxChanY) {
            maxChanY = detSizeY;
        }
        if (maxChanY < modSize.y) {
            std::stringstream os;
            os << "The max det size in y dim (" << maxChanY
               << ") is less than the module size in y dim (" << modSize.y
               << "). Probably using shared memory of a different detector "
                  "type. Please free and try again.";
            throw RuntimeError(os.str());
        }
        nMody = maxChanY / modSize.y;
        if (nMody == 0)
            throw RuntimeError(
                "number of modules in y dimension is 0. Unable to proceed.");
        nModx = size() / nMody;
        if ((maxChanY % modSize.y) > 0) {
            ++nModx;
        }
    }

    shm()->numberOfModules.x = nModx;
    shm()->numberOfModules.y = nMody;
    shm()->numberOfChannels.x = modSize.x * nModx;
    shm()->numberOfChannels.y = modSize.y * nMody;

    LOG(logDEBUG) << "\n\tNumber of Modules in X direction:"
                  << shm()->numberOfModules.x
                  << "\n\tNumber of Modules in Y direction:"
                  << shm()->numberOfModules.y
                  << "\n\tNumber of Channels in X direction:"
                  << shm()->numberOfChannels.x
                  << "\n\tNumber of Channels in Y direction:"
                  << shm()->numberOfChannels.y;

    for (auto &module : modules) {
        if (module->getUpdateMode() == 0) {
            module->updateNumberOfModule(shm()->numberOfModules);
        }
    }
}

int DetectorImpl::size() const { return modules.size(); }

slsDetectorDefs::xy DetectorImpl::getNumberOfModules() const {
    return shm()->numberOfModules;
}

slsDetectorDefs::xy DetectorImpl::getNumberOfChannels() const {
    return shm()->numberOfChannels;
}

void DetectorImpl::setNumberOfChannels(const slsDetectorDefs::xy c) {
    // detsize is set before hostname
    if (size() >= 1) {
        throw RuntimeError(
            "Set the number of channels before setting hostname.");
    }
    shm()->numberOfChannels = c;
}

bool DetectorImpl::getGapPixelsinCallback() const { return shm()->gapPixels; }

void DetectorImpl::setGapPixelsinCallback(const bool enable) {
    if (enable) {
        switch (shm()->detType) {
        case JUNGFRAU:
        case MOENCH:
            break;
        case EIGER:
            if (size() && modules[0]->getQuad()) {
                break;
            }
            if (shm()->numberOfModules.y % 2 != 0) {
                throw RuntimeError("Gap pixels can only be used "
                                   "for full modules.");
            }
            break;
        default:
            throw RuntimeError("Gap Pixels is not implemented for " +
                               ToString(shm()->detType));
        }
    }
    shm()->gapPixels = enable;
}

int DetectorImpl::getTransmissionDelay() const {
    bool eiger = false;
    switch (shm()->detType) {
    case JUNGFRAU:
    case MOENCH:
    case MYTHEN3:
        break;
    case EIGER:
        eiger = true;
        break;
    default:
        throw RuntimeError(
            "Transmission delay is not implemented for the this detector.");
    }
    if (!eiger && size() <= 1) {
        throw RuntimeError(
            "Cannot get intermodule transmission delays with just one module");
    }
    int step = 0;
    if (eiger) {
        // between left and right
        step = modules[0]->getTransmissionDelayRight();
    } else {
        // between first and second
        step = modules[1]->getTransmissionDelayFrame();
    }
    for (int i = 0; i != size(); ++i) {
        if (eiger) {
            if ((modules[i]->getTransmissionDelayLeft() != (2 * i * step)) ||
                (modules[i]->getTransmissionDelayRight() !=
                 ((2 * i + 1) * step)) ||
                (modules[i]->getTransmissionDelayFrame() !=
                 (2 * size() * step))) {
                return -1;
            }
        } else {
            if (modules[i]->getTransmissionDelayFrame() != (i * step)) {
                return -1;
            }
        }
    }
    return step;
}

void DetectorImpl::setTransmissionDelay(int step) {
    bool eiger = false;
    switch (shm()->detType) {
    case JUNGFRAU:
    case MOENCH:
    case MYTHEN3:
        break;
    case EIGER:
        eiger = true;
        break;
    default:
        throw RuntimeError(
            "Transmission delay is not implemented for the this detector.");
    }

    // using a asyc+future directly (instead of Parallel) to pass different
    // values
    std::vector<std::future<void>> futures;
    for (int i = 0; i != size(); ++i) {
        if (eiger) {
            futures.push_back(std::async(std::launch::async,
                                         &Module::setTransmissionDelayLeft,
                                         modules[i].get(), 2 * i * step));
            futures.push_back(std::async(std::launch::async,
                                         &Module::setTransmissionDelayRight,
                                         modules[i].get(), (2 * i + 1) * step));
            futures.push_back(std::async(std::launch::async,
                                         &Module::setTransmissionDelayFrame,
                                         modules[i].get(), 2 * size() * step));

        } else {
            futures.push_back(std::async(std::launch::async,
                                         &Module::setTransmissionDelayFrame,
                                         modules[i].get(), i * step));
        }
    }

    // wait for calls to complete
    for (auto &f : futures)
        f.get();
}

void DetectorImpl::destroyReceivingDataSockets() {
    LOG(logINFO) << "Going to destroy data sockets";
    // close socket
    zmqSocket.clear();

    client_downstream = false;
    LOG(logINFO) << "Destroyed Receiving Data Socket(s)";
}

void DetectorImpl::createReceivingDataSockets() {
    if (client_downstream) {
        return;
    }
    LOG(logINFO) << "Going to create data sockets";

    size_t numUDPInterfaces =
        Parallel(&Module::getNumberofUDPInterfacesFromShm, {}).squash(1);
    // gotthard2 second interface is only for veto debugging (not in gui)
    if (shm()->detType == GOTTHARD2) {
        numUDPInterfaces = 1;
    }
    size_t numSockets = modules.size() * numUDPInterfaces;

    for (size_t iSocket = 0; iSocket < numSockets; ++iSocket) {
        uint32_t portnum =
            (modules[iSocket / numUDPInterfaces]->getClientStreamingPort());
        portnum += (iSocket % numUDPInterfaces);
        try {
            zmqSocket.push_back(
                make_unique<ZmqSocket>(modules[iSocket / numUDPInterfaces]
                                           ->getClientStreamingIP()
                                           .str()
                                           .c_str(),
                                       portnum));
            // set high water mark
            int hwm = shm()->zmqHwm;
            if (hwm >= 0) {
                zmqSocket[iSocket]->SetReceiveHighWaterMark(hwm);
                // need not reconnect. cannot be connected (detector idle)
            }
            LOG(logINFO) << "Zmq Client[" << iSocket << "] at "
                         << zmqSocket.back()->GetZmqServerAddress() << "[hwm: "
                         << zmqSocket.back()->GetReceiveHighWaterMark() << "]";
        } catch (std::exception &e) {
            destroyReceivingDataSockets();
            std::ostringstream oss;
            oss << "Could not create zmq sub socket on port " << portnum;
            oss << " [" << e.what() << ']';
            throw RuntimeError(oss.str());
        }
    }

    client_downstream = true;
    LOG(logINFO) << "Receiving Data Socket(s) created";
}

void DetectorImpl::readFrameFromReceiver() {

    bool gapPixels = shm()->gapPixels;
    LOG(logDEBUG) << "Gap pixels: " << gapPixels;
    int nX = 0;
    int nY = 0;
    int nDetPixelsX = 0;
    int nDetPixelsY = 0;
    bool quadEnable = false;
    // to flip image
    bool eiger = false;
    std::array<int, 4> rxRoi = shm()->rx_roi.getIntArray();

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
    uint32_t currentSubFrameIndex = -1, coordX = -1, coordY = -1, flipRows = -1;

    while (numZmqRunning != 0) {
        // reset data
        data = false;
        if (multiframe != nullptr) {
            memset(multiframe.get(), 0xFF, multisize);
        }

        completeImage = true;

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
                        --numZmqRunning;
                        continue;
                    }

                    // if first message, allocate (all one time stuff)
                    if (image == nullptr) {
                        // allocate
                        size = zHeader.imageSize;
                        multisize = size * zmqSocket.size();
                        image = make_unique<char[]>(size);
                        multiframe = make_unique<char[]>(multisize);
                        memset(multiframe.get(), 0xFF, multisize);
                        // dynamic range
                        dynamicRange = zHeader.dynamicRange;
                        bytesPerPixel = (float)dynamicRange / 8;
                        // shape
                        nPixelsX = zHeader.npixelsx;
                        nPixelsY = zHeader.npixelsy;
                        // port geometry
                        nX = zHeader.ndetx;
                        nY = zHeader.ndety;
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
                    flipRows = zHeader.flipRows;
                    if (zHeader.completeImage == 0) {
                        completeImage = false;
                    }
                    LOG(logDEBUG1)
                        << zmqSocket[isocket]->GetPortNumber() << " "
                        << "Header Info:"
                           "\n\tcurrentFileName: "
                        << currentFileName << "\n\tcurrentAcquisitionIndex: "
                        << currentAcquisitionIndex
                        << "\n\tcurrentFrameIndex: " << currentFrameIndex
                        << "\n\tcurrentFileIndex: " << currentFileIndex
                        << "\n\tcurrentSubFrameIndex: " << currentSubFrameIndex
                        << "\n\tcurrentProgress: " << currentProgress
                        << "\n\tcoordX: " << coordX << "\n\tcoordY: " << coordY
                        << "\n\tflipRows: " << flipRows
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
                    if (shm()->detType == CHIPTESTBOARD ||
                        shm()->detType == defs::XILINX_CHIPTESTBOARD) {
                        singledetrowoffset = size;
                    }
                    LOG(logDEBUG1)
                        << "Multi Image Info:"
                           "\n\txoffset: "
                        << xoffset << "\n\tyoffset: " << yoffset
                        << "\n\tsingledetrowoffset: " << singledetrowoffset
                        << "\n\trowoffset: " << rowoffset;

                    if (eiger && (flipRows != 0U)) {
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
                int n = insertGapPixels(multiframe.get(), multigappixels,
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
                                        currentFileIndex, completeImage, rxRoi);
            try {
                dataReady(
                    thisData, currentFrameIndex,
                    ((dynamicRange == 32 && eiger) ? currentSubFrameIndex : -1),
                    pCallbackArg);
            } catch (const std::exception &e) {
                LOG(logERROR) << "Exception caught from callback: " << e.what();
            }
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

int DetectorImpl::insertGapPixels(char *image, char *&gpImage, bool quadEnable,
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
    // jungfrau/moench prefers same inter chip gap pixels as the boundary pixels
    int divisionValue = 2;
    slsDetectorDefs::detectorType detType = shm()->detType;
    if (detType == JUNGFRAU || detType == MOENCH) {
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
        createReceivingDataSockets();
    }
}

int DetectorImpl::getClientStreamingHwm() const {
    // disabled
    if (!client_downstream) {
        return shm()->zmqHwm;
    }
    // enabled
    Result<int> result;
    result.reserve(zmqSocket.size());
    for (auto &it : zmqSocket) {
        result.push_back(it->GetReceiveHighWaterMark());
    }
    int res = result.tsquash("Inconsistent zmq receive hwm values");
    return res;
}

void DetectorImpl::setClientStreamingHwm(const int limit) {
    if (limit < -1) {
        throw RuntimeError(
            "Cannot set hwm to less than -1 (-1 is lib default).");
    }
    // update shm
    shm()->zmqHwm = limit;

    // streaming enabled
    if (client_downstream) {
        // custom limit, set it directly
        if (limit >= 0) {
            for (auto &it : zmqSocket) {
                it->SetReceiveHighWaterMark(limit);
                // need not reconnect. cannot be connected (detector idle)
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

        // start receiver
        if (receiver) {
            Parallel(&Module::startReceiver, {});
        }

        startProcessingThread(receiver);

        // start and read all
        try {
            startAcquisition(true, {});
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
        } else if (receiver) {
            // wait for postprocessor to process dummies
            if (dataReady != nullptr) {
                // process dummy from stop receier
                if (numZmqRunning != 0)
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                while (numZmqRunning != 0) {
                    Parallel(&Module::restreamStopFromReceiver, {});
                    // time to process restream dummy
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    // increase time for fewer dummies and to catch up
                    if (numZmqRunning != 0)
                        std::this_thread::sleep_for(std::chrono::seconds(2));
                }
            }
        }
        dataProcessingThread.join();

        if (acquisition_finished != nullptr) {
            // status
            auto statusList = Parallel(&Module::getRunStatus, {});
            // if any slave still waiting, wait up to 1s (gotthard)
            for (int i = 0; i != 20 && statusList.any(WAITING); ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                statusList = Parallel(&Module::getRunStatus, {});
            }
            runStatus status = statusList.squash(ERROR);
            // inconsistent status (squash error), but none of them in error
            if (status == ERROR && (!statusList.any(ERROR))) {
                // handle jf sync issue (master idle, slaves stopped)
                if (statusList.contains_only(IDLE, STOPPED)) {
                    status = STOPPED;
                } else
                    status = statusList.squash(RUNNING);
            }

            // progress
            auto a = Parallel(&Module::getReceiverProgress, {});
            double progress = (*std::max_element(a.begin(), a.end()));

            // callback
            acquisition_finished(progress, static_cast<int>(status),
                                 acqFinished_p);
        }

        clock_gettime(CLOCK_REALTIME, &end);
        LOG(logDEBUG1) << "Elapsed time for acquisition:"
                       << ((end.tv_sec - begin.tv_sec) +
                           (end.tv_nsec - begin.tv_nsec) / 1000000000.0)
                       << " seconds";
    } catch (...) {
        if (dataProcessingThread.joinable()) {
            setJoinThreadFlag(true);
            dataProcessingThread.join();
        }
        setAcquiringFlag(false);
        throw;
    }
    setAcquiringFlag(false);
    return OK;
}

bool DetectorImpl::handleSynchronization(Positions pos) {
    bool handleSync = false;
    // multi module m3 or multi module sync enabled jungfrau
    if (size() > 1) {
        switch (shm()->detType) {
        case defs::MYTHEN3:
        case defs::GOTTHARD2:
        case defs::GOTTHARD:
            handleSync = true;
            break;
        case defs::JUNGFRAU:
        case defs::MOENCH:
            if (Parallel(&Module::getSynchronizationFromStopServer, pos)
                    .tsquash("Inconsistent synchronization among modules")) {
                handleSync = true;
            }
            break;
        default:
            break;
        }
    }
    return handleSync;
}

void DetectorImpl::getMasterSlaveList(std::vector<int> positions,
                                      std::vector<int> &masters,
                                      std::vector<int> &slaves) {
    // expand positions list
    if (positions.empty() || (positions.size() == 1 && positions[0] == -1)) {
        positions.resize(modules.size());
        std::iota(begin(positions), end(positions), 0);
    }
    // could be all slaves in positions
    slaves.reserve(positions.size());
    auto is_master = Parallel(&Module::isMaster, positions);
    for (size_t i : positions) {
        if (is_master[i])
            masters.push_back(i);
        else
            slaves.push_back(i);
    }
}

void DetectorImpl::startAcquisition(const bool blocking, Positions pos) {

    // slaves first
    if (handleSynchronization(pos)) {
        std::vector<int> masters;
        std::vector<int> slaves;
        getMasterSlaveList(pos, masters, slaves);
        if (masters.empty()) {
            throw RuntimeError("Cannot start acquisition in sync mode. No "
                               "master module found");
        }
        if (!slaves.empty()) {
            Parallel(&Module::startAcquisition, slaves);
        }
        if (blocking) {
            Parallel(&Module::startAndReadAll, masters);
            // ensure all status normal (slaves not blocking)
            // to catch those slaves that are still 'waiting'
            auto statusList = Parallel(&Module::getRunStatus, pos);
            // if any slave still waiting, wait up to 1s (gotthard)
            for (int i = 0; i != 20 && statusList.any(WAITING); ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                statusList = Parallel(&Module::getRunStatus, pos);
            }
            if (!statusList.contains_only(IDLE, STOPPED, RUN_FINISHED)) {
                throw RuntimeError("Acquisition not successful. "
                                   "Unexpected detector status");
            }
        } else {
            Parallel(&Module::startAcquisition, masters);
        }
    }
    // all in parallel
    else {
        Parallel(
            (blocking ? &Module::startAndReadAll : &Module::startAcquisition),
            pos);
    }
}

void DetectorImpl::sendSoftwareTrigger(const bool block, Positions pos) {
    // slaves first
    if (handleSynchronization(pos)) {
        std::vector<int> masters;
        std::vector<int> slaves;
        getMasterSlaveList(pos, masters, slaves);
        if (!slaves.empty())
            Parallel(&Module::sendSoftwareTrigger, slaves, false);
        if (!masters.empty())
            Parallel(&Module::sendSoftwareTrigger, masters, block);
    }
    // all in parallel
    else {
        Parallel(&Module::sendSoftwareTrigger, pos, block);
    }
}

void DetectorImpl::stopDetector(Positions pos) {
    // masters first
    if (handleSynchronization(pos)) {
        std::vector<int> masters;
        std::vector<int> slaves;
        getMasterSlaveList(pos, masters, slaves);
        if (!masters.empty())
            Parallel(&Module::stopAcquisition, masters);
        if (!slaves.empty())
            Parallel(&Module::stopAcquisition, slaves);
    }
    // all in parallel
    else {
        Parallel(&Module::stopAcquisition, pos);
    }
}

void DetectorImpl::printProgress(double progress) {
    // spaces for python printout
    std::cout << "    " << std::fixed << std::setprecision(2) << std::setw(10)
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
            LOG(logINFO) << "Type 'q' and hit enter to stop acquisition";
            double progress = 0;
            printProgress(progress);

            while (true) {
                // to exit acquire by typing q
                if (kbhit() != 0) {
                    if (fgetc(stdin) == 'q') {
                        LOG(logINFO)
                            << "Caught the command to stop acquisition";
                        stopDetector({});
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
    switch (shm()->detType) {
    case JUNGFRAU:
    case MOENCH:
    case CHIPTESTBOARD:
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
    case EIGER:
    case GOTTHARD:
        throw RuntimeError("programfpga not implemented for this detector");
    default:
        throw RuntimeError(
            "Unknown detector type. Did the 'hostname' command execute "
            "successfully? Or use update mode in the detector server "
            "side.");
    }

    LOG(logINFO) << "This can take awhile. Please be patient.";
    LOG(logDEBUG1) << "Programming FPGA with file name:" << fname;

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

    // get srcSize to print progress
    ssize_t srcSize = getFileSize(src, "Program FPGA");

    // create temp destination file
    char destfname[] = "/tmp/SLS_DET_MCB.XXXXXX";
    int dst = mkstemp(destfname); // create temporary file and open it in r/w
    if (dst == -1) {
        fclose(src);
        throw RuntimeError(std::string("Could not create destination file "
                                       "in /tmp for programming: ") +
                           destfname);
    }

    // convert src to dst rawbin
    LOG(logDEBUG1) << "Converting " << fname << " to " << destfname;
    LOG(logINFO) << "Converting program to rawbin";
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
        int oldProgress = 0;
        while (!feof(src)) {
            // print progress
            int progress = (int)(((double)(dstFilePos) / srcSize) * 100);
            if (oldProgress != progress) {
                printf("%d%%\r", progress);
                fflush(stdout);
                oldProgress = progress;
            }
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
            throw RuntimeError("Could not convert programming file. EOF "
                               "before end of flash");
        }
    }
    if (fclose(src) != 0) {
        throw RuntimeError("Program FPGA: Could not close source file");
    }
    if (close(dst) != 0) {
        throw RuntimeError("Program FPGA: Could not close destination file");
    }
    LOG(logINFO) << "File has been converted to " << destfname;

    // load converted file to memory
    std::vector<char> buffer = readBinaryFile(destfname, "Program FPGA");
    // delete temporary
    unlink(destfname);
    return buffer;
}

Result<int> DetectorImpl::getDefaultDac(defs::dacIndex index,
                                        defs::detectorSettings sett,
                                        Positions pos) {
    return Parallel(&Module::getDefaultDac, pos, index, sett);
}

void DetectorImpl::setDefaultDac(defs::dacIndex index, int defaultValue,
                                 defs::detectorSettings sett, Positions pos) {
    Parallel(&Module::setDefaultDac, pos, index, defaultValue, sett);
}

defs::xy DetectorImpl::getPortGeometry() const {
    defs::xy portGeometry(1, 1);
    switch (shm()->detType) {
    case EIGER:
        portGeometry.x = modules[0]->getNumberofUDPInterfacesFromShm();
        break;
    case JUNGFRAU:
    case MOENCH:
        portGeometry.y = modules[0]->getNumberofUDPInterfacesFromShm();
        break;
    default:
        break;
    }
    return portGeometry;
}

defs::xy DetectorImpl::calculatePosition(int moduleIndex,
                                         defs::xy geometry) const {
    defs::xy pos{};
    int maxYMods = shm()->numberOfModules.y;
    pos.y = (moduleIndex % maxYMods) * geometry.y;
    pos.x = (moduleIndex / maxYMods) * geometry.x;
    return pos;
}

void DetectorImpl::verifyUniqueDetHost(const uint16_t port,
                                       std::vector<int> positions) const {
    // port for given positions
    if (positions.empty() || (positions.size() == 1 && positions[0] == -1)) {
        positions.resize(modules.size());
        std::iota(begin(positions), end(positions), 0);
    }
    std::vector<std::pair<std::string, uint16_t>> hosts(size());
    for (auto it : positions) {
        hosts[it].second = port;
    }
    verifyUniqueHost(true, hosts);
}

void DetectorImpl::verifyUniqueRxHost(const uint16_t port,
                                      const int moduleId) const {
    std::vector<std::pair<std::string, uint16_t>> hosts(size());
    hosts[moduleId].second = port;
    verifyUniqueHost(false, hosts);
}

std::pair<std::string, uint16_t>
DetectorImpl::verifyUniqueDetHost(const std::string &name) {
    // extract port
    // C++17 could be auto [hostname, port] = ParseHostPort(name);
    auto res = ParseHostPort(name);
    std::string hostname = res.first;
    uint16_t port = res.second;
    if (port == 0) {
        port = DEFAULT_TCP_CNTRL_PORTNO;
    }

    int detSize = size();
    // mod not yet added
    std::vector<std::pair<std::string, uint16_t>> hosts(detSize + 1);
    hosts[detSize].first = hostname;
    hosts[detSize].second = port;

    verifyUniqueHost(true, hosts);
    return std::make_pair(hostname, port);
}

std::pair<std::string, uint16_t>
DetectorImpl::verifyUniqueRxHost(const std::string &name,
                                 std::vector<int> positions) const {
    // no checks if setting to none
    if (name == "none" || name.empty()) {
        return make_pair(name, 0);
    }
    // extract port
    // C++17 could be auto [hostname, port] = ParseHostPort(name);
    auto res = ParseHostPort(name);
    std::string hostname = res.first;
    uint16_t port = res.second;

    // hostname and port for given positions
    if (positions.empty() || (positions.size() == 1 && positions[0] == -1)) {
        positions.resize(modules.size());
        std::iota(begin(positions), end(positions), 0);
    }

    std::vector<std::pair<std::string, uint16_t>> hosts(size());
    for (auto it : positions) {
        hosts[it].first = hostname;
        hosts[it].second = port;
    }

    verifyUniqueHost(false, hosts);
    return std::make_pair(hostname, port);
}

std::vector<std::pair<std::string, uint16_t>>
DetectorImpl::verifyUniqueRxHost(const std::vector<std::string> &names) const {
    if ((int)names.size() != size()) {
        throw RuntimeError(
            "Receiver hostnames size " + std::to_string(names.size()) +
            " does not match detector size " + std::to_string(size()));
    }

    // extract ports
    std::vector<std::pair<std::string, uint16_t>> hosts;
    for (const auto &name : names) {
        hosts.push_back(ParseHostPort(name));
    }

    verifyUniqueHost(false, hosts);
    return hosts;
}

void DetectorImpl::verifyUniqueHost(
    bool isDet, std::vector<std::pair<std::string, uint16_t>> &hosts) const {

    // fill from shm if not provided
    for (int i = 0; i != size(); ++i) {
        if (hosts[i].first.empty()) {
            hosts[i].first = (isDet ? modules[i]->getHostname()
                                    : modules[i]->getReceiverHostname());
        }
        if (hosts[i].second == 0) {
            hosts[i].second = (isDet ? modules[i]->getControlPort()
                                     : modules[i]->getReceiverPort());
        }
    }

    // remove the ones without a hostname
    hosts.erase(std::remove_if(hosts.begin(), hosts.end(),
                               [](const std::pair<std::string, uint16_t> &x) {
                                   return (x.first == "none" ||
                                           x.first.empty());
                               }),
                hosts.end());

    // must be unique
    if (hasDuplicates(hosts)) {
        throw RuntimeError(
            "Cannot set due to duplicate hostname-port number pairs.");
    }

    for (auto it : hosts) {
        LOG(logDEBUG) << it.first << " " << it.second << std::endl;
    }
}

defs::ROI DetectorImpl::getRxROI() const {
    if (shm()->detType == CHIPTESTBOARD ||
        shm()->detType == defs::XILINX_CHIPTESTBOARD) {
        throw RuntimeError("RxRoi not implemented for this Detector");
    }
    if (modules.size() == 0) {
        throw RuntimeError("No Modules added");
    }
    // complete detector in roi
    auto t = Parallel(&Module::getRxROI, {});
    if (t.equal() && t.front().completeRoi()) {
        LOG(logDEBUG) << "no roi";
        return defs::ROI(0, shm()->numberOfChannels.x - 1, 0,
                         shm()->numberOfChannels.y - 1);
    }

    defs::xy numChansPerMod = modules[0]->getNumberOfChannels();
    bool is2D = (numChansPerMod.y > 1 ? true : false);
    defs::xy geometry = getPortGeometry();

    defs::ROI retval{};
    for (size_t iModule = 0; iModule != modules.size(); ++iModule) {

        defs::ROI moduleRoi = modules[iModule]->getRxROI();
        if (moduleRoi.noRoi()) {
            LOG(logDEBUG) << iModule << ": no roi";
        } else {
            // expand complete roi
            if (moduleRoi.completeRoi()) {
                moduleRoi.xmin = 0;
                moduleRoi.xmax = numChansPerMod.x;
                if (is2D) {
                    moduleRoi.ymin = 0;
                    moduleRoi.ymax = numChansPerMod.y;
                }
            }
            LOG(logDEBUG) << iModule << ": " << moduleRoi;

            // get roi at detector level
            defs::xy pos = calculatePosition(iModule, geometry);
            defs::ROI moduleFullRoi{};
            moduleFullRoi.xmin = numChansPerMod.x * pos.x + moduleRoi.xmin;
            moduleFullRoi.xmax = numChansPerMod.x * pos.x + moduleRoi.xmax;
            if (is2D) {
                moduleFullRoi.ymin = numChansPerMod.y * pos.y + moduleRoi.ymin;
                moduleFullRoi.ymax = numChansPerMod.y * pos.y + moduleRoi.ymax;
            }
            LOG(logDEBUG) << iModule << ": (full roi)" << moduleFullRoi;

            // get min and max
            if (retval.xmin == -1 || moduleFullRoi.xmin < retval.xmin) {
                LOG(logDEBUG) << iModule << ": xmin updated";
                retval.xmin = moduleFullRoi.xmin;
            }
            if (retval.xmax == -1 || moduleFullRoi.xmax > retval.xmax) {
                LOG(logDEBUG) << iModule << ": xmax updated";
                retval.xmax = moduleFullRoi.xmax;
            }
            if (retval.ymin == -1 || moduleFullRoi.ymin < retval.ymin) {
                LOG(logDEBUG) << iModule << ": ymin updated";
                retval.ymin = moduleFullRoi.ymin;
            }
            if (retval.ymax == -1 || moduleFullRoi.ymax > retval.ymax) {
                LOG(logDEBUG) << iModule << ": ymax updated";
                retval.ymax = moduleFullRoi.ymax;
            }
        }
        LOG(logDEBUG) << iModule << ": (retval): " << retval;
    }
    if (retval.ymin == -1) {
        retval.ymin = 0;
        retval.ymax = 0;
    }
    return retval;
}

void DetectorImpl::setRxROI(const defs::ROI arg) {
    if (shm()->detType == CHIPTESTBOARD ||
        shm()->detType == defs::XILINX_CHIPTESTBOARD) {
        throw RuntimeError("RxRoi not implemented for this Detector");
    }
    if (modules.size() == 0) {
        throw RuntimeError("No Modules added");
    }
    if (arg.noRoi()) {
        throw RuntimeError("Invalid Roi of size 0.");
    }
    if (arg.completeRoi()) {
        throw RuntimeError("Did you mean the clear roi command (API: "
                           "clearRxROI, cmd: rx_clearroi)?");
    }
    if (arg.xmin > arg.xmax || arg.ymin > arg.ymax) {
        throw RuntimeError(
            "Invalid Receiver Roi. xmin/ymin exceeds xmax/ymax.");
    }

    defs::xy numChansPerMod = modules[0]->getNumberOfChannels();
    bool is2D = (numChansPerMod.y > 1 ? true : false);
    defs::xy geometry = getPortGeometry();

    if (!is2D && ((arg.ymin != -1 && arg.ymin != 0) ||
                  (arg.ymax != -1 && arg.ymax != 0))) {
        throw RuntimeError(
            "Invalid Receiver roi. Cannot set 2d roi for a 1d detector.");
    }

    if (arg.xmin < 0 || arg.xmax >= shm()->numberOfChannels.x ||
        (is2D && (arg.ymin < 0 || arg.ymax >= shm()->numberOfChannels.y))) {
        throw RuntimeError("Invalid Receiver Roi. Outside detector range.");
    }

    for (size_t iModule = 0; iModule != modules.size(); ++iModule) {
        // default init = complete roi
        defs::ROI moduleRoi{};

        // incomplete roi
        if (!arg.completeRoi()) {
            // multi module Gotthard2
            if (shm()->detType == GOTTHARD2 && size() > 1) {
                moduleRoi.xmin = arg.xmin / 2;
                moduleRoi.xmax = arg.xmax / 2;
                if (iModule == 0) {
                    // all should be even
                    if (arg.xmin % 2 != 0) {
                        ++moduleRoi.xmin;
                    }
                } else if (iModule == 1) {
                    // all should be odd
                    if (arg.xmax % 2 == 0) {
                        --moduleRoi.xmax;
                    }
                } else {
                    throw RuntimeError("Cannot have more than 2 modules for a "
                                       "Gotthard2 detector");
                }
            } else {
                // get module limits
                defs::xy pos = calculatePosition(iModule, geometry);
                defs::ROI moduleFullRoi{};
                moduleFullRoi.xmin = numChansPerMod.x * pos.x;
                moduleFullRoi.xmax = numChansPerMod.x * (pos.x + 1) - 1;
                if (is2D) {
                    moduleFullRoi.ymin = numChansPerMod.y * pos.y;
                    moduleFullRoi.ymax = numChansPerMod.y * (pos.y + 1) - 1;
                }

                // no roi
                if (arg.xmin > moduleFullRoi.xmax ||
                    arg.xmax < moduleFullRoi.xmin ||
                    (is2D && (arg.ymin > moduleFullRoi.ymax ||
                              arg.ymax < moduleFullRoi.ymin))) {
                    moduleRoi.setNoRoi();
                }
                // incomplete module roi
                else if (arg.xmin > moduleFullRoi.xmin ||
                         arg.xmax < moduleFullRoi.xmax ||
                         (is2D && (arg.ymin > moduleFullRoi.ymin ||
                                   arg.ymax < moduleFullRoi.ymax))) {
                    moduleRoi.xmin = (arg.xmin <= moduleFullRoi.xmin)
                                         ? 0
                                         : (arg.xmin % numChansPerMod.x);
                    moduleRoi.xmax = (arg.xmax >= moduleFullRoi.xmax)
                                         ? numChansPerMod.x - 1
                                         : (arg.xmax % numChansPerMod.x);
                    if (is2D) {
                        moduleRoi.ymin = (arg.ymin <= moduleFullRoi.ymin)
                                             ? 0
                                             : (arg.ymin % numChansPerMod.y);
                        moduleRoi.ymax = (arg.ymax >= moduleFullRoi.ymax)
                                             ? numChansPerMod.y - 1
                                             : (arg.ymax % numChansPerMod.y);
                    }
                }
            }
        }
        modules[iModule]->setRxROI(moduleRoi);
    }
    // updating shm rx_roi for gui purposes
    shm()->rx_roi = arg;

    // metadata
    if (arg.completeRoi()) {
        modules[0]->setRxROIMetadata(defs::ROI(0, shm()->numberOfChannels.x - 1,
                                               0,
                                               shm()->numberOfChannels.y - 1));
    } else {
        modules[0]->setRxROIMetadata(arg);
    }
}

void DetectorImpl::clearRxROI() {
    Parallel(&Module::setRxROI, {}, defs::ROI{});
    shm()->rx_roi.xmin = -1;
    shm()->rx_roi.ymin = -1;
    shm()->rx_roi.xmax = -1;
    shm()->rx_roi.ymax = -1;
}

void DetectorImpl::getBadChannels(const std::string &fname,
                                  Positions pos) const {
    auto res = Parallel(&Module::getBadChannels, pos);
    std::vector<int> badchannels(res[0]);

    // update to multi values if multi modules
    if (isAllPositions(pos)) {
        badchannels.clear();
        int nchan = modules[0]->getNumberOfChannels().x;
        if (shm()->detType == MYTHEN3) {
            // assuming single counter
            nchan /= MAX_NUM_COUNTERS;
        }
        int imod = 0;
        for (auto vec : res) {
            for (auto badch : vec) {
                badchannels.push_back(imod * nchan + badch);
            }
            ++imod;
        }
    } else if (pos.size() != 1) {
        throw RuntimeError("Can get bad channels only for 1 or all modules.\n");
    }

    // save to file
    LOG(logDEBUG1) << "Getting bad channels to " << fname;
    std::ofstream outfile(fname);
    if (!outfile) {
        throw RuntimeError("Could not create file to save bad channels");
    }
    for (auto ch : badchannels)
        outfile << ch << '\n';
    LOG(logDEBUG1) << badchannels.size() << " bad channels saved to file";
}

void DetectorImpl::setBadChannels(const std::string &fname, Positions pos) {
    std::vector<int> list = sls::getChannelsFromFile(fname);
    if (list.empty()) {
        throw RuntimeError("Bad channel file is empty.");
    }
    setBadChannels(list, pos);
}

void DetectorImpl::setBadChannels(const std::vector<int> list, Positions pos) {

    // update to multi values if multi modules
    if (isAllPositions(pos)) {
        std::vector<std::vector<int>> badchannels(modules.size());
        int nchan = modules[0]->getNumberOfChannels().x;
        if (shm()->detType == MYTHEN3) {
            // assuming single counter
            nchan /= MAX_NUM_COUNTERS;
        }
        for (auto badchannel : list) {
            if (badchannel < 0) {
                throw RuntimeError("Invalid bad channel list. " +
                                   std::to_string(badchannel) +
                                   " out of bounds.");
            }
            int ch = badchannel % nchan;
            size_t imod = badchannel / nchan;
            if (imod >= modules.size()) {
                throw RuntimeError("Invalid bad channel list. " +
                                   std::to_string(badchannel) +
                                   " out of bounds.");
            }
            badchannels[imod].push_back(ch);
        }
        for (size_t imod = 0; imod != modules.size(); ++imod) {
            Parallel(&Module::setBadChannels, {static_cast<int>(imod)},
                     badchannels[imod]);
        }

    } else if (pos.size() != 1) {
        throw RuntimeError("Can set bad channels only for 1 or all modules.\n");
    } else {
        Parallel(&Module::setBadChannels, pos, list);
    }
}

std::vector<std::string> DetectorImpl::getCtbDacNames() const {
    return ctb_shm()->getDacNames();
}

void DetectorImpl::setCtbDacNames(const std::vector<std::string> &names) {
    ctb_shm()->setDacNames(names);
}

std::string DetectorImpl::getCtbDacName(defs::dacIndex i) const {
    return ctb_shm()->getDacName(static_cast<int>(i));
}

void DetectorImpl::setCtbDacName(const defs::dacIndex index,
                                 const std::string &name) {
    ctb_shm()->setDacName(index, name);
}

std::vector<std::string> DetectorImpl::getCtbAdcNames() const {
    return ctb_shm()->getAdcNames();
}

void DetectorImpl::setCtbAdcNames(const std::vector<std::string> &names) {
    ctb_shm()->setAdcNames(names);
}

std::string DetectorImpl::getCtbAdcName(const int i) const {
    return ctb_shm()->getAdcName(i);
}

void DetectorImpl::setCtbAdcName(const int index, const std::string &name) {
    ctb_shm()->setAdcName(index, name);
}

std::vector<std::string> DetectorImpl::getCtbSignalNames() const {
    return ctb_shm()->getSignalNames();
}

void DetectorImpl::setCtbSignalNames(const std::vector<std::string> &names) {
    ctb_shm()->setSignalNames(names);
}

std::string DetectorImpl::getCtbSignalName(const int i) const {
    return ctb_shm()->getSignalName(i);
}

void DetectorImpl::setCtbSignalName(const int index, const std::string &name) {
    ctb_shm()->setSignalName(index, name);
}

std::vector<std::string> DetectorImpl::getCtbPowerNames() const {
    return ctb_shm()->getPowerNames();
}

void DetectorImpl::setCtbPowerNames(const std::vector<std::string> &names) {
    ctb_shm()->setPowerNames(names);
}

std::string DetectorImpl::getCtbPowerName(const defs::dacIndex i) const {
    return ctb_shm()->getPowerName(static_cast<int>(i - defs::V_POWER_A));
}

void DetectorImpl::setCtbPowerName(const defs::dacIndex index,
                                   const std::string &name) {
    ctb_shm()->setPowerName(static_cast<int>(index - defs::V_POWER_A), name);
}

std::vector<std::string> DetectorImpl::getCtbSlowADCNames() const {
    return ctb_shm()->getSlowADCNames();
}

void DetectorImpl::setCtbSlowADCNames(const std::vector<std::string> &names) {
    ctb_shm()->setSlowADCNames(names);
}

std::string DetectorImpl::getCtbSlowADCName(const defs::dacIndex i) const {
    return ctb_shm()->getSlowADCName(static_cast<int>(i - defs::SLOW_ADC0));
}

void DetectorImpl::setCtbSlowADCName(const defs::dacIndex index,
                                     const std::string &name) {
    ctb_shm()->setSlowADCName(static_cast<int>(index - defs::SLOW_ADC0), name);
}

} // namespace sls