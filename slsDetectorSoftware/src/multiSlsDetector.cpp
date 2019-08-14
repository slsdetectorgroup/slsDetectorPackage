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

bool multiSlsDetector::getAcquiringFlag() const {
    return multi_shm()->acquiringFlag;
}

void multiSlsDetector::checkDetectorVersionCompatibility(int detPos) {
    if (detPos >= 0) {
        detectors[detPos]->checkDetectorVersionCompatibility();
    }

    parallelCall(&slsDetector::checkDetectorVersionCompatibility);
}

void multiSlsDetector::checkReceiverVersionCompatibility(int detPos) {
    if (detPos >= 0) {
        detectors[detPos]->checkReceiverVersionCompatibility();
    }

    parallelCall(&slsDetector::checkReceiverVersionCompatibility);
}

int64_t multiSlsDetector::getId(idMode mode, int detPos) {
    if (detPos >= 0) {
        return detectors[detPos]->getId(mode);
    }

    auto r = parallelCall(&slsDetector::getId, mode);
    return sls::minusOneIfDifferent(r);
}

int64_t multiSlsDetector::getClientSoftwareVersion() const { return APILIB; }

int64_t multiSlsDetector::getReceiverSoftwareVersion(int detPos) {
    if (detPos >= 0) {
        return detectors[detPos]->getReceiverSoftwareVersion();
    }

    auto r = parallelCall(&slsDetector::getReceiverSoftwareVersion);
    return sls::minusOneIfDifferent(r);
}

std::vector<int64_t> multiSlsDetector::getDetectorNumber() {
    return parallelCall(&slsDetector::getId,
                        slsDetectorDefs::DETECTOR_SERIAL_NUMBER);
}

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

void multiSlsDetector::freeSharedMemory(int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->freeSharedMemory();
        return;
    }

    // multi
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
        sstream << (d->isFixedPatternSharedMemoryCompatible() ? d->getHostname() : "Unknown") << "+";
    }
    sstream << "\nType: ";
    // get type from multi shm
    if (multi_shm()->shmversion >= MULTI_SHMAPIVERSION) {
        sstream << slsDetectorDefs::detectorTypeToString(getDetectorTypeAsEnum());
    } 
    // get type from slsdet shm
    else {
        for (auto &d : detectors) {
            sstream << (d->isFixedPatternSharedMemoryCompatible() ? d->getDetectorTypeAsString() : "Unknown") << "+";
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
    multi_shm()->numberOfDetector[X] = 0;
    multi_shm()->numberOfDetector[Y] = 0;
    multi_shm()->numberOfChannels = 0;
    multi_shm()->numberOfChannel[X] = 0;
    multi_shm()->numberOfChannel[Y] = 0;
    multi_shm()->numberOfChannelInclGapPixels[X] = 0;
    multi_shm()->numberOfChannelInclGapPixels[Y] = 0;
    multi_shm()->maxNumberOfChannelsPerDetector[X] = -1;
    multi_shm()->maxNumberOfChannelsPerDetector[Y] = -1;
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
        return FAIL != 0u;
    }
    multi_shm()->acquiringFlag = true;
    return OK != 0u;
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
}

void multiSlsDetector::setHostname(const char *name, int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->setHostname(name);
        return;
    }

    // multi
    // this check is there only to allow the previous detsizechan command
    if (multi_shm()->numberOfDetectors != 0) {
        FILE_LOG(logWARNING)
            << "There are already detector(s) in shared memory."
               "Freeing Shared memory now.";
        freeSharedMemory();
        setupMultiDetector();
    }
    addMultipleDetectors(name);
}

std::string multiSlsDetector::getHostname(int detPos) const {
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
    updateDetectorSize();
}

void multiSlsDetector::addSlsDetector(const std::string &hostname) {
    FILE_LOG(logDEBUG1) << "Adding detector " << hostname;

    for (auto &d : detectors) {
        if (d->getHostname() == hostname) {
            FILE_LOG(logWARNING)
                << "Detector " << hostname
                << "already part of the multiDetector!" << std::endl
                << "Remove it before adding it back in a new position!";
            return;
        }
    }

    // get type by connecting
    detectorType type =
        slsDetector::getTypeFromDetector(hostname, DEFAULT_PORTNO);
    int pos = (int)detectors.size();
    detectors.push_back(
        sls::make_unique<slsDetector>(type, multiId, pos, false));
    multi_shm()->numberOfDetectors = detectors.size();
    multi_shm()->numberOfChannels += detectors[pos]->getTotalNumberOfChannels();
    detectors[pos]->setHostname(hostname);
    multi_shm()->multiDetectorType = getDetectorTypeAsEnum(-1);// -1 needed here
}

void multiSlsDetector::updateDetectorSize() {
    FILE_LOG(logDEBUG) << "Updating Multi-Detector Size: " << size();
    
    int my = detectors[0]->getTotalNumberOfChannels(Y);
    int mx = detectors[0]->getTotalNumberOfChannels(X);
    int mgy = detectors[0]->getTotalNumberOfChannelsInclGapPixels(Y);
    int mgx = detectors[0]->getTotalNumberOfChannelsInclGapPixels(X);
    if (mgy == 0) {
        mgy = my;
        mgx = mx;    
    }

    int maxy = multi_shm()->maxNumberOfChannelsPerDetector[Y];
    if (maxy == -1) {
        maxy = my * size();
    }

    int ndety = maxy / my;
    int ndetx = size() / ndety;
    if ((maxy % my) > 0) {
        ++ndetx;
    }

    multi_shm()->numberOfDetector[X] = ndetx;
    multi_shm()->numberOfDetector[Y] = ndety;
    multi_shm()->numberOfChannel[X] = mx * ndetx;
    multi_shm()->numberOfChannel[Y] = my * ndety; 
    multi_shm()->numberOfChannelInclGapPixels[X] = mgx * ndetx;
    multi_shm()->numberOfChannelInclGapPixels[Y] = mgy * ndety;   

    FILE_LOG(logDEBUG)
        << "\n\tNumber of Detectors in X direction:"
        << multi_shm()->numberOfDetector[X]
        << "\n\tNumber of Detectors in Y direction:"
        << multi_shm()->numberOfDetector[Y]    
        << "\n\tNumber of Channels in X direction:"
        << multi_shm()->numberOfChannel[X]
        << "\n\tNumber of Channels in Y direction:"
        << multi_shm()->numberOfChannel[Y]
        << "\n\tNumber of Channels in X direction with Gap Pixels:"
        << multi_shm()->numberOfChannelInclGapPixels[X]
        << "\n\tNumber of Channels in Y direction with Gap Pixels:"
        << multi_shm()->numberOfChannelInclGapPixels[Y];

    multi_shm()->numberOfChannels =
        multi_shm()->numberOfChannel[0] * multi_shm()->numberOfChannel[1];

    for (auto &d : detectors) {
        d->updateMultiSize(multi_shm()->numberOfDetector[0],
                           multi_shm()->numberOfDetector[1]);
    }
}

