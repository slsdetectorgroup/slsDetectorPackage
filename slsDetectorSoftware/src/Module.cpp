#include "Module.h"
#include "sls/ClientSocket.h"
#include "SharedMemory.h"
#include "sls/ToString.h"
#include "sls/container_utils.h"
#include "sls/file_utils.h"
#include "sls/network_utils.h"
#include "sls/sls_detector_exceptions.h"
#include "sls/sls_detector_funcs.h"
#include "sls/string_utils.h"
#include "sls/versionAPI.h"

#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <thread>

namespace sls {

// creating new shm
Module::Module(detectorType type, int det_id, int module_id, bool verify)
    : moduleId(module_id), shm(det_id, module_id) {

    // ensure shared memory was not created before
    if (shm.IsExisting()) {
        LOG(logWARNING) << "This shared memory should have been "
                           "deleted before! "
                        << shm.GetName() << ". Freeing it again";
        shm.RemoveSharedMemory();
    }

    initSharedMemory(type, det_id, verify);
}

// opening existing shm
Module::Module(int det_id, int module_id, bool verify)
    : moduleId(module_id), shm(det_id, module_id) {

    // getDetectorType From shm will check if existing
    detectorType type = getDetectorTypeFromShm(det_id, verify);
    initSharedMemory(type, det_id, verify);
}

Module::~Module() = default;

void Module::freeSharedMemory() {
    if (shm.IsExisting()) {
        shm.RemoveSharedMemory();
    }
}

bool Module::isFixedPatternSharedMemoryCompatible() const {
    return (shm()->shmversion >= SLS_SHMAPIVERSION);
}

std::string Module::getHostname() const { return shm()->hostname; }

void Module::setHostname(const std::string &hostname,
                         const bool initialChecks) {
    sls::strcpy_safe(shm()->hostname, hostname.c_str());
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    client.close();
    try {
        checkDetectorVersionCompatibility();
        LOG(logINFO) << "Detector Version Compatibility - Success";
    } catch (const DetectorError &e) {
        if (!initialChecks) {
            LOG(logWARNING) << "Bypassing Initial Checks at your own risk!";
        } else {
            throw;
        }
    }
    if (shm()->myDetectorType == EIGER) {
        setActivate(true);
    }
}

int64_t Module::getFirmwareVersion() const {
    return sendToDetector<int64_t>(F_GET_FIRMWARE_VERSION);
}

int64_t Module::getDetectorServerVersion() const {
    return sendToDetector<int64_t>(F_GET_SERVER_VERSION);
}

int64_t Module::getSerialNumber() const {
    return sendToDetector<int64_t>(F_GET_SERIAL_NUMBER);
}

int64_t Module::getReceiverSoftwareVersion() const {
    if (shm()->useReceiverFlag) {
        return sendToReceiver<int64_t>(F_GET_RECEIVER_VERSION);
    }
    return -1;
}

// static function
slsDetectorDefs::detectorType
Module::getTypeFromDetector(const std::string &hostname, int cport) {
    LOG(logDEBUG1) << "Getting detector type ";
    sls::ClientSocket socket("Detector", hostname, cport);
    socket.Send(F_GET_DETECTOR_TYPE);
    socket.Receive<int>(); // TODO! Should we look at this OK/FAIL?
    auto retval = socket.Receive<detectorType>();
    LOG(logDEBUG1) << "Detector type is " << retval;
    return retval;
}

slsDetectorDefs::detectorType Module::getDetectorType() const {
    return shm()->myDetectorType;
}

void Module::updateNumberOfChannels() {
    if (shm()->myDetectorType == CHIPTESTBOARD ||
        shm()->myDetectorType == MOENCH) {
        std::array<int, 2> retvals{};
        sendToDetector(F_GET_NUM_CHANNELS, nullptr, retvals);
        shm()->nChan.x = retvals[0];
        shm()->nChan.y = retvals[1];
    }
}

slsDetectorDefs::xy Module::getNumberOfChannels() const {
    slsDetectorDefs::xy coord{};
    coord.x = (shm()->nChan.x * shm()->nChip.x);
    coord.y = (shm()->nChan.y * shm()->nChip.y);
    return coord;
}

void Module::updateNumberOfDetector(slsDetectorDefs::xy det) {
    shm()->numberOfDetector = det;
    int args[2] = {shm()->numberOfDetector.y, moduleId};
    sendToDetector(F_SET_POSITION, args, nullptr);
}

slsDetectorDefs::detectorSettings Module::getSettings() const {
    return sendToDetector<detectorSettings>(F_SET_SETTINGS, GET_FLAG);
}

void Module::setSettings(detectorSettings isettings) {
    if (shm()->myDetectorType == EIGER) {
        throw RuntimeError(
            "Cannot set settings for Eiger. Use threshold energy.");
    }
    sendToDetector<int>(F_SET_SETTINGS, isettings);
}

void Module::loadSettingsFile(const std::string &fname) {
    // find specific file if it has detid in file name (.snxxx)
    if (shm()->myDetectorType == EIGER || shm()->myDetectorType == MYTHEN3) {
        std::ostringstream ostfn;
        ostfn << fname;
        if (fname.find(".sn") == std::string::npos &&
            fname.find(".trim") == std::string::npos &&
            fname.find(".settings") == std::string::npos) {
            ostfn << ".sn" << std::setfill('0') << std::setw(3) << std::dec
                  << getSerialNumber();
        }
        auto myMod = readSettingsFile(ostfn.str());
        setModule(myMod);
    } else {
        throw RuntimeError("not implemented for this detector");
    }
}

int Module::getAllTrimbits() const {
    return sendToDetector<int>(F_SET_ALL_TRIMBITS, GET_FLAG);
}

void Module::setAllTrimbits(int val) {
    sendToDetector<int>(F_SET_ALL_TRIMBITS, val);
}

int64_t Module::getNumberOfFrames() const {
    return sendToDetector<int64_t>(F_GET_NUM_FRAMES);
}

void Module::setNumberOfFrames(int64_t value) {
    sendToDetector(F_SET_NUM_FRAMES, value, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_SET_NUM_FRAMES, value, nullptr);
    }
}

int64_t Module::getNumberOfTriggers() const {
    return sendToDetector<int64_t>(F_GET_NUM_TRIGGERS);
}

void Module::setNumberOfTriggers(int64_t value) {
    sendToDetector(F_SET_NUM_TRIGGERS, value, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_NUM_TRIGGERS, value, nullptr);
    }
}

int64_t Module::getExptime(int gateIndex) const {
    return sendToDetector<int64_t>(F_GET_EXPTIME, gateIndex);
}

void Module::setExptime(int gateIndex, int64_t value) {
    int64_t prevVal = value;
    if (shm()->myDetectorType == EIGER) {
        prevVal = getExptime(-1);
    }
    int64_t args[]{static_cast<int64_t>(gateIndex), value};
    sendToDetector(F_SET_EXPTIME, args, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_SET_EXPTIME, args, nullptr);
    }
    if (prevVal != value) {
        updateRateCorrection();
    }
}

int64_t Module::getPeriod() const {
    return sendToDetector<int64_t>(F_GET_PERIOD);
}

void Module::setPeriod(int64_t value) {
    sendToDetector(F_SET_PERIOD, value, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_SET_PERIOD, value, nullptr);
    }
}

int64_t Module::getDelayAfterTrigger() const {
    return sendToDetector<int64_t>(F_GET_DELAY_AFTER_TRIGGER);
}

void Module::setDelayAfterTrigger(int64_t value) {
    sendToDetector(F_SET_DELAY_AFTER_TRIGGER, value, nullptr);
}

int64_t Module::getNumberOfFramesLeft() const {
    return sendToDetectorStop<int64_t>(F_GET_FRAMES_LEFT);
}

int64_t Module::getNumberOfTriggersLeft() const {
    return sendToDetectorStop<int64_t>(F_GET_TRIGGERS_LEFT);
}

int64_t Module::getDelayAfterTriggerLeft() const {
    return sendToDetectorStop<int64_t>(F_GET_DELAY_AFTER_TRIGGER_LEFT);
}

int64_t Module::getPeriodLeft() const {
    return sendToDetectorStop<int64_t>(F_GET_PERIOD_LEFT);
}

int Module::getDynamicRange() const {
    return sendToDetector<int>(F_SET_DYNAMIC_RANGE, GET_FLAG);
}

void Module::setDynamicRange(int dr) {
    int prev_val = dr;
    if (shm()->myDetectorType == EIGER) {
        prev_val = getDynamicRange();
    }

    auto retval = sendToDetector<int>(F_SET_DYNAMIC_RANGE, dr);
    if (shm()->useReceiverFlag) {
        sendToReceiver<int>(F_SET_RECEIVER_DYNAMIC_RANGE, retval);
    }

    // update speed
    if (shm()->myDetectorType == EIGER) {
        if (dr == 32) {
            LOG(logINFO) << "Setting Clock to Quarter Speed to cope with "
                            "Dynamic Range of 32";
            setClockDivider(RUN_CLOCK, 2);
        } else {
            LOG(logINFO) << "Setting Clock to Full Speed for Dynamic Range of "
                         << dr;
            setClockDivider(RUN_CLOCK, 0);
        }
        // EIGER only, update speed and rate correction when dr changes
        if (dr != prev_val) {
            updateRateCorrection();
        }
    }
}

slsDetectorDefs::timingMode Module::getTimingMode() const {
    return sendToDetector<timingMode>(F_SET_TIMING_MODE, GET_FLAG);
}

void Module::setTimingMode(timingMode value) {
    sendToDetector<int>(F_SET_TIMING_MODE, value);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_TIMING_MODE, value, nullptr);
    }
}

int Module::getClockDivider(int clkIndex) const {
    return sendToDetector<int>(F_GET_CLOCK_DIVIDER, clkIndex);
}

void Module::setClockDivider(int clkIndex, int value) {
    int args[]{clkIndex, value};
    sendToDetector(F_SET_CLOCK_DIVIDER, args, nullptr);
}

int Module::getClockPhase(int clkIndex, bool inDegrees) const {
    int args[]{clkIndex, static_cast<int>(inDegrees)};
    return sendToDetector<int>(F_GET_CLOCK_PHASE, args);
}

void Module::setClockPhase(int clkIndex, int value, bool inDegrees) {
    int args[]{clkIndex, value, static_cast<int>(inDegrees)};
    sendToDetector(F_SET_CLOCK_PHASE, args, nullptr);
}

int Module::getMaxClockPhaseShift(int clkIndex) const {
    return sendToDetector<int>(F_GET_MAX_CLOCK_PHASE_SHIFT, clkIndex);
}

int Module::getClockFrequency(int clkIndex) const {
    return sendToDetector<int>(F_GET_CLOCK_FREQUENCY, clkIndex);
}

void Module::setClockFrequency(int clkIndex, int value) {
    int args[]{clkIndex, value};
    sendToDetector(F_SET_CLOCK_FREQUENCY, args, nullptr);
}

int Module::getDAC(dacIndex index, bool mV) const {
    int args[]{static_cast<int>(index), static_cast<int>(mV), GET_FLAG};
    return sendToDetector<int>(F_SET_DAC, args);
}

void Module::setDAC(int val, dacIndex index, bool mV) {
    int args[]{static_cast<int>(index), static_cast<int>(mV), val};
    sendToDetector<int>(F_SET_DAC, args);
}

bool Module::getPowerChip() const {
    return sendToDetector<int>(F_POWER_CHIP, GET_FLAG);
}

void Module::setPowerChip(bool on) {
    sendToDetector<int>(F_POWER_CHIP, static_cast<int>(on));
}

int Module::getImageTestMode() const {
    return sendToDetector<int>(F_GET_IMAGE_TEST_MODE);
}

void Module::setImageTestMode(const int value) {
    sendToDetector(F_SET_IMAGE_TEST_MODE, value, nullptr);
}

int Module::getADC(dacIndex index) const {
    return sendToDetectorStop<int>(F_GET_ADC, index);
}

int Module::getOnChipDAC(slsDetectorDefs::dacIndex index, int chipIndex) const {
    int args[]{static_cast<int>(index), chipIndex};
    return sendToDetector<int>(F_GET_ON_CHIP_DAC, args);
}

void Module::setOnChipDAC(slsDetectorDefs::dacIndex index, int chipIndex,
                          int value) {
    int args[]{static_cast<int>(index), chipIndex, value};
    sendToDetector(F_SET_ON_CHIP_DAC, args, nullptr);
}

slsDetectorDefs::externalSignalFlag
Module::getExternalSignalFlags(int signalIndex) const {
    return sendToDetector<slsDetectorDefs::externalSignalFlag>(
        F_GET_EXTERNAL_SIGNAL_FLAG, signalIndex);
}

void Module::setExternalSignalFlags(int signalIndex, externalSignalFlag type) {
    int args[]{signalIndex, static_cast<int>(type)};
    sendToDetector(F_SET_EXTERNAL_SIGNAL_FLAG, args, nullptr);
}

bool Module::getParallelMode() const {
    return sendToDetector<int>(F_GET_PARALLEL_MODE);
}

void Module::setParallelMode(const bool enable) {
    sendToDetector(F_SET_PARALLEL_MODE, static_cast<int>(enable), nullptr);
}

// Acquisition

void Module::startReceiver() {
    shm()->stoppedFlag = false;
    sendToReceiver(F_START_RECEIVER);
}

void Module::stopReceiver() {
    sendToReceiver(F_STOP_RECEIVER, static_cast<int>(shm()->stoppedFlag),
                   nullptr);
}

void Module::startAcquisition() {
    shm()->stoppedFlag = false;
    sendToDetector(F_START_ACQUISITION);
}

void Module::startReadout() {
    shm()->stoppedFlag = false;
    sendToDetector(F_START_READOUT);
}

void Module::stopAcquisition() {
    // get status before stopping acquisition
    runStatus s = ERROR, r = ERROR;
    bool zmqstreaming = false;
    try {
        if (shm()->useReceiverFlag && getReceiverStreaming()) {
            zmqstreaming = true;
            s = getRunStatus();
            r = getReceiverStatus();
        }
    } catch (...) {
        // if receiver crashed, stop detector in any case
        zmqstreaming = false;
    }

    sendToDetectorStop(F_STOP_ACQUISITION);
    shm()->stoppedFlag = true;

    // if rxr streaming and acquisition finished, restream dummy stop packet
    if (zmqstreaming && (s == IDLE) && (r == IDLE)) {
        restreamStopFromReceiver();
    }
}

void Module::restreamStopFromReceiver() {
    sendToReceiver(F_RESTREAM_STOP_FROM_RECEIVER);
}

void Module::startAndReadAll() {
    shm()->stoppedFlag = false;
    sendToDetector(F_START_AND_READ_ALL);
}

