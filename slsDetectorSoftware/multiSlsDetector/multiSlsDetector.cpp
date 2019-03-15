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

#include "string_utils.h"

#include <cstring>
#include <iomanip>
#include <iostream>
#include <rapidjson/document.h> //json header in zmq stream
#include <sstream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
//#include <time.h> //clock()

#include "container_utils.h"
#include <chrono>
#include <future>
#include <vector>

using sls::SharedMemory;
using sls::SharedMemoryError;

multiSlsDetector::multiSlsDetector(int id, bool verify, bool update)
    : detId(id) {
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
std::vector<RT> multiSlsDetector::serialCall(RT (slsDetector::*somefunc)(CT...),
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
multiSlsDetector::parallelCall(RT (slsDetector::*somefunc)(CT...), typename NonDeduced<CT>::type... Args) {
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

int multiSlsDetector::decodeNChannel(int offsetX, int offsetY, int &channelX,
                                     int &channelY) {
    channelX = -1;
    channelY = -1;
    // loop over
    for (size_t i = 0; i < detectors.size(); ++i) {
        int x = detectors[i]->getDetectorOffset(X);
        int y = detectors[i]->getDetectorOffset(Y);
        // check x offset range
        if ((offsetX >= x) &&
            (offsetX <
             (x + detectors[i]->getTotalNumberOfChannelsInclGapPixels(X)))) {
            if (offsetY == -1) {
                channelX = offsetX - x;
                return i;
            } else {
                // check y offset range
                if ((offsetY >= y) &&
                    (offsetY <
                     (y + detectors[i]->getTotalNumberOfChannelsInclGapPixels(
                              Y)))) {
                    channelX = offsetX - x;
                    channelY = offsetY - y;
                    return i;
                }
            }
        }
    }
    return -1;
}

std::string multiSlsDetector::getErrorMessage(int &critical, int detPos) {
    int64_t multiMask = 0, slsMask = 0;
    std::string retval = "";
    critical = 0;
    size_t posmin = 0, posmax = detectors.size();

    // single
    if (detPos >= 0) {

        slsMask = detectors[detPos]->getErrorMask();
        posmin = (size_t)detPos;
        posmax = posmin + 1;
    }

    multiMask = getErrorMask();
    if (multiMask || slsMask) {
        if (multiMask & MULTI_DETECTORS_NOT_ADDED) {
            retval.append("Detectors not added:\n" +
                          std::string(getNotAddedList()) + std::string("\n"));
            critical = 1;
        }
        if (multiMask & MULTI_HAVE_DIFFERENT_VALUES) {
            retval.append(
                "A previous multi detector command gave different values\n"
                "Please check the console\n");
            critical = 0;
        }
        if (multiMask & MULTI_CONFIG_FILE_ERROR) {
            retval.append("Could not load Config File\n");
            critical = 1;
        }
        if (multiMask & MULTI_POS_EXCEEDS_LIST) {
            retval.append("Position exceeds multi detector list\n");
            critical = 0;
        }
        if (multiMask & MUST_BE_MULTI_CMD) {
            retval.append("Must be a multi detector level command.\n");
            critical = 0;
        }
        if (multiMask & MULTI_OTHER_ERROR) {
            retval.append("Some error occured from multi level.\n");
            critical =
                0; // FIXME: with exceptions/appropriate errors wherever used
        }

        for (size_t idet = posmin; idet < posmax; ++idet) {
            // if the detector has error
            if ((multiMask & (1 << idet)) || (detPos >= 0)) {

                // append detector id
                retval.append("Detector " + std::to_string(idet) +
                              std::string(":\n"));

                // get sls det error mask
                slsMask = detectors[idet]->getErrorMask();

                // get the error critical level
                if ((slsMask > 0xFFFFFFFF) | critical) {
                    critical = 1;
                }

                // append error message
                retval.append(errorDefs::getErrorMessage(slsMask));
            }
        }
    }
    return retval;
}

int64_t multiSlsDetector::clearAllErrorMask(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->clearErrorMask();
    }

    // multi
    clearErrorMask();
    clearNotAddedList();
    for (auto &d : detectors) {
        d->clearErrorMask();
    }
    return getErrorMask();
}

void multiSlsDetector::setErrorMaskFromAllDetectors() {
    for (size_t idet = 0; idet < detectors.size(); ++idet) {
        if (detectors[idet]->getErrorMask()) {
            setErrorMask(getErrorMask() | (1 << idet));
        }
    }
}

void multiSlsDetector::setAcquiringFlag(bool b) {
    multi_shm()->acquiringFlag = b;
}

bool multiSlsDetector::getAcquiringFlag() const {
    return multi_shm()->acquiringFlag;
}

bool multiSlsDetector::isAcquireReady() {
    if (multi_shm()->acquiringFlag) {
        FILE_LOG(logWARNING) << "Acquire has already started. "
                                "If previous acquisition terminated unexpectedly, "
                                "reset busy flag to restart.(sls_detector_put busy 0)";
        return FAIL;
    }
    multi_shm()->acquiringFlag = true;
    return OK;
}

int multiSlsDetector::checkDetectorVersionCompatibility(int detPos) {
    if (detPos >= 0) {
        return detectors[detPos]->checkDetectorVersionCompatibility();
    }

    auto r = parallelCall(&slsDetector::checkDetectorVersionCompatibility);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::checkReceiverVersionCompatibility(int detPos) {
    if (detPos >= 0) {
        return detectors[detPos]->checkReceiverVersionCompatibility();
    }

    auto r = parallelCall(&slsDetector::checkReceiverVersionCompatibility);
    return sls::minusOneIfDifferent(r);
}

int64_t multiSlsDetector::getId(idMode mode, int detPos) {
    if (detPos >= 0) {
        return detectors[detPos]->getId(mode);
    }

    auto r = parallelCall(&slsDetector::getId, mode);
    return sls::minusOneIfDifferent(r);
}

void multiSlsDetector::freeSharedMemory(int multiId, int detPos) {
    // single
    if (detPos >= 0) {
        slsDetector::freeSharedMemory(multiId, detPos);
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

void multiSlsDetector::freeSharedMemory(int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->freeSharedMemory();
        return;
    }

    // multi
    zmqSocket.clear();
    clearAllErrorMask();
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
    sstream << "\nHostname: " << getHostname();
    sstream << "\nType: ";
    for (auto &d : detectors) {
        sstream << d->getDetectorTypeAsString() << "+";
    }

    sstream << "\nPID: " << multi_shm()->lastPID
            << "\nUser: " << multi_shm()->lastUser
            << "\nDate: " << multi_shm()->lastDate << std::endl;

    return sstream.str();
}

/*
 * pre: multi_shm=0, multi_shm() = 0, detectors.size() = 0
 */
void multiSlsDetector::initSharedMemory(bool verify) {
    multi_shm = SharedMemory<sharedMultiSlsDetector>(detId, -1);
    if (!multi_shm.IsExisting()) {
        multi_shm.CreateSharedMemory();
        initializeDetectorStructure();
    }
    else {
        multi_shm.OpenSharedMemory();
        if (verify && multi_shm()->shmversion != MULTI_SHMVERSION) {
            FILE_LOG(logERROR) << "Multi shared memory (" << detId << ") version mismatch "
                                                                      "(expected 0x"
                               << std::hex << MULTI_SHMVERSION << " but got 0x" << multi_shm()->shmversion << std::dec;
            throw SharedMemoryError("Shared memory version mismatch!");
        }
    }
}

void multiSlsDetector::initializeDetectorStructure() {
    multi_shm()->shmversion = MULTI_SHMVERSION;
    multi_shm()->numberOfDetectors = 0;
    multi_shm()->numberOfDetector[X] = 0;
    multi_shm()->numberOfDetector[Y] = 0;
    multi_shm()->onlineFlag = 1;
    multi_shm()->stoppedFlag = 0;
    multi_shm()->dataBytes = 0;
    multi_shm()->dataBytesInclGapPixels = 0;
    multi_shm()->numberOfChannels = 0;
    multi_shm()->numberOfChannel[X] = 0;
    multi_shm()->numberOfChannel[Y] = 0;
    multi_shm()->numberOfChannelInclGapPixels[X] = 0;
    multi_shm()->numberOfChannelInclGapPixels[Y] = 0;
    multi_shm()->maxNumberOfChannelsPerDetector[X] = 0;
    multi_shm()->maxNumberOfChannelsPerDetector[Y] = 0;
    for (int64_t &i : multi_shm()->timerValue) {
        i = 0;
    }

    multi_shm()->acquiringFlag = false;
    multi_shm()->receiverOnlineFlag = OFFLINE_FLAG;
    multi_shm()->receiver_upstream = false;
}

void multiSlsDetector::initializeMembers(bool verify) {
    // multiSlsDetector
    zmqSocket.clear();

    // get objects from single det shared memory (open)
    for (int i = 0; i < multi_shm()->numberOfDetectors; i++) {
        try {
            detectors.push_back(
                sls::make_unique<slsDetector>(detId, i, verify));
        } catch (...) {
            detectors.clear();
            throw;
        }
    }

    // depend on number of detectors
    updateOffsets();
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

std::string multiSlsDetector::exec(const char *cmd) {
    int bufsize = 128;
    char buffer[bufsize];
    std::string result = "";
    FILE *pipe = popen(cmd, "r");
    if (!pipe) {
        throw RuntimeError("Could not open pipe");
    }
    try {
        while (!feof(pipe)) {
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

void multiSlsDetector::setHostname(const char *name, int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->setHostname(name);
        return;
    }

    // multi
    // this check is there only to allow the previous detsizechan command
    if (multi_shm()->numberOfDetectors) {
        FILE_LOG(logWARNING) << "There are already detector(s) in shared memory."
                                "Freeing Shared memory now.";
        freeSharedMemory();
        setupMultiDetector();
    }
    addMultipleDetectors(name);
}

std::string multiSlsDetector::getHostname(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getHostname();
    }

    // multi
    auto r = serialCall(&slsDetector::getHostname);
    return sls::concatenateIfDifferent(r);
}

void multiSlsDetector::addMultipleDetectors(const char *name) {
    for (const auto &hostname : sls::split(name, '+')) {
        addSlsDetector(hostname);
    }

    setOnline();
    updateOffsets();
}

void multiSlsDetector::addSlsDetector(const std::string &hostname) {
    FILE_LOG(logDEBUG1) << "Adding detector " << hostname;

    for (auto &d : detectors) {
        if (d->getHostname() == hostname) {
            FILE_LOG(logWARNING) << "Detector " << hostname
                                 << "already part of the multiDetector!" << std::endl
                                 << "Remove it before adding it back in a new position!";
            return;
        }
    }

    // get type by connecting
    detectorType type = slsDetector::getTypeFromDetector(hostname.c_str(), DEFAULT_PORTNO);
    if (type == GENERIC) {
        FILE_LOG(logERROR) << "Could not connect to Detector " << hostname
                           << " to determine the type!";
        setErrorMask(getErrorMask() | MULTI_DETECTORS_NOT_ADDED);
        appendNotAddedList(hostname.c_str());
        return;
    }

    int pos = (int)detectors.size();
    detectors.push_back(sls::make_unique<slsDetector>(type, detId, pos, false));
    multi_shm()->numberOfDetectors = detectors.size();
    multi_shm()->dataBytes += detectors[pos]->getDataBytes();
    multi_shm()->dataBytesInclGapPixels +=
        detectors[pos]->getDataBytesInclGapPixels();
    multi_shm()->numberOfChannels +=
        detectors[pos]->getTotalNumberOfChannels();

    detectors[pos]->setHostname(hostname);
    detectors[pos]->setOnline(true);
}

slsDetectorDefs::detectorType multiSlsDetector::getDetectorTypeAsEnum(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getDetectorTypeAsEnum();
    }

    // multi
    auto r = serialCall(&slsDetector::getDetectorTypeAsEnum);
    return (detectorType)sls::minusOneIfDifferent(r);
}

std::string multiSlsDetector::getDetectorTypeAsString(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getDetectorTypeAsString();
    }

    // multi
    auto r = serialCall(&slsDetector::getDetectorTypeAsString);
    return sls::concatenateIfDifferent(r);
}

int multiSlsDetector::getNumberOfDetectors() const {
    return detectors.size();
}

int multiSlsDetector::getNumberOfDetectors(dimension d) const {
    return multi_shm()->numberOfDetector[d];
}

void multiSlsDetector::getNumberOfDetectors(int &nx, int &ny) const {
    nx = multi_shm()->numberOfDetector[X];
    ny = multi_shm()->numberOfDetector[Y];
}

int multiSlsDetector::getTotalNumberOfChannels(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getTotalNumberOfChannels();
    }

    // multi
    return multi_shm()->numberOfChannels;
}

int multiSlsDetector::getTotalNumberOfChannels(dimension d, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getTotalNumberOfChannels(d);
    }

    // multi
    return multi_shm()->numberOfChannel[d];
}

int multiSlsDetector::getTotalNumberOfChannelsInclGapPixels(dimension d,
                                                            int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getTotalNumberOfChannelsInclGapPixels(d);
    }

    // multi
    return multi_shm()->numberOfChannelInclGapPixels[d];
}

int multiSlsDetector::getMaxNumberOfChannelsPerDetector(dimension d) {
    return multi_shm()->maxNumberOfChannelsPerDetector[d];
}

int multiSlsDetector::setMaxNumberOfChannelsPerDetector(dimension d, int i) {
    multi_shm()->maxNumberOfChannelsPerDetector[d] = i;
    return multi_shm()->maxNumberOfChannelsPerDetector[d];
}

int multiSlsDetector::getDetectorOffset(dimension d, int detPos) {
    return detectors[detPos]->getDetectorOffset(d);
}

void multiSlsDetector::setDetectorOffset(dimension d, int off, int detPos) {
    detectors[detPos]->setDetectorOffset(d, off);
}

void multiSlsDetector::updateOffsets() {
    FILE_LOG(logDEBUG1) << "Updating Multi-Detector Offsets";

    int offsetX = 0, offsetY = 0, numX = 0, numY = 0;
    int maxChanX = multi_shm()->maxNumberOfChannelsPerDetector[X];
    int maxChanY = multi_shm()->maxNumberOfChannelsPerDetector[Y];
    int prevChanX = 0;
    int prevChanY = 0;
    bool firstTime = true;

    multi_shm()->numberOfChannel[X] = 0;
    multi_shm()->numberOfChannel[Y] = 0;
    multi_shm()->numberOfDetector[X] = 0;
    multi_shm()->numberOfDetector[Y] = 0;

    // gap pixels
    int offsetX_gp = 0, offsetY_gp = 0, numX_gp = 0, numY_gp = 0;
    int prevChanX_gp = 0, prevChanY_gp = 0;
    multi_shm()->numberOfChannelInclGapPixels[X] = 0;
    multi_shm()->numberOfChannelInclGapPixels[Y] = 0;

    for (size_t idet = 0; idet < detectors.size(); ++idet) {
        FILE_LOG(logDEBUG1) << "offsetX:" << offsetX << " prevChanX:" << prevChanX << " offsetY:" << offsetY << " prevChanY:" << prevChanY << " offsetX_gp:" << offsetX_gp << " prevChanX_gp:" << prevChanX_gp << " offsetY_gp:" << offsetY_gp << " prevChanY_gp:" << prevChanY_gp;

        // incrementing in both direction
        if (firstTime) {
            // incrementing in both directions
            firstTime = false;
            if ((maxChanX > 0) &&
                ((offsetX + detectors[idet]->getTotalNumberOfChannels(X)) > maxChanX)) {
                FILE_LOG(logWARNING) << "\nDetector[" << idet
                                     << "] exceeds maximum channels "
                                        "allowed for complete detector set in X dimension!";
            }
            if ((maxChanY > 0) &&
                ((offsetY + detectors[idet]->getTotalNumberOfChannels(Y)) > maxChanY)) {
                FILE_LOG(logERROR) << "\nDetector[" << idet
                                   << "] exceeds maximum channels "
                                      "allowed for complete detector set in Y dimension!";
            }
            prevChanX = detectors[idet]->getTotalNumberOfChannels(X);
            prevChanY = detectors[idet]->getTotalNumberOfChannels(Y);
            prevChanX_gp =
                detectors[idet]->getTotalNumberOfChannelsInclGapPixels(X);
            prevChanY_gp =
                detectors[idet]->getTotalNumberOfChannelsInclGapPixels(Y);
            numX += detectors[idet]->getTotalNumberOfChannels(X);
            numY += detectors[idet]->getTotalNumberOfChannels(Y);
            numX_gp +=
                detectors[idet]->getTotalNumberOfChannelsInclGapPixels(X);
            numY_gp +=
                detectors[idet]->getTotalNumberOfChannelsInclGapPixels(Y);
            ++multi_shm()->numberOfDetector[X];
            ++multi_shm()->numberOfDetector[Y];
            FILE_LOG(logDEBUG1) << "incrementing in both direction";
        }

        // incrementing in y direction
        else if ((maxChanY == -1) ||
                 ((maxChanY > 0) && ((offsetY + prevChanY +
                                      detectors[idet]->getTotalNumberOfChannels(
                                          Y)) <= maxChanY))) {
            offsetY += prevChanY;
            offsetY_gp += prevChanY_gp;
            prevChanY = detectors[idet]->getTotalNumberOfChannels(Y);
            prevChanY_gp =
                detectors[idet]->getTotalNumberOfChannelsInclGapPixels(Y);
            numY += detectors[idet]->getTotalNumberOfChannels(Y);
            numY_gp +=
                detectors[idet]->getTotalNumberOfChannelsInclGapPixels(Y);
            // increment in y again only in the first column (else you double increment)
            if (multi_shm()->numberOfDetector[X] == 1)
                ++multi_shm()->numberOfDetector[Y];
            FILE_LOG(logDEBUG1) << "incrementing in y direction";
        }

        // incrementing in x direction
        else {
            if ((maxChanX > 0) &&
                ((offsetX + prevChanX +
                  detectors[idet]->getTotalNumberOfChannels(X)) > maxChanX)) {
                FILE_LOG(logDEBUG1) << "\nDetector[" << idet
                                    << "] exceeds maximum channels "
                                       "allowed for complete detector set in X dimension!";
            }
            offsetY = 0;
            offsetY_gp = 0;
            prevChanY = detectors[idet]->getTotalNumberOfChannels(Y);
            prevChanY_gp =
                detectors[idet]->getTotalNumberOfChannelsInclGapPixels(Y);
            numY = 0; // assuming symmetry with this statement.
            // whats on 1st column should be on 2nd column
            numY_gp = 0;
            offsetX += prevChanX;
            offsetX_gp += prevChanX_gp;
            prevChanX = detectors[idet]->getTotalNumberOfChannels(X);
            prevChanX_gp =
                detectors[idet]->getTotalNumberOfChannelsInclGapPixels(X);
            numX += detectors[idet]->getTotalNumberOfChannels(X);
            numX_gp +=
                detectors[idet]->getTotalNumberOfChannelsInclGapPixels(X);
            ++multi_shm()->numberOfDetector[X];
            FILE_LOG(logDEBUG1) << "incrementing in x direction";
        }

        double bytesperchannel =
            (double)detectors[idet]->getDataBytes() /
            (double)(detectors[idet]->getTotalNumberOfChannels(X) *
                     detectors[idet]->getTotalNumberOfChannels(Y));
        detectors[idet]->setDetectorOffset(
            X, (bytesperchannel >= 1.0) ? offsetX_gp : offsetX);
        detectors[idet]->setDetectorOffset(
            Y, (bytesperchannel >= 1.0) ? offsetY_gp : offsetY);

        FILE_LOG(logDEBUG1) << "Detector[" << idet << "] has offsets ("
                            << detectors[idet]->getDetectorOffset(X) << ", "
                            << detectors[idet]->getDetectorOffset(Y) << ")";
        // offsetY has been reset sometimes and offsetX the first time,
        // but remember the highest values
        if (numX > multi_shm()->numberOfChannel[X]) {
            multi_shm()->numberOfChannel[X] = numX;
        }
        if (numY > multi_shm()->numberOfChannel[Y]) {
            multi_shm()->numberOfChannel[Y] = numY;
        }
        if (numX_gp > multi_shm()->numberOfChannelInclGapPixels[X]) {
            multi_shm()->numberOfChannelInclGapPixels[X] = numX_gp;
        }
        if (numY_gp > multi_shm()->numberOfChannelInclGapPixels[Y]) {
            multi_shm()->numberOfChannelInclGapPixels[Y] = numY_gp;
        }
    }
    FILE_LOG(logDEBUG1) << "\n\tNumber of Channels in X direction:" << multi_shm()->numberOfChannel[X] << "\n\tNumber of Channels in Y direction:" << multi_shm()->numberOfChannel[Y] << "\n\tNumber of Channels in X direction with Gap Pixels:" << multi_shm()->numberOfChannelInclGapPixels[X] << "\n\tNumber of Channels in Y direction with Gap Pixels:" << multi_shm()->numberOfChannelInclGapPixels[Y];

    multi_shm()->numberOfChannels =
        multi_shm()->numberOfChannel[0] *
        multi_shm()->numberOfChannel[1];

    for (auto &d : detectors) {
        d->updateMultiSize(multi_shm()->numberOfDetector[0],
                           multi_shm()->numberOfDetector[1]);
    }
}

int multiSlsDetector::setOnline(int value, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setOnline(value);
    }

    // multi
    if (value != GET_ONLINE_FLAG) {
        auto r = parallelCall(&slsDetector::setOnline, value);
        multi_shm()->onlineFlag = sls::minusOneIfDifferent(r);
    }
    return multi_shm()->onlineFlag;
}