slsDetectorDefs::detectorType multiSlsDetector::getDetectorTypeAsEnum() const {
    return multi_shm()->multiDetectorType;
}

slsDetectorDefs::detectorType
multiSlsDetector::getDetectorTypeAsEnum(int detPos) {
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

size_t multiSlsDetector::size() const { return detectors.size(); }

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

slsDetectorDefs::coordinates multiSlsDetector::getNumberOfChannels() const {
    slsDetectorDefs::coordinates coord;
    coord.x = multi_shm()->numberOfChannel[X];
    coord.y = multi_shm()->numberOfChannel[Y]; 
    return coord;
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

slsDetectorDefs::coordinates multiSlsDetector::getTotalNumberOfChannelsInclGapPixels() const {
    slsDetectorDefs::coordinates coord;
    coord.x = multi_shm()->numberOfChannelInclGapPixels[X];
    coord.y = multi_shm()->numberOfChannelInclGapPixels[Y]; 
    return coord;
}

int multiSlsDetector::getMaxNumberOfChannelsPerDetector(dimension d) {
    return multi_shm()->maxNumberOfChannelsPerDetector[d];
}

int multiSlsDetector::setMaxNumberOfChannelsPerDetector(dimension d, int i) {
    multi_shm()->maxNumberOfChannelsPerDetector[d] = i;
    return multi_shm()->maxNumberOfChannelsPerDetector[d];
}

slsDetectorDefs::coordinates multiSlsDetector::getMaxNumberOfChannels() const {
    slsDetectorDefs::coordinates coord;
    coord.x = multi_shm()->maxNumberOfChannelsPerDetector[X];
    coord.y = multi_shm()->maxNumberOfChannelsPerDetector[Y]; 
    return coord;
}

void multiSlsDetector::setMaxNumberOfChannels(const slsDetectorDefs::coordinates c) {
    multi_shm()->maxNumberOfChannelsPerDetector[X] = c.x;
    multi_shm()->maxNumberOfChannelsPerDetector[Y] = c.y; 
}

int multiSlsDetector::getQuad(int detPos) {
    int retval = detectors[0]->getQuad();
    if (retval && size() > 1) {
        throw RuntimeError("Quad type is available only for 1 Eiger Quad Half "
                           "module, but it Quad is enabled for 1st readout");
    }
    return retval;
}

void multiSlsDetector::setQuad(const bool enable, int detPos) {
    if (enable && size() > 1) {
        throw RuntimeError("Cannot set Quad type as it is available only for 1 "
                           "Eiger Quad Half module.");
    }

    detectors[0]->setQuad(enable);
}

void multiSlsDetector::setReadNLines(const int value, int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->setReadNLines(value);
    }

    // multi
    parallelCall(&slsDetector::setReadNLines, value);
}

int multiSlsDetector::getReadNLines(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getReadNLines();
    }

    // multi
    auto r = parallelCall(&slsDetector::getReadNLines);
    return sls::minusOneIfDifferent(r);
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

int multiSlsDetector::getReceiverPort(int detPos) const {
    if (detPos >= 0) {
        return detectors[detPos]->getReceiverPort();
    }

    auto r = serialCall(&slsDetector::getReceiverPort);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::lockServer(int p, int detPos) {
    if (detPos >= 0) {
        return detectors[detPos]->lockServer(p);
    }
    auto r = parallelCall(&slsDetector::lockServer, p);
    return sls::minusOneIfDifferent(r);
}

std::string multiSlsDetector::getLastClientIP(int detPos) {
    if (detPos >= 0) {
        return detectors[detPos]->getLastClientIP();
    }
    auto r = parallelCall(&slsDetector::getLastClientIP);
    return sls::concatenateIfDifferent(r);
}

void multiSlsDetector::exitServer(int detPos) {
    if (detPos >= 0) {
        detectors[detPos]->exitServer();
    }
    parallelCall(&slsDetector::exitServer);
}

void multiSlsDetector::execCommand(const std::string &cmd, int detPos) {
    if (detPos >= 0) {
        detectors[detPos]->execCommand(cmd);
    }
    parallelCall(&slsDetector::execCommand, cmd);
}

void multiSlsDetector::readConfigurationFile(const std::string &fname) {
    freeSharedMemory();
    setupMultiDetector();
    FILE_LOG(logINFO) << "Loading configuration file: " << fname;

    std::ifstream input_file;
    input_file.open(fname, std::ios_base::in);
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
            multiSlsDetectorClient(current_line, PUT_ACTION, this);
        }
    }
    input_file.close();
}

void multiSlsDetector::writeConfigurationFile(const std::string &fname) {
    // TODO! make exception safe!
    const std::vector<std::string> header{"detsizechan", "hostname"};
    std::ofstream outfile;

    outfile.open(fname.c_str(), std::ios_base::out);
    if (outfile.is_open()) {
        for (const auto &cmd : header)
            multiSlsDetectorClient(cmd, GET_ACTION, this, outfile);

        // single detector configuration
        for (auto &detector : detectors) {
            outfile << '\n';
            auto det_commands = detector->getConfigFileCommands();
            for (const auto &cmd : det_commands)
                multiSlsDetectorClient(cmd, GET_ACTION, this, outfile);
        }
    } else {
        throw RuntimeError("Could not open configuration file " + fname +
                           " for writing");
    }
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

void multiSlsDetector::loadSettingsFile(const std::string &fname, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->loadSettingsFile(fname);
    }

    // multi
    parallelCall(&slsDetector::loadSettingsFile, fname);
}

void multiSlsDetector::saveSettingsFile(const std::string &fname, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->saveSettingsFile(fname);
    }

    // multi
    parallelCall(&slsDetector::saveSettingsFile, fname);
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

void multiSlsDetector::prepareAcquisition(int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->prepareAcquisition();
    }

    // multi
    parallelCall(&slsDetector::prepareAcquisition);
}

void multiSlsDetector::startAcquisition(int detPos) {
    // single
    if (detPos >= 0) {
        if (detectors[detPos]->getDetectorTypeAsEnum() == EIGER) {
            detectors[detPos]->prepareAcquisition();
        }
        detectors[detPos]->startAcquisition();
    }

    // multi
    if (getDetectorTypeAsEnum() == EIGER) {
        prepareAcquisition();
    }
    parallelCall(&slsDetector::startAcquisition);
}

void multiSlsDetector::stopAcquisition(int detPos) {
    // locks to synchronize using client->receiver simultaneously (processing
    // thread)
    std::lock_guard<std::mutex> lock(mg);
    if (detPos >= 0) {
        detectors[detPos]->stopAcquisition();
    } else {
        parallelCall(&slsDetector::stopAcquisition);
    }
}

void multiSlsDetector::sendSoftwareTrigger(int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->sendSoftwareTrigger();
    }

    // multi
    parallelCall(&slsDetector::sendSoftwareTrigger);
}