slsDetectorDefs::runStatus Module::getRunStatus() const {
    return sendToDetectorStop<runStatus>(F_GET_RUN_STATUS);
}

slsDetectorDefs::runStatus Module::getReceiverStatus() const {
    return sendToReceiver<runStatus>(F_GET_RECEIVER_STATUS);
}

double Module::getReceiverProgress() const {
    return sendToReceiver<double>(F_GET_RECEIVER_PROGRESS);
}

int64_t Module::getFramesCaughtByReceiver() const {
    return sendToReceiver<int64_t>(F_GET_RECEIVER_FRAMES_CAUGHT);
}

std::vector<uint64_t> Module::getNumMissingPackets() const {
    // TODO!(Erik) Refactor
    LOG(logDEBUG1) << "Getting num missing packets";
    if (shm()->useReceiverFlag) {
        auto client = ReceiverSocket(shm()->rxHostname, shm()->rxTCPPort);
        client.Send(F_GET_NUM_MISSING_PACKETS);
        if (client.Receive<int>() == FAIL) {
            throw RuntimeError("Receiver " + std::to_string(moduleId) +
                               " returned error: " + client.readErrorMessage());
        } else {
            auto nports = client.Receive<int>();
            std::vector<uint64_t> retval(nports);
            client.Receive(retval);
            LOG(logDEBUG1) << "Missing packets of Receiver" << moduleId << ": "
                           << sls::ToString(retval);
            return retval;
        }
    }
    throw RuntimeError("No receiver to get missing packets.");
}

uint64_t Module::getStartingFrameNumber() const {
    return sendToDetector<uint64_t>(F_GET_STARTING_FRAME_NUMBER);
}

void Module::setStartingFrameNumber(uint64_t value) {
    sendToDetector(F_SET_STARTING_FRAME_NUMBER, value, nullptr);
}

void Module::sendSoftwareTrigger() { sendToDetectorStop(F_SOFTWARE_TRIGGER); }

defs::scanParameters Module::getScan() const {
    return sendToDetector<defs::scanParameters>(F_GET_SCAN);
}

void Module::setScan(const defs::scanParameters t) {
    auto retval = sendToDetector<int64_t>(F_SET_SCAN, t);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_SCAN, t, nullptr);
    }
    // if disabled, retval is 1, else its number of steps
    setNumberOfFrames(retval);
}

std::string Module::getScanErrorMessage() const {
    char retval[MAX_STR_LENGTH]{};
    sendToDetector(F_GET_SCAN_ERROR_MESSAGE, nullptr, retval);
    return retval;
}

// Network Configuration (Detector<->Receiver)

int Module::getNumberofUDPInterfacesFromShm() const {
    return shm()->numUDPInterfaces;
}

int Module::getNumberofUDPInterfaces() const {
    shm()->numUDPInterfaces = sendToDetector<int>(F_GET_NUM_INTERFACES);
    return shm()->numUDPInterfaces;
}

void Module::setNumberofUDPInterfaces(int n) {
    sendToDetector(F_SET_NUM_INTERFACES, n, nullptr);
    shm()->numUDPInterfaces = n;
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_NUM_INTERFACES, n, nullptr);
    }
}

int Module::getSelectedUDPInterface() const {
    return sendToDetector<int>(F_GET_INTERFACE_SEL);
}

void Module::selectUDPInterface(int n) {
    sendToDetector(F_SET_INTERFACE_SEL, n, nullptr);
}

sls::IpAddr Module::getSourceUDPIP() const {
    return sendToDetector<sls::IpAddr>(F_GET_SOURCE_UDP_IP);
}

void Module::setSourceUDPIP(const IpAddr ip) {
    if (ip == 0) {
        throw RuntimeError("Invalid source udp ip address");
    }
    sendToDetector(F_SET_SOURCE_UDP_IP, ip, nullptr);
}

sls::IpAddr Module::getSourceUDPIP2() const {
    return sendToDetector<sls::IpAddr>(F_GET_SOURCE_UDP_IP2);
}

void Module::setSourceUDPIP2(const IpAddr ip) {
    if (ip == 0) {
        throw RuntimeError("Invalid source udp ip address2");
    }
    sendToDetector(F_SET_SOURCE_UDP_IP2, ip, nullptr);
}

sls::MacAddr Module::getSourceUDPMAC() const {
    return sendToDetector<sls::MacAddr>(F_GET_SOURCE_UDP_MAC);
}

void Module::setSourceUDPMAC(const sls::MacAddr mac) {
    if (mac == 0) {
        throw RuntimeError("Invalid source udp mac address");
    }
    sendToDetector(F_SET_SOURCE_UDP_MAC, mac, nullptr);
}

sls::MacAddr Module::getSourceUDPMAC2() const {
    return sendToDetector<sls::MacAddr>(F_GET_SOURCE_UDP_MAC2);
}

void Module::setSourceUDPMAC2(const sls::MacAddr mac) {
    if (mac == 0) {
        throw RuntimeError("Invalid source udp mac address2");
    }
    sendToDetector(F_SET_SOURCE_UDP_MAC2, mac, nullptr);
}

sls::IpAddr Module::getDestinationUDPIP() const {
    return sendToDetector<sls::IpAddr>(F_GET_DEST_UDP_IP);
}

void Module::setDestinationUDPIP(const IpAddr ip) {
    if (ip == 0) {
        throw RuntimeError("Invalid destination udp ip address");
    }
    sendToDetector(F_SET_DEST_UDP_IP, ip, nullptr);
    if (shm()->useReceiverFlag) {
        sls::MacAddr retval(0LU);
        sendToReceiver(F_SET_RECEIVER_UDP_IP, ip, retval);
        LOG(logINFO) << "Setting destination udp mac of detector " << moduleId
                     << " to " << retval;
        sendToDetector(F_SET_DEST_UDP_MAC, retval, nullptr);
    }
}

sls::IpAddr Module::getDestinationUDPIP2() const {
    return sendToDetector<sls::IpAddr>(F_GET_DEST_UDP_IP2);
}

void Module::setDestinationUDPIP2(const IpAddr ip) {
    LOG(logDEBUG1) << "Setting destination udp ip2 to " << ip;
    if (ip == 0) {
        throw RuntimeError("Invalid destination udp ip address2");
    }

    sendToDetector(F_SET_DEST_UDP_IP2, ip, nullptr);
    if (shm()->useReceiverFlag) {
        sls::MacAddr retval(0LU);
        sendToReceiver(F_SET_RECEIVER_UDP_IP2, ip, retval);
        LOG(logINFO) << "Setting destination udp mac2 of detector " << moduleId
                     << " to " << retval;
        sendToDetector(F_SET_DEST_UDP_MAC2, retval, nullptr);
    }
}

sls::MacAddr Module::getDestinationUDPMAC() const {
    return sendToDetector<sls::MacAddr>(F_GET_DEST_UDP_MAC);
}

void Module::setDestinationUDPMAC(const MacAddr mac) {
    if (mac == 0) {
        throw RuntimeError("Invalid destination udp mac address");
    }
    sendToDetector(F_SET_DEST_UDP_MAC, mac, nullptr);
}

sls::MacAddr Module::getDestinationUDPMAC2() const {
    return sendToDetector<sls::MacAddr>(F_GET_DEST_UDP_MAC2);
}

void Module::setDestinationUDPMAC2(const MacAddr mac) {
    if (mac == 0) {
        throw RuntimeError("Invalid desinaion udp mac address2");
    }
    sendToDetector(F_SET_DEST_UDP_MAC2, mac, nullptr);
}

int Module::getDestinationUDPPort() const {
    return sendToDetector<int>(F_GET_DEST_UDP_PORT);
}

void Module::setDestinationUDPPort(const int port) {
    sendToDetector(F_SET_DEST_UDP_PORT, port, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_UDP_PORT, port, nullptr);
    }
}

int Module::getDestinationUDPPort2() const {
    return sendToDetector<int>(F_GET_DEST_UDP_PORT2);
}

void Module::setDestinationUDPPort2(const int port) {
    sendToDetector(F_SET_DEST_UDP_PORT2, port, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_UDP_PORT2, port, nullptr);
    }
}

void Module::reconfigureUDPDestination() { sendToDetector(F_RECONFIGURE_UDP); }

void Module::validateUDPConfiguration() {
    sendToDetector(F_VALIDATE_UDP_CONFIG);
}

std::string Module::printReceiverConfiguration() {
    std::ostringstream os;
    os << "\n\nDetector " << moduleId << "\nReceiver Hostname:\t"
       << getReceiverHostname();

    if (shm()->myDetectorType == JUNGFRAU) {
        os << "\nNumber of Interfaces:\t" << getNumberofUDPInterfaces()
           << "\nSelected Interface:\t" << getSelectedUDPInterface();
    }

    os << "\nDetector UDP IP:\t" << getSourceUDPIP() << "\nDetector UDP MAC:\t"
       << getSourceUDPMAC() << "\nReceiver UDP IP:\t" << getDestinationUDPIP()
       << "\nReceiver UDP MAC:\t" << getDestinationUDPMAC();

    if (shm()->myDetectorType == JUNGFRAU) {
        os << "\nDetector UDP IP2:\t" << getSourceUDPIP2()
           << "\nDetector UDP MAC2:\t" << getSourceUDPMAC2()
           << "\nReceiver UDP IP2:\t" << getDestinationUDPIP2()
           << "\nReceiver UDP MAC2:\t" << getDestinationUDPMAC2();
    }
    os << "\nReceiver UDP Port:\t" << getDestinationUDPPort();
    if (shm()->myDetectorType == JUNGFRAU || shm()->myDetectorType == EIGER) {
        os << "\nReceiver UDP Port2:\t" << getDestinationUDPPort2();
    }
    os << "\n";
    return os.str();
}

bool Module::getTenGiga() const {
    return sendToDetector<int>(F_ENABLE_TEN_GIGA, GET_FLAG);
}

void Module::setTenGiga(bool value) {
    auto arg = static_cast<int>(value);
    auto retval = sendToDetector<int>(F_ENABLE_TEN_GIGA, arg);
    sendToDetectorStop<int>(F_ENABLE_TEN_GIGA, arg);
    arg = retval;
    if (shm()->useReceiverFlag && arg != GET_FLAG) {
        sendToReceiver<int>(F_ENABLE_RECEIVER_TEN_GIGA, arg);
    }
}

bool Module::getTenGigaFlowControl() const {
    return sendToDetector<int>(F_GET_TEN_GIGA_FLOW_CONTROL);
}

void Module::setTenGigaFlowControl(bool enable) {
    sendToDetector(F_SET_TEN_GIGA_FLOW_CONTROL, static_cast<int>(enable),
                   nullptr);
}

int Module::getTransmissionDelayFrame() const {
    return sendToDetector<int>(F_GET_TRANSMISSION_DELAY_FRAME);
}

void Module::setTransmissionDelayFrame(int value) {
    sendToDetector(F_SET_TRANSMISSION_DELAY_FRAME, value, nullptr);
}

int Module::getTransmissionDelayLeft() const {
    return sendToDetector<int>(F_GET_TRANSMISSION_DELAY_LEFT);
}

void Module::setTransmissionDelayLeft(int value) {
    sendToDetector(F_SET_TRANSMISSION_DELAY_LEFT, value, nullptr);
}

int Module::getTransmissionDelayRight() const {
    return sendToDetector<int>(F_GET_TRANSMISSION_DELAY_RIGHT);
}

void Module::setTransmissionDelayRight(int value) {
    sendToDetector(F_SET_TRANSMISSION_DELAY_RIGHT, value, nullptr);
}

// Receiver Config

bool Module::getUseReceiverFlag() const { return shm()->useReceiverFlag; }

std::string Module::getReceiverHostname() const {
    return std::string(shm()->rxHostname);
}

void Module::setReceiverHostname(const std::string &receiverIP) {
    LOG(logDEBUG1) << "Setting up Receiver with " << receiverIP;

    if (receiverIP == "none") {
        memset(shm()->rxHostname, 0, MAX_STR_LENGTH);
        sls::strcpy_safe(shm()->rxHostname, "none");
        shm()->useReceiverFlag = false;
    }

    if (getRunStatus() == RUNNING) {
        LOG(logWARNING) << "Acquisition already running, Stopping it.";
        stopAcquisition();
    }

    // start updating
    std::string host = receiverIP;
    auto res = sls::split(host, ':');
    if (res.size() > 1) {
        host = res[0];
        shm()->rxTCPPort = std::stoi(res[1]);
    }
    sls::strcpy_safe(shm()->rxHostname, host.c_str());
    shm()->useReceiverFlag = true;
    checkReceiverVersionCompatibility();

    // populate parameters from detector
    rxParameters retval;
    sendToDetector(F_GET_RECEIVER_PARAMETERS, nullptr, retval);

    // populate from shared memory
    retval.detType = shm()->myDetectorType;
    retval.numberOfDetector.x = shm()->numberOfDetector.x;
    retval.numberOfDetector.y = shm()->numberOfDetector.y;
    retval.moduleId = moduleId;
    memset(retval.hostname, 0, sizeof(retval.hostname));
    strcpy_safe(retval.hostname, shm()->hostname);

    sls::MacAddr retvals[2];
    sendToReceiver(F_SETUP_RECEIVER, retval, retvals);
    // update detectors with dest mac
    if (retval.udp_dstmac == 0 && retvals[0] != 0) {
        LOG(logINFO) << "Setting destination udp mac of "
                        "detector "
                     << moduleId << " to " << retvals[0];
        sendToDetector(F_SET_DEST_UDP_MAC, retvals[0], nullptr);
    }
    if (retval.udp_dstmac2 == 0 && retvals[1] != 0) {
        LOG(logINFO) << "Setting destination udp mac2 of "
                        "detector "
                     << moduleId << " to " << retvals[1];
        sendToDetector(F_SET_DEST_UDP_MAC2, retvals[1], nullptr);
    }

    shm()->numUDPInterfaces = retval.udpInterfaces;

    // to use rx_hostname if empty and also update client zmqip
    updateReceiverStreamingIP();
}

int Module::getReceiverPort() const { return shm()->rxTCPPort; }

int Module::setReceiverPort(int port_number) {
    if (port_number >= 0 && port_number != shm()->rxTCPPort) {
        if (shm()->useReceiverFlag) {
            shm()->rxTCPPort =
                sendToReceiver<int>(F_SET_RECEIVER_PORT, port_number);
        } else {
            shm()->rxTCPPort = port_number;
        }
    }
    return shm()->rxTCPPort;
}

int Module::getReceiverFifoDepth() const {
    return sendToReceiver<int>(F_SET_RECEIVER_FIFO_DEPTH, GET_FLAG);
}