std::string multiSlsDetector::checkOnline(int detPos) {
    if (detPos >= 0) {
        return detectors[detPos]->checkOnline();
    }

    auto r = parallelCall(&slsDetector::checkOnline);
    return sls::concatenateNonEmptyStrings(r);
}

int multiSlsDetector::setControlPort(int port_number, int detPos) {
    if (detPos >= 0) {
        return detectors[detPos]->setControlPort(port_number);
    }

    auto r = serialCall(&slsDetector::setControlPort, port_number);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setStopPort(int port_number, int detPos) {
    if (detPos >= 0) {
        return detectors[detPos]->setStopPort(port_number);
    }

    auto r = serialCall(&slsDetector::setStopPort, port_number);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setReceiverPort(int port_number, int detPos) {
    if (detPos >= 0) {
        return detectors[detPos]->setReceiverPort(port_number);
    }

    auto r = serialCall(&slsDetector::setReceiverPort, port_number);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::lockServer(int p, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->lockServer(p);
    }

    // multi
    auto r = parallelCall(&slsDetector::lockServer, p);
    return sls::minusOneIfDifferent(r);
}

std::string multiSlsDetector::getLastClientIP(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getLastClientIP();
    }

    // multi
    auto r = parallelCall(&slsDetector::getLastClientIP);
    return sls::concatenateIfDifferent(r);
}

int multiSlsDetector::exitServer(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->exitServer();
    }

    // multi
    auto r = parallelCall(&slsDetector::exitServer);
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

int multiSlsDetector::execCommand(const std::string &cmd, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->execCommand(cmd);
    }

    // multi
    auto r = parallelCall(&slsDetector::execCommand, cmd);
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

int multiSlsDetector::readConfigurationFile(const std::string &fname) {
    freeSharedMemory();
    setupMultiDetector();
    FILE_LOG(logINFO) << "Loading configuration file: " << fname;

    std::ifstream input_file;
    input_file.open(fname, std::ios_base::in);
    if (input_file.is_open()) {
        std::string current_line;
        while (input_file.good()) {
            getline(input_file, current_line);
            if (current_line.find('#') != std::string::npos) {
                current_line.erase(current_line.find('#'));
            }
            FILE_LOG(logDEBUG1) << "current_line after removing comments:\n\t" << current_line;
            if (current_line.length() > 1) {
                multiSlsDetectorClient(current_line, PUT_ACTION, this);
            }
        }
        input_file.close();
    } else {
        FILE_LOG(logERROR) << "Could not openconfiguration file " << fname << " for reading";
        setErrorMask(getErrorMask() | MULTI_CONFIG_FILE_ERROR);
        return FAIL;
    }

    if (getErrorMask()) {
        int c;
        FILE_LOG(logERROR) << "----------------\n Error Messages\n----------------";
        FILE_LOG(logERROR) << getErrorMessage(c);
        return FAIL;
    }
    return OK;
}

int multiSlsDetector::writeConfigurationFile(const std::string &fname) {
    const std::vector<std::string> names = {"detsizechan", "hostname", "outdir",
                                            "threaded"};

    char *args[100];
    for (auto &arg : args) {
        arg = new char[1000];
    }
    int ret = OK, ret1 = OK;
    std::ofstream outfile;
    size_t iline = 0;

    outfile.open(fname.c_str(), std::ios_base::out);
    if (outfile.is_open()) {
        auto cmd = slsDetectorCommand(this);

        // complete size of detector
        FILE_LOG(logINFO) << "Command to write: " << iline << " " << names[iline];
        strcpy(args[0], names[iline].c_str());
        outfile << names[iline] << " " << cmd.executeLine(1, args, GET_ACTION)
                << std::endl;
        ++iline;

        // hostname of the detectors
        FILE_LOG(logINFO) << "Command to write: " << iline << " " << names[iline];
        strcpy(args[0], names[iline].c_str());
        outfile << names[iline] << " " << cmd.executeLine(1, args, GET_ACTION)
                << std::endl;
        ++iline;

        // single detector configuration
        for (size_t idet = 0; idet < detectors.size(); ++idet) {
            outfile << std::endl;
            ret1 = detectors[idet]->writeConfigurationFile(outfile, this);
            if (detectors[idet]->getErrorMask()) {
                setErrorMask(getErrorMask() | (1 << idet));
            }
            if (ret1 == FAIL) {
                ret = FAIL;
            }
        }

        outfile << std::endl;
        // other configurations
        while (iline < names.size()) {
            FILE_LOG(logINFO) << "Command to write:" << iline << " " << names[iline];
            strcpy(args[0], names[iline].c_str());
            outfile << names[iline] << " "
                    << cmd.executeLine(1, args, GET_ACTION) << std::endl;
            ++iline;
        }
        outfile.close();
        FILE_LOG(logDEBUG1) << "wrote " << iline << " lines to configuration file ";
    } else {
        FILE_LOG(logERROR) << "Could not open configuration file " << fname << " for writing";
        setErrorMask(getErrorMask() | MULTI_CONFIG_FILE_ERROR);
        ret = FAIL;
    }

    for (auto &arg : args) {
        delete[] arg;
    }

    return ret;
}

std::string multiSlsDetector::getSettingsFile(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getSettingsFile();
    }

    // multi
    auto r = serialCall(&slsDetector::getSettingsFile);
    return sls::concatenateIfDifferent(r);
}