void multiSlsDetector::startAndReadAll(int detPos) {
    // single
    if (detPos >= 0) {
        if (detectors[detPos]->getDetectorTypeAsEnum() == EIGER) {
            detectors[detPos]->prepareAcquisition();
        }
        detectors[detPos]->startAndReadAll();
    }

    // multi
    if (getDetectorTypeAsEnum() == EIGER) {
        prepareAcquisition();
    }
    parallelCall(&slsDetector::startAndReadAll);
}

void multiSlsDetector::startReadOut(int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->startReadOut();
    }

    // multi
    parallelCall(&slsDetector::startReadOut);
}

void multiSlsDetector::readAll(int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->readAll();
    }

    // multi
   parallelCall(&slsDetector::readAll);
}

void multiSlsDetector::configureMAC(int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->configureMAC();
    }

    // multi
    parallelCall(&slsDetector::configureMAC);
}

void multiSlsDetector::setStartingFrameNumber(const uint64_t value,
                                              int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setStartingFrameNumber(value);
    }

    // multi
    parallelCall(&slsDetector::setStartingFrameNumber, value);
}

uint64_t multiSlsDetector::getStartingFrameNumber(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getStartingFrameNumber();
    }

    // multi
    auto r = parallelCall(&slsDetector::getStartingFrameNumber);
    if (sls::allEqual(r)) {
        return r.front();
    }

    // can't have different values for next acquisition
    std::ostringstream ss;
    ss << "Error: Different Values for starting frame number";
    throw RuntimeError(ss.str());
}

int64_t multiSlsDetector::setTimer(timerIndex index, int64_t t, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setTimer(index, t);
    }

    // multi
    auto r = parallelCall(&slsDetector::setTimer, index, t);
    return sls::minusOneIfDifferent(r);
}

int64_t multiSlsDetector::secondsToNanoSeconds(double t) {
    int64_t ns = lround(t * 1E9);
    return (ns < 0) ? -1 : ns;
}

double multiSlsDetector::setExposureTime(double t, bool inseconds, int detPos) {
    if (!inseconds) {
        return setTimer(ACQUISITION_TIME, (int64_t)t, detPos);
    }
    auto t_ns = setTimer(ACQUISITION_TIME, secondsToNanoSeconds(t), detPos);
    return (t_ns < 0) ? -1 : 1E-9 * t_ns;
}

double multiSlsDetector::setExposurePeriod(double t, bool inseconds,
                                           int detPos) {
    if (!inseconds) {
        return setTimer(FRAME_PERIOD, (int64_t)t, detPos);
    }
    auto t_ns = setTimer(FRAME_PERIOD, secondsToNanoSeconds(t), detPos);
    return (t_ns < 0) ? -1 : 1E-9 * t_ns;
}

double multiSlsDetector::setDelayAfterTrigger(double t, bool inseconds,
                                              int detPos) {
    if (!inseconds) {
        return setTimer(DELAY_AFTER_TRIGGER, (int64_t)t, detPos);
    }
    auto t_ns = setTimer(DELAY_AFTER_TRIGGER, secondsToNanoSeconds(t), detPos);
    return (t_ns < 0) ? -1 : 1E-9 * t_ns;
}

double multiSlsDetector::setSubFrameExposureTime(double t, bool inseconds,
                                                 int detPos) {
    if (!inseconds) {
        return setTimer(SUBFRAME_ACQUISITION_TIME, (int64_t)t, detPos);
    }
    auto t_ns =
        setTimer(SUBFRAME_ACQUISITION_TIME, secondsToNanoSeconds(t), detPos);
    return (t_ns < 0) ? -1 : 1E-9 * t_ns;
}

double multiSlsDetector::setSubFrameExposureDeadTime(double t, bool inseconds,
                                                     int detPos) {
    if (!inseconds) {
        return setTimer(SUBFRAME_DEADTIME, (int64_t)t, detPos);
    }
    auto t_ns = setTimer(SUBFRAME_DEADTIME, secondsToNanoSeconds(t), detPos);
    return (t_ns < 0) ? -1 : 1E-9 * t_ns;
}

int64_t multiSlsDetector::setNumberOfFrames(int64_t t, int detPos) {
    return setTimer(FRAME_NUMBER, t, detPos);
}