void Module::setReceiverFifoDepth(int n_frames) {
    sendToReceiver<int>(F_SET_RECEIVER_FIFO_DEPTH, n_frames);
}

bool Module::getReceiverSilentMode() const {
    return sendToReceiver<int>(F_GET_RECEIVER_SILENT_MODE);
}

void Module::setReceiverSilentMode(bool enable) {
    sendToReceiver(F_SET_RECEIVER_SILENT_MODE, static_cast<int>(enable),
                   nullptr);
}

slsDetectorDefs::frameDiscardPolicy
Module::getReceiverFramesDiscardPolicy() const {
    return sendToReceiver<frameDiscardPolicy>(F_GET_RECEIVER_DISCARD_POLICY);
}

void Module::setReceiverFramesDiscardPolicy(frameDiscardPolicy f) {
    sendToReceiver(F_SET_RECEIVER_DISCARD_POLICY, static_cast<int>(f), nullptr);
}

bool Module::getPartialFramesPadding() const {
    return sendToReceiver<int>(F_GET_RECEIVER_PADDING);
}

void Module::setPartialFramesPadding(bool padding) {
    sendToReceiver(F_SET_RECEIVER_PADDING, static_cast<int>(padding), nullptr);
}

int Module::getReceiverUDPSocketBufferSize() const {
    int arg = GET_FLAG;
    return sendToReceiver<int>(F_RECEIVER_UDP_SOCK_BUF_SIZE, arg);
}

int Module::getReceiverRealUDPSocketBufferSize() const {
    return sendToReceiver<int>(F_RECEIVER_REAL_UDP_SOCK_BUF_SIZE);
}

void Module::setReceiverUDPSocketBufferSize(int udpsockbufsize) {
    sendToReceiver<int>(F_RECEIVER_UDP_SOCK_BUF_SIZE, udpsockbufsize);
}

bool Module::getReceiverLock() const {
    return sendToReceiver<int>(F_LOCK_RECEIVER, GET_FLAG);
}

void Module::setReceiverLock(bool lock) {
    sendToReceiver<int>(F_LOCK_RECEIVER, static_cast<int>(lock));
}

sls::IpAddr Module::getReceiverLastClientIP() const {
    return sendToReceiver<sls::IpAddr>(F_GET_LAST_RECEIVER_CLIENT_IP);
}

std::array<pid_t, NUM_RX_THREAD_IDS> Module::getReceiverThreadIds() const {
    return sendToReceiver<std::array<pid_t, NUM_RX_THREAD_IDS>>(
        F_GET_RECEIVER_THREAD_IDS);
}

// File
slsDetectorDefs::fileFormat Module::getFileFormat() const {
    return sendToReceiver<fileFormat>(F_GET_RECEIVER_FILE_FORMAT);
}

void Module::setFileFormat(fileFormat f) {
    sendToReceiver(F_SET_RECEIVER_FILE_FORMAT, f, nullptr);
}

std::string Module::getFilePath() const {
    char ret[MAX_STR_LENGTH]{};
    sendToReceiver(F_GET_RECEIVER_FILE_PATH, nullptr, ret);
    return ret;
}

void Module::setFilePath(const std::string &path) {
    if (path.empty()) {
        throw RuntimeError("Cannot set empty file path");
    }
    char args[MAX_STR_LENGTH]{};
    sls::strcpy_safe(args, path.c_str());
    sendToReceiver(F_SET_RECEIVER_FILE_PATH, args, nullptr);
}

std::string Module::getFileName() const {
    char buff[MAX_STR_LENGTH]{};
    sendToReceiver(F_GET_RECEIVER_FILE_NAME, nullptr, buff);
    return buff;
}

void Module::setFileName(const std::string &fname) {
    if (fname.empty()) {
        throw RuntimeError("Cannot set empty file name prefix");
    }
    char args[MAX_STR_LENGTH]{};
    sls::strcpy_safe(args, fname.c_str());
    sendToReceiver(F_SET_RECEIVER_FILE_NAME, args, nullptr);
}

int64_t Module::getFileIndex() const {
    return sendToReceiver<int64_t>(F_GET_RECEIVER_FILE_INDEX);
}

void Module::setFileIndex(int64_t file_index) {
    sendToReceiver(F_SET_RECEIVER_FILE_INDEX, file_index, nullptr);
}

void Module::incrementFileIndex() { sendToReceiver(F_INCREMENT_FILE_INDEX); }

bool Module::getFileWrite() const {
    return sendToReceiver<int>(F_GET_RECEIVER_FILE_WRITE);
}

void Module::setFileWrite(bool value) {
    sendToReceiver(F_SET_RECEIVER_FILE_WRITE, static_cast<int>(value), nullptr);
}

bool Module::getMasterFileWrite() const {
    return sendToReceiver<int>(F_GET_RECEIVER_MASTER_FILE_WRITE);
}

void Module::setMasterFileWrite(bool value) {
    sendToReceiver(F_SET_RECEIVER_MASTER_FILE_WRITE, static_cast<int>(value),
                   nullptr);
}

bool Module::getFileOverWrite() const {
    return sendToReceiver<int>(F_GET_RECEIVER_OVERWRITE);
}

void Module::setFileOverWrite(bool value) {
    sendToReceiver(F_SET_RECEIVER_OVERWRITE, static_cast<int>(value), nullptr);
}

int Module::getFramesPerFile() const {
    return sendToReceiver<int>(F_GET_RECEIVER_FRAMES_PER_FILE);
}

void Module::setFramesPerFile(int n_frames) {
    sendToReceiver(F_SET_RECEIVER_FRAMES_PER_FILE, n_frames, nullptr);
}

// ZMQ Streaming Parameters (Receiver<->Client)

bool Module::getReceiverStreaming() const {
    return sendToReceiver<int>(F_GET_RECEIVER_STREAMING);
}

void Module::setReceiverStreaming(bool enable) {
    sendToReceiver(F_SET_RECEIVER_STREAMING, static_cast<int>(enable), nullptr);
}

int Module::getReceiverStreamingFrequency() const {
    return sendToReceiver<int>(F_GET_RECEIVER_STREAMING_FREQUENCY);
}

void Module::setReceiverStreamingFrequency(int freq) {
    if (freq < 0) {
        throw RuntimeError("Invalid streaming frequency " +
                           std::to_string(freq));
    }
    sendToReceiver(F_SET_RECEIVER_STREAMING_FREQUENCY, freq, nullptr);
}

int Module::getReceiverStreamingTimer() const {
    return sendToReceiver<int>(F_RECEIVER_STREAMING_TIMER, GET_FLAG);
}

void Module::setReceiverStreamingTimer(int time_in_ms) {
    sendToReceiver<int>(F_RECEIVER_STREAMING_TIMER, time_in_ms);
}

int Module::getReceiverStreamingStartingFrame() const {
    return sendToReceiver<int>(F_GET_RECEIVER_STREAMING_START_FNUM);
}

void Module::setReceiverStreamingStartingFrame(int fnum) {
    if (fnum < 0) {
        throw RuntimeError("Invalid streaming starting frame number " +
                           std::to_string(fnum));
    }
    sendToReceiver(F_SET_RECEIVER_STREAMING_START_FNUM, fnum, nullptr);
}

int Module::getReceiverStreamingPort() const {
    return sendToReceiver<int>(F_GET_RECEIVER_STREAMING_PORT);
}

void Module::setReceiverStreamingPort(int port) {
    sendToReceiver(F_SET_RECEIVER_STREAMING_PORT, port, nullptr);
}

sls::IpAddr Module::getReceiverStreamingIP() const {
    return sendToReceiver<sls::IpAddr>(F_GET_RECEIVER_STREAMING_SRC_IP);
}

void Module::setReceiverStreamingIP(const sls::IpAddr ip) {
    if (ip == 0) {
        throw RuntimeError("Invalid receiver zmq ip address");
    }
    // if client zmqip is empty, update it
    if (shm()->zmqip == 0) {
        shm()->zmqip = ip;
    }
    sendToReceiver(F_SET_RECEIVER_STREAMING_SRC_IP, ip, nullptr);
}

int Module::getClientStreamingPort() const { return shm()->zmqport; }

void Module::setClientStreamingPort(int port) { shm()->zmqport = port; }

sls::IpAddr Module::getClientStreamingIP() const { return shm()->zmqip; }

void Module::setClientStreamingIP(const sls::IpAddr ip) {
    if (ip == 0) {
        throw RuntimeError("Invalid client zmq ip address");
    }
    shm()->zmqip = ip;
}

int Module::getReceiverStreamingHwm() const {
    return sendToReceiver<int>(F_GET_RECEIVER_STREAMING_HWM);
}

void Module::setReceiverStreamingHwm(const int limit) {
    sendToReceiver(F_SET_RECEIVER_STREAMING_HWM, limit, nullptr);
}

//  Eiger Specific

int64_t Module::getSubExptime() const {
    return sendToDetector<int64_t>(F_GET_SUB_EXPTIME);
}

void Module::setSubExptime(int64_t value) {
    int64_t prevVal = value;
    if (shm()->myDetectorType == EIGER) {
        prevVal = getSubExptime();
    }
    sendToDetector(F_SET_SUB_EXPTIME, value, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_SET_SUB_EXPTIME, value, nullptr);
    }
    if (prevVal != value) {
        updateRateCorrection();
    }
}

int64_t Module::getSubDeadTime() const {
    return sendToDetector<int64_t>(F_GET_SUB_DEADTIME);
}

void Module::setSubDeadTime(int64_t value) {
    sendToDetector(F_SET_SUB_DEADTIME, value, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_SET_SUB_DEADTIME, value, nullptr);
    }
}

int Module::getThresholdEnergy() const {
    return sendToDetector<int>(F_GET_THRESHOLD_ENERGY);
}

void Module::setThresholdEnergy(int e_eV, detectorSettings isettings,
                                bool trimbits) {
    // check as there is client processing
    if (shm()->myDetectorType == EIGER) {
        setThresholdEnergyAndSettings(e_eV, isettings, trimbits);
        if (shm()->useReceiverFlag) {
            sendToReceiver(F_RECEIVER_SET_THRESHOLD, e_eV, nullptr);
        }
    } else {
        throw RuntimeError(
            "Set threshold energy not implemented for this detector");
    }
}

std::string Module::getSettingsDir() const {
    return std::string(shm()->settingsDir);
}

std::string Module::setSettingsDir(const std::string &dir) {
    sls::strcpy_safe(shm()->settingsDir, dir.c_str());
    return shm()->settingsDir;
}

bool Module::getOverFlowMode() const {
    return sendToDetector<int>(F_GET_OVERFLOW_MODE);
}

void Module::setOverFlowMode(const bool enable) {
    sendToDetector(F_SET_OVERFLOW_MODE, static_cast<int>(enable), nullptr);
}

bool Module::getFlippedDataX() const {
    return sendToReceiver<int>(F_SET_FLIPPED_DATA_RECEIVER, GET_FLAG);
}

void Module::setFlippedDataX(bool value) {
    sendToReceiver<int>(F_SET_FLIPPED_DATA_RECEIVER, static_cast<int>(value));
}

std::vector<int> Module::getTrimEn() const {
    if (shm()->myDetectorType != EIGER) {
        throw RuntimeError("getTrimEn not implemented for this detector.");
    }
    return std::vector<int>(shm()->trimEnergies.begin(),
                            shm()->trimEnergies.end());
}

int Module::setTrimEn(const std::vector<int> &energies) {
    if (shm()->myDetectorType != EIGER) {
        throw RuntimeError("setTrimEn not implemented for this detector.");
    }
    if (energies.size() > MAX_TRIMEN) {
        std::ostringstream os;
        os << "Size of trim energies: " << energies.size()
           << " exceeds what can be stored in shared memory: " << MAX_TRIMEN
           << "\n";
        throw RuntimeError(os.str());
    }
    shm()->trimEnergies = energies;
    return shm()->trimEnergies.size();
}

int64_t Module::getRateCorrection() const {
    return sendToDetector<int64_t>(F_GET_RATE_CORRECT);
}

void Module::setDefaultRateCorrection() {
    int64_t arg = -1;
    sendToDetector(F_SET_RATE_CORRECT, arg, nullptr);
}

void Module::setRateCorrection(int64_t t) {
    sendToDetector(F_SET_RATE_CORRECT, t, nullptr);
}

void Module::sendReceiverRateCorrections(const std::vector<int64_t> &t) {
    LOG(logDEBUG) << "Sending to detector [rate corrections: " << ToString(t)
                  << ']';
    auto receiver = ReceiverSocket(shm()->rxHostname, shm()->rxTCPPort);
    receiver.Send(F_SET_RECEIVER_RATE_CORRECT);
    receiver.Send(static_cast<int>(t.size()));
    receiver.Send(t);
    if (receiver.Receive<int>() == FAIL) {
        throw RuntimeError("Receiver " + std::to_string(moduleId) +
                           " returned error: " + receiver.readErrorMessage());
    }
}

int Module::getReadNLines() const {
    return sendToDetector<int>(F_GET_READ_N_LINES);
}

void Module::setReadNLines(const int value) {
    sendToDetector(F_SET_READ_N_LINES, value, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_READ_N_LINES, value, nullptr);
    }
}

bool Module::getInterruptSubframe() const {
    return sendToDetector<int>(F_GET_INTERRUPT_SUBFRAME);
}

void Module::setInterruptSubframe(const bool enable) {
    sendToDetector(F_SET_INTERRUPT_SUBFRAME, static_cast<int>(enable), nullptr);
}

int64_t Module::getMeasuredPeriod() const {
    return sendToDetectorStop<int64_t>(F_GET_MEASURED_PERIOD);
}

int64_t Module::getMeasuredSubFramePeriod() const {
    return sendToDetectorStop<int64_t>(F_GET_MEASURED_SUBPERIOD);
}

bool Module::getActivate() const {
    auto retval = sendToDetector<int>(F_ACTIVATE, GET_FLAG);
    auto retval2 = sendToDetectorStop<int>(F_ACTIVATE, GET_FLAG);
    if (retval != retval2) {
        std::ostringstream oss;
        oss << "Inconsistent activate state. Control Server: " << retval
            << ". Stop Server: " << retval2;
        throw RuntimeError(oss.str());
    }
    return retval;
}

void Module::setActivate(const bool enable) {
    int arg = static_cast<int>(enable);
    auto retval = sendToDetector<int>(F_ACTIVATE, arg);
    sendToDetectorStop<int>(F_ACTIVATE, arg);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_ACTIVATE, retval, nullptr);
    }
}

bool Module::getDeactivatedRxrPaddingMode() const {
    return sendToReceiver<int>(F_GET_RECEIVER_DEACTIVATED_PADDING);
}

void Module::setDeactivatedRxrPaddingMode(bool padding) {
    sendToReceiver(F_SET_RECEIVER_DEACTIVATED_PADDING,
                   static_cast<int>(padding), nullptr);
}