slsDetectorDefs::detectorSettings multiSlsDetector::getSettings(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getSettings();
    }

    // multi
    auto r = parallelCall(&slsDetector::getSettings);
    return (detectorSettings)sls::minusOneIfDifferent(r);
}

slsDetectorDefs::detectorSettings
multiSlsDetector::setSettings(detectorSettings isettings, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setSettings(isettings);
    }

    // multi
    auto r = parallelCall(&slsDetector::setSettings, isettings);
    return (detectorSettings)sls::minusOneIfDifferent(r);
}

int multiSlsDetector::getThresholdEnergy(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getThresholdEnergy();
    }

    // multi
    auto r = parallelCall(&slsDetector::getThresholdEnergy);
    if (sls::allEqualWithTol(r, 200)) {
        return r.front();
    }
    return -1;
}

int multiSlsDetector::setThresholdEnergy(int e_eV, detectorSettings isettings,
                                         int tb, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setThresholdEnergy(e_eV, isettings, tb);
    }

    // multi
    auto r =
        parallelCall(&slsDetector::setThresholdEnergy, e_eV, isettings, tb);
    if (sls::allEqualWithTol(r, 200)) {
        return r.front();
    }
    return -1;
}

std::string multiSlsDetector::getSettingsDir(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getSettingsDir();
    }

    // multi
    auto r = serialCall(&slsDetector::getSettingsDir);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::setSettingsDir(const std::string &directory,
                                             int detPos) {
    if (detPos >= 0) {
        return detectors[detPos]->setSettingsDir(directory);
    }

    auto r = parallelCall(&slsDetector::setSettingsDir, directory);
    return sls::concatenateIfDifferent(r);
}

int multiSlsDetector::loadSettingsFile(const std::string &fname, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->loadSettingsFile(fname);
    }

    // multi
    auto r = parallelCall(&slsDetector::loadSettingsFile, fname);
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

int multiSlsDetector::saveSettingsFile(const std::string &fname, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->saveSettingsFile(fname);
    }

    // multi
    auto r = parallelCall(&slsDetector::saveSettingsFile, fname);
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

slsDetectorDefs::runStatus multiSlsDetector::getRunStatus(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getRunStatus();
    }

    // multi
    auto r = parallelCall(&slsDetector::getRunStatus);
    if (sls::allEqual(r)) {
        return r.front();
    }
    if (sls::anyEqualTo(r, ERROR)) {
        return ERROR;
    }
    for (const auto &value : r) {
        if (value != IDLE) {
            return value;
        }
    }
    return IDLE;
}

int multiSlsDetector::prepareAcquisition(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->prepareAcquisition();
    }

    // multi
    auto r = parallelCall(&slsDetector::prepareAcquisition);
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

int multiSlsDetector::startAcquisition(int detPos) {
    // single
    if (detPos >= 0) {
        if (detectors[detPos]->getDetectorTypeAsEnum() == EIGER) {
            if (detectors[detPos]->prepareAcquisition() == FAIL) {
                return FAIL;
            }
        }
        return detectors[detPos]->startAcquisition();
    }

    // multi
    if (getDetectorTypeAsEnum() == EIGER) {
        if (prepareAcquisition() == FAIL) {
            return FAIL;
        }
    }
    auto r = parallelCall(&slsDetector::startAcquisition);
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

int multiSlsDetector::stopAcquisition(int detPos) {
    // locks to synchronize using client->receiver simultaneously (processing
    // thread)
    std::lock_guard<std::mutex> lock(mg);
    if (detPos >= 0) {
        // if only 1 detector, set flag to stop current acquisition
        if (detectors.size() == 1) {
            multi_shm()->stoppedFlag = 1;
        }

        return detectors[detPos]->stopAcquisition();
    } else {
        multi_shm()->stoppedFlag = 1;
        auto r = parallelCall(&slsDetector::stopAcquisition);
        return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
    }
}

int multiSlsDetector::sendSoftwareTrigger(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->sendSoftwareTrigger();
    }

    // multi
    auto r = parallelCall(&slsDetector::sendSoftwareTrigger);
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

int multiSlsDetector::startAndReadAll(int detPos) {
    // single
    if (detPos >= 0) {
        if (detectors[detPos]->getDetectorTypeAsEnum() == EIGER) {
            if (detectors[detPos]->prepareAcquisition() == FAIL) {
                return FAIL;
            }
        }
        return detectors[detPos]->startAndReadAll();
    }

    // multi
    if (getDetectorTypeAsEnum() == EIGER) {
        if (prepareAcquisition() == FAIL) {
            return FAIL;
        }
    }
    auto r = parallelCall(&slsDetector::startAndReadAll);
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

int multiSlsDetector::startReadOut(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->startReadOut();
    }

    // multi
    auto r = parallelCall(&slsDetector::startReadOut);
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

int multiSlsDetector::readAll(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->readAll();
    }

    // multi
    auto r = parallelCall(&slsDetector::readAll);
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

int multiSlsDetector::configureMAC(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->configureMAC();
    }

    // multi
    auto r = parallelCall(&slsDetector::configureMAC);
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

int64_t multiSlsDetector::setTimer(timerIndex index, int64_t t, int detPos) {
    // single
    if (detPos >= 0) {

        // error for setting values individually
        // FIXME: what else? and error code
        if (t != -1) {
            switch (index) {
            case FRAME_NUMBER:
            case CYCLES_NUMBER:
            case STORAGE_CELL_NUMBER:
            case MEASUREMENTS_NUMBER:
                FILE_LOG(logERROR)
                    << "Cannot set number of frames, cycles, "
                       "storage cells or measurements individually.";
                setErrorMask(getErrorMask() | MUST_BE_MULTI_CMD);
                return multi_shm()->timerValue[index];
            default:
                break;
            }
        }

        return detectors[detPos]->setTimer(index, t);
    }

    // multi
    auto r = parallelCall(&slsDetector::setTimer, index, t);
    int64_t ret = sls::minusOneIfDifferent(r);

    if (index == SAMPLES) {
        setDynamicRange();
    }

    // set progress
    if (t != -1) {
        switch (index) {
        case FRAME_NUMBER:
        case CYCLES_NUMBER:
        case STORAGE_CELL_NUMBER:
        case MEASUREMENTS_NUMBER:
            setTotalProgress();
            break;
        default:
            break;
        }
    }

    multi_shm()->timerValue[index] = ret;
    return ret;
}

double multiSlsDetector::setExposureTime(double t, bool inseconds, int detPos) {
    if (!inseconds) {
        return setTimer(ACQUISITION_TIME, (int64_t)t, detPos);
    }

    // + 0.5 to round for precision lost from converting double to int64_t
    int64_t tms = (int64_t)(t * (1E+9) + 0.5);
    if (t < 0) {
        tms = -1;
    }
    tms = setTimer(ACQUISITION_TIME, tms, detPos);
    if (tms < 0) {
        return -1;
    }
    return ((1E-9) * (double)tms);
}

double multiSlsDetector::setExposurePeriod(double t, bool inseconds,
                                           int detPos) {
    if (!inseconds) {
        return setTimer(FRAME_PERIOD, (int64_t)t, detPos);
    }

    // + 0.5 to round for precision lost from converting double to int64_t
    int64_t tms = (int64_t)(t * (1E+9) + 0.5);
    if (t < 0) {
        tms = -1;
    }
    tms = setTimer(FRAME_PERIOD, tms, detPos);
    if (tms < 0) {
        return -1;
    }
    return ((1E-9) * (double)tms);
}

double multiSlsDetector::setDelayAfterTrigger(double t, bool inseconds,
                                              int detPos) {
    if (!inseconds) {
        return setTimer(DELAY_AFTER_TRIGGER, (int64_t)t, detPos);
    }

    // + 0.5 to round for precision lost from converting double to int64_t
    int64_t tms = (int64_t)(t * (1E+9) + 0.5);
    if (t < 0) {
        tms = -1;
    }
    tms = setTimer(DELAY_AFTER_TRIGGER, tms, detPos);
    if (tms < 0) {
        return -1;
    }
    return ((1E-9) * (double)tms);
}

double multiSlsDetector::setSubFrameExposureTime(double t, bool inseconds,
                                                 int detPos) {
    if (!inseconds) {
        return setTimer(SUBFRAME_ACQUISITION_TIME, (int64_t)t, detPos);
    } else {
        // + 0.5 to round for precision lost from converting double to int64_t
        int64_t tms = (int64_t)(t * (1E+9) + 0.5);
        if (t < 0) {
            tms = -1;
        }
        tms = setTimer(SUBFRAME_ACQUISITION_TIME, tms, detPos);
        if (tms < 0) {
            return -1;
        }
        return ((1E-9) * (double)tms);
    }
}

double multiSlsDetector::setSubFrameExposureDeadTime(double t, bool inseconds,
                                                     int detPos) {
    if (!inseconds) {
        return setTimer(SUBFRAME_DEADTIME, (int64_t)t, detPos);
    } else {
        // + 0.5 to round for precision lost from converting double to int64_t
        int64_t tms = (int64_t)(t * (1E+9) + 0.5);
        if (t < 0) {
            tms = -1;
        }
        tms = setTimer(SUBFRAME_DEADTIME, tms, detPos);
        if (tms < 0) {
            return -1;
        }
        return ((1E-9) * (double)tms);
    }
}

int64_t multiSlsDetector::setNumberOfFrames(int64_t t, int detPos) {
    return setTimer(FRAME_NUMBER, t, detPos);
}

int64_t multiSlsDetector::setNumberOfCycles(int64_t t, int detPos) {
    return setTimer(CYCLES_NUMBER, t, detPos);
}

int64_t multiSlsDetector::setNumberOfGates(int64_t t, int detPos) {
    return setTimer(GATES_NUMBER, t, detPos);
}

int64_t multiSlsDetector::setNumberOfStorageCells(int64_t t, int detPos) {
    return setTimer(STORAGE_CELL_NUMBER, t, detPos);
}

double multiSlsDetector::getMeasuredPeriod(bool inseconds, int detPos) {
    if (!inseconds) {
        return getTimeLeft(MEASURED_PERIOD, detPos);
    } else {
        int64_t tms = getTimeLeft(MEASURED_PERIOD, detPos);
        if (tms < 0) {
            return -1;
        }
        return ((1E-9) * (double)tms);
    }
}

double multiSlsDetector::getMeasuredSubFramePeriod(bool inseconds, int detPos) {
    if (!inseconds) {
        return getTimeLeft(MEASURED_SUBPERIOD, detPos);
    } else {
        int64_t tms = getTimeLeft(MEASURED_SUBPERIOD, detPos);
        if (tms < 0) {
            return -1;
        }
        return ((1E-9) * (double)tms);
    }
}

int64_t multiSlsDetector::getTimeLeft(timerIndex index, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getTimeLeft(index);
    }

    // multi
    auto r = parallelCall(&slsDetector::getTimeLeft, index);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setSpeed(speedVariable index, int value, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setSpeed(index, value);
    }

    // multi
    auto r = parallelCall(&slsDetector::setSpeed, index, value);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setDynamicRange(int dr, int detPos) {
    // single
    if (detPos >= 0) {
        FILE_LOG(logERROR) << "Dynamic Range cannot be set individually";
        setErrorMask(getErrorMask() | MUST_BE_MULTI_CMD);
        return -1;
    }

    // multi
    auto r = parallelCall(&slsDetector::setDynamicRange, dr);
    int ret = sls::minusOneIfDifferent(r);

    // update shm
    int prevValue = multi_shm()->dataBytes;
    int prevGValue = multi_shm()->dataBytesInclGapPixels;
    multi_shm()->dataBytes = 0;
    multi_shm()->dataBytesInclGapPixels = 0;
    multi_shm()->numberOfChannels = 0;
    for (auto &d : detectors) {
        multi_shm()->dataBytes += d->getDataBytes();
        multi_shm()->dataBytesInclGapPixels +=
            d->getDataBytesInclGapPixels();
        multi_shm()->numberOfChannels += d->getTotalNumberOfChannels();
    }

    // for usability
    if (getDetectorTypeAsEnum() == EIGER) {
        switch (dr) {
        case 32:
            FILE_LOG(logINFO) << "Setting Clock to Quarter Speed to cope with "
                                 "Dynamic Range of 32";
            setSpeed(CLOCK_DIVIDER, 2);
            break;
        case 16:
            FILE_LOG(logINFO)
                << "Setting Clock to Half Speed for Dynamic Range of 16";
            setSpeed(CLOCK_DIVIDER, 1);
            break;
        default:
            break;
        }
    }

    // update offsets if there was a change FIXME:add dr to sls shm and check
    // that instead
    if ((prevValue != multi_shm()->dataBytes) ||
        (prevGValue != multi_shm()->dataBytesInclGapPixels)) {
        updateOffsets();
    }

    return ret;
}

int multiSlsDetector::getDataBytes(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getDataBytes();
    }

    // multi
    auto r = parallelCall(&slsDetector::getDataBytes);
    return sls::sum(r);
}

int multiSlsDetector::setDAC(int val, dacIndex index, int mV, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setDAC(val, index, mV);
    }

    // multi
    auto r = parallelCall(&slsDetector::setDAC, val, index, mV);
    if (getDetectorTypeAsEnum() != EIGER && index != HIGH_VOLTAGE) {
        return sls::minusOneIfDifferent(r);
    }

    // ignore slave values for hv (-999)
    int firstValue = r.front();
    for (const auto &value : r) {
        if ((value != -999) && (value != firstValue)) {
            return -1;
        }
    }

    return firstValue;
}