int64_t multiSlsDetector::setNumberOfCycles(int64_t t, int detPos) {
    return setTimer(CYCLES_NUMBER, t, detPos);
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

int multiSlsDetector::setSpeed(speedVariable index, int value, int mode,
                               int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setSpeed(index, value, mode);
    }

    // multi
    auto r = parallelCall(&slsDetector::setSpeed, index, value, mode);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setDynamicRange(int dr, int detPos) {
    // single
    if (detPos >= 0) {
        throw RuntimeError("Dynamic Range cannot be set individually");
    }

    // multi
    int prevValue = -1;
    auto temp = Parallel(&slsDetector::getDynamicRangeFromShm, {});
    if (temp.equal()) {
        prevValue = temp.squash();
    }

    auto r = parallelCall(&slsDetector::setDynamicRange, dr);
    int ret = sls::minusOneIfDifferent(r);

  
    // change in dr
    if (dr != -1 && dr != prevValue) {

        // update speed, check ratecorrection
        if (getDetectorTypeAsEnum() == EIGER) {

            // rate correction before speed for consistency
            // (else exception at speed makes ratecorr inconsistent)
            parallelCall(&slsDetector::updateRateCorrection);

            // speed(usability)
            switch (dr) {
            case 32:
                FILE_LOG(logINFO)
                    << "Setting Clock to Quarter Speed to cope with "
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
    }

    return ret;
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
multiSlsDetector::setExternalSignalFlags(externalSignalFlag pol, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setExternalSignalFlags(pol);
    }

    // multi
    auto r = parallelCall(&slsDetector::setExternalSignalFlags, pol);
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

void multiSlsDetector::setInterruptSubframe(const bool enable, int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->setInterruptSubframe(enable);
    }

    // multi
    parallelCall(&slsDetector::setInterruptSubframe, enable);
}

int multiSlsDetector::getInterruptSubframe(int detPos) {
    // single
    if (detPos >= 0) {
        return static_cast<int>(detectors[detPos]->getInterruptSubframe());
    }

    // multi
    auto r = parallelCall(&slsDetector::getInterruptSubframe);
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
    std::ostringstream ss;
    ss << "Error: Different Values for function writeRegister (write 0x"
       << std::hex << val << " to addr 0x" << std::hex << addr << std::dec
       << ")";
    throw RuntimeError(ss.str());
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
    std::ostringstream ss;
    ss << "Error: Different Values for function readRegister (read from 0x"
       << std::hex << addr << std::dec << ")";
    throw RuntimeError(ss.str());
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
    std::ostringstream ss;
    ss << "Error: Different Values for function setBit "
          "(set bit "
       << n << " to addr 0x" << std::hex << addr << std::dec << ")";
    throw RuntimeError(ss.str());
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
    std::ostringstream ss;
    ss << "Error: Different Values for function clearBit (clear bit " << n
       << " to addr 0x" << std::hex << addr << std::dec << ")";
    throw RuntimeError(ss.str());
}

std::string multiSlsDetector::setDetectorMAC(const std::string &detectorMAC,
                                             int detPos) {
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
        return detectors[detPos]->getDetectorMAC().str();
    }

    // multi
    auto r = serialCall(&slsDetector::getDetectorMAC);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::setDetectorMAC2(const std::string &detectorMAC,
                                              int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setDetectorMAC2(detectorMAC);
    }

    // multi
    auto r = parallelCall(&slsDetector::setDetectorMAC2, detectorMAC);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::getDetectorMAC2(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getDetectorMAC2().str();
    }

    // multi
    auto r = serialCall(&slsDetector::getDetectorMAC2);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::setDetectorIP(const std::string &detectorIP,
                                            int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setDetectorIP(detectorIP);
    }

    // multi
    auto r = parallelCall(&slsDetector::setDetectorIP, detectorIP);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::getDetectorIP(int detPos) const {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getDetectorIP().str();
    }

    // multi
    auto r = serialCall(&slsDetector::getDetectorIP);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::setDetectorIP2(const std::string &detectorIP,
                                             int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setDetectorIP2(detectorIP);
    }

    // multi
    auto r = parallelCall(&slsDetector::setDetectorIP2, detectorIP);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::getDetectorIP2(int detPos) const {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getDetectorIP2().str();
    }

    // multi
    auto r = serialCall(&slsDetector::getDetectorIP2);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::setReceiverHostname(const std::string &receiver,
                                                  int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setReceiverHostname(receiver);
    }

    // multi
    auto r = parallelCall(&slsDetector::setReceiverHostname, receiver);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::getReceiverHostname(int detPos) const {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getReceiverHostname();
    }

    // multi
    auto r = parallelCall(&slsDetector::getReceiverHostname);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::setReceiverUDPIP(const std::string &udpip,
                                               int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setReceiverUDPIP(udpip);
    }

    // multi
    auto r = parallelCall(&slsDetector::setReceiverUDPIP, udpip);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::getReceiverUDPIP(int detPos) const {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getReceiverUDPIP().str();
    }

    // multi
    auto r = serialCall(&slsDetector::getReceiverUDPIP);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::setReceiverUDPIP2(const std::string &udpip,
                                                int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setReceiverUDPIP2(udpip);
    }

    // multi
    auto r = parallelCall(&slsDetector::setReceiverUDPIP2, udpip);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::getReceiverUDPIP2(int detPos) const {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getReceiverUDPIP2().str();
    }

    // multi
    auto r = serialCall(&slsDetector::getReceiverUDPIP2);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::setReceiverUDPMAC(const std::string &udpmac,
                                                int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setReceiverUDPMAC(udpmac);
    }

    // multi
    auto r = parallelCall(&slsDetector::setReceiverUDPMAC, udpmac);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::getReceiverUDPMAC(int detPos) const {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getReceiverUDPMAC().str();
    }

    // multi
    auto r = serialCall(&slsDetector::getReceiverUDPMAC);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::setReceiverUDPMAC2(const std::string &udpmac,
                                                 int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setReceiverUDPMAC2(udpmac);
    }

    // multi
    auto r = parallelCall(&slsDetector::setReceiverUDPMAC2, udpmac);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::getReceiverUDPMAC2(int detPos) const {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getReceiverUDPMAC2().str();
    }

    // multi
    auto r = serialCall(&slsDetector::getReceiverUDPMAC2);
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

int multiSlsDetector::getReceiverUDPPort(int detPos) const {
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

int multiSlsDetector::getReceiverUDPPort2(int detPos) const {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getReceiverUDPPort2();
    }

    // multi
    auto r = serialCall(&slsDetector::getReceiverUDPPort2);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setNumberofUDPInterfaces(int n, int detPos) {

    bool previouslyClientStreaming = enableDataStreamingToClient();
    int previouslyReceiverStreaming = enableDataStreamingFromReceiver();

    // single
    int ret = OK;
    if (detPos >= 0) {
        ret = detectors[detPos]->setNumberofUDPInterfaces(n);
    }

    // multi
    auto r = parallelCall(&slsDetector::setNumberofUDPInterfaces, n);

    // redo the zmq sockets
    if (previouslyClientStreaming) {
        enableDataStreamingToClient(0);
        enableDataStreamingToClient(1);
    }
    if (previouslyReceiverStreaming != 0) {
        enableDataStreamingFromReceiver(0);
        enableDataStreamingFromReceiver(1);
    }

    // return single
    if (detPos >= 0)
        return ret;

    // return multi
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::getNumberofUDPInterfaces(int detPos) const {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getNumberofUDPInterfaces();
    }

    // multi
    auto r = serialCall(&slsDetector::getNumberofUDPInterfaces);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::selectUDPInterface(int n, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->selectUDPInterface(n);
    }

    // multi
    auto r = parallelCall(&slsDetector::selectUDPInterface, n);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::getSelectedUDPInterface(int detPos) const {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getSelectedUDPInterface();
    }

    // multi
    auto r = serialCall(&slsDetector::getSelectedUDPInterface);
    return sls::minusOneIfDifferent(r);
}

void multiSlsDetector::setClientDataStreamingInPort(int i, int detPos) {
    if (i >= 0) {
        bool prev_streaming = enableDataStreamingToClient();

        // single
        if (detPos >= 0) {
            detectors[detPos]->setClientStreamingPort(i);
        }
        // multi
        else {
            // calculate ports individually
            int firstPort = i;
            int numSockets = (getDetectorTypeAsEnum() == EIGER) ? 2 : 1;
            if (getNumberofUDPInterfaces() == 2)
                numSockets *= 2;

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
            if (getNumberofUDPInterfaces() == 2)
                numSockets *= 2;

            for (size_t idet = 0; idet < detectors.size(); ++idet) {
                auto port = firstPort + (idet * numSockets);
                detectors[idet]->setReceiverStreamingPort(port);
            }
        }

        if (prev_streaming != 0) {
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
    if (ip.length() != 0u) {
        bool prev_streaming = enableDataStreamingToClient(-1);

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
    if (ip.length() != 0u) {
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

        if (prev_streaming != 0) {
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

int multiSlsDetector::setDetectorNetworkParameter(networkParameter index,
                                                  int value, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setDetectorNetworkParameter(index, value);
    }

    // multi
    auto r =
        parallelCall(&slsDetector::setDetectorNetworkParameter, index, value);
    return sls::minusOneIfDifferent(r);
}

std::string
multiSlsDetector::setAdditionalJsonHeader(const std::string &jsonheader,
                                          int detPos) {
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

std::string multiSlsDetector::setAdditionalJsonParameter(
    const std::string &key, const std::string &value, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setAdditionalJsonParameter(key, value);
    }

    // multi
    auto r = parallelCall(&slsDetector::setAdditionalJsonParameter, key, value);
    return sls::concatenateIfDifferent(r);
}

std::string multiSlsDetector::getAdditionalJsonParameter(const std::string &key,
                                                         int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getAdditionalJsonParameter(key);
    }

    // multi
    auto r = serialCall(&slsDetector::getAdditionalJsonParameter, key);
    return sls::concatenateIfDifferent(r);
}

int multiSlsDetector::setDetectorMinMaxEnergyThreshold(const int index,
                                                       int value, int detPos) {
    std::string parameter = (index != 0 ? "emax" : "emin");

    std::string result;
    if (value < 0) {
        result = getAdditionalJsonParameter(parameter, detPos);
    } else {
        result = setAdditionalJsonParameter(parameter, std::to_string(value),
                                            detPos);
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
        result = setAdditionalJsonParameter(parameter, getFrameModeType(value),
                                            detPos);
    }

    return getFrameModeType(result);
}

int multiSlsDetector::setDetectorMode(detectorModeType value, int detPos) {
    std::string parameter = "detectorMode";
    std::string result;

    if (value == GET_DETECTOR_MODE) {
        result = getAdditionalJsonParameter(parameter, detPos);
    } else {
        result = setAdditionalJsonParameter(parameter,
                                            getDetectorModeType(value), detPos);
    }

    return getDetectorModeType(result);
}

int64_t multiSlsDetector::setReceiverUDPSocketBufferSize(int64_t udpsockbufsize,
                                                         int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setReceiverUDPSocketBufferSize(
            udpsockbufsize);
    }

    // multi
    auto r = parallelCall(&slsDetector::setReceiverUDPSocketBufferSize,
                          udpsockbufsize);
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

void multiSlsDetector::loadImageToDetector(imageType index,
                                          const std::string &fname,
                                          int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->loadImageToDetector(index, fname);
    }

    // multi

    // read image for all
    int nch = multi_shm()->numberOfChannels;
    short int imageVals[nch];
    if (readDataFile(fname, imageVals, nch) < nch * (int)sizeof(short int)) {
        throw RuntimeError("Could not open file or not enough data in file to "
                           "load image to detector.");
    }

    // send image to all
    for (size_t idet = 0; idet < detectors.size(); ++idet) {
        detectors[idet]->sendImageToDetector(index, imageVals + idet * detectors[idet]->getTotalNumberOfChannels());
    }
}

void multiSlsDetector::writeCounterBlockFile(const std::string &fname,
                                            int startACQ, int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->writeCounterBlockFile(fname, startACQ);
    }

    // multi
    int nch = multi_shm()->numberOfChannels;
    short int imageVals[nch];
    for (size_t idet = 0; idet < detectors.size(); ++idet) {
        detectors[idet]->getCounterBlock(
            imageVals + idet * detectors[idet]->getTotalNumberOfChannels(),
            startACQ);
    }

    if (writeDataFile(fname, nch, imageVals) < nch * (int)sizeof(short int)) {
        throw RuntimeError(
            "Could not open file to write or did not write enough data"
            " in file to write counter block file from detector.");
    }

}

void multiSlsDetector::resetCounterBlock(int startACQ, int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->resetCounterBlock(startACQ);
    }

    // multi
    parallelCall(&slsDetector::resetCounterBlock, startACQ);
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

void multiSlsDetector::clearROI(int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->clearROI();
    }

    // multi
    parallelCall(&slsDetector::clearROI);
}

void multiSlsDetector::setROI(slsDetectorDefs::ROI arg, int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->setROI(arg);
    }

    // multi
    if (detPos < 0 && size() > 1) {
        throw RuntimeError("Cannot set ROI for all modules simultaneously");
    }
    detectors[0]->setROI(arg);
}

slsDetectorDefs::ROI multiSlsDetector::getROI(int detPos) const {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getROI();
    }

    // multi
    if (detPos < 0 && size() > 1) {
        throw RuntimeError("Cannot get ROI for all modules simultaneously");
    }
    return detectors[0]->getROI();
}

void multiSlsDetector::setADCEnableMask(uint32_t mask, int detPos) {
    if (detPos >= 0) {
        detectors[detPos]->setADCEnableMask(mask);
    }

    parallelCall(&slsDetector::setADCEnableMask, mask);
}

uint32_t multiSlsDetector::getADCEnableMask(int detPos) {
    if (detPos >= 0) {
        return detectors[detPos]->getADCEnableMask();
    }

    auto r = parallelCall(&slsDetector::getADCEnableMask);
    if (sls::allEqual(r)) {
        return r.front();
    }

    // can't have different values
    throw RuntimeError("Error: Different Values for function getADCEnableMask");
}

void multiSlsDetector::setADCInvert(uint32_t value, int detPos) {
    if (detPos >= 0) {
        detectors[detPos]->setADCInvert(value);
    }

    parallelCall(&slsDetector::setADCInvert, value);
}

uint32_t multiSlsDetector::getADCInvert(int detPos) {
    if (detPos >= 0) {
        return detectors[detPos]->getADCInvert();
    }

    auto r = parallelCall(&slsDetector::getADCInvert);
    if (sls::allEqual(r)) {
        return r.front();
    }

    // can't have different values
    throw RuntimeError("Error: Different Values for function getADCInvert");
}

void multiSlsDetector::setExternalSamplingSource(int value, int detPos) {
    if (detPos >= 0) {
        detectors[detPos]->setExternalSamplingSource(value);
    }

    parallelCall(&slsDetector::setExternalSamplingSource, value);
}

int multiSlsDetector::getExternalSamplingSource(int detPos) {
    if (detPos >= 0) {
        return detectors[detPos]->getExternalSamplingSource();
    }

    auto r = parallelCall(&slsDetector::getExternalSamplingSource);
    return sls::minusOneIfDifferent(r);
}

void multiSlsDetector::setExternalSampling(bool value, int detPos) {
    if (detPos >= 0) {
        detectors[detPos]->setExternalSampling(static_cast<int>(value));
    }

    parallelCall(&slsDetector::setExternalSampling, static_cast<int>(value));
}

int multiSlsDetector::getExternalSampling(int detPos) {
    if (detPos >= 0) {
        return detectors[detPos]->getExternalSampling();
    }

    auto r = parallelCall(&slsDetector::getExternalSampling);
    return sls::minusOneIfDifferent(r);
}

void multiSlsDetector::setReceiverDbitList(std::vector<int> list, int detPos) {
    if (detPos >= 0) {
        detectors[detPos]->setReceiverDbitList(list);
    }

    parallelCall(&slsDetector::setReceiverDbitList, list);
}

std::vector<int> multiSlsDetector::getReceiverDbitList(int detPos) {
    if (detPos >= 0) {
        return detectors[detPos]->getReceiverDbitList();
    }

    auto r = parallelCall(&slsDetector::getReceiverDbitList);
    return sls::minusOneIfDifferent(r);
}

void multiSlsDetector::setReceiverDbitOffset(int value, int detPos) {
    if (detPos >= 0) {
        detectors[detPos]->setReceiverDbitOffset(value);
    }

    parallelCall(&slsDetector::setReceiverDbitOffset, value);
}

int multiSlsDetector::getReceiverDbitOffset(int detPos) {
    if (detPos >= 0) {
        return detectors[detPos]->getReceiverDbitOffset();
    }

    auto r = parallelCall(&slsDetector::getReceiverDbitOffset);
    return sls::minusOneIfDifferent(r);
}

void multiSlsDetector::writeAdcRegister(uint32_t addr, uint32_t val,
                                       int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->writeAdcRegister(addr, val);
    }

    // multi
    parallelCall(&slsDetector::writeAdcRegister, addr, val);
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
        return static_cast<int>(
            detectors[detPos]->setDeactivatedRxrPaddingMode(padding));
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
            throw NotImplementedError(
                "Function (enableGapPixels) not implemented for this detector");
        }
        return 0;
    }

    // single
    if (detPos >= 0) {
        if (val >= 0) {
            throw RuntimeError("Function (enableGapPixels) must be called from "
                               "a multi detector level.");
        }
        return detectors[detPos]->enableGapPixels(val);
    }

    // multi
    auto r = parallelCall(&slsDetector::enableGapPixels, val);
    int ret = sls::minusOneIfDifferent(r);

    if (val != -1) {
        multi_shm()->numberOfChannelInclGapPixels[X] = sls::sum(parallelCall(&slsDetector::getTotalNumberOfChannelsInclGapPixels, X));
        multi_shm()->numberOfChannelInclGapPixels[Y] = sls::sum(parallelCall(&slsDetector::getTotalNumberOfChannelsInclGapPixels, Y));
    }
    return ret;
}