bool Module::getCounterBit() const {
    return (
        !static_cast<bool>(sendToDetector<int>(F_SET_COUNTER_BIT, GET_FLAG)));
}

void Module::setCounterBit(bool cb) {
    sendToDetector<int>(F_SET_COUNTER_BIT, static_cast<int>(!cb));
}

void Module::pulsePixel(int n, int x, int y) {
    const int args[]{n, x, y};
    sendToDetector(F_PULSE_PIXEL, args, nullptr);
}

void Module::pulsePixelNMove(int n, int x, int y) {
    const int args[]{n, x, y};
    sendToDetector(F_PULSE_PIXEL_AND_MOVE, args, nullptr);
}

void Module::pulseChip(int n_pulses) {
    sendToDetector(F_PULSE_CHIP, n_pulses, nullptr);
}

bool Module::getQuad() const { return sendToDetector<int>(F_GET_QUAD) != 0; }

void Module::setQuad(const bool enable) {
    int value = enable ? 1 : 0;
    sendToDetector(F_SET_QUAD, value, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_QUAD, value, nullptr);
    }
}

// Jungfrau Specific

int Module::getThresholdTemperature() const {
    auto retval = sendToDetectorStop<int>(F_THRESHOLD_TEMP, GET_FLAG);
    return retval / 1000;
}

void Module::setThresholdTemperature(int val) {
    if (val <= 0) {
        throw RuntimeError("Invalid threshold temperature " +
                           std::to_string(val));
    }
    val *= 1000;
    sendToDetectorStop<int>(F_THRESHOLD_TEMP, val);
}

bool Module::getTemperatureControl() const {
    return sendToDetectorStop<int>(F_TEMP_CONTROL, GET_FLAG);
}

void Module::setTemperatureControl(bool val) {
    sendToDetectorStop<int>(F_TEMP_CONTROL, static_cast<int>(val));
}

int Module::getTemperatureEvent() const {
    return sendToDetectorStop<int>(F_TEMP_EVENT, GET_FLAG);
}

void Module::resetTemperatureEvent() {
    sendToDetectorStop<int>(F_TEMP_EVENT, 0);
}

bool Module::getAutoComparatorDisableMode() const {
    return sendToDetector<int>(F_AUTO_COMP_DISABLE, GET_FLAG);
}

void Module::setAutoComparatorDisableMode(bool val) {
    sendToDetector<int>(F_AUTO_COMP_DISABLE, static_cast<int>(val));
}

int Module::getNumberOfAdditionalStorageCells() const {
    return sendToDetector<int>(F_GET_NUM_ADDITIONAL_STORAGE_CELLS);
}

void Module::setNumberOfAdditionalStorageCells(int value) {
    sendToDetector(F_SET_NUM_ADDITIONAL_STORAGE_CELLS, value, nullptr);
}

int Module::getStorageCellStart() const {
    return sendToDetector<int>(F_STORAGE_CELL_START, GET_FLAG);
}

void Module::setStorageCellStart(int pos) {
    sendToDetector<int>(F_STORAGE_CELL_START, pos);
}

int64_t Module::getStorageCellDelay() const {
    return sendToDetector<int64_t>(F_GET_STORAGE_CELL_DELAY);
}

void Module::setStorageCellDelay(int64_t value) {
    sendToDetector(F_SET_STORAGE_CELL_DELAY, value, nullptr);
}

// Gotthard Specific

slsDetectorDefs::ROI Module::getROI() const {
    return sendToDetector<slsDetectorDefs::ROI>(F_GET_ROI);
}

void Module::setROI(slsDetectorDefs::ROI arg) {
    if (arg.xmin < 0 || arg.xmax >= getNumberOfChannels().x) {
        arg.xmin = -1;
        arg.xmax = -1;
    }
    sendToDetector(F_SET_ROI, arg, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_SET_ROI, arg, nullptr);
    }
}

void Module::clearROI() { setROI(slsDetectorDefs::ROI{}); }

int64_t Module::getExptimeLeft() const {
    return sendToDetectorStop<int64_t>(F_GET_EXPTIME_LEFT);
}

// Gotthard2 Specific

int64_t Module::getNumberOfBursts() const {
    return sendToDetector<int64_t>(F_GET_NUM_BURSTS);
}

void Module::setNumberOfBursts(int64_t value) {
    sendToDetector(F_SET_NUM_BURSTS, value, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_NUM_BURSTS, value, nullptr);
    }
}

int64_t Module::getBurstPeriod() const {
    return sendToDetector<int64_t>(F_GET_BURST_PERIOD);
}

void Module::setBurstPeriod(int64_t value) {
    sendToDetector(F_SET_BURST_PERIOD, value, nullptr);
}

int64_t Module::getNumberOfBurstsLeft() const {
    return sendToDetectorStop<int64_t>(F_GET_BURSTS_LEFT);
}

std::array<int, 2> Module::getInjectChannel() const {
    return sendToDetector<std::array<int, 2>>(F_GET_INJECT_CHANNEL);
}

void Module::setInjectChannel(const int offsetChannel,
                              const int incrementChannel) {
    int args[]{offsetChannel, incrementChannel};
    sendToDetector(F_SET_INJECT_CHANNEL, args, nullptr);
}

void Module::sendVetoPhoton(const int chipIndex,
                            const std::vector<int> &gainIndices,
                            const std::vector<int> &values) {
    const int nch = gainIndices.size();
    if (gainIndices.size() != values.size()) {
        throw RuntimeError("Number of Gain Indices and values do not match! "
                           "Gain Indices size: " +
                           std::to_string(gainIndices.size()) +
                           ", values size: " + std::to_string(values.size()));
    }
    LOG(logDEBUG1) << "Sending veto photon/file to detector [chip:" << chipIndex
                   << ", nch:" << nch << "]";

    const int args[]{chipIndex, nch};
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    client.Send(F_SET_VETO_PHOTON);
    client.Send(args);
    client.Send(gainIndices);
    client.Send(values);
    if (client.Receive<int>() == FAIL) {
        throw RuntimeError("Detector " + std::to_string(moduleId) +
                           " returned error: " + client.readErrorMessage());
    }
}

void Module::getVetoPhoton(const int chipIndex,
                           const std::string &fname) const {
    LOG(logDEBUG1) << "Getting veto photon [" << chipIndex << "]\n";
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    client.Send(F_GET_VETO_PHOTON);
    client.Send(chipIndex);
    if (client.Receive<int>() == FAIL) {
        throw RuntimeError("Detector " + std::to_string(moduleId) +
                           " returned error: " + client.readErrorMessage());
    }

    auto nch = client.Receive<int>();
    if (nch != shm()->nChan.x) {
        throw RuntimeError("Could not get veto photon. Expected " +
                           std::to_string(shm()->nChan.x) + " channels, got " +
                           std::to_string(nch));
    }
    std::vector<int> gainIndices(nch);
    std::vector<int> values(nch);
    client.Receive(gainIndices);
    client.Receive(values);

    // save to file
    std::ofstream outfile(fname);
    if (!outfile) {
        throw RuntimeError("Could not create file to save veto photon");
    }
    for (int i = 0; i < nch; ++i) {
        outfile << gainIndices[i] << ' ' << values[i] << '\n';
    }
    LOG(logDEBUG1) << nch << " veto photon saved to file";
}

void Module::setVetoPhoton(const int chipIndex, const int numPhotons,
                           const int energy, const std::string &fname) {
    if (shm()->myDetectorType != GOTTHARD2) {
        throw RuntimeError(
            "Set Veto reference is not implemented for this detector");
    }
    if (chipIndex < -1 || chipIndex >= shm()->nChip.x) {
        throw RuntimeError("Could not set veto photon. Invalid chip index: " +
                           std::to_string(chipIndex));
    }
    if (numPhotons < 1) {
        throw RuntimeError(
            "Could not set veto photon. Invalid number of photons: " +
            std::to_string(numPhotons));
    }
    if (energy < 1) {
        throw RuntimeError("Could not set veto photon. Invalid energy: " +
                           std::to_string(energy));
    }
    std::ifstream infile(fname.c_str());
    if (!infile) {
        throw RuntimeError("Could not set veto photon. Could not open file: " +
                           fname);
    }
    LOG(logDEBUG1) << "Setting veto photon. Reading Gain values from file";

    const int totalEnergy = numPhotons * energy;
    std::vector<int> gainIndices;
    std::vector<int> values;

    while (infile.good()) {
        std::string line;
        getline(infile, line);
        if (line.find('#') != std::string::npos) {
            line.erase(line.find('#'));
        }
        if (line.length() < 1) {
            continue;
        }
        std::istringstream ss(line);

        // read pedestal, gain values, gain thresholds
        double gainPedestal[3] = {-1, -1, -1};
        double gainValue[3] = {-1, -1, -1};
        int gainThreshold[2] = {-1, -1};
        ss >> gainPedestal[0] >> gainPedestal[1] >> gainPedestal[2] >>
            gainValue[0] >> gainValue[1] >> gainValue[2] >> gainThreshold[0] >>
            gainThreshold[1];
        if (ss.fail()) {
            throw RuntimeError("Could not set veto photon. Invalid pedestals, "
                               "gain values or gain thresholds for channel " +
                               std::to_string(gainIndices.size()));
        }

        // caluclate gain index from gain thresholds and threshold energy
        int gainIndex = 2;
        if (totalEnergy < gainThreshold[0]) {
            gainIndex = 0;
        } else if (totalEnergy < gainThreshold[1]) {
            gainIndex = 1;
        }
        gainIndices.push_back(gainIndex);
        // calculate ADU value
        values.push_back(gainPedestal[gainIndex] +
                         (gainValue[gainIndex] * totalEnergy));
    }
    // check size
    if ((int)gainIndices.size() != shm()->nChan.x) {
        throw RuntimeError("Could not set veto photon. Invalid number of "
                           "entries in file. Expected " +
                           std::to_string(shm()->nChan.x) + ", read " +
                           std::to_string(gainIndices.size()));
    }

    sendVetoPhoton(chipIndex, gainIndices, values);
}

void Module::setVetoReference(const int gainIndex, const int value) {
    const int args[]{gainIndex, value};
    sendToDetector(F_SET_VETO_REFERENCE, args, nullptr);
}

void Module::setVetoFile(const int chipIndex, const std::string &fname) {
    if (shm()->myDetectorType != GOTTHARD2) {
        throw RuntimeError(
            "Set Veto file is not implemented for this detector");
    }
    if (chipIndex < -1 || chipIndex >= shm()->nChip.x) {
        throw RuntimeError("Could not set veto file. Invalid chip index: " +
                           std::to_string(chipIndex));
    }

    std::ifstream input_file(fname);
    if (!input_file) {
        throw RuntimeError("Could not set veto file for chip " +
                           std::to_string(chipIndex) +
                           ". Could not open file: " + fname);
    }

    std::vector<int> gainIndices;
    std::vector<int> values;

    for (std::string line; std::getline(input_file, line);) {
        if (line.find('#') != std::string::npos) {
            line.erase(line.find('#'));
        }
        LOG(logDEBUG1) << "line after removing comments:\n\t" << line;
        if (line.length() > 1) {
            // convert command and string to a vector
            std::istringstream iss(line);
            std::string val;
            int gainIndex = -1;
            int value = -1;
            iss >> gainIndex >> val;
            if (iss.fail()) {
                throw RuntimeError("Could not set veto file. Invalid gain "
                                   "or reference value for channel " +
                                   std::to_string(gainIndices.size()));
            }
            try {
                value = StringTo<int>(val);
            } catch (...) {
                throw RuntimeError("Could not set veto file. Invalid value " +
                                   val + " for channel " +
                                   std::to_string(gainIndices.size()));
            }
            gainIndices.push_back(gainIndex);
            values.push_back(value);
        }
    }
    // check size
    if ((int)gainIndices.size() != shm()->nChan.x) {
        throw RuntimeError("Could not set veto file. Invalid number of "
                           "entries in file. Expected " +
                           std::to_string(shm()->nChan.x) + ", read " +
                           std::to_string(gainIndices.size()));
    }

    sendVetoPhoton(chipIndex, gainIndices, values);
}

slsDetectorDefs::burstMode Module::getBurstMode() const {
    return sendToDetector<burstMode>(F_GET_BURST_MODE);
}

void Module::setBurstMode(slsDetectorDefs::burstMode value) {
    sendToDetector(F_SET_BURST_MODE, value, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_BURST_MODE, value, nullptr);
    }
}

bool Module::getCDSGain() const { return sendToDetector<int>(F_GET_CDS_GAIN); }

void Module::setCDSGain(bool value) {
    sendToDetector(F_SET_CDS_GAIN, static_cast<int>(value), nullptr);
}

int Module::getFilter() const { return sendToDetector<int>(F_GET_FILTER); }

void Module::setFilter(int value) {
    sendToDetector(F_SET_FILTER, value, nullptr);
}

bool Module::getCurrentSource() const {
    return sendToDetector<int>(F_GET_CURRENT_SOURCE);
}

void Module::setCurrentSource(bool value) {
    sendToDetector(F_SET_CURRENT_SOURCE, static_cast<int>(value), nullptr);
}

slsDetectorDefs::timingSourceType Module::getTimingSource() const {
    return sendToDetector<timingSourceType>(F_GET_TIMING_SOURCE);
}

void Module::setTimingSource(slsDetectorDefs::timingSourceType value) {
    sendToDetector(F_SET_TIMING_SOURCE, static_cast<int>(value), nullptr);
}

bool Module::getVeto() const { return sendToDetector<int>(F_GET_VETO); }

void Module::setVeto(bool enable) {
    sendToDetector(F_SET_VETO, static_cast<int>(enable), nullptr);
}

int Module::getADCConfiguration(const int chipIndex, const int adcIndex) const {
    int args[]{chipIndex, adcIndex};
    return sendToDetector<int>(F_GET_ADC_CONFIGURATION, args);
}

void Module::setADCConfiguration(const int chipIndex, const int adcIndex,
                                 int value) {
    int args[]{chipIndex, adcIndex, value};
    sendToDetector(F_SET_ADC_CONFIGURATION, args, nullptr);
}

void Module::getBadChannels(const std::string &fname) const {
    LOG(logDEBUG1) << "Getting bad channels to " << fname;
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    client.Send(F_GET_BAD_CHANNELS);
    if (client.Receive<int>() == FAIL) {
        throw RuntimeError("Detector " + std::to_string(moduleId) +
                           " returned error: " + client.readErrorMessage());
    }
    // receive badchannels
    auto nch = client.Receive<int>();
    std::vector<int> badchannels(nch);
    if (nch > 0) {
        client.Receive(badchannels);
        for (size_t i = 0; i < badchannels.size(); ++i) {
            LOG(logDEBUG1) << i << ":" << badchannels[i];
        }
    }

    // save to file
    std::ofstream outfile(fname);
    if (!outfile) {
        throw RuntimeError("Could not create file to save bad channels");
    }
    for (auto ch : badchannels)
        outfile << ch << '\n';
    LOG(logDEBUG1) << nch << " bad channels saved to file";
}