int multiSlsDetector::getADC(dacIndex index, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getADC(index);
    }

    // multi
    auto r = parallelCall(&slsDetector::getADC, index);
    return sls::minusOneIfDifferent(r);
}

slsDetectorDefs::externalCommunicationMode
multiSlsDetector::setExternalCommunicationMode(externalCommunicationMode pol,
                                               int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setExternalCommunicationMode(pol);
    }

    // multi
    auto r = parallelCall(&slsDetector::setExternalCommunicationMode, pol);
    return sls::minusOneIfDifferent(r);
}

slsDetectorDefs::externalSignalFlag
multiSlsDetector::setExternalSignalFlags(externalSignalFlag pol,
                                         int signalindex, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setExternalSignalFlags(pol, signalindex);
    }

    // multi
    auto r =
        parallelCall(&slsDetector::setExternalSignalFlags, pol, signalindex);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setReadOutFlags(readOutFlags flag, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setReadOutFlags(flag);
    }

    // multi
    auto r = parallelCall(&slsDetector::setReadOutFlags, flag);
    return sls::minusOneIfDifferent(r);
}

uint32_t multiSlsDetector::writeRegister(uint32_t addr, uint32_t val,
                                         int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->writeRegister(addr, val);
    }

    // multi
    auto r = parallelCall(&slsDetector::writeRegister, addr, val);
    if (sls::allEqual(r)) {
        return r.front();
    }

    // can't have different values
    FILE_LOG(logERROR) << "Error: Different Values for function writeRegister "
                          "(write 0x"
                       << std::hex << val << " to addr 0x" << std::hex << addr
                       << std::dec << ")";
    setErrorMask(getErrorMask() | MULTI_HAVE_DIFFERENT_VALUES);
    return -1;
}

uint32_t multiSlsDetector::readRegister(uint32_t addr, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->readRegister(addr);
    }

    // multi
    auto r = parallelCall(&slsDetector::readRegister, addr);
    if (sls::allEqual(r)) {
        return r.front();
    }

    // can't have different values
    FILE_LOG(logERROR) << "Error: Different Values for function readRegister "
                          "(read from 0x"
                       << std::hex << addr << std::dec << ")";
    setErrorMask(getErrorMask() | MULTI_HAVE_DIFFERENT_VALUES);
    return -1;
}

uint32_t multiSlsDetector::setBit(uint32_t addr, int n, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setBit(addr, n);
    }

    // multi
    auto r = parallelCall(&slsDetector::setBit, addr, n);
    if (sls::allEqual(r)) {
        return r.front();
    }

    // can't have different values
    FILE_LOG(logERROR) << "Error: Different Values for function setBit "
                          "(set bit "
                       << n << " to addr 0x" << std::hex << addr << std::dec
                       << ")";
    setErrorMask(getErrorMask() | MULTI_HAVE_DIFFERENT_VALUES);
    return -1;
}

uint32_t multiSlsDetector::clearBit(uint32_t addr, int n, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->clearBit(addr, n);
    }

    // multi
    auto r = parallelCall(&slsDetector::clearBit, addr, n);
    if (sls::allEqual(r)) {
        return r.front();
    }

    // can't have different values
    FILE_LOG(logERROR) << "Error: Different Values for function clearBit "
                          "(clear bit "
                       << n << " to addr 0x" << std::hex << addr << std::dec
                       << ")";
    setErrorMask(getErrorMask() | MULTI_HAVE_DIFFERENT_VALUES);
    return -1;
}

std::string multiSlsDetector::setDetectorMAC(const std::string &detectorMAC, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setDetectorMAC(detectorMAC);
    }

    // multi
    auto r = parallelCall(&slsDetector::setDetectorMAC, detectorMAC);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::getDetectorMAC(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getDetectorMAC();
    }

    // multi
    auto r = serialCall(&slsDetector::getDetectorMAC);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::setDetectorIP(const std::string &detectorIP, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setDetectorIP(detectorIP);
    }

    // multi
    auto r = parallelCall(&slsDetector::setDetectorIP, detectorIP);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::getDetectorIP(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getDetectorIP();
    }

    // multi
    auto r = serialCall(&slsDetector::getDetectorIP);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::setReceiver(const std::string &receiver, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setReceiver(receiver);
    }

    // multi
    auto r = parallelCall(&slsDetector::setReceiver, receiver);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::getReceiver(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getReceiver();
    }

    // multi
    auto r = serialCall(&slsDetector::getReceiver);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::setReceiverUDPIP(const std::string &udpip, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setReceiverUDPIP(udpip);
    }

    // multi
    auto r = parallelCall(&slsDetector::setReceiverUDPIP, udpip);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::getReceiverUDPIP(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getReceiverUDPIP();
    }

    // multi
    auto r = serialCall(&slsDetector::getReceiverUDPIP);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::setReceiverUDPMAC(const std::string &udpmac, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setReceiverUDPMAC(udpmac);
    }

    // multi
    auto r = parallelCall(&slsDetector::setReceiverUDPMAC, udpmac);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::getReceiverUDPMAC(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getReceiverUDPMAC();
    }

    // multi
    auto r = serialCall(&slsDetector::getReceiverUDPMAC);
    return sls::concatenateIfDifferent(r);
}

int multiSlsDetector::setReceiverUDPPort(int udpport, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setReceiverUDPPort(udpport);
    }

    // multi
    auto r = parallelCall(&slsDetector::setReceiverUDPPort, udpport);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::getReceiverUDPPort(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getReceiverUDPPort();
    }

    // multi
    auto r = serialCall(&slsDetector::getReceiverUDPPort);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setReceiverUDPPort2(int udpport, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setReceiverUDPPort2(udpport);
    }

    // multi
    auto r = parallelCall(&slsDetector::setReceiverUDPPort2, udpport);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::getReceiverUDPPort2(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getReceiverUDPPort2();
    }

    // multi
    auto r = serialCall(&slsDetector::getReceiverUDPPort2);
    return sls::minusOneIfDifferent(r);
}

void multiSlsDetector::setClientDataStreamingInPort(int i, int detPos) {
    if (i >= 0) {
        int prev_streaming = enableDataStreamingToClient();

        // single
        if (detPos >= 0) {
            detectors[detPos]->setClientStreamingPort(i);
        }
        // multi
        else {
            // calculate ports individually
            int firstPort = i;
            int numSockets = (getDetectorTypeAsEnum() == EIGER) ? 2 : 1;

            for (size_t idet = 0; idet < detectors.size(); ++idet) {
                auto port = firstPort + (idet * numSockets);
                detectors[idet]->setClientStreamingPort(port);
            }
        }

        if (prev_streaming) {
            enableDataStreamingToClient(0);
            enableDataStreamingToClient(1);
        }
    }
}

int multiSlsDetector::getClientStreamingPort(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getClientStreamingPort();
    }

    // multi
    auto r = serialCall(&slsDetector::getClientStreamingPort);
    return sls::minusOneIfDifferent(r);
}

void multiSlsDetector::setReceiverDataStreamingOutPort(int i, int detPos) {
    if (i >= 0) {
        int prev_streaming = enableDataStreamingFromReceiver(-1, detPos);

        // single
        if (detPos >= 0) {
            detectors[detPos]->setReceiverStreamingPort(i);
        }
        // multi
        else {
            // calculate ports individually
            int firstPort = i;
            int numSockets = (getDetectorTypeAsEnum() == EIGER) ? 2 : 1;

            for (size_t idet = 0; idet < detectors.size(); ++idet) {
                auto port = firstPort + (idet * numSockets);
                detectors[idet]->setReceiverStreamingPort(port);
            }
        }

        if (prev_streaming) {
            enableDataStreamingFromReceiver(0, detPos);
            enableDataStreamingFromReceiver(1, detPos);
        }
    }
}

int multiSlsDetector::getReceiverStreamingPort(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getReceiverStreamingPort();
    }

    // multi
    auto r = serialCall(&slsDetector::getReceiverStreamingPort);
    return sls::minusOneIfDifferent(r);
}

void multiSlsDetector::setClientDataStreamingInIP(const std::string &ip,
                                                  int detPos) {
    if (ip.length()) {
        int prev_streaming = enableDataStreamingToClient(-1);

        // single
        if (detPos >= 0) {
            detectors[detPos]->setClientStreamingIP(ip);
        }
        // multi
        else {
            for (auto &d : detectors) {
                d->setClientStreamingIP(ip);
            }
        }

        if (prev_streaming) {
            enableDataStreamingToClient(0);
            enableDataStreamingToClient(1);
        }
    }
}

std::string multiSlsDetector::getClientStreamingIP(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getClientStreamingIP();
    }

    // multi
    auto r = serialCall(&slsDetector::getClientStreamingIP);
    return sls::concatenateIfDifferent(r);
}

void multiSlsDetector::setReceiverDataStreamingOutIP(const std::string &ip,
                                                     int detPos) {
    if (ip.length()) {
        int prev_streaming = enableDataStreamingFromReceiver(-1, detPos);

        // single
        if (detPos >= 0) {
            detectors[detPos]->setReceiverStreamingIP(ip);
        }
        // multi
        else {
            for (auto &d : detectors) {
                d->setReceiverStreamingIP(ip);
            }
        }

        if (prev_streaming) {
            enableDataStreamingFromReceiver(0, detPos);
            enableDataStreamingFromReceiver(1, detPos);
        }
    }
}

std::string multiSlsDetector::getReceiverStreamingIP(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getReceiverStreamingIP();
    }

    // multi
    auto r = serialCall(&slsDetector::getReceiverStreamingIP);
    return sls::concatenateIfDifferent(r);
}

int multiSlsDetector::setDetectorNetworkParameter(networkParameter index, int delay, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setDetectorNetworkParameter(index, delay);
    }

    // multi
    auto r = parallelCall(&slsDetector::setDetectorNetworkParameter, index, delay);
    return sls::minusOneIfDifferent(r);
}

std::string multiSlsDetector::setAdditionalJsonHeader(const std::string &jsonheader, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setAdditionalJsonHeader(jsonheader);
    }

    // multi
    auto r = parallelCall(&slsDetector::setAdditionalJsonHeader, jsonheader);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::getAdditionalJsonHeader(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getAdditionalJsonHeader();
    }

    // multi
    auto r = serialCall(&slsDetector::getAdditionalJsonHeader);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::setAdditionalJsonParameter(const std::string &key, const std::string &value, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setAdditionalJsonParameter(key, value);
    }

    // multi
    auto r = parallelCall(&slsDetector::setAdditionalJsonParameter, key, value);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::getAdditionalJsonParameter(const std::string &key, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getAdditionalJsonParameter(key);
    }

    // multi
    auto r = serialCall(&slsDetector::getAdditionalJsonParameter, key);
    return sls::concatenateIfDifferent(r);
}