void multiSlsDetector::setGapPixelsEnable(bool enable, Positions pos){
    Parallel(&slsDetector::enableGapPixels, pos, static_cast<int>(enable));
    multi_shm()->numberOfChannelInclGapPixels[X] = sls::sum(parallelCall(&slsDetector::getTotalNumberOfChannelsInclGapPixels, X));
    multi_shm()->numberOfChannelInclGapPixels[Y] = sls::sum(parallelCall(&slsDetector::getTotalNumberOfChannelsInclGapPixels, Y));
}

int multiSlsDetector::setTrimEn(std::vector<int> energies, int detPos) {
    if (detPos >= 0) {
        return detectors[detPos]->setTrimEn(energies);
    }
    auto r = parallelCall(&slsDetector::setTrimEn, energies);
    return sls::minusOneIfDifferent(r);
}

std::vector<int> multiSlsDetector::getTrimEn(int detPos) {
    if (detPos >= 0) {
        return detectors[detPos]->getTrimEn();
    }
    auto r = parallelCall(&slsDetector::getTrimEn);
    return sls::minusOneIfDifferent(r);
}

void multiSlsDetector::pulsePixel(int n, int x, int y, int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->pulsePixel(n, x, y);
    }

    // multi
    parallelCall(&slsDetector::pulsePixel, n, x, y);
}