void Module::setBadChannels(const std::string &fname) {
    // read bad channels file
    std::ifstream input_file(fname);
    if (!input_file) {
        throw RuntimeError("Could not open bad channels file " + fname +
                           " for reading");
    }
    std::vector<int> badchannels;
    for (std::string line; std::getline(input_file, line);) {
        line.erase(std::remove_if(begin(line), end(line), isspace),
                   end(line)); // remove space
        if (!line.empty()) {
            std::istringstream iss(line);
            int ival = 0;
            iss >> ival;
            if (iss.fail()) {
                throw RuntimeError("Could not load bad channels file. Invalid "
                                   "channel number at position " +
                                   std::to_string(badchannels.size()));
            }
            badchannels.push_back(ival);
        }
    }

    // send bad channels to module
    auto nch = static_cast<int>(badchannels.size());
    LOG(logDEBUG1) << "Sending bad channels to detector, nch:" << nch;
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    client.Send(F_SET_BAD_CHANNELS);
    client.Send(nch);
    if (nch > 0) {
        client.Send(badchannels);
    }
    if (client.Receive<int>() == FAIL) {
        throw RuntimeError("Detector " + std::to_string(moduleId) +
                           " returned error: " + client.readErrorMessage());
    }
}

// Mythen3 Specific

uint32_t Module::getCounterMask() const {
    return sendToDetector<uint32_t>(F_GET_COUNTER_MASK);
}

void Module::setCounterMask(uint32_t countermask) {
    LOG(logDEBUG1) << "Setting Counter mask to " << countermask;
    sendToDetector(F_SET_COUNTER_MASK, countermask, nullptr);
    if (shm()->useReceiverFlag) {
        LOG(logDEBUG1) << "Sending Reciver counter mask: " << countermask;
        sendToReceiver(F_RECEIVER_SET_COUNTER_MASK, countermask, nullptr);
    }
}

int Module::getNumberOfGates() const {
    return sendToDetector<int>(F_GET_NUM_GATES);
}

void Module::setNumberOfGates(int value) {
    sendToDetector(F_SET_NUM_GATES, value, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_NUM_GATES, value, nullptr);
    }
}

std::array<time::ns, 3> Module::getExptimeForAllGates() const {
    return sendToDetector<std::array<time::ns, 3>>(F_GET_EXPTIME_ALL_GATES);
}

int64_t Module::getGateDelay(int gateIndex) const {
    return sendToDetector<int64_t>(F_GET_GATE_DELAY, gateIndex);
}

void Module::setGateDelay(int gateIndex, int64_t value) {
    int64_t args[]{static_cast<int64_t>(gateIndex), value};
    sendToDetector(F_SET_GATE_DELAY, args, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_GATE_DELAY, args, nullptr);
    }
}

std::array<time::ns, 3> Module::getGateDelayForAllGates() const {
    return sendToDetector<std::array<time::ns, 3>>(F_GET_GATE_DELAY_ALL_GATES);
}

// CTB / Moench Specific
int Module::getNumberOfAnalogSamples() const {
    return sendToDetector<int>(F_GET_NUM_ANALOG_SAMPLES);
}

void Module::setNumberOfAnalogSamples(int value) {
    sendToDetector(F_SET_NUM_ANALOG_SAMPLES, value, nullptr);
    // update #nchan, as it depends on #samples, adcmask
    updateNumberOfChannels();
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_SET_NUM_ANALOG_SAMPLES, value, nullptr);
    }
}

int Module::getPipeline(int clkIndex) const {
    return sendToDetector<int>(F_GET_PIPELINE, clkIndex);
}

void Module::setPipeline(int clkIndex, int value) {
    int args[]{clkIndex, value};
    sendToDetector(F_SET_PIPELINE, args, nullptr);
}

uint32_t Module::getADCEnableMask() const {
    return sendToDetector<uint32_t>(F_GET_ADC_ENABLE_MASK);
}

void Module::setADCEnableMask(uint32_t mask) {
    sendToDetector(F_SET_ADC_ENABLE_MASK, mask, nullptr);
    // update #nchan, as it depends on #samples, adcmask,
    updateNumberOfChannels();

    if (shm()->useReceiverFlag) {
        sendToReceiver<int>(F_RECEIVER_SET_ADC_MASK, mask);
    }
}

uint32_t Module::getTenGigaADCEnableMask() const {
    return sendToDetector<uint32_t>(F_GET_ADC_ENABLE_MASK_10G);
}

void Module::setTenGigaADCEnableMask(uint32_t mask) {
    sendToDetector(F_SET_ADC_ENABLE_MASK_10G, mask, nullptr);
    updateNumberOfChannels(); // depends on samples and adcmask

    if (shm()->useReceiverFlag) {
        sendToReceiver<int>(F_RECEIVER_SET_ADC_MASK_10G, mask);
    }
}

// CTB Specific

int Module::getNumberOfDigitalSamples() const {
    return sendToDetector<int>(F_GET_NUM_DIGITAL_SAMPLES);
}

void Module::setNumberOfDigitalSamples(int value) {
    LOG(logDEBUG1) << "Setting number of digital samples to " << value;
    sendToDetector(F_SET_NUM_DIGITAL_SAMPLES, value, nullptr);
    updateNumberOfChannels(); // depends on samples and adcmask
    if (shm()->useReceiverFlag) {
        LOG(logDEBUG1) << "Sending number of digital samples to Receiver: "
                       << value;
        sendToReceiver(F_RECEIVER_SET_NUM_DIGITAL_SAMPLES, value, nullptr);
    }
}

slsDetectorDefs::readoutMode Module::getReadoutMode() const {
    return sendToDetector<readoutMode>(F_GET_READOUT_MODE);
}

void Module::setReadoutMode(const slsDetectorDefs::readoutMode mode) {
    auto arg = static_cast<uint32_t>(mode); // TODO! unit?
    sendToDetector(F_SET_READOUT_MODE, arg, nullptr);
    // update #nchan, as it depends on #samples, adcmask,
    if (shm()->myDetectorType == CHIPTESTBOARD) {
        updateNumberOfChannels();
    }
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_SET_READOUT_MODE, mode, nullptr);
    }
}

int Module::getExternalSamplingSource() {
    return setExternalSamplingSource(GET_FLAG);
}

int Module::setExternalSamplingSource(int value) {
    return sendToDetector<int>(F_EXTERNAL_SAMPLING_SOURCE, value);
}

bool Module::getExternalSampling() const {
    return sendToDetector<int>(F_EXTERNAL_SAMPLING, GET_FLAG);
}

void Module::setExternalSampling(bool value) {
    sendToDetector<int>(F_EXTERNAL_SAMPLING, static_cast<int>(value));
}

std::vector<int> Module::getReceiverDbitList() const {
    return sendToReceiver<sls::StaticVector<int, MAX_RX_DBIT>>(
        F_GET_RECEIVER_DBIT_LIST);
}

void Module::setReceiverDbitList(std::vector<int> list) {
    LOG(logDEBUG1) << "Setting Receiver Dbit List";
    if (list.size() > 64) {
        throw sls::RuntimeError("Dbit list size cannot be greater than 64\n");
    }
    for (auto &it : list) {
        if (it < 0 || it > 63) {
            throw sls::RuntimeError(
                "Dbit list value must be between 0 and 63\n");
        }
    }
    std::sort(begin(list), end(list));
    auto last = std::unique(begin(list), end(list));
    list.erase(last, list.end());

    sls::StaticVector<int, MAX_RX_DBIT> arg = list;
    sendToReceiver(F_SET_RECEIVER_DBIT_LIST, arg, nullptr);
}

int Module::getReceiverDbitOffset() const {
    return sendToReceiver<int>(F_GET_RECEIVER_DBIT_OFFSET);
}

void Module::setReceiverDbitOffset(int value) {
    sendToReceiver(F_SET_RECEIVER_DBIT_OFFSET, value, nullptr);
}

void Module::setDigitalIODelay(uint64_t pinMask, int delay) {
    uint64_t args[]{pinMask, static_cast<uint64_t>(delay)};
    sendToDetector(F_DIGITAL_IO_DELAY, args, nullptr);
}

bool Module::getLEDEnable() const {
    return sendToDetector<int>(F_LED, GET_FLAG);
}

void Module::setLEDEnable(bool enable) {
    sendToDetector<int>(F_LED, static_cast<int>(enable));
}

// Pattern

void Module::setPattern(const std::string &fname) {
    auto pat = sls::make_unique<patternParameters>();
    std::ifstream input_file(fname);
    if (!input_file) {
        throw RuntimeError("Could not open pattern file " + fname +
                           " for reading");
    }
    for (std::string line; std::getline(input_file, line);) {
        if (line.find('#') != std::string::npos) {
            line.erase(line.find('#'));
        }
        LOG(logDEBUG1) << "line after removing comments:\n\t" << line;
        if (line.length() > 1) {

            // convert command and string to a vector
            std::istringstream iss(line);
            auto it = std::istream_iterator<std::string>(iss);
            std::vector<std::string> args = std::vector<std::string>(
                it, std::istream_iterator<std::string>());

            std::string cmd = args[0];
            int nargs = args.size() - 1;

            if (cmd == "patword") {
                if (nargs != 2) {
                    throw RuntimeError("Invalid arguments for " +
                                       ToString(args));
                }
                uint32_t addr = StringTo<uint32_t>(args[1]);
                if (addr >= MAX_PATTERN_LENGTH) {
                    throw RuntimeError("Invalid address for " + ToString(args));
                }
                pat->word[addr] = StringTo<uint64_t>(args[2]);
            } else if (cmd == "patioctrl") {
                if (nargs != 1) {
                    throw RuntimeError("Invalid arguments for " +
                                       ToString(args));
                }
                pat->patioctrl = StringTo<uint64_t>(args[1]);
            } else if (cmd == "patlimits") {
                if (nargs != 2) {
                    throw RuntimeError("Invalid arguments for " +
                                       ToString(args));
                }
                pat->patlimits[0] = StringTo<uint32_t>(args[1]);
                pat->patlimits[1] = StringTo<uint32_t>(args[2]);
                if (pat->patlimits[0] >= MAX_PATTERN_LENGTH ||
                    pat->patlimits[1] >= MAX_PATTERN_LENGTH) {
                    throw RuntimeError("Invalid address for " + ToString(args));
                }
            } else if (cmd == "patloop0" || cmd == "patloop1" ||
                       cmd == "patloop2") {
                if (nargs != 2) {
                    throw RuntimeError("Invalid arguments for " +
                                       ToString(args));
                }
                int level = cmd[cmd.find_first_of("012")] - '0';
                int patloop1 = StringTo<uint32_t>(args[1]);
                int patloop2 = StringTo<uint32_t>(args[2]);
                pat->patloop[level * 2 + 0] = patloop1;
                pat->patloop[level * 2 + 1] = patloop2;
                if (patloop1 >= MAX_PATTERN_LENGTH ||
                    patloop2 >= MAX_PATTERN_LENGTH) {
                    throw RuntimeError("Invalid address for " + ToString(args));
                }
            } else if (cmd == "patnloop0" || cmd == "patnloop1" ||
                       cmd == "patnloop2") {
                if (nargs != 1) {
                    throw RuntimeError("Invalid arguments for " +
                                       ToString(args));
                }
                int level = cmd[cmd.find_first_of("012")] - '0';
                pat->patnloop[level] = StringTo<uint32_t>(args[1]);
            } else if (cmd == "patwait0" || cmd == "patwait1" ||
                       cmd == "patwait2") {
                if (nargs != 1) {
                    throw RuntimeError("Invalid arguments for " +
                                       ToString(args));
                }
                int level = cmd[cmd.find_first_of("012")] - '0';
                pat->patwait[level] = StringTo<uint32_t>(args[1]);
                if (pat->patwait[level] >= MAX_PATTERN_LENGTH) {
                    throw RuntimeError("Invalid address for " + ToString(args));
                }
            } else if (cmd == "patwaittime0" || cmd == "patwaittime1" ||
                       cmd == "patwaittime2") {
                if (nargs != 1) {
                    throw RuntimeError("Invalid arguments for " +
                                       ToString(args));
                }
                int level = cmd[cmd.find_first_of("012")] - '0';
                pat->patwaittime[level] = StringTo<uint64_t>(args[1]);
            } else {
                throw RuntimeError("Unknown command in pattern file " + cmd);
            }
        }
    }
    LOG(logDEBUG1) << "Sending pattern from file to detector:" << *pat;
    sendToDetector(F_SET_PATTERN, pat.get(), sizeof(patternParameters), nullptr,
                   0);
}

uint64_t Module::getPatternIOControl() const {
    return sendToDetector<uint64_t>(F_SET_PATTERN_IO_CONTROL,
                                    int64_t(GET_FLAG));
}

void Module::setPatternIOControl(uint64_t word) {
    sendToDetector<uint64_t>(F_SET_PATTERN_IO_CONTROL, word);
}

uint64_t Module::getPatternWord(int addr) const {
    uint64_t args[]{static_cast<uint64_t>(addr),
                    static_cast<uint64_t>(GET_FLAG)};
    return sendToDetector<uint64_t>(F_SET_PATTERN_WORD, args);
}

void Module::setPatternWord(int addr, uint64_t word) {
    uint64_t args[]{static_cast<uint64_t>(addr), word};
    sendToDetector<uint64_t>(F_SET_PATTERN_WORD, args);
}

std::array<int, 2> Module::getPatternLoopAddresses(int level) const {
    int args[]{level, GET_FLAG, GET_FLAG};
    std::array<int, 2> retvals{};
    sendToDetector(F_SET_PATTERN_LOOP_ADDRESSES, args, retvals);
    return retvals;
}

void Module::setPatternLoopAddresses(int level, int start, int stop) {
    int args[]{level, start, stop};
    std::array<int, 2> retvals{};
    sendToDetector(F_SET_PATTERN_LOOP_ADDRESSES, args, retvals);
}

int Module::getPatternLoopCycles(int level) const {
    int args[]{level, GET_FLAG};
    return sendToDetector<int>(F_SET_PATTERN_LOOP_CYCLES, args);
}

void Module::setPatternLoopCycles(int level, int n) {
    int args[]{level, n};
    sendToDetector<int>(F_SET_PATTERN_LOOP_CYCLES, args);
}