int multiSlsDetector::setDetectorMinMaxEnergyThreshold(const int index, int value, int detPos) {
    std::string parameter = (index ? "emax" : "emin");

    std::string result;
    if (value < 0) {
        result = getAdditionalJsonParameter(parameter, detPos);
    } else {
        result = setAdditionalJsonParameter(parameter, std::to_string(value), detPos);
    }

    // convert to integer
    try {
        return stoi(result);
    }
    // not found or cannot scan integer
    catch (...) {
        return -1;
    }
}

int multiSlsDetector::setFrameMode(frameModeType value, int detPos) {
    std::string parameter = "frameMode";
    std::string result;

    if (value == GET_FRAME_MODE) {
        result = getAdditionalJsonParameter(parameter, detPos);
    } else {
        result = setAdditionalJsonParameter(parameter, getFrameModeType(value), detPos);
    }

    return getFrameModeType(result);
}

int multiSlsDetector::setDetectorMode(detectorModeType value, int detPos) {
    std::string parameter = "detectorMode";
    std::string result;

    if (value == GET_DETECTOR_MODE) {
        result = getAdditionalJsonParameter(parameter, detPos);
    } else {
        result = setAdditionalJsonParameter(parameter, getDetectorModeType(value), detPos);
    }

    return getDetectorModeType(result);
}

int64_t multiSlsDetector::setReceiverUDPSocketBufferSize(int64_t udpsockbufsize, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setReceiverUDPSocketBufferSize(udpsockbufsize);
    }

    // multi
    auto r = parallelCall(&slsDetector::setReceiverUDPSocketBufferSize, udpsockbufsize);
    return sls::minusOneIfDifferent(r);
}

int64_t multiSlsDetector::getReceiverUDPSocketBufferSize(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getReceiverUDPSocketBufferSize();
    }

    // multi
    auto r = serialCall(&slsDetector::getReceiverUDPSocketBufferSize);
    return sls::minusOneIfDifferent(r);
}

int64_t multiSlsDetector::getReceiverRealUDPSocketBufferSize(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getReceiverRealUDPSocketBufferSize();
    }

    // multi
    auto r = serialCall(&slsDetector::getReceiverRealUDPSocketBufferSize);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setFlowControl10G(int enable, int detPos) {
    if (enable != -1) {
        enable = ((enable >= 1) ? 1 : 0);
    }
    return setDetectorNetworkParameter(FLOW_CONTROL_10G, enable, detPos);
}

int multiSlsDetector::digitalTest(digitalTestMode mode, int ival, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->digitalTest(mode, ival);
    }

    // multi
    auto r = parallelCall(&slsDetector::digitalTest, mode, ival);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::loadImageToDetector(imageType index,
                                          const std::string &fname,
                                          int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->loadImageToDetector(index, fname);
    }

    // multi

    // read image for all
    int nch = multi_shm()->numberOfChannels;
    short int imageVals[nch];
    if (readDataFile(fname, imageVals, nch) < nch * (int)sizeof(short int)) {
        FILE_LOG(logERROR) << "Could not open file or not enough data in file "
                              "to load image to detector.";
        setErrorMask(getErrorMask() | MULTI_OTHER_ERROR);
        return -1;
    }

    // send image to all
    std::vector<int> r;
    for (size_t idet = 0; idet < detectors.size(); ++idet) {
        r.push_back(detectors[idet]->sendImageToDetector(
            index,
            imageVals + idet * detectors[idet]->getTotalNumberOfChannels()));
    }
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

int multiSlsDetector::writeCounterBlockFile(const std::string &fname,
                                            int startACQ, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->writeCounterBlockFile(fname, startACQ);
    }

    // multi

    // get image from all
    int nch = multi_shm()->numberOfChannels;
    short int imageVals[nch];
    std::vector<int> r;
    for (size_t idet = 0; idet < detectors.size(); ++idet) {
        r.push_back(detectors[idet]->getCounterBlock(
            imageVals + idet * detectors[idet]->getTotalNumberOfChannels(),
            startACQ));
    }

    // write image if all ok
    if (sls::allEqualTo(r, static_cast<int>(OK))) {
        if (writeDataFile(fname, nch, imageVals) <
            nch * (int)sizeof(short int)) {
            FILE_LOG(logERROR) << "Could not open file to write or did not "
                                  "write enough data in file "
                                  "to wrte counter block file from detector.";
            setErrorMask(getErrorMask() | MULTI_OTHER_ERROR);
            return -1;
        }
        return OK;
    }
    return FAIL;
}

int multiSlsDetector::resetCounterBlock(int startACQ, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->resetCounterBlock(startACQ);
    }

    // multi
    auto r = parallelCall(&slsDetector::resetCounterBlock, startACQ);
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

int multiSlsDetector::setCounterBit(int i, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setCounterBit(i);
    }

    // multi
    auto r = parallelCall(&slsDetector::setCounterBit, i);
    return sls::minusOneIfDifferent(r);
}

void multiSlsDetector::verifyMinMaxROI(int n, ROI r[]) {
    int temp;
    for (int i = 0; i < n; ++i) {
        if ((r[i].xmax) < (r[i].xmin)) {
            temp = r[i].xmax;
            r[i].xmax = r[i].xmin;
            r[i].xmin = temp;
        }
        if ((r[i].ymax) < (r[i].ymin)) {
            temp = r[i].ymax;
            r[i].ymax = r[i].ymin;
            r[i].ymin = temp;
        }
    }
}

int multiSlsDetector::setROI(int n, ROI roiLimits[], int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setROI(n, roiLimits);
    }

    // multi
    int xmin = 0, xmax = 0, ymin = 0, ymax = 0, channelX = 0, channelY = 0, idet = 0, lastChannelX = 0,
        lastChannelY = 0, index = 0, offsetX = 0, offsetY = 0;

    bool invalidroi = false;
    int ndet = detectors.size();
    ROI allroi[ndet][n];
    int nroi[ndet];
    for (int i = 0; i < ndet; ++i) {
        nroi[i] = 0;
    }

    if ((n < 0) || (roiLimits == nullptr)) {
        return FAIL;
    }

    // ensures min < max
    verifyMinMaxROI(n, roiLimits);
    FILE_LOG(logDEBUG1) << "Setting ROI for " << n << "rois:";
    for (int i = 0; i < n; ++i) {
        FILE_LOG(logDEBUG1) << i << ":" << roiLimits[i].xmin << "\t" << roiLimits[i].xmax
                            << "\t" << roiLimits[i].ymin << "\t" << roiLimits[i].ymax;
    }
    // for each roi
    for (int i = 0; i < n; ++i) {
        xmin = roiLimits[i].xmin;
        xmax = roiLimits[i].xmax;
        ymin = roiLimits[i].ymin;
        ymax = roiLimits[i].ymax;

        if (getNumberOfDetectors() > 1) {
            // check roi max values
            idet = decodeNChannel(xmax, ymax, channelX, channelY);
            FILE_LOG(logDEBUG1) << "Decoded Channel max vals: " << std::endl
                                << "det:" << idet << "\t" << xmax << "\t" << ymax << "\t" << channelX << "\t" << channelY;
            if (idet == -1) {
                FILE_LOG(logERROR) << "invalid roi";
                continue;
            }

            // split in x dir
            while (xmin <= xmax) {
                invalidroi = false;
                ymin = roiLimits[i].ymin;
                // split in y dir
                while (ymin <= ymax) {
                    // get offset for each detector
                    idet = decodeNChannel(xmin, ymin, channelX, channelY);
                    FILE_LOG(logDEBUG1) << "Decoded Channel min vals: " << std::endl
                                        << "det:" << idet << "\t" << xmin << "\t" << ymin << "\t" << channelX << "\t" << channelY;
                    if (idet < 0 || idet >= (int)detectors.size()) {
                        FILE_LOG(logDEBUG1) << "invalid roi";
                        invalidroi = true;
                        break;
                    }
                    // get last channel for each det in x and y dir
                    lastChannelX =
                        (detectors[idet]->getTotalNumberOfChannelsInclGapPixels(
                            X)) -
                        1;
                    lastChannelY =
                        (detectors[idet]->getTotalNumberOfChannelsInclGapPixels(
                            Y)) -
                        1;

                    offsetX = detectors[idet]->getDetectorOffset(X);
                    offsetY = detectors[idet]->getDetectorOffset(Y);
                    // at the end in x dir
                    if ((offsetX + lastChannelX) >= xmax) {
                        lastChannelX = xmax - offsetX;
                    }
                    // at the end in y dir
                    if ((offsetY + lastChannelY) >= ymax) {
                        lastChannelY = ymax - offsetY;
                    }

                    FILE_LOG(logDEBUG1) << "lastChannelX:" << lastChannelX << "\t"
                                        << "lastChannelY:" << lastChannelY;

                    // creating the list of roi for corresponding detector
                    index = nroi[idet];
                    allroi[idet][index].xmin = channelX;
                    allroi[idet][index].xmax = lastChannelX;
                    allroi[idet][index].ymin = channelY;
                    allroi[idet][index].ymax = lastChannelY;
                    nroi[idet] = nroi[idet] + 1;

                    ymin = lastChannelY + offsetY + 1;
                    if ((lastChannelY + offsetY) == ymax) {
                        ymin = ymax + 1;
                    }

                    FILE_LOG(logDEBUG1) << "nroi[idet]:" << nroi[idet] << "\tymin:" << ymin;
                }
                if (invalidroi) {
                    break;
                }

                xmin = lastChannelX + offsetX + 1;
                if ((lastChannelX + offsetX) == xmax) {
                    xmin = xmax + 1;
                }
            }
        } else { // FIXME: check if xmax is greater? or reduce logic above?
            idet = 0;
            nroi[idet] = n;
            index = 0;
            allroi[idet][index].xmin = xmin;
            allroi[idet][index].xmax = xmax;
            allroi[idet][index].ymin = ymin;
            allroi[idet][index].ymax = ymax;
            // nroi[idet]               = nroi[idet] + 1;
        }
    }

    FILE_LOG(logDEBUG1) << "Setting ROI :";
    for (size_t i = 0; i < detectors.size(); ++i) {
        FILE_LOG(logDEBUG1) << "detector " << i;
        for (int j = 0; j < nroi[i]; ++j) {
            FILE_LOG(logDEBUG1) << allroi[i][j].xmin << "\t" << allroi[i][j].xmax << "\t"
                                << allroi[i][j].ymin << "\t" << allroi[i][j].ymax;
        }
    }

    // settings the rois for each detector
    std::vector<int> r;
    for (size_t idet = 0; idet < detectors.size(); ++idet) {
        r.push_back(detectors[idet]->setROI(nroi[idet], allroi[idet]));
    }
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