void multiSlsDetector::pulsePixelNMove(int n, int x, int y, int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->pulsePixelNMove(n, x, y);
    }

    // multi
    parallelCall(&slsDetector::pulsePixelNMove, n, x, y);
}

void multiSlsDetector::pulseChip(int n, int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->pulseChip(n);
    }

    // multi
    parallelCall(&slsDetector::pulseChip, n);
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

void multiSlsDetector::programFPGA(const std::string &fname, int detPos) {
    FILE_LOG(logINFO) << "This can take awhile. Please be patient...";
    // read pof file
    std::vector<char> buffer = readPofFile(fname);

    // single
    if (detPos >= 0) {
        detectors[detPos]->programFPGA(buffer);
    }

    // multi
    parallelCall(&slsDetector::programFPGA, buffer);
}

void multiSlsDetector::resetFPGA(int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->resetFPGA();
    }

    // multi
    parallelCall(&slsDetector::resetFPGA);
}

void multiSlsDetector::copyDetectorServer(const std::string &fname,
                                         const std::string &hostname,
                                         int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->copyDetectorServer(fname, hostname);
        detectors[detPos]->rebootController(); 
        // reboot and copy should be independant for
                                  // update command
    }

    // multi
    parallelCall(&slsDetector::copyDetectorServer, fname, hostname);
    parallelCall(&slsDetector::rebootController);
}

void multiSlsDetector::rebootController(int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->rebootController();
    }

    // multi
    parallelCall(&slsDetector::rebootController);
}

void multiSlsDetector::update(const std::string &sname,
                             const std::string &hostname,
                             const std::string &fname, int detPos) {
    FILE_LOG(logINFO) << "This can take awhile. Please be patient...";
    // read pof file
    std::vector<char> buffer = readPofFile(fname);

    // single
    if (detPos >= 0) {
        detectors[detPos]->copyDetectorServer(sname, hostname);
        detectors[detPos]->programFPGA(buffer);
    }

    // multi
    parallelCall(&slsDetector::copyDetectorServer, sname, hostname);
    parallelCall(&slsDetector::programFPGA, buffer);
}

int multiSlsDetector::powerChip(int ival, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->powerChip(ival);
    }

    // multi delayed call for safety
    if (ival >= 0 && size() > 3) {
        std::vector<int> r;
        r.reserve(detectors.size());
        for (auto &d : detectors) {
            r.push_back(d->powerChip(ival));
            usleep(1000 * 1000);
        }
        return sls::minusOneIfDifferent(r);
    }
    // multi parallel
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

void multiSlsDetector::setRateCorrection(int64_t t, int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->setRateCorrection(t);
    }

    // multi
    parallelCall(&slsDetector::setRateCorrection, t);
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

std::string multiSlsDetector::printReceiverConfiguration(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->printReceiverConfiguration();
    }

    // multi
    auto r = parallelCall(&slsDetector::printReceiverConfiguration);
    return sls::concatenateIfDifferent(r);
}

bool multiSlsDetector::getUseReceiverFlag(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getUseReceiverFlag();
    }

    // multi
    auto r = parallelCall(&slsDetector::getUseReceiverFlag);
    if (sls::allEqual(r)) {
        return r.front();
    } else {
        throw RuntimeError("Inconsistent Use receiver flags");
    }
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

void multiSlsDetector::exitReceiver(int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->exitReceiver();
    }

    // multi
    parallelCall(&slsDetector::exitReceiver);
}

void multiSlsDetector::execReceiverCommand(const std::string &cmd, int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->execReceiverCommand(cmd);
    }

    // multi
    parallelCall(&slsDetector::execReceiverCommand, cmd);
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