int Module::getPatternWaitAddr(int level) const {
    int args[]{level, GET_FLAG};
    return sendToDetector<int>(F_SET_PATTERN_WAIT_ADDR, args);
}

void Module::setPatternWaitAddr(int level, int addr) {
    int args[]{level, addr};
    sendToDetector<int>(F_SET_PATTERN_WAIT_ADDR, args);
}

uint64_t Module::getPatternWaitTime(int level) const {
    uint64_t args[]{static_cast<uint64_t>(level),
                    static_cast<uint64_t>(GET_FLAG)};
    return sendToDetector<uint64_t>(F_SET_PATTERN_WAIT_TIME, args);
}

void Module::setPatternWaitTime(int level, uint64_t t) {
    uint64_t args[]{static_cast<uint64_t>(level), t};
    sendToDetector<uint64_t>(F_SET_PATTERN_WAIT_TIME, args);
}

uint64_t Module::getPatternMask() const {
    return sendToDetector<uint64_t>(F_GET_PATTERN_MASK);
}

void Module::setPatternMask(uint64_t mask) {
    sendToDetector(F_SET_PATTERN_MASK, mask, nullptr);
}

uint64_t Module::getPatternBitMask() const {
    return sendToDetector<uint64_t>(F_GET_PATTERN_BIT_MASK);
}

void Module::setPatternBitMask(uint64_t mask) {
    sendToDetector(F_SET_PATTERN_BIT_MASK, mask, nullptr);
}

void Module::startPattern() { sendToDetector(F_START_PATTERN); }

// Moench

std::map<std::string, std::string> Module::getAdditionalJsonHeader() const {
    // TODO, refactor this function with a more robust sending.
    // Now assuming whitespace separated key value
    if (!shm()->useReceiverFlag) {
        throw RuntimeError("Set rx_hostname first to use receiver parameters "
                           "(zmq json header)");
    }
    auto client = ReceiverSocket(shm()->rxHostname, shm()->rxTCPPort);
    client.Send(F_GET_ADDITIONAL_JSON_HEADER);
    if (client.Receive<int>() == FAIL) {
        throw RuntimeError("Receiver " + std::to_string(moduleId) +
                           " returned error: " + client.readErrorMessage());
    } else {
        auto size = client.Receive<int>();
        std::string buff(size, '\0');
        std::map<std::string, std::string> retval;
        if (size > 0) {
            client.Receive(&buff[0], buff.size());
            std::istringstream iss(buff);
            std::string key, value;
            while (iss >> key) {
                iss >> value;
                retval[key] = value;
            }
        }
        LOG(logDEBUG) << "Getting additional json header " << ToString(retval);
        return retval;
    }
}

void Module::setAdditionalJsonHeader(
    const std::map<std::string, std::string> &jsonHeader) {
    if (!shm()->useReceiverFlag) {
        throw RuntimeError("Set rx_hostname first to use receiver parameters "
                           "(zmq json header)");
    }
    for (auto &it : jsonHeader) {
        if (it.first.empty() || it.first.length() > SHORT_STR_LENGTH ||
            it.second.length() > SHORT_STR_LENGTH) {
            throw RuntimeError(
                it.first + " or " + it.second +
                " pair has invalid size. "
                "Key cannot be empty. Both can have max 20 characters");
        }
    }
    std::ostringstream oss;
    for (auto &it : jsonHeader)
        oss << it.first << ' ' << it.second << ' ';
    auto buff = oss.str();
    const auto size = static_cast<int>(buff.size());
    LOG(logDEBUG) << "Sending to receiver additional json header "
                  << ToString(jsonHeader);
    auto client = ReceiverSocket(shm()->rxHostname, shm()->rxTCPPort);
    client.Send(F_SET_ADDITIONAL_JSON_HEADER);
    client.Send(size);
    if (size > 0)
        client.Send(&buff[0], buff.size());

    if (client.Receive<int>() == FAIL) {
        throw RuntimeError("Receiver " + std::to_string(moduleId) +
                           " returned error: " + client.readErrorMessage());
    }
}

std::string Module::getAdditionalJsonParameter(const std::string &key) const {
    char arg[SHORT_STR_LENGTH]{};
    sls::strcpy_safe(arg, key.c_str());
    char retval[SHORT_STR_LENGTH]{};
    sendToReceiver(F_GET_ADDITIONAL_JSON_PARAMETER, arg, retval);
    return retval;
}

void Module::setAdditionalJsonParameter(const std::string &key,
                                        const std::string &value) {
    if (key.empty() || key.length() > SHORT_STR_LENGTH ||
        value.length() > SHORT_STR_LENGTH) {
        throw RuntimeError(
            key + " or " + value +
            " pair has invalid size. "
            "Key cannot be empty. Both can have max 2 characters");
    }
    char args[2][SHORT_STR_LENGTH]{};
    sls::strcpy_safe(args[0], key.c_str());
    sls::strcpy_safe(args[1], value.c_str());
    sendToReceiver(F_SET_ADDITIONAL_JSON_PARAMETER, args, nullptr);
}

// Advanced
void Module::programFPGA(std::vector<char> buffer) {
    switch (shm()->myDetectorType) {
    case JUNGFRAU:
    case CHIPTESTBOARD:
    case MOENCH:
        programFPGAviaBlackfin(buffer);
        break;
    case MYTHEN3:
    case GOTTHARD2:
        programFPGAviaNios(buffer);
        break;
    default:
        throw RuntimeError("Program FPGA is not implemented for this detector");
    }
}

void Module::resetFPGA() { sendToDetector(F_RESET_FPGA); }

void Module::copyDetectorServer(const std::string &fname,
                                const std::string &hostname) {
    char args[2][MAX_STR_LENGTH]{};
    sls::strcpy_safe(args[0], fname.c_str());
    sls::strcpy_safe(args[1], hostname.c_str());
    LOG(logINFO) << "Sending detector server " << args[0] << " from host "
                 << args[1];
    sendToDetector(F_COPY_DET_SERVER, args, nullptr);
}

void Module::rebootController() {
    sendToDetector(F_REBOOT_CONTROLLER);
    LOG(logINFO) << "Controller rebooted successfully!";
}

uint32_t Module::readRegister(uint32_t addr) const {
    return sendToDetectorStop<uint32_t>(F_READ_REGISTER, addr);
}

uint32_t Module::writeRegister(uint32_t addr, uint32_t val) {
    uint32_t args[]{addr, val};
    return sendToDetectorStop<uint32_t>(F_WRITE_REGISTER, args);
}

void Module::setBit(uint32_t addr, int n) {
    if (n < 0 || n > 31) {
        throw RuntimeError("Bit number " + std::to_string(n) + " out of Range");
    } else {
        uint32_t val = readRegister(addr);
        writeRegister(addr, val | 1 << n);
    }
}

void Module::clearBit(uint32_t addr, int n) {
    if (n < 0 || n > 31) {
        throw RuntimeError("Bit number " + std::to_string(n) + " out of Range");
    } else {
        uint32_t val = readRegister(addr);
        writeRegister(addr, val & ~(1 << n));
    }
}

int Module::getBit(uint32_t addr, int n) {
    if (n < 0 || n > 31) {
        throw RuntimeError("Bit number " + std::to_string(n) + " out of Range");
    } else {
        return ((readRegister(addr) >> n) & 0x1);
    }
}

void Module::executeFirmwareTest() { sendToDetector(F_SET_FIRMWARE_TEST); }

void Module::executeBusTest() { sendToDetector(F_SET_BUS_TEST); }

void Module::writeAdcRegister(uint32_t addr, uint32_t val) {
    uint32_t args[]{addr, val};
    sendToDetector(F_WRITE_ADC_REG, args, nullptr);
}

uint32_t Module::getADCInvert() const {
    return sendToDetector<uint32_t>(F_GET_ADC_INVERT);
}

void Module::setADCInvert(uint32_t value) {
    sendToDetector(F_SET_ADC_INVERT, value, nullptr);
}

// Insignificant
int Module::getControlPort() const { return shm()->controlPort; }

void Module::setControlPort(int port_number) {
    if (strlen(shm()->hostname) > 0) {
        shm()->controlPort = sendToDetector<int>(F_SET_PORT, port_number);
    } else {
        shm()->controlPort = port_number;
    }
}

int Module::getStopPort() const { return shm()->stopPort; }

void Module::setStopPort(int port_number) {
    if (strlen(shm()->hostname) > 0) {
        shm()->stopPort = sendToDetectorStop<int>(F_SET_PORT, port_number);
    } else {
        shm()->stopPort = port_number;
    }
}

bool Module::getLockDetector() const {
    return sendToDetector<int>(F_LOCK_SERVER, GET_FLAG);
}

void Module::setLockDetector(bool lock) {
    sendToDetector<int>(F_LOCK_SERVER, static_cast<int>(lock));
}

sls::IpAddr Module::getLastClientIP() const {
    return sendToDetector<sls::IpAddr>(F_GET_LAST_CLIENT_IP);
}

std::string Module::execCommand(const std::string &cmd) {
    char arg[MAX_STR_LENGTH]{};
    char retval[MAX_STR_LENGTH]{};
    sls::strcpy_safe(arg, cmd.c_str());
    sendToDetector(F_EXEC_COMMAND, arg, retval);
    return retval;
}

int64_t Module::getNumberOfFramesFromStart() const {
    return sendToDetectorStop<int64_t>(F_GET_FRAMES_FROM_START);
}

int64_t Module::getActualTime() const {
    return sendToDetectorStop<int64_t>(F_GET_ACTUAL_TIME);
}

int64_t Module::getMeasurementTime() const {
    return sendToDetectorStop<int64_t>(F_GET_MEASUREMENT_TIME);
}

uint64_t Module::getReceiverCurrentFrameIndex() const {
    return sendToReceiver<uint64_t>(F_GET_RECEIVER_FRAME_INDEX);
}

// private

void Module::checkArgs(const void *args, size_t args_size, void *retval,
                       size_t retval_size) const {
    if (args == nullptr && args_size != 0)
        throw RuntimeError(
            "Passed nullptr as args to Send function but size is not 0");
    if (args != nullptr && args_size == 0)
        throw RuntimeError(
            "Passed size 0 to Send function but args is not nullptr");
    if (retval == nullptr && retval_size != 0)
        throw RuntimeError(
            "Passed nullptr as retval to Send function but size is not 0");
    if (retval != nullptr && retval_size == 0)
        throw RuntimeError(
            "Passed size 0 to Send function but retval is not nullptr");
}

// Macro to check arguments passed to send to receiver and send to detector
// Should detect and fail on pointer types and on nullptr_t
#define STATIC_ASSERT_ARG(ARG, DST)                                            \
    static_assert(!std::is_pointer<ARG>::value,                                \
                  "Pointer type is incompatible with templated " DST);         \
    static_assert(!std::is_same<ARG, std::nullptr_t>::value,                   \
                  "nullptr_t type is incompatible with templated " DST);

void Module::sendToDetector(int fnum, const void *args, size_t args_size,
                            void *retval, size_t retval_size) const {
    // This is the only function that actually sends data to the detector
    // the other versions use templates to deduce sizes and create
    // the return type
    checkArgs(args, args_size, retval, retval_size);
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    client.sendCommandThenRead(fnum, args, args_size, retval, retval_size);
    client.close();
}

void Module::sendToDetector(int fnum, const void *args, size_t args_size,
                            void *retval, size_t retval_size) {
    static_cast<const Module &>(*this).sendToDetector(fnum, args, args_size,
                                                      retval, retval_size);
}