slsDetectorDefs::ROI *multiSlsDetector::getROI(int &n, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getROI(n);
    }

    // multi
    n = 0;
    int num = 0, i, j;
    int ndet = detectors.size();
    int maxroi = ndet * MAX_ROIS;
    ROI temproi;
    ROI roiLimits[maxroi];
    ROI *retval = new ROI[maxroi];
    ROI *temp = nullptr;
    int index = 0;

    // get each detector's roi array
    for (size_t idet = 0; idet < detectors.size(); ++idet) {
        temp = detectors[idet]->getROI(index);
        if (temp) {
            if (index) {
                FILE_LOG(logINFO) << "detector " << idet << ":";
            }
            for (j = 0; j < index; ++j) {
                FILE_LOG(logINFO) << temp[j].xmin << "\t" << temp[j].xmax << "\t"
                                  << temp[j].ymin << "\t" << temp[j].ymax;
                int x = detectors[idet]->getDetectorOffset(X);
                int y = detectors[idet]->getDetectorOffset(Y);
                roiLimits[n].xmin = temp[j].xmin + x;
                roiLimits[n].xmax = temp[j].xmax + x;
                roiLimits[n].ymin = temp[j].ymin + y;
                roiLimits[n].ymax = temp[j].ymin + y;
                ++n;
            }
        }
    }

    // empty roi
    if (!n) {
        return nullptr;
    }

    FILE_LOG(logDEBUG1) << "ROI :" << std::endl;
    for (int j = 0; j < n; ++j) {
        FILE_LOG(logDEBUG1) << roiLimits[j].xmin << "\t" << roiLimits[j].xmax << "\t"
                            << roiLimits[j].ymin << "\t" << roiLimits[j].ymax;
    }

    // combine all the adjacent rois in x direction
    for (i = 0; i < n; ++i) {
        // since the ones combined are replaced by -1
        if ((roiLimits[i].xmin) == -1) {
            continue;
        }
        for (j = i + 1; j < n; ++j) {
            // since the ones combined are replaced by -1
            if ((roiLimits[j].xmin) == -1) {
                continue;
            }
            // if y values are same
            if (((roiLimits[i].ymin) == (roiLimits[j].ymin)) &&
                ((roiLimits[i].ymax) == (roiLimits[j].ymax))) {
                // if adjacent, increase [i] range and replace all [j] with -1
                if ((roiLimits[i].xmax) + 1 == roiLimits[j].xmin) {
                    roiLimits[i].xmax = roiLimits[j].xmax;
                    roiLimits[j].xmin = -1;
                    roiLimits[j].xmax = -1;
                    roiLimits[j].ymin = -1;
                    roiLimits[j].ymax = -1;
                }
                // if adjacent, increase [i] range and replace all [j] with -1
                else if ((roiLimits[i].xmin) - 1 == roiLimits[j].xmax) {
                    roiLimits[i].xmin = roiLimits[j].xmin;
                    roiLimits[j].xmin = -1;
                    roiLimits[j].xmax = -1;
                    roiLimits[j].ymin = -1;
                    roiLimits[j].ymax = -1;
                }
            }
        }
    }

    FILE_LOG(logDEBUG1) << "Combined along x axis Getting ROI :\ndetector " << i;
    for (int j = 0; j < n; ++j) {
        FILE_LOG(logDEBUG1) << roiLimits[j].xmin << "\t" << roiLimits[j].xmax << "\t"
                            << roiLimits[j].ymin << "\t" << roiLimits[j].ymax;
    }

    // combine all the adjacent rois in y direction
    for (i = 0; i < n; ++i) {
        // since the ones combined are replaced by -1
        if ((roiLimits[i].ymin) == -1) {
            continue;
        }
        for (j = i + 1; j < n; ++j) {
            // since the ones combined are replaced by -1
            if ((roiLimits[j].ymin) == -1) {
                continue;
            }
            // if x values are same
            if (((roiLimits[i].xmin) == (roiLimits[j].xmin)) &&
                ((roiLimits[i].xmax) == (roiLimits[j].xmax))) {
                // if adjacent, increase [i] range and replace all [j] with -1
                if ((roiLimits[i].ymax) + 1 == roiLimits[j].ymin) {
                    roiLimits[i].ymax = roiLimits[j].ymax;
                    roiLimits[j].xmin = -1;
                    roiLimits[j].xmax = -1;
                    roiLimits[j].ymin = -1;
                    roiLimits[j].ymax = -1;
                }
                // if adjacent, increase [i] range and replace all [j] with -1
                else if ((roiLimits[i].ymin) - 1 == roiLimits[j].ymax) {
                    roiLimits[i].ymin = roiLimits[j].ymin;
                    roiLimits[j].xmin = -1;
                    roiLimits[j].xmax = -1;
                    roiLimits[j].ymin = -1;
                    roiLimits[j].ymax = -1;
                }
            }
        }
    }

    // get rid of -1s
    for (i = 0; i < n; ++i) {
        if ((roiLimits[i].xmin) != -1) {
            retval[num] = roiLimits[i];
            ++num;
        }
    }
    // sort final roi
    for (i = 0; i < num; ++i) {
        for (j = i + 1; j < num; ++j) {
            if (retval[j].xmin < retval[i].xmin) {
                temproi = retval[i];
                retval[i] = retval[j];
                retval[j] = temproi;
            }
        }
    }
    n = num;

    FILE_LOG(logDEBUG1) << "\nxmin\txmax\tymin\tymax";
    for (i = 0; i < n; ++i) {
        FILE_LOG(logDEBUG1) << retval[i].xmin << "\t" << retval[i].xmax << "\t"
                            << retval[i].ymin << "\t" << retval[i].ymax;
    }
    return retval;
}

int multiSlsDetector::writeAdcRegister(int addr, int val, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->writeAdcRegister(addr, val);
    }

    // multi
    auto r = parallelCall(&slsDetector::writeAdcRegister, addr, val);
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

int multiSlsDetector::activate(int const enable, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->activate(enable);
    }

    // multi
    auto r = parallelCall(&slsDetector::activate, enable);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setDeactivatedRxrPaddingMode(int padding, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setDeactivatedRxrPaddingMode(padding);
    }

    // multi
    auto r = parallelCall(&slsDetector::setDeactivatedRxrPaddingMode, padding);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::getFlippedData(dimension d, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getFlippedData(d);
    }

    // multi
    auto r = serialCall(&slsDetector::getFlippedData, d);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setFlippedData(dimension d, int value, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setFlippedData(d, value);
    }

    // multi
    auto r = parallelCall(&slsDetector::setFlippedData, d, value);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setAllTrimbits(int val, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setAllTrimbits(val);
    }

    // multi
    auto r = parallelCall(&slsDetector::setAllTrimbits, val);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::enableGapPixels(int val, int detPos) {
    if (getDetectorTypeAsEnum() != EIGER) {
        if (val >= 0) {
            FILE_LOG(logERROR) << "Function (enableGapPixels) not implemented "
                                  "for this detector";
            setErrorMask(getErrorMask() | MULTI_OTHER_ERROR);
        }
        return 0;
    }

    // single
    if (detPos >= 0) {
        if (val >= 0) {
            FILE_LOG(logERROR) << "Function (enableGapPixels) must be called "
                                  "from a multi detector level.";
            setErrorMask(getErrorMask() | MUST_BE_MULTI_CMD);
            return -1;
        }
        return detectors[detPos]->enableGapPixels(val);
    }

    // multi
    auto r = parallelCall(&slsDetector::enableGapPixels, val);
    int ret = sls::minusOneIfDifferent(r);

    // update data bytes incl gap pixels
    if (val != -1) {
        auto r = serialCall(&slsDetector::getDataBytesInclGapPixels);
        multi_shm()->dataBytesInclGapPixels = sls::sum(r);

        // update
        updateOffsets();
    }
    return ret;
}

int multiSlsDetector::setTrimEn(int ne, int *ene, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setTrimEn(ne, ene);
    }

    // multi
    auto r = serialCall(&slsDetector::setTrimEn, ne, ene);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::getTrimEn(int *ene, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getTrimEn(ene);
    }

    // multi
    auto r = serialCall(&slsDetector::getTrimEn, ene);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::pulsePixel(int n, int x, int y, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->pulsePixel(n, x, y);
    }

    // multi
    auto r = parallelCall(&slsDetector::pulsePixel, n, x, y);
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

int multiSlsDetector::pulsePixelNMove(int n, int x, int y, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->pulsePixelNMove(n, x, y);
    }

    // multi
    auto r = parallelCall(&slsDetector::pulsePixelNMove, n, x, y);
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

int multiSlsDetector::pulseChip(int n, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->pulseChip(n);
    }

    // multi
    auto r = parallelCall(&slsDetector::pulseChip, n);
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

int multiSlsDetector::setThresholdTemperature(int val, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setThresholdTemperature(val);
    }

    // multi
    auto r = parallelCall(&slsDetector::setThresholdTemperature, val);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setTemperatureControl(int val, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setTemperatureControl(val);
    }

    // multi
    auto r = parallelCall(&slsDetector::setTemperatureControl, val);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setTemperatureEvent(int val, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setTemperatureEvent(val);
    }

    // multi
    auto r = parallelCall(&slsDetector::setTemperatureEvent, val);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setStoragecellStart(int pos, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setStoragecellStart(pos);
    }

    // multi
    auto r = parallelCall(&slsDetector::setStoragecellStart, pos);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::programFPGA(const std::string &fname, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->programFPGA(fname);
    }

    // multi
    auto r = serialCall(&slsDetector::programFPGA, fname);
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

int multiSlsDetector::resetFPGA(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->resetFPGA();
    }

    // multi
    auto r = parallelCall(&slsDetector::resetFPGA);
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

int multiSlsDetector::powerChip(int ival, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->powerChip(ival);
    }

    // multi
    auto r = parallelCall(&slsDetector::powerChip, ival);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setAutoComparatorDisableMode(int ival, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setAutoComparatorDisableMode(ival);
    }

    // multi
    auto r = parallelCall(&slsDetector::setAutoComparatorDisableMode, ival);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::getChanRegs(double *retval, int detPos) {

    int offset = 0;
    std::vector<int> r;
    for (auto &d : detectors) {
        int nch = d->getTotalNumberOfChannels();
        double result[nch];
        r.push_back(d->getChanRegs(result));
        memcpy(retval + offset, result, nch * sizeof(double));
    }
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setRateCorrection(int64_t t, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setRateCorrection(t);
    }

    // multi
    auto r = parallelCall(&slsDetector::setRateCorrection, t);
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

int64_t multiSlsDetector::getRateCorrection(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getRateCorrection();
    }

    // multi
    auto r = parallelCall(&slsDetector::getRateCorrection);
    return sls::minusOneIfDifferent(r);
}

void multiSlsDetector::printReceiverConfiguration(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->printReceiverConfiguration();
    }

    // multi
    for (auto &d : detectors) {
        d->printReceiverConfiguration();
    }
}

int multiSlsDetector::setReceiverOnline(int value, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setReceiverOnline(value);
    }

    // multi
    if (value != GET_ONLINE_FLAG) {
        auto r = parallelCall(&slsDetector::setReceiverOnline, value);
        multi_shm()->receiverOnlineFlag = sls::minusOneIfDifferent(r);
    }
    return multi_shm()->receiverOnlineFlag;
}

std::string multiSlsDetector::checkReceiverOnline(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->checkReceiverOnline();
    }

    // multi
    auto r = parallelCall(&slsDetector::checkReceiverOnline);
    return sls::concatenateNonEmptyStrings(r);
}

int multiSlsDetector::lockReceiver(int lock, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->lockReceiver(lock);
    }

    // multi
    auto r = parallelCall(&slsDetector::lockReceiver, lock);
    return sls::minusOneIfDifferent(r);
}

std::string multiSlsDetector::getReceiverLastClientIP(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getReceiverLastClientIP();
    }

    // multi
    auto r = parallelCall(&slsDetector::getReceiverLastClientIP);
    return sls::concatenateIfDifferent(r);
}

int multiSlsDetector::exitReceiver(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->exitReceiver();
    }

    // multi
    auto r = parallelCall(&slsDetector::exitReceiver);
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

int multiSlsDetector::execReceiverCommand(const std::string &cmd, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->execReceiverCommand(cmd);
    }

    // multi
    auto r = parallelCall(&slsDetector::execReceiverCommand, cmd);
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

std::string multiSlsDetector::getFilePath(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getFilePath();
    }

    // multi
    auto r = serialCall(&slsDetector::getFilePath);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::setFilePath(const std::string &path, int detPos) {
    if (path.empty()) {
        return getFilePath(detPos);
    }

    // single
    if (detPos >= 0) {
        return detectors[detPos]->setFilePath(path);
    }

    // multi
    auto r = parallelCall(&slsDetector::setFilePath, path);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::getFileName(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getFileName();
    }

    // multi
    auto r = serialCall(&slsDetector::getFileName);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::setFileName(const std::string &fname, int detPos) {
    if (fname.empty()) {
        return getFileName(detPos);
    }

    // single
    if (detPos >= 0) {
        return detectors[detPos]->setFileName(fname);
    }

    // multi
    auto r = parallelCall(&slsDetector::setFileName, fname);
    return sls::concatenateIfDifferent(r);
}

int multiSlsDetector::setReceiverFramesPerFile(int f, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setReceiverFramesPerFile(f);
    }

    // multi
    auto r = parallelCall(&slsDetector::setReceiverFramesPerFile, f);
    return sls::minusOneIfDifferent(r);
}

slsDetectorDefs::frameDiscardPolicy
multiSlsDetector::setReceiverFramesDiscardPolicy(frameDiscardPolicy f,
                                                 int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setReceiverFramesDiscardPolicy(f);
    }

    // multi
    auto r = parallelCall(&slsDetector::setReceiverFramesDiscardPolicy, f);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setReceiverPartialFramesPadding(int f, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setReceiverPartialFramesPadding(f);
    }

    // multi
    auto r = parallelCall(&slsDetector::setReceiverPartialFramesPadding, f);
    return sls::minusOneIfDifferent(r);
}

slsDetectorDefs::fileFormat multiSlsDetector::getFileFormat(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getFileFormat();
    }

    // multi
    auto r = serialCall(&slsDetector::getFileFormat);
    return sls::minusOneIfDifferent(r);
}