std::string multiSlsDetector::setFileName(const std::string &fname,
                                          int detPos) {
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

int multiSlsDetector::setFramesPerFile(int f, int detPos) {
    if (detPos >= 0) {
        return detectors[detPos]->setFramesPerFile(f);
    }
    auto r = parallelCall(&slsDetector::setFramesPerFile, f);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::getFramesPerFile(int detPos) const {
    if (detPos >= 0) {
        return detectors[detPos]->getFramesPerFile();
    }
    auto r = parallelCall(&slsDetector::getFramesPerFile);
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

int multiSlsDetector::setPartialFramesPadding(bool padding, int detPos) {
    if (detPos >= 0)
        return static_cast<int>(
            detectors[detPos]->setPartialFramesPadding(padding));
    auto r = parallelCall(&slsDetector::setPartialFramesPadding, padding);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::getPartialFramesPadding(int detPos) const {
    if (detPos >= 0)
        return static_cast<int>(detectors[detPos]->getPartialFramesPadding());
    auto r = parallelCall(&slsDetector::getPartialFramesPadding);
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

int multiSlsDetector::getFileIndex(int detPos) const {
    if (detPos >= 0)
        return detectors[detPos]->getFileIndex();
    auto r = parallelCall(&slsDetector::getFileIndex);
    return sls::minusOneIfDifferent(r);
}

void multiSlsDetector::startReceiver(int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->startReceiver();
    }

    // multi
    parallelCall(&slsDetector::startReceiver);
}

void multiSlsDetector::stopReceiver(int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->stopReceiver();
    }

    // multi
    parallelCall(&slsDetector::stopReceiver);
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
    if ((detectors.empty()) || (sls::anyEqualTo(r, -1))) {
        return -1;
    }

    // return average
    return ((sls::sum(r)) / (int)detectors.size());
}

uint64_t multiSlsDetector::getReceiverCurrentFrameIndex(int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->getReceiverCurrentFrameIndex();
    }

    // multi
    auto r = parallelCall(&slsDetector::getReceiverCurrentFrameIndex);

    // prevent divide by all or do not take avg when -1 for "did not connect"
    if ((detectors.empty()) ||
        (sls::anyEqualTo(r, static_cast<uint64_t>(-1)))) {
        return -1;
    }

    // return average
    return ((sls::sum(r)) / (int)detectors.size());
}

void multiSlsDetector::resetFramesCaught(int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->resetFramesCaught();
    }

    // multi
    parallelCall(&slsDetector::resetFramesCaught);
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
    if (getNumberofUDPInterfaces() == 2) {
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
    bool numInterfaces = getNumberofUDPInterfaces(); // cannot pick up from zmq

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
                        quadEnable = (doc["quad"].GetUint() == 0) ? false : true;
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
                    if (getDetectorTypeAsEnum() == CHIPTESTBOARD) {
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
        FILE_LOG(logINFOBLUE)
            << "Call Back Info:"
            << "\n\t nDetPixelsX: "  << nDetPixelsX
            << "\n\t nDetPixelsY: "  << nDetPixelsY
            << "\n\t databytes: "  << multisize
            << "\n\t dynamicRange: "  << dynamicRange; 

        // send data to callback
        if (data) {
            setCurrentProgress(currentAcquisitionIndex + 1);
            // 4bit gap pixels
            if (dynamicRange == 4 && gappixelsenable) {
                if (quadEnable) {
					nDetPixelsX += 2;
					nDetPixelsY += 2;
				} else {
					nDetPixelsX = nX * (nPixelsX + 3);
					nDetPixelsY = nY * (nPixelsY + 1);
				}
                int n = processImageWithGapPixels(multiframe, multigappixels, quadEnable);
                FILE_LOG(logINFORED)
                    << "Call Back Info Recalculated:"
                    << "\n\t nDetPixelsX: "  << nDetPixelsX
                    << "\n\t nDetPixelsY: "  << nDetPixelsY
                    << "\n\t databytes: "  << n; 
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

int multiSlsDetector::processImageWithGapPixels(char *image, char *&gpImage, bool quadEnable) {
    // eiger 4 bit mode 
    int nxb = multi_shm()->numberOfDetector[X] * (512 + 3); //(divided by 2 already)
    int nyb = multi_shm()->numberOfDetector[Y] * (256 + 1);
    int nchipInRow = 4;
    int nxchip = multi_shm()->numberOfDetector[X] * 4;
    int nychip = multi_shm()->numberOfDetector[Y] * 1;
    if (quadEnable) {
        nxb = multi_shm()->numberOfDetector[X] * (256 + 1); //(divided by 2 already)
        nyb = multi_shm()->numberOfDetector[Y] * (512 + 2);
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
             ++ichipy) { // for each row in a chip
            for (int col = 0; col < nxchip; ++col) {// for each chip in a row
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
                for (int col = 0; col < nxchip; ++col) {// for each chip in a row
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

int multiSlsDetector::setFileWrite(bool value, int detPos) {
    if (detPos >= 0) {
        return static_cast<int>(detectors[detPos]->setFileWrite(value));
    }
    auto r = parallelCall(&slsDetector::setFileWrite, value);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::getFileWrite(int detPos) const {
    if (detPos >= 0) {
        return static_cast<int>(detectors[detPos]->getFileWrite());
    }
    auto r = parallelCall(&slsDetector::getFileWrite);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setMasterFileWrite(bool value, int detPos) {
    if (detPos >= 0) {
        return static_cast<int>(detectors[detPos]->setMasterFileWrite(value));
    }
    auto r = parallelCall(&slsDetector::setMasterFileWrite, value);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::getMasterFileWrite(int detPos) const {
    if (detPos >= 0) {
        return static_cast<int>(detectors[detPos]->getMasterFileWrite());
    }
    auto r = parallelCall(&slsDetector::getMasterFileWrite);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::setFileOverWrite(bool enable, int detPos) {
    if (detPos >= 0) {
        return static_cast<int>(detectors[detPos]->setFileOverWrite(enable));
    }
    auto r = parallelCall(&slsDetector::setFileOverWrite, enable);
    return sls::minusOneIfDifferent(r);
}

int multiSlsDetector::getFileOverWrite(int detPos) const {
    if (detPos >= 0) {
        return static_cast<int>(detectors[detPos]->getFileOverWrite());
    }
    auto r = parallelCall(&slsDetector::getFileOverWrite);
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

int multiSlsDetector::enableDataStreamingFromReceiver(int enable, int detPos) {
    // single
    if (detPos >= 0) {
        return static_cast<int>(
            detectors[detPos]->enableDataStreamingFromReceiver(enable));
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
        return static_cast<int>(detectors[detPos]->setReceiverSilentMode(i));
    }

    // multi
    auto r = parallelCall(&slsDetector::setReceiverSilentMode, i);
    return sls::minusOneIfDifferent(r);
}

void multiSlsDetector::setPattern(const std::string &fname, int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->setPattern(fname);
    }

    // multi
    parallelCall(&slsDetector::setPattern, fname);
}

uint64_t multiSlsDetector::setPatternIOControl(uint64_t word, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setPatternIOControl(word);
    }

    // multi
    auto r = parallelCall(&slsDetector::setPatternIOControl, word);
    return sls::minusOneIfDifferent(r);
}

uint64_t multiSlsDetector::setPatternClockControl(uint64_t word, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setPatternClockControl(word);
    }

    // multi
    auto r = parallelCall(&slsDetector::setPatternClockControl, word);
    return sls::minusOneIfDifferent(r);
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

void multiSlsDetector::setPatternLoops(int level, int start, int stop, int n,
                                       int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->setPatternLoops(level, start, stop, n);
    }

    // multi
    parallelCall(&slsDetector::setPatternLoops, level, start, stop, n);
}

std::array<int, 3> multiSlsDetector::getPatternLoops(int level, int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setPatternLoops(level, -1, -1, -1);
    }

    // multi
    auto r = parallelCall(&slsDetector::setPatternLoops, level, -1, -1, -1);
    return sls::minusOneIfDifferent(r);
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

uint64_t multiSlsDetector::setPatternWaitTime(int level, uint64_t t,
                                              int detPos) {
    // single
    if (detPos >= 0) {
        return detectors[detPos]->setPatternWaitTime(level, t);
    }

    // multi
    auto r = parallelCall(&slsDetector::setPatternWaitTime, level, t);
    return sls::minusOneIfDifferent(r);
}

void multiSlsDetector::setPatternMask(uint64_t mask, int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->setPatternMask(mask);
    }

    // multi
    parallelCall(&slsDetector::setPatternMask, mask);
}

uint64_t multiSlsDetector::getPatternMask(int detPos) {
    if (detPos >= 0)
        return detectors[detPos]->getPatternMask();
    auto r = parallelCall(&slsDetector::getPatternMask);
    if (sls::allEqual(r)) {
        return r.front();
    } else {

        throw RuntimeError("multiSlsDetector::getPatternMask: Error: Different "
                           "Values returned)");
    }
}

void multiSlsDetector::setPatternBitMask(uint64_t mask, int detPos) {
    if (detPos >= 0) {
        detectors[detPos]->setPatternBitMask(mask);
    }
    parallelCall(&slsDetector::setPatternBitMask, mask);
}

uint64_t multiSlsDetector::getPatternBitMask(int detPos) {
    if (detPos >= 0) {
        return detectors[detPos]->getPatternBitMask();
    }
    auto r = parallelCall(&slsDetector::getPatternBitMask);
    if (sls::allEqual(r)) {
        return r.front();
    }

    // should not have different values
    throw RuntimeError(
        "multiSlsDetector::getPatternBitMask Different Values returned)");
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

void multiSlsDetector::setDigitalIODelay(uint64_t pinMask, int delay,
                                        int detPos) {
    // single
    if (detPos >= 0) {
        detectors[detPos]->setDigitalIODelay(pinMask, delay);
    }

    // multi
    parallelCall(&slsDetector::setDigitalIODelay, pinMask, delay);
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
                FILE_LOG(logDEBUG1) << "Line is a comment \n" << str;
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
        throw RuntimeError("Error opening  " + fname + " for reading");
    }
    FILE_LOG(logDEBUG1) << "Read  " << iline << " lines";
    return OK;
}

int multiSlsDetector::dumpDetectorSetup(const std::string &fname, int level) {
    detectorType type = getDetectorTypeAsEnum();
    std::vector<std::string> names;
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

    // Workaround to bo able to suplly ecexuteLine with char**
    const int n_arguments = 1;
    char buffer[1000]; // TODO! this should not be hardcoded!
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
            sls::strcpy_safe(buffer, name.c_str()); // this is...
            outfile << name << " "
                    << cmd.executeLine(n_arguments, args, GET_ACTION)
                    << std::endl;
        }
        outfile.close();
    } else {
        throw RuntimeError("Error opening parameters file " + fname +
                           " for writing");
    }

    FILE_LOG(logDEBUG1) << "wrote " << names.size() << " lines to  "
                        << outfname;
    return OK;
}

void multiSlsDetector::registerAcquisitionFinishedCallback(
    void (*func)(double, int, void *), void *pArg) {
    acquisition_finished = func;
    acqFinished_p = pArg;
}

void multiSlsDetector::registerProgressCallback(void (*func)(double, void *),
                                                void *pArg) {
    progress_call = func;
    pProgressCallArg = pArg;
}

void multiSlsDetector::registerDataCallback(
    void (*userCallback)(detectorData *, uint64_t, uint32_t, void *),
    void *pArg) {
    dataReady = userCallback;
    pCallbackArg = pArg;
    if (getUseReceiverFlag()) {
        if (dataReady == nullptr) {
            enableDataStreamingToClient(0);
            enableDataStreamingFromReceiver(0);
        } else {
            enableDataStreamingToClient(1);
            enableDataStreamingFromReceiver(1);
        }
    }
}

int multiSlsDetector::setTotalProgress() {
    int nf = Parallel(&slsDetector::setTimer, {}, FRAME_NUMBER, -1).tsquash("Inconsistent number of frames");
    int nc  = Parallel(&slsDetector::setTimer, {}, CYCLES_NUMBER, -1).tsquash("Inconsistent number of cycles");
    if (nf == 0 || nc == 0) {
        throw RuntimeError("Number of frames or cycles is 0");
    }

    int ns = 1;
    if (getDetectorTypeAsEnum() == JUNGFRAU) {
        ns = Parallel(&slsDetector::setTimer, {}, STORAGE_CELL_NUMBER, -1).tsquash("Inconsistent number of additional storage cells");
        ++ns;
    }

    totalProgress = nf * nc * ns;
    FILE_LOG(logDEBUG1) << "nf " << nf << " nc " << nc << " ns " << ns;
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
    if (static_cast<int>(isAcquireReady()) == FAIL) {
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

    bool receiver = getUseReceiverFlag();
    progressIndex = 0;
    setJoinThreadFlag(false);

    // verify receiver is idle
    if (receiver) {
        std::lock_guard<std::mutex> lock(mg);
        if (getReceiverStatus() != IDLE) {
            stopReceiver();
        }
    }
    setTotalProgress();

    startProcessingThread();

    // resets frames caught in receiver
    if (receiver) {
        std::lock_guard<std::mutex> lock(mg);
        resetFramesCaught();
    }

    // start receiver
    if (receiver) {
        std::lock_guard<std::mutex> lock(mg);
        startReceiver();
        // let processing thread listen to these packets
        sem_post(&sem_newRTAcquisition);
    }

    startAndReadAll();

    // stop receiver
    if (receiver) {
        std::lock_guard<std::mutex> lock(mg);
        stopReceiver();
            if (dataReady != nullptr) {
                sem_wait(&sem_endRTAcquisition); // waits for receiver's
            }
            // external process to be
            // done sending data to gui
        
        incrementFileIndex();
    }

    // waiting for the data processing thread to finish!
    setJoinThreadFlag(true);
    sem_post(&sem_newRTAcquisition);
    dataProcessingThread.join();

    if (progress_call != nullptr) {
        progress_call(getCurrentProgress(), pProgressCallArg);
    }

    if (acquisition_finished != nullptr) {
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

void multiSlsDetector::processData() {
    if (getUseReceiverFlag()) {
        if (dataReady != nullptr) {
            readFrameFromReceiver();
        }
        // only update progress
        else {
            int caught = -1;
            while (true) {
                // to exit acquire by typing q
                if (kbhit() != 0) {
                    if (fgetc(stdin) == 'q') {
                        FILE_LOG(logINFO)
                            << "Caught the command to stop acquisition";
                        stopAcquisition();
                    }
                }
                // get progress
                {
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