template <typename Arg, typename Ret>
void Module::sendToDetector(int fnum, const Arg &args, Ret &retval) const {
    LOG(logDEBUG1) << "Sending: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", nullptr, 0, " << typeid(Ret).name() << ", "
                   << sizeof(Ret) << "]";
    STATIC_ASSERT_ARG(Arg, "sendToDetector")
    STATIC_ASSERT_ARG(Ret, "sendToDetector")
    sendToDetector(fnum, &args, sizeof(args), &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << ToString(retval);
}

template <typename Arg, typename Ret>
void Module::sendToDetector(int fnum, const Arg &args, Ret &retval) {
    static_cast<const Module &>(*this).sendToDetector(fnum, args, retval);
}

template <typename Arg>
void Module::sendToDetector(int fnum, const Arg &args, std::nullptr_t) const {
    LOG(logDEBUG1) << "Sending: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", " << typeid(Arg).name() << ", " << sizeof(Arg)
                   << ", nullptr, 0 ]";
    STATIC_ASSERT_ARG(Arg, "sendToDetector")
    sendToDetector(fnum, &args, sizeof(args), nullptr, 0);
}

template <typename Arg>
void Module::sendToDetector(int fnum, const Arg &args, std::nullptr_t) {
    static_cast<const Module &>(*this).sendToDetector(fnum, args, nullptr);
}

template <typename Ret>
void Module::sendToDetector(int fnum, std::nullptr_t, Ret &retval) const {
    LOG(logDEBUG1) << "Sending: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", nullptr, 0, " << typeid(Ret).name() << ", "
                   << sizeof(Ret) << "]";
    STATIC_ASSERT_ARG(Ret, "sendToDetector")
    sendToDetector(fnum, nullptr, 0, &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << ToString(retval);
}

template <typename Ret>
void Module::sendToDetector(int fnum, std::nullptr_t, Ret &retval) {
    static_cast<const Module &>(*this).sendToDetector(fnum, nullptr, retval);
}

void Module::sendToDetector(int fnum) const {
    LOG(logDEBUG1) << "Sending: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << "]";
    sendToDetector(fnum, nullptr, 0, nullptr, 0);
}

void Module::sendToDetector(int fnum) {
    static_cast<const Module &>(*this).sendToDetector(fnum);
}

template <typename Ret> Ret Module::sendToDetector(int fnum) const {
    LOG(logDEBUG1) << "Sending: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", nullptr, 0, " << typeid(Ret).name() << ", "
                   << sizeof(Ret) << "]";
    STATIC_ASSERT_ARG(Ret, "sendToDetector")
    Ret retval{};
    sendToDetector(fnum, nullptr, 0, &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << ToString(retval);
    return retval;
}

template <typename Ret> Ret Module::sendToDetector(int fnum) {
    return static_cast<const Module &>(*this).sendToDetector<Ret>(fnum);
}

template <typename Ret, typename Arg>
Ret Module::sendToDetector(int fnum, const Arg &args) const {
    LOG(logDEBUG1) << "Sending: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", " << args << ", " << sizeof(args) << ", "
                   << typeid(Ret).name() << ", " << sizeof(Ret) << "]";
    STATIC_ASSERT_ARG(Arg, "sendToDetector")
    STATIC_ASSERT_ARG(Ret, "sendToDetector")
    Ret retval{};
    sendToDetector(fnum, &args, sizeof(args), &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << ToString(retval);
    return retval;
}

template <typename Ret, typename Arg>
Ret Module::sendToDetector(int fnum, const Arg &args) {
    return static_cast<const Module &>(*this).sendToDetector<Ret>(fnum, args);
}

//---------------------------------------------------------- sendToDetectorStop

void Module::sendToDetectorStop(int fnum, const void *args, size_t args_size,
                                void *retval, size_t retval_size) const {
    // This is the only function that actually sends data to the detector stop
    // the other versions use templates to deduce sizes and create
    // the return type
    checkArgs(args, args_size, retval, retval_size);
    auto stop = DetectorSocket(shm()->hostname, shm()->stopPort);
    stop.sendCommandThenRead(fnum, args, args_size, retval, retval_size);
    stop.close();
}

void Module::sendToDetectorStop(int fnum, const void *args, size_t args_size,
                                void *retval, size_t retval_size) {
    static_cast<const Module &>(*this).sendToDetectorStop(fnum, args, args_size,
                                                          retval, retval_size);
}

template <typename Arg, typename Ret>
void Module::sendToDetectorStop(int fnum, const Arg &args, Ret &retval) const {
    LOG(logDEBUG1) << "Sending to Stop: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", " << args << ", " << sizeof(args) << ", "
                   << typeid(Ret).name() << ", " << sizeof(Ret) << "]";
    STATIC_ASSERT_ARG(Arg, "sendToDetectorStop")
    STATIC_ASSERT_ARG(Ret, "sendToDetectorStop")
    sendToDetectorStop(fnum, &args, sizeof(args), &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << ToString(retval);
}

template <typename Arg, typename Ret>
void Module::sendToDetectorStop(int fnum, const Arg &args, Ret &retval) {
    static_cast<const Module &>(*this).sendToDetectorStop(fnum, args, retval);
}

template <typename Arg>
void Module::sendToDetectorStop(int fnum, const Arg &args,
                                std::nullptr_t) const {
    LOG(logDEBUG1) << "Sending to Stop: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", " << typeid(Arg).name() << ", " << sizeof(Arg)
                   << ", nullptr, 0 ]";
    STATIC_ASSERT_ARG(Arg, "sendToDetectorStop")
    sendToDetectorStop(fnum, &args, sizeof(args), nullptr, 0);
}

template <typename Arg>
void Module::sendToDetectorStop(int fnum, const Arg &args, std::nullptr_t) {
    static_cast<const Module &>(*this).sendToDetectorStop(fnum, args, nullptr);
}

template <typename Ret>
void Module::sendToDetectorStop(int fnum, std::nullptr_t, Ret &retval) const {
    LOG(logDEBUG1) << "Sending to Stop: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", nullptr, 0, " << typeid(Ret).name() << ", "
                   << sizeof(Ret) << "]";
    STATIC_ASSERT_ARG(Ret, "sendToDetectorStop")
    sendToDetectorStop(fnum, nullptr, 0, &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << retval;
}

template <typename Ret>
void Module::sendToDetectorStop(int fnum, std::nullptr_t, Ret &retval) {
    static_cast<const Module &>(*this).sendToDetectorStop(fnum, nullptr,
                                                          retval);
}

void Module::sendToDetectorStop(int fnum) const {
    LOG(logDEBUG1) << "Sending to Stop: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", nullptr, 0, nullptr, 0]";
    sendToDetectorStop(fnum, nullptr, 0, nullptr, 0);
}

void Module::sendToDetectorStop(int fnum) {
    static_cast<const Module &>(*this).sendToDetectorStop(fnum);
}

template <typename Ret> Ret Module::sendToDetectorStop(int fnum) const {
    LOG(logDEBUG1) << "Sending to Stop: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", nullptr, 0, " << typeid(Ret).name() << ", "
                   << sizeof(Ret) << "]";
    STATIC_ASSERT_ARG(Ret, "sendToDetectorStop")
    Ret retval{};
    sendToDetectorStop(fnum, nullptr, 0, &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << retval;
    return retval;
}

template <typename Ret> Ret Module::sendToDetectorStop(int fnum) {
    return static_cast<const Module &>(*this).sendToDetectorStop<Ret>(fnum);
}

template <typename Ret, typename Arg>
Ret Module::sendToDetectorStop(int fnum, const Arg &args) const {
    LOG(logDEBUG1) << "Sending to Stop: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", " << args << ", " << sizeof(args) << ", "
                   << typeid(Ret).name() << ", " << sizeof(Ret) << "]";
    STATIC_ASSERT_ARG(Arg, "sendToDetectorStop")
    STATIC_ASSERT_ARG(Ret, "sendToDetectorStop")
    Ret retval{};
    sendToDetectorStop(fnum, &args, sizeof(args), &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << ToString(retval);
    return retval;
}

template <typename Ret, typename Arg>
Ret Module::sendToDetectorStop(int fnum, const Arg &args) {
    return static_cast<const Module &>(*this).sendToDetectorStop<Ret>(fnum,
                                                                      args);
}

//-------------------------------------------------------------- sendToReceiver

void Module::sendToReceiver(int fnum, const void *args, size_t args_size,
                            void *retval, size_t retval_size) const {
    // This is the only function that actually sends data to the receiver
    // the other versions use templates to deduce sizes and create
    // the return type
    if (!shm()->useReceiverFlag) {
        std::ostringstream oss;
        oss << "Set rx_hostname first to use receiver parameters, ";
        oss << getFunctionNameFromEnum(static_cast<detFuncs>(fnum));
        throw RuntimeError(oss.str());
    }
    checkArgs(args, args_size, retval, retval_size);
    auto receiver = ReceiverSocket(shm()->rxHostname, shm()->rxTCPPort);
    receiver.sendCommandThenRead(fnum, args, args_size, retval, retval_size);
    receiver.close();
}

void Module::sendToReceiver(int fnum, const void *args, size_t args_size,
                            void *retval, size_t retval_size) {
    static_cast<const Module &>(*this).sendToReceiver(fnum, args, args_size,
                                                      retval, retval_size);
}

template <typename Arg, typename Ret>
void Module::sendToReceiver(int fnum, const Arg &args, Ret &retval) const {
    LOG(logDEBUG1) << "Sending to Receiver: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", " << args << ", " << sizeof(args) << ", "
                   << typeid(Ret).name() << ", " << sizeof(Ret) << "]";
    STATIC_ASSERT_ARG(Arg, "sendToReceiver")
    STATIC_ASSERT_ARG(Ret, "sendToReceiver")
    sendToReceiver(fnum, &args, sizeof(args), &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << retval;
}

template <typename Arg, typename Ret>
void Module::sendToReceiver(int fnum, const Arg &args, Ret &retval) {
    static_cast<const Module &>(*this).sendToReceiver(fnum, args, retval);
}

template <typename Arg>
void Module::sendToReceiver(int fnum, const Arg &args, std::nullptr_t) const {
    LOG(logDEBUG1) << "Sending to Receiver: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", " << typeid(Arg).name() << ", " << sizeof(Arg)
                   << ", nullptr, 0 ]";
    STATIC_ASSERT_ARG(Arg, "sendToReceiver")
    sendToReceiver(fnum, &args, sizeof(args), nullptr, 0);
}

template <typename Arg>
void Module::sendToReceiver(int fnum, const Arg &args, std::nullptr_t) {
    static_cast<const Module &>(*this).sendToReceiver(fnum, args, nullptr);
}

template <typename Ret>
void Module::sendToReceiver(int fnum, std::nullptr_t, Ret &retval) const {
    LOG(logDEBUG1) << "Sending to Receiver: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", nullptr, 0, " << typeid(Ret).name() << ", "
                   << sizeof(Ret) << "]";
    STATIC_ASSERT_ARG(Ret, "sendToReceiver")
    sendToReceiver(fnum, nullptr, 0, &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << ToString(retval);
}

template <typename Ret>
void Module::sendToReceiver(int fnum, std::nullptr_t, Ret &retval) {
    static_cast<const Module &>(*this).sendToReceiver(fnum, nullptr, retval);
}

template <typename Ret> Ret Module::sendToReceiver(int fnum) const {
    LOG(logDEBUG1) << "Sending to Receiver: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", nullptr, 0, " << typeid(Ret).name() << ", "
                   << sizeof(Ret) << "]";
    STATIC_ASSERT_ARG(Ret, "sendToReceiver")
    Ret retval{};
    sendToReceiver(fnum, nullptr, 0, &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << ToString(retval);
    return retval;
}

template <typename Ret> Ret Module::sendToReceiver(int fnum) {
    return static_cast<const Module &>(*this).sendToReceiver<Ret>(fnum);
}

void Module::sendToReceiver(int fnum) const {
    LOG(logDEBUG1) << "Sending to Receiver: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", nullptr, 0, nullptr, 0]";
    sendToReceiver(fnum, nullptr, 0, nullptr, 0);
}

void Module::sendToReceiver(int fnum) {
    static_cast<const Module &>(*this).sendToReceiver(fnum);
}

template <typename Ret, typename Arg>
Ret Module::sendToReceiver(int fnum, const Arg &args) const {
    LOG(logDEBUG1) << "Sending to Receiver: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", " << args << ", " << sizeof(args) << ", "
                   << typeid(Ret).name() << ", " << sizeof(Ret) << "]";
    STATIC_ASSERT_ARG(Arg, "sendToReceiver")
    STATIC_ASSERT_ARG(Ret, "sendToReceiver")
    Ret retval{};
    sendToReceiver(fnum, &args, sizeof(args), &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << retval;
    return retval;
}

template <typename Ret, typename Arg>
Ret Module::sendToReceiver(int fnum, const Arg &args) {
    return static_cast<const Module &>(*this).sendToReceiver<Ret>(fnum, args);
}

slsDetectorDefs::detectorType Module::getDetectorTypeFromShm(int det_id,
                                                             bool verify) {
    if (!shm.IsExisting()) {
        throw SharedMemoryError("Shared memory " + shm.GetName() +
                                "does not exist.\n Corrupted Multi Shared "
                                "memory. Please free shared memory.");
    }

    shm.OpenSharedMemory();
    if (verify && shm()->shmversion != SLS_SHMVERSION) {
        std::ostringstream ss;
        ss << "Single shared memory (" << det_id << "-" << moduleId
           << ":)version mismatch (expected 0x" << std::hex << SLS_SHMVERSION
           << " but got 0x" << shm()->shmversion << ")" << std::dec
           << ". Clear Shared memory to continue.";
        shm.UnmapSharedMemory();
        throw SharedMemoryError(ss.str());
    }
    return shm()->myDetectorType;
}

void Module::initSharedMemory(detectorType type, int det_id, bool verify) {
    shm = SharedMemory<sharedSlsDetector>(det_id, moduleId);
    if (!shm.IsExisting()) {
        shm.CreateSharedMemory();
        initializeDetectorStructure(type);
    } else {
        shm.OpenSharedMemory();
        if (verify && shm()->shmversion != SLS_SHMVERSION) {
            std::ostringstream ss;
            ss << "Single shared memory (" << det_id << "-" << moduleId
               << ":) version mismatch (expected 0x" << std::hex
               << SLS_SHMVERSION << " but got 0x" << shm()->shmversion << ")"
               << std::dec << ". Clear Shared memory to continue.";
            throw SharedMemoryError(ss.str());
        }
    }
}

void Module::initializeDetectorStructure(detectorType type) {
    shm()->shmversion = SLS_SHMVERSION;
    memset(shm()->hostname, 0, MAX_STR_LENGTH);
    shm()->myDetectorType = type;
    shm()->numberOfDetector.x = 0;
    shm()->numberOfDetector.y = 0;
    shm()->controlPort = DEFAULT_PORTNO;
    shm()->stopPort = DEFAULT_PORTNO + 1;
    sls::strcpy_safe(shm()->settingsDir, getenv("HOME"));
    sls::strcpy_safe(shm()->rxHostname, "none");
    shm()->rxTCPPort = DEFAULT_PORTNO + 2;
    shm()->useReceiverFlag = false;
    shm()->zmqport = DEFAULT_ZMQ_CL_PORTNO +
                     (moduleId * ((shm()->myDetectorType == EIGER) ? 2 : 1));
    shm()->zmqip = IpAddr{};
    shm()->numUDPInterfaces = 1;
    shm()->stoppedFlag = false;

    // get the detector parameters based on type
    detParameters parameters{type};
    shm()->nChan.x = parameters.nChanX;
    shm()->nChan.y = parameters.nChanY;
    shm()->nChip.x = parameters.nChipX;
    shm()->nChip.y = parameters.nChipY;
    shm()->nDacs = parameters.nDacs;
}

void Module::checkDetectorVersionCompatibility() {
    int64_t arg = 0;
    switch (shm()->myDetectorType) {
    case EIGER:
        arg = APIEIGER;
        break;
    case JUNGFRAU:
        arg = APIJUNGFRAU;
        break;
    case GOTTHARD:
        arg = APIGOTTHARD;
        break;
    case CHIPTESTBOARD:
        arg = APICTB;
        break;
    case MOENCH:
        arg = APIMOENCH;
        break;
    case MYTHEN3:
        arg = APIMYTHEN3;
        break;
    case GOTTHARD2:
        arg = APIGOTTHARD2;
        break;
    default:
        throw NotImplementedError(
            "Check version compatibility is not implemented for this detector");
    }
    sendToDetector(F_CHECK_VERSION, arg, nullptr);
    sendToDetectorStop(F_CHECK_VERSION, arg, nullptr);
}

void Module::checkReceiverVersionCompatibility() {
    // TODO! Verify that this works as intended when version don't match
    sendToReceiver(F_RECEIVER_CHECK_VERSION, int64_t(APIRECEIVER), nullptr);
}

int Module::sendModule(sls_detector_module *myMod, sls::ClientSocket &client) {
    constexpr TLogLevel level = logDEBUG1;
    LOG(level) << "Sending Module";
    int ts = 0;
    int n = 0;
    n = client.Send(&(myMod->serialnumber), sizeof(myMod->serialnumber));
    ts += n;
    LOG(level) << "Serial number sent. " << n
               << " bytes. serialno: " << myMod->serialnumber;

    n = client.Send(&(myMod->nchan), sizeof(myMod->nchan));
    ts += n;
    LOG(level) << "nchan sent. " << n << " bytes. nchan: " << myMod->nchan;

    n = client.Send(&(myMod->nchip), sizeof(myMod->nchip));
    ts += n;
    LOG(level) << "nchip sent. " << n << " bytes. nchip: " << myMod->nchip;

    n = client.Send(&(myMod->ndac), sizeof(myMod->ndac));
    ts += n;
    LOG(level) << "ndac sent. " << n << " bytes. ndac: " << myMod->ndac;

    n = client.Send(&(myMod->reg), sizeof(myMod->reg));
    ts += n;
    LOG(level) << "reg sent. " << n << " bytes. reg: " << myMod->reg;

    n = client.Send(&(myMod->iodelay), sizeof(myMod->iodelay));
    ts += n;
    LOG(level) << "iodelay sent. " << n
               << " bytes. iodelay: " << myMod->iodelay;

    n = client.Send(&(myMod->tau), sizeof(myMod->tau));
    ts += n;
    LOG(level) << "tau sent. " << n << " bytes. tau: " << myMod->tau;

    n = client.Send(&(myMod->eV), sizeof(myMod->eV));
    ts += n;
    LOG(level) << "ev sent. " << n << " bytes. ev: " << myMod->eV;

    n = client.Send(myMod->dacs, sizeof(int) * (myMod->ndac));
    ts += n;
    LOG(level) << "dacs sent. " << n << " bytes";

    if (shm()->myDetectorType == EIGER || shm()->myDetectorType == MYTHEN3) {
        n = client.Send(myMod->chanregs, sizeof(int) * (myMod->nchan));
        ts += n;
        LOG(level) << "channels sent. " << n << " bytes";
    }
    return ts;
}

void Module::setModule(sls_detector_module &module, bool trimbits) {
    LOG(logDEBUG1) << "Setting module with trimbits:" << trimbits;
    // to exclude trimbits
    if (!trimbits) {
        module.nchan = 0;
        module.nchip = 0;
    }
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    client.Send(F_SET_MODULE);
    sendModule(&module, client);
    if (client.Receive<int>() == FAIL) {
        throw RuntimeError("Detector " + std::to_string(moduleId) +
                           " returned error: " + client.readErrorMessage());
    }
}

void Module::updateReceiverStreamingIP() {
    auto ip = getReceiverStreamingIP();
    if (ip == 0) {
        // Hostname could be ip try to decode otherwise look up the hostname
        ip = sls::IpAddr{shm()->rxHostname};
        if (ip == 0) {
            ip = HostnameToIp(shm()->rxHostname);
        }
        LOG(logINFO) << "Setting default receiver " << moduleId
                     << " streaming zmq ip to " << ip;
    }
    setReceiverStreamingIP(ip);
}

void Module::updateRateCorrection() {
    sendToDetector(F_UPDATE_RATE_CORRECTION);
}

void Module::setThresholdEnergyAndSettings(int e_eV, detectorSettings isettings,
                                           bool trimbits) {

    // verify e_eV exists in trimEneregies[]
    if (shm()->trimEnergies.empty() || (e_eV < shm()->trimEnergies.front()) ||
        (e_eV > shm()->trimEnergies.back())) {
        throw RuntimeError("This energy " + std::to_string(e_eV) +
                           " not defined for this module!");
    }

    bool interpolate =
        std::all_of(shm()->trimEnergies.begin(), shm()->trimEnergies.end(),
                    [e_eV](const int &e) { return e != e_eV; });

    sls_detector_module myMod{shm()->myDetectorType};

    if (!interpolate) {
        std::string settingsfname = getTrimbitFilename(isettings, e_eV);
        LOG(logDEBUG1) << "Settings File is " << settingsfname;
        myMod = readSettingsFile(settingsfname, trimbits);
    } else {
        // find the trim values
        int trim1 = -1, trim2 = -1;
        for (size_t i = 0; i < shm()->trimEnergies.size(); ++i) {
            if (e_eV < shm()->trimEnergies[i]) {
                trim2 = shm()->trimEnergies[i];
                trim1 = shm()->trimEnergies[i - 1];
                break;
            }
        }
        std::string settingsfname1 = getTrimbitFilename(isettings, trim1);
        std::string settingsfname2 = getTrimbitFilename(isettings, trim2);
        LOG(logDEBUG1) << "Settings Files are " << settingsfname1 << " and "
                       << settingsfname2;
        auto myMod1 = readSettingsFile(settingsfname1, trimbits);
        auto myMod2 = readSettingsFile(settingsfname2, trimbits);
        if (myMod1.iodelay != myMod2.iodelay) {
            throw RuntimeError("setThresholdEnergyAndSettings: Iodelays do not "
                               "match between files");
        }
        myMod = interpolateTrim(&myMod1, &myMod2, e_eV, trim1, trim2, trimbits);
        myMod.iodelay = myMod1.iodelay;
        myMod.tau =
            linearInterpolation(e_eV, trim1, trim2, myMod1.tau, myMod2.tau);
    }

    myMod.reg = isettings;
    myMod.eV = e_eV;
    setModule(myMod, trimbits);
    if (getSettings() != isettings) {
        throw RuntimeError("setThresholdEnergyAndSettings: Could not set "
                           "settings in detector");
    }
}

sls_detector_module Module::interpolateTrim(sls_detector_module *a,
                                            sls_detector_module *b,
                                            const int energy, const int e1,
                                            const int e2, bool trimbits) {

    // only implemented for eiger currently (in terms of which dacs)
    if (shm()->myDetectorType != EIGER) {
        throw NotImplementedError(
            "Interpolation of Trim values not implemented for this detector!");
    }

    sls_detector_module myMod{shm()->myDetectorType};
    enum eiger_DacIndex {
        E_SVP,
        E_VTR,
        E_VRF,
        E_VRS,
        E_SVN,
        E_VTGSTV,
        E_VCMP_LL,
        E_VCMP_LR,
        E_CAL,
        E_VCMP_RL,
        E_RXB_RB,
        E_RXB_LB,
        E_VCMP_RR,
        E_VCP,
        E_VCN,
        E_VIS
    };

    // Copy other dacs
    int dacs_to_copy[] = {E_SVP,    E_VTR,    E_SVN, E_VTGSTV,
                          E_RXB_RB, E_RXB_LB, E_VCN, E_VIS};
    int num_dacs_to_copy = sizeof(dacs_to_copy) / sizeof(dacs_to_copy[0]);
    for (int i = 0; i < num_dacs_to_copy; ++i) {
        if (a->dacs[dacs_to_copy[i]] != b->dacs[dacs_to_copy[i]]) {
            throw RuntimeError("Interpolate module: dacs different");
        }
        myMod.dacs[dacs_to_copy[i]] = a->dacs[dacs_to_copy[i]];
    }

    // Copy irrelevant dacs (without failing): CAL
    if (a->dacs[E_CAL] != b->dacs[E_CAL]) {
        LOG(logWARNING) << "DAC CAL differs in both energies ("
                        << a->dacs[E_CAL] << "," << b->dacs[E_CAL]
                        << ")!\nTaking first: " << a->dacs[E_CAL];
    }
    myMod.dacs[E_CAL] = a->dacs[E_CAL];

    // Interpolate vrf, vcmp, vcp
    int dacs_to_interpolate[] = {E_VRF,     E_VCMP_LL, E_VCMP_LR, E_VCMP_RL,
                                 E_VCMP_RR, E_VCP,     E_VRS};
    int num_dacs_to_interpolate =
        sizeof(dacs_to_interpolate) / sizeof(dacs_to_interpolate[0]);
    for (int i = 0; i < num_dacs_to_interpolate; ++i) {
        myMod.dacs[dacs_to_interpolate[i]] =
            linearInterpolation(energy, e1, e2, a->dacs[dacs_to_interpolate[i]],
                                b->dacs[dacs_to_interpolate[i]]);
    }
    // Interpolate all trimbits
    if (trimbits) {
        for (int i = 0; i < myMod.nchan; ++i) {
            myMod.chanregs[i] = linearInterpolation(
                energy, e1, e2, a->chanregs[i], b->chanregs[i]);
        }
    }
    return myMod;
}

std::string Module::getTrimbitFilename(detectorSettings s, int e_eV) {
    std::string ssettings;
    switch (s) {
    case STANDARD:
        ssettings = "/standard";
        break;
    case HIGHGAIN:
        ssettings = "/highgain";
        break;
    case LOWGAIN:
        ssettings = "/lowgain";
        break;
    case VERYHIGHGAIN:
        ssettings = "/veryhighgain";
        break;
    case VERYLOWGAIN:
        ssettings = "/verylowgain";
        break;
    default:
        std::ostringstream ss;
        ss << "Unknown settings " << ToString(s) << " for this detector!";
        throw RuntimeError(ss.str());
    }
    std::ostringstream ostfn;
    ostfn << shm()->settingsDir << ssettings << "/" << e_eV << "eV"
          << "/noise.sn" << std::setfill('0') << std::setw(3) << std::dec
          << getSerialNumber() << std::setbase(10);
    return ostfn.str();
}

sls_detector_module Module::readSettingsFile(const std::string &fname,
                                             bool trimbits) {
    LOG(logDEBUG1) << "Read settings file " << fname;
    sls_detector_module myMod(shm()->myDetectorType);
    // open file
    std::ifstream infile;
    if (shm()->myDetectorType == EIGER || shm()->myDetectorType == MYTHEN3) {
        infile.open(fname.c_str(), std::ifstream::binary);
    } else {
        infile.open(fname.c_str(), std::ios_base::in);
    }
    if (!infile) {
        throw RuntimeError("Could not open settings file: " + fname);
    }

    // eiger
    if (shm()->myDetectorType == EIGER) {
        infile.read(reinterpret_cast<char *>(myMod.dacs),
                    sizeof(int) * (myMod.ndac));
        infile.read(reinterpret_cast<char *>(&myMod.iodelay),
                    sizeof(myMod.iodelay));
        infile.read(reinterpret_cast<char *>(&myMod.tau), sizeof(myMod.tau));
        if (trimbits) {
            infile.read(reinterpret_cast<char *>(myMod.chanregs),
                        sizeof(int) * (myMod.nchan));
        }
        if (!infile) {
            throw RuntimeError("readSettingsFile: Could not load all values "
                               "for settings for " +
                               fname);
        }
        for (int i = 0; i < myMod.ndac; ++i) {
            LOG(logDEBUG1) << "dac " << i << ":" << myMod.dacs[i];
        }
        LOG(logDEBUG1) << "iodelay:" << myMod.iodelay;
        LOG(logDEBUG1) << "tau:" << myMod.tau;
    }

    // mythen3 (dacs, trimbits)
    else if (shm()->myDetectorType == MYTHEN3) {
        infile.read(reinterpret_cast<char *>(myMod.dacs),
                    sizeof(int) * (myMod.ndac));
        infile.read(reinterpret_cast<char *>(myMod.chanregs),
                    sizeof(int) * (myMod.nchan));

        if (!infile) {
            throw RuntimeError("readSettingsFile: Could not load all values "
                               "for settings for " +
                               fname);
        }
        for (int i = 0; i < myMod.ndac; ++i) {
            LOG(logDEBUG1) << "dac " << i << ":" << myMod.dacs[i];
        }
    }

    else {
        throw RuntimeError("Not implemented for this detector");
    }
    LOG(logINFO) << "Settings file loaded: " << fname.c_str();
    return myMod;
}

void Module::programFPGAviaBlackfin(std::vector<char> buffer) {
    uint64_t filesize = buffer.size();
    // send program from memory to detector
    LOG(logINFO) << "Sending programming binary (from pof) to detector "
                 << moduleId << " (" << shm()->hostname << ")";
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    client.Send(F_PROGRAM_FPGA);
    client.Send(filesize);
    // error in detector at opening file pointer to flash
    if (client.Receive<int>() == FAIL) {
        std::ostringstream os;
        os << "Detector " << moduleId << " (" << shm()->hostname << ")"
           << " returned error: " << client.readErrorMessage();
        throw RuntimeError(os.str());
    }

    // erasing flash
    LOG(logINFO) << "Erasing Flash for detector " << moduleId << " ("
                 << shm()->hostname << ")";
    printf("%d%%\r", 0);
    std::cout << std::flush;
    // erasing takes 65 seconds, printing here (otherwise need threads
    // in server-unnecessary)
    const int ERASE_TIME = 65;
    int count = ERASE_TIME + 1;
    while (count > 0) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        --count;
        printf(
            "%d%%\r",
            static_cast<int>(
                (static_cast<double>(ERASE_TIME - count) / ERASE_TIME) * 100));
        std::cout << std::flush;
    }
    printf("\n");
    LOG(logINFO) << "Writing to Flash to detector " << moduleId << " ("
                 << shm()->hostname << ")";
    printf("%d%%\r", 0);
    std::cout << std::flush;

    // sending program in parts of 2mb each
    uint64_t unitprogramsize = 0;
    int currentPointer = 0;
    uint64_t totalsize = filesize;
    while (filesize > 0) {
        unitprogramsize = MAX_FPGAPROGRAMSIZE; // 2mb
        if (unitprogramsize > filesize) {      // less than 2mb
            unitprogramsize = filesize;
        }
        LOG(logDEBUG1) << "unitprogramsize:" << unitprogramsize
                       << "\t filesize:" << filesize;

        client.Send(&buffer[currentPointer], unitprogramsize);
        if (client.Receive<int>() == FAIL) {
            std::cout << '\n';
            std::ostringstream os;
            os << "Detector " << moduleId << " (" << shm()->hostname << ")"
               << " returned error: " << client.readErrorMessage();
            throw RuntimeError(os.str());
        }
        filesize -= unitprogramsize;
        currentPointer += unitprogramsize;

        // print progress
        printf(
            "%d%%\r",
            static_cast<int>(
                (static_cast<double>(totalsize - filesize) / totalsize) * 100));
        std::cout << std::flush;
    }
    std::cout << '\n';

    // fpga has picked up from flash successfully
    if (client.Receive<int>() == FAIL) {
        std::ostringstream os;
        os << "Detector " << moduleId << " (" << shm()->hostname << ")"
           << " returned error: " << client.readErrorMessage();
        throw RuntimeError(os.str());
    }

    LOG(logINFO) << "FPGA programmed successfully";
    rebootController();
}

void Module::programFPGAviaNios(std::vector<char> buffer) {
    LOG(logINFO) << "Sending programming binary (from rbf) to detector "
                 << moduleId << " (" << shm()->hostname << ")";

    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    client.Send(F_PROGRAM_FPGA);
    uint64_t filesize = buffer.size();
    client.Send(filesize);
    if (client.Receive<int>() == FAIL) {
        std::ostringstream os;
        os << "Detector " << moduleId << " (" << shm()->hostname << ")"
           << " returned error: " << client.readErrorMessage();
        throw RuntimeError(os.str());
    }
    client.Send(buffer);
    if (client.Receive<int>() == FAIL) {
        std::ostringstream os;
        os << "Detector " << moduleId << " (" << shm()->hostname << ")"
           << " returned error: " << client.readErrorMessage();
        throw RuntimeError(os.str());
    }
    LOG(logINFO) << "FPGA programmed successfully";
    rebootController();
}
} // namespace sls