slsDetectorDefs::fileFormat multiSlsDetector::setFileFormat(fileFormat f,
                                                            int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setFileFormat(f);
    }

    // multi
    auto r = parallelCall(&slsDetector::setFileFormat, f);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::getFileIndex(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getFileIndex();
    }

    // multi
    auto r = serialCall(&slsDetector::getFileIndex);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::incrementFileIndex(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->incrementFileIndex();
    }

    // multi
    auto r = parallelCall(&slsDetector::incrementFileIndex);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setFileIndex(int i, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setFileIndex(i);
    }

    // multi
    auto r = parallelCall(&slsDetector::setFileIndex, i);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::startReceiver(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->startReceiver();
    }

    // multi
    auto r = parallelCall(&slsDetector::startReceiver);
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

int multiSlsDetector::stopReceiver(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->stopReceiver();
    }

    // multi
    auto r = parallelCall(&slsDetector::stopReceiver);
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

slsDetectorDefs::runStatus multiSlsDetector::getReceiverStatus(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getReceiverStatus();
    }

    // multi
    auto r = parallelCall(&slsDetector::getReceiverStatus);
    if (sls::allEqual(r)) {
        return r.front();
    }
    if (sls::anyEqualTo(r, ERROR)) {
        return ERROR;
    }
    for (const auto &value : r) {
        if (value != IDLE) {
            return value;
        }
    }
    return IDLE;
}

int multiSlsDetector::getFramesCaughtByReceiver(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getFramesCaughtByReceiver();
    }

    // multi
    auto r = parallelCall(&slsDetector::getFramesCaughtByReceiver);

    // prevent divide by all or do not take avg when -1 for "did not connect"
    if ((!detectors.size()) || (sls::anyEqualTo(r, -1))) {
        return -1;
    }

    // return average
    return ((sls::sum(r)) / (int)detectors.size());
}

int multiSlsDetector::getReceiverCurrentFrameIndex(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getReceiverCurrentFrameIndex();
    }

    // multi
    auto r = parallelCall(&slsDetector::getReceiverCurrentFrameIndex);

    // prevent divide by all or do not take avg when -1 for "did not connect"
    if ((!detectors.size()) || (sls::anyEqualTo(r, -1))) {
        return -1;
    }

    // return average
    return ((sls::sum(r)) / (int)detectors.size());
}

int multiSlsDetector::resetFramesCaught(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->resetFramesCaught();
    }

    // multi
    auto r = parallelCall(&slsDetector::resetFramesCaught);
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
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
    if (getDetectorTypeAsEnum() == EIGER) {
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
                    ->getClientStreamingIP()
                    .c_str(),
                portnum));
            FILE_LOG(logINFO) << "Zmq Client[" << iSocket << "] at " << zmqSocket.back()->GetZmqServerAddress();
        } catch (...) {
            FILE_LOG(logERROR) << "Could not create Zmq socket on port " << portnum;
            createReceivingDataSockets(true);
            return FAIL;
        }
    }

    client_downstream = true;
    FILE_LOG(logINFO) << "Receiving Data Socket(s) created";
    return OK;
}

void multiSlsDetector::readFrameFromReceiver() {

    int nX =
        multi_shm()->numberOfDetector[X]; // to copy data in multi module
    int nY = multi_shm()
                 ->numberOfDetector[Y]; // for eiger, to reverse the data
    bool gappixelsenable = false;
    bool eiger = false;
    if (getDetectorTypeAsEnum() == EIGER) {
        eiger = true;
        nX *= 2;
        gappixelsenable = detectors[0]->enableGapPixels(-1) >= 1 ? true : false;
    }

    bool runningList[zmqSocket.size()], connectList[zmqSocket.size()];
    int numRunning = 0;
    for (size_t i = 0; i < zmqSocket.size(); ++i) {
        if (!zmqSocket[i]->Connect()) {
            connectList[i] = true;
            runningList[i] = true;
            ++numRunning;
        } else {
            // to remember the list it connected to, to disconnect later
            connectList[i] = false;
            FILE_LOG(logERROR) << "Could not connect to socket  " << zmqSocket[i]->GetZmqServerAddress();
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
                    if (!zmqSocket[isocket]->ReceiveHeader(
                            isocket, doc, SLS_DETECTOR_JSON_HEADER_VERSION)) {
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

                        FILE_LOG(logDEBUG1) << "One Time Header Info:"
                                               "\n\tsize: "
                                            << size << "\n\tmultisize: " << multisize << "\n\tdynamicRange: " << dynamicRange << "\n\tbytesPerPixel: " << bytesPerPixel << "\n\tnPixelsX: " << nPixelsX << "\n\tnPixelsY: " << nPixelsY;
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
                    FILE_LOG(logDEBUG1) << "Header Info:"
                                           "\n\tcurrentFileName: "
                                        << currentFileName << "\n\tcurrentAcquisitionIndex: " << currentAcquisitionIndex << "\n\tcurrentFrameIndex: " << currentFrameIndex << "\n\tcurrentFileIndex: " << currentFileIndex << "\n\tcurrentSubFrameIndex: " << currentSubFrameIndex << "\n\tcoordX: " << coordX << "\n\tcoordY: " << coordY << "\n\tflippedDataX: " << flippedDataX;
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
                    FILE_LOG(logDEBUG1) << "Multi Image Info:"
                                           "\n\txoffset: "
                                        << xoffset << "\n\tyoffset: " << yoffset << "\n\tsingledetrowoffset: " << singledetrowoffset << "\n\trowoffset: " << rowoffset;

                    if (eiger && flippedDataX) {
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

        // send data to callback
        if (data) {
            // 4bit gap pixels
            if (dynamicRange == 4 && gappixelsenable) {
                int n = processImageWithGapPixels(multiframe, multigappixels);
                nPixelsX = multi_shm()->numberOfChannelInclGapPixels[X];
                nPixelsY = multi_shm()->numberOfChannelInclGapPixels[Y];
                thisData = new detectorData(getCurrentProgress(),
                                            currentFileName.c_str(), nPixelsX,
                                            nPixelsY, multigappixels, n,
                                            dynamicRange, currentFileIndex);
            }
            // normal pixels
            else {
                thisData = new detectorData(getCurrentProgress(),
                                            currentFileName.c_str(), nPixelsX,
                                            nPixelsY, multiframe, multisize,
                                            dynamicRange, currentFileIndex);
            }
            dataReady(thisData, currentFrameIndex,
                      ((dynamicRange == 32) ? currentSubFrameIndex : -1),
                      pCallbackArg);
            delete thisData;
            setCurrentProgress(currentAcquisitionIndex + 1);
        }

        // all done
        if (!numRunning) {
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
    delete[] image;
    delete[] multiframe;
    delete[] multigappixels;
}

int multiSlsDetector::processImageWithGapPixels(char *image, char *&gpImage) {
    // eiger 4 bit mode
    int nxb = multi_shm()->numberOfDetector[X] * (512 + 3);
    int nyb = multi_shm()->numberOfDetector[Y] * (256 + 1);
    int gapdatabytes = nxb * nyb;

    int nxchip = multi_shm()->numberOfDetector[X] * 4;
    int nychip = multi_shm()->numberOfDetector[Y] * 1;

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
    for (int row = 0; row < nychip; ++row) { // for each chip in a row
        for (int ichipy = 0; ichipy < b1chipy;
             ++ichipy) { // for each row in a chip
            for (int col = 0; col < nxchip; ++col) {
                memcpy(dst, src, b1chipx);
                src += b1chipx;
                dst += b1chipx;
                if ((col + 1) % 4) {
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
        for (int row = 0; row < nychip; ++row) { // for each chip in a row
            for (int ichipy = 0; ichipy < b1chipy;
                 ++ichipy) { // for each row in a chip
                for (int col = 0; col < nxchip; ++col) {
                    dst += b1chipx;
                    mod = (col + 1) % 4;
                    // copy gap pixel(chip 0, 1, 2)
                    if (mod) {
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
        for (int row = 0; row < nychip; ++row) { // for each chip in a row
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

int multiSlsDetector::enableWriteToFile(int enable, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->enableWriteToFile(enable);
    }

    // multi
    auto r = parallelCall(&slsDetector::enableWriteToFile, enable);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::overwriteFile(int enable, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->overwriteFile(enable);
    }

    // multi
    auto r = parallelCall(&slsDetector::overwriteFile, enable);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setReceiverStreamingFrequency(int freq, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setReceiverStreamingFrequency(freq);
    }

    // multi
    auto r = parallelCall(&slsDetector::setReceiverStreamingFrequency, freq);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setReceiverStreamingTimer(int time_in_ms, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setReceiverStreamingTimer(time_in_ms);
    }

    // multi
    auto r = parallelCall(&slsDetector::setReceiverStreamingTimer, time_in_ms);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::enableDataStreamingToClient(int enable) {
    if (enable >= 0) {
        // destroy data threads
        if (!enable) {
            createReceivingDataSockets(true);
            // create data threads
        } else {
            if (createReceivingDataSockets() == FAIL) {
                FILE_LOG(logERROR)
                    << "Could not create data threads in client.";
                detectors[0]->setErrorMask((detectors[0]->getErrorMask()) |
                                           (DATA_STREAMING));
                // only for the first det as theres no general one
                setErrorMask(getErrorMask() | (1 << 0));
            }
        }
    }
    return client_downstream;
}

int multiSlsDetector::enableDataStreamingFromReceiver(int enable, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->enableDataStreamingFromReceiver(enable);
    }

    // multi
    auto r =
        parallelCall(&slsDetector::enableDataStreamingFromReceiver, enable);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::enableTenGigabitEthernet(int i, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->enableTenGigabitEthernet(i);
    }

    // multi
    auto r = parallelCall(&slsDetector::enableTenGigabitEthernet, i);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setReceiverFifoDepth(int i, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setReceiverFifoDepth(i);
    }

    // multi
    auto r = parallelCall(&slsDetector::setReceiverFifoDepth, i);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setReceiverSilentMode(int i, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setReceiverSilentMode(i);
    }

    // multi
    auto r = parallelCall(&slsDetector::setReceiverSilentMode, i);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setPattern(const std::string &fname, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setPattern(fname);
    }

    // multi
    int addr = 0;

    FILE *fd = fopen(fname.c_str(), "r");
    if (fd == nullptr) {
        FILE_LOG(logERROR) << "Could not open file";
        setErrorMask(getErrorMask() | MULTI_OTHER_ERROR);
        return -1;
    }

    uint64_t word;
    while (fread(&word, sizeof(word), 1, fd)) {
        serialCall(&slsDetector::setPatternWord, addr, word);
        ++addr;
    }

    fclose(fd);
    return addr;
}

uint64_t multiSlsDetector::setPatternWord(int addr, uint64_t word, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setPatternWord(addr, word);
    }

    // multi
    auto r = parallelCall(&slsDetector::setPatternWord, addr, word);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setPatternLoops(int level, int &start, int &stop, int &n,
                                      int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setPatternLoops(level, start, stop, n);
    }

    // multi
    std::vector<int> r;
    for (auto &d : detectors) {
        r.push_back(d->setPatternLoops(level, start, stop, n));
    }
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

int multiSlsDetector::setPatternWaitAddr(int level, int addr, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setPatternWaitAddr(level, addr);
    }

    // multi
    auto r = parallelCall(&slsDetector::setPatternWaitAddr, level, addr);
    return sls::minusOneIfDifferent(r);
}

uint64_t multiSlsDetector::setPatternWaitTime(int level, uint64_t t, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setPatternWaitTime(level, t);
    }

    // multi
    auto r = parallelCall(&slsDetector::setPatternWaitTime, level, t);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setPatternMask(uint64_t mask, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setPatternMask(mask);
    }

    // multi
    auto r = parallelCall(&slsDetector::setPatternMask, mask);
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

uint64_t multiSlsDetector::getPatternMask(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getPatternMask();
    }

    // multi
    auto r = parallelCall(&slsDetector::getPatternMask);
    if (sls::allEqual(r)) {
        return r.front();
    }

    // can't have different values
    FILE_LOG(logERROR) << "Error: Different Values returned)";
    setErrorMask(getErrorMask() | MULTI_HAVE_DIFFERENT_VALUES);
    return -1;
}

int multiSlsDetector::setPatternBitMask(uint64_t mask, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setPatternBitMask(mask);
    }

    // multi
    auto r = parallelCall(&slsDetector::setPatternBitMask, mask);
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

uint64_t multiSlsDetector::getPatternBitMask(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getPatternBitMask();
    }

    // multi
    auto r = parallelCall(&slsDetector::getPatternBitMask);
    if (sls::allEqual(r)) {
        return r.front();
    }

    // can't have different values
    FILE_LOG(logERROR) << "Error: Different Values returned)";
    setErrorMask(getErrorMask() | MULTI_HAVE_DIFFERENT_VALUES);
    return -1;
}

int multiSlsDetector::setLEDEnable(int enable, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setLEDEnable(enable);
    }

    // multi
    auto r = parallelCall(&slsDetector::setLEDEnable, enable);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setDigitalIODelay(uint64_t pinMask, int delay, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setDigitalIODelay(pinMask, delay);
    }

    // multi
    auto r = parallelCall(&slsDetector::setDigitalIODelay, pinMask, delay);
    return sls::allEqualTo(r, static_cast<int>(OK)) ? OK : FAIL;
}

int multiSlsDetector::retrieveDetectorSetup(const std::string &fname1,
                                            int level) {

    int skip = 0;
    std::string fname;
    std::string str;
    std::ifstream infile;
    int iargval;
    int interrupt = 0;
    char *args[10];

    char myargs[10][1000];

    std::string sargname, sargval;
    int iline = 0;

    if (level == 2) {
        FILE_LOG(logDEBUG1) << "config file read";
        fname = fname1 + std::string(".det");
    } else {
        fname = fname1;
    }

    infile.open(fname.c_str(), std::ios_base::in);
    if (infile.is_open()) {
        auto cmd = slsDetectorCommand(this);
        while (infile.good() and interrupt == 0) {
            sargname = "none";
            sargval = "0";
            getline(infile, str);
            iline++;
            FILE_LOG(logDEBUG1) << str;
            if (str.find('#') != std::string::npos) {
                FILE_LOG(logDEBUG1) << "Line is a comment \n"
                                    << str;
                continue;
            } else {
                std::istringstream ssstr(str);
                iargval = 0;
                while (ssstr.good()) {
                    ssstr >> sargname;
                    //  if (ssstr.good()) {
                    sls::strcpy_safe(myargs[iargval], sargname.c_str());
                    args[iargval] = myargs[iargval];
                    FILE_LOG(logDEBUG1) << args[iargval];
                    iargval++;
                    // }
                    skip = 0;
                }

                if (level != 2) {
                    if (std::string(args[0]) == std::string("trimbits")) {
                        skip = 1;
                    }
                }
                if (skip == 0) {
                    cmd.executeLine(iargval, args, PUT_ACTION);
                }
            }
            iline++;
        }
        infile.close();

    } else {
        FILE_LOG(logERROR) << "Error opening  " << fname << " for reading";
        return FAIL;
    }
    FILE_LOG(logDEBUG1) << "Read  " << iline << " lines";

    if (getErrorMask()) {
        return FAIL;
    }

    return OK;
}

int multiSlsDetector::dumpDetectorSetup(const std::string &fname, int level) {
    detectorType type = getDetectorTypeAsEnum();
    // std::string names[100];
    std::vector<std::string> names;
    // int nvar = 0;

    // common config
    names.emplace_back("fname");
    names.emplace_back("index");
    names.emplace_back("enablefwrite");
    names.emplace_back("overwrite");
    names.emplace_back("dr");
    names.emplace_back("settings");
    names.emplace_back("exptime");
    names.emplace_back("period");
    names.emplace_back("frames");
    names.emplace_back("cycles");
    names.emplace_back("measurements");
    names.emplace_back("timing");

    switch (type) {
    case EIGER:
        names.emplace_back("flags");
        names.emplace_back("clkdivider");
        names.emplace_back("threshold");
        names.emplace_back("ratecorr");
        names.emplace_back("trimbits");
        break;
    case GOTTHARD:
        names.emplace_back("delay");
        break;
    case JUNGFRAU:
        names.emplace_back("delay");
        names.emplace_back("clkdivider");
        break;
    case CHIPTESTBOARD:
        names.emplace_back("dac:0");
        names.emplace_back("dac:1");
        names.emplace_back("dac:2");
        names.emplace_back("dac:3");
        names.emplace_back("dac:4");
        names.emplace_back("dac:5");
        names.emplace_back("dac:6");
        names.emplace_back("dac:7");
        names.emplace_back("dac:8");
        names.emplace_back("dac:9");
        names.emplace_back("dac:10");
        names.emplace_back("dac:11");
        names.emplace_back("dac:12");
        names.emplace_back("dac:13");
        names.emplace_back("dac:14");
        names.emplace_back("dac:15");
        names.emplace_back("dac:16");
        names.emplace_back("dac:17");
        names.emplace_back("dac:18");
        names.emplace_back("dac:19");
        names.emplace_back("dac:20");
        names.emplace_back("dac:21");
        names.emplace_back("dac:22");
        names.emplace_back("dac:23");
        names.emplace_back("adcvpp");
        names.emplace_back("adcclk");
        names.emplace_back("clkdivider");
        names.emplace_back("adcphase");
        names.emplace_back("adcpipeline");
        names.emplace_back("adcinvert"); //
        names.emplace_back("adcdisable");
        names.emplace_back("patioctrl");
        names.emplace_back("patclkctrl");
        names.emplace_back("patlimits");
        names.emplace_back("patloop0");
        names.emplace_back("patnloop0");
        names.emplace_back("patwait0");
        names.emplace_back("patwaittime0");
        names.emplace_back("patloop1");
        names.emplace_back("patnloop1");
        names.emplace_back("patwait1");
        names.emplace_back("patwaittime1");
        names.emplace_back("patloop2");
        names.emplace_back("patnloop2");
        names.emplace_back("patwait2");
        names.emplace_back("patwaittime2");
        break;
    default:
        break;
    }

    //Workaround to bo able to suplly ecexuteLine with char**
    const int n_arguments = 1;
    char buffer[1000]; //TODO! this should not be hardcoded!
    char *args[n_arguments] = {buffer};

    std::string outfname;
    if (level == 2) {
        writeConfigurationFile(fname + ".config");
        outfname = fname + ".det";
    } else {
        outfname = fname;
    }

    std::ofstream outfile;
    outfile.open(outfname.c_str(), std::ios_base::out);
    if (outfile.is_open()) {
        auto cmd = slsDetectorCommand(this);
        for (auto &name : names) {
            sls::strcpy_safe(buffer, name.c_str()); //this is...
            outfile << name << " " << cmd.executeLine(n_arguments, args, GET_ACTION)
                    << std::endl;
        }
        outfile.close();
    } else {
        FILE_LOG(logERROR) << "Could not open parameters file " << outfname << " for writing";
        return FAIL;
    }

    FILE_LOG(logDEBUG1) << "wrote " << names.size() << " lines to  " << outfname;
    return OK;
}

void multiSlsDetector::registerAcquisitionFinishedCallback(
    int (*func)(double, int, void *), void *pArg) {
    acquisition_finished = func;
    acqFinished_p = pArg;
}

void multiSlsDetector::registerMeasurementFinishedCallback(int (*func)(int, int,
                                                                       void *),
                                                           void *pArg) {
    measurement_finished = func;
    measFinished_p = pArg;
}

void multiSlsDetector::registerProgressCallback(int (*func)(double, void *),
                                                void *pArg) {
    progress_call = func;
    pProgressCallArg = pArg;
}

void multiSlsDetector::registerDataCallback(
    int (*userCallback)(detectorData *, int, int, void *), void *pArg) {
    dataReady = userCallback;
    pCallbackArg = pArg;
    if (setReceiverOnline() == slsDetectorDefs::ONLINE_FLAG) {
        enableDataStreamingToClient(1);
        enableDataStreamingFromReceiver(1);
    }
}

int multiSlsDetector::setTotalProgress() {
    int nf = 1, nc = 1, ns = 1, nm = 1;

    if (multi_shm()->timerValue[FRAME_NUMBER]) {
        nf = multi_shm()->timerValue[FRAME_NUMBER];
    }

    if (multi_shm()->timerValue[CYCLES_NUMBER] > 0) {
        nc = multi_shm()->timerValue[CYCLES_NUMBER];
    }

    if (multi_shm()->timerValue[STORAGE_CELL_NUMBER] > 0) {
        ns = multi_shm()->timerValue[STORAGE_CELL_NUMBER] + 1;
    }

    if (multi_shm()->timerValue[MEASUREMENTS_NUMBER] > 0) {
        nm = multi_shm()->timerValue[MEASUREMENTS_NUMBER];
    }

    totalProgress = nm * nf * nc * ns;

    FILE_LOG(logDEBUG1) << "nm " << nm << " nf " << nf << " nc " << nc << " ns " << ns;
    FILE_LOG(logDEBUG1) << "Set total progress " << totalProgress << std::endl;
    return totalProgress;
}

double multiSlsDetector::getCurrentProgress() {
    std::lock_guard<std::mutex> lock(mp);
    return 100. * ((double)progressIndex) / ((double)totalProgress);
}

void multiSlsDetector::incrementProgress() {
    std::lock_guard<std::mutex> lock(mp);
    progressIndex++;
    std::cout << std::fixed << std::setprecision(2) << std::setw(6)
              << 100. * ((double)progressIndex) / ((double)totalProgress)
              << " \%";
    std::cout << '\r' << std::flush;
}

void multiSlsDetector::setCurrentProgress(int i) {
    std::lock_guard<std::mutex> lock(mp);
    progressIndex = i;
    std::cout << std::fixed << std::setprecision(2) << std::setw(6)
              << 100. * ((double)progressIndex) / ((double)totalProgress)
              << " \%";
    std::cout << '\r' << std::flush;
}

int multiSlsDetector::acquire() {
    // ensure acquire isnt started multiple times by same client
    if (isAcquireReady() == FAIL) {
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

    bool receiver = (setReceiverOnline() == ONLINE_FLAG);
    progressIndex = 0;
    multi_shm()->stoppedFlag = 0;
    setJoinThreadFlag(false);

    int nm = multi_shm()->timerValue[MEASUREMENTS_NUMBER];
    if (nm < 1) {
        nm = 1;
    }

    // verify receiver is idle
    if (receiver) {
        std::lock_guard<std::mutex> lock(mg);
        if (getReceiverStatus() != IDLE) {
            if (stopReceiver() == FAIL) {
                multi_shm()->stoppedFlag = 1;
            }
        }
    }

    startProcessingThread();

    // resets frames caught in receiver
    if (receiver) {
        std::lock_guard<std::mutex> lock(mg);
        if (resetFramesCaught() == FAIL) {
            multi_shm()->stoppedFlag = 1;
        }
    }

    // loop through measurements
    for (int im = 0; im < nm; ++im) {
        if (multi_shm()->stoppedFlag) {
            break;
        }

        // start receiver
        if (receiver) {
            std::lock_guard<std::mutex> lock(mg);
            if (startReceiver() == FAIL) {
                FILE_LOG(logERROR) << "Start receiver failed ";
                stopReceiver();
                multi_shm()->stoppedFlag = 1;
                break;
            }
            // let processing thread listen to these packets
            sem_post(&sem_newRTAcquisition);
        }

        startAndReadAll();

        // stop receiver
        std::lock_guard<std::mutex> lock(mg);
        if (receiver) {
            if (stopReceiver() == FAIL) {
                multi_shm()->stoppedFlag = 1;
            } else {
                if (dataReady) {
                    sem_wait(&sem_endRTAcquisition); // waits for receiver's
                }
                // external process to be
                // done sending data to gui
            }
        }
        int findex = 0;
        findex = incrementFileIndex();

        if (measurement_finished) {
            measurement_finished(im, findex, measFinished_p);
        }
        if (multi_shm()->stoppedFlag) {
            break;
        }

    } // end measurements loop im

    // waiting for the data processing thread to finish!
    setJoinThreadFlag(true);
    sem_post(&sem_newRTAcquisition);
    dataProcessingThread.join();

    if (progress_call) {
        progress_call(getCurrentProgress(), pProgressCallArg);
    }

    if (acquisition_finished) {
        acquisition_finished(getCurrentProgress(), getRunStatus(),
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

// void* multiSlsDetector::startProcessData(void *n) {
// 	((multiSlsDetector*)n)->processData();
// 	return n;
// }

void multiSlsDetector::processData() {
    if (setReceiverOnline() == OFFLINE_FLAG) {
        return;
    } else {
        if (dataReady) {
            readFrameFromReceiver();
        }
        // only update progress
        else {
            int caught = -1;
            while (true) {
                // to exit acquire by typing q
                if (kbhit() != 0) {
                    if (fgetc(stdin) == 'q') {
                        FILE_LOG(logINFO) << "Caught the command to stop acquisition";
                        stopAcquisition();
                    }
                }
                // get progress
                if (setReceiverOnline() == ONLINE_FLAG) {
                    std::lock_guard<std::mutex> lock(mg);
                    caught = getFramesCaughtByReceiver(0);
                }
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
    return;
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

bool multiSlsDetector::isDetectorIndexOutOfBounds(int detPos) {
    // position exceeds multi list size
    if (detPos >= static_cast<int>(detectors.size())) {
        FILE_LOG(logERROR) << "Position " << detPos
                           << " is out of bounds with "
                              "a detector list of "
                           << detectors.size();
        setErrorMask(getErrorMask() | MULTI_POS_EXCEEDS_LIST);
        return true;
    }
    return false;
}
