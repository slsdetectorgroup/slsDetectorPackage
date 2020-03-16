#include "Detector.h"
#include "CmdParser.h"
#include "CmdProxy.h"
#include "container_utils.h"
#include "detectorData.h"
#include "logger.h"
#include "DetectorImpl.h"
#include "Module.h"
#include "sls_detector_defs.h"
#include "versionAPI.h"

#include <fstream>

namespace sls {

void freeSharedMemory(int multiId, int detPos) {
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

using defs = slsDetectorDefs;

Detector::Detector(int shm_id)
    : pimpl(sls::make_unique<DetectorImpl>(shm_id)) {}

Detector::~Detector() = default;

// Configuration
void Detector::freeSharedMemory() { pimpl->freeSharedMemory(); }

void Detector::loadConfig(const std::string &fname) {
    int shm_id = getShmId();
    freeSharedMemory();
    pimpl = sls::make_unique<DetectorImpl>(shm_id);
    LOG(logINFO) << "Loading configuration file: " << fname;
    loadParameters(fname);
}

void Detector::loadParameters(const std::string &fname) {
    CmdProxy proxy(this);
    CmdParser parser;
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
        LOG(logDEBUG1)
            << "current_line after removing comments:\n\t" << current_line;
        if (current_line.length() > 1) {
            parser.Parse(current_line);
            proxy.Call(parser.command(), parser.arguments(),
                       parser.detector_id(), defs::PUT_ACTION);
        }
    }
    input_file.close();
}

Result<std::string> Detector::getHostname(Positions pos) const {
    return pimpl->Parallel(&Module::getHostname, pos);
}

void Detector::setHostname(const std::vector<std::string> &hostname) {
    pimpl->setHostname(hostname);
}

void Detector::setVirtualDetectorServers(int numServers, int startingPort) {
    pimpl->setVirtualDetectorServers(numServers, startingPort);
}

int Detector::getShmId() const { return pimpl->getMultiId(); }

std::string Detector::getPackageVersion() const {
    return GITBRANCH;
}

int64_t Detector::getClientVersion() const {
    return APILIB;
}

Result<int64_t> Detector::getFirmwareVersion(Positions pos) const {
    return pimpl->Parallel(&Module::getFirmwareVersion, pos);
}

Result<int64_t> Detector::getDetectorServerVersion(Positions pos) const {
    return pimpl->Parallel(&Module::getDetectorServerVersion, pos);
}

Result<int64_t> Detector::getSerialNumber(Positions pos) const {
    return pimpl->Parallel(&Module::getSerialNumber, pos);
}

Result<int64_t> Detector::getReceiverVersion(Positions pos) const {
    return pimpl->Parallel(&Module::getReceiverSoftwareVersion, pos);
}

Result<defs::detectorType> Detector::getDetectorType(Positions pos) const {
    return pimpl->Parallel(&Module::getDetectorType, pos);
}

int Detector::size() const { return pimpl->size(); }

bool Detector::empty() const { return pimpl->size() == 0; }

defs::xy Detector::getModuleGeometry() const {
    return pimpl->getNumberOfDetectors();
}

Result<defs::xy> Detector::getModuleSize(Positions pos) const {
    return pimpl->Parallel(&Module::getNumberOfChannels, pos);
}

defs::xy Detector::getDetectorSize() const {
    return pimpl->getNumberOfChannels();
}

void Detector::setDetectorSize(const defs::xy value) {
    pimpl->setNumberOfChannels(value);
}

Result<defs::detectorSettings> Detector::getSettings(Positions pos) const {
    return pimpl->Parallel(&Module::getSettings, pos);
}

void Detector::setSettings(const defs::detectorSettings value, Positions pos) {
    pimpl->Parallel(&Module::setSettings, pos, value);
}

// Callback

void Detector::registerAcquisitionFinishedCallback(void (*func)(double, int,
                                                                void *),
                                                   void *pArg) {
    pimpl->registerAcquisitionFinishedCallback(func, pArg);
}

void Detector::registerDataCallback(void (*func)(detectorData *, uint64_t,
                                                 uint32_t, void *),
                                    void *pArg) {
    pimpl->registerDataCallback(func, pArg);
}

// Acquisition Parameters

Result<int64_t> Detector::getNumberOfFrames(Positions pos) const {
    return pimpl->Parallel(&Module::getNumberOfFrames, pos);
}

void Detector::setNumberOfFrames(int64_t value) {
    pimpl->Parallel(&Module::setNumberOfFrames, {}, value);
}

Result<int64_t> Detector::getNumberOfTriggers(Positions pos) const {
    return pimpl->Parallel(&Module::getNumberOfTriggers, pos);
}

void Detector::setNumberOfTriggers(int64_t value) {
    pimpl->Parallel(&Module::setNumberOfTriggers, {}, value);
}

Result<ns> Detector::getExptime(Positions pos) const {
    return pimpl->Parallel(&Module::getExptime, pos);
}

void Detector::setExptime(ns t, Positions pos) {
    pimpl->Parallel(&Module::setExptime, pos, t.count());
}

Result<ns> Detector::getPeriod(Positions pos) const {
    return pimpl->Parallel(&Module::getPeriod, pos);
}

void Detector::setPeriod(ns t, Positions pos) {
    pimpl->Parallel(&Module::setPeriod, pos, t.count());
}

Result<ns> Detector::getDelayAfterTrigger(Positions pos) const {
    return pimpl->Parallel(&Module::getDelayAfterTrigger, pos);
}

void Detector::setDelayAfterTrigger(ns value, Positions pos) {
    pimpl->Parallel(&Module::setDelayAfterTrigger, pos, value.count());
}

Result<int64_t> Detector::getNumberOfFramesLeft(Positions pos) const {
    return pimpl->Parallel(&Module::getNumberOfFramesLeft, pos);
}

Result<int64_t> Detector::getNumberOfTriggersLeft(Positions pos) const {
    return pimpl->Parallel(&Module::getNumberOfTriggersLeft, pos);
}

Result<ns> Detector::getDelayAfterTriggerLeft(Positions pos) const {
    return pimpl->Parallel(&Module::getDelayAfterTriggerLeft, pos);
}

Result<ns> Detector::getPeriodLeft(Positions pos) const {
    return pimpl->Parallel(&Module::getPeriodLeft, pos);
}

Result<defs::timingMode> Detector::getTimingMode(Positions pos) const {
    return pimpl->Parallel(&Module::setTimingMode, pos,
                           defs::GET_TIMING_MODE);
}

void Detector::setTimingMode(defs::timingMode value, Positions pos) {
    pimpl->Parallel(&Module::setTimingMode, pos, value);
}

Result<defs::speedLevel> Detector::getSpeed(Positions pos) const {
    auto res =
        pimpl->Parallel(&Module::getClockDivider, pos, defs::RUN_CLOCK);
    Result<defs::speedLevel> speedResult(res.size());
    for (unsigned int i = 0; i < res.size(); ++i) {
        speedResult[i] = static_cast<defs::speedLevel>(res[i]);
    }
    return speedResult;
}

void Detector::setSpeed(defs::speedLevel value, Positions pos) {
    pimpl->Parallel(&Module::setClockDivider, pos, defs::RUN_CLOCK,
                    static_cast<int>(value));
}

Result<int> Detector::getADCPhase(Positions pos) const {
    return pimpl->Parallel(&Module::getClockPhase, pos, defs::ADC_CLOCK,
                           false);
}

void Detector::setADCPhase(int value, Positions pos) {
    pimpl->Parallel(&Module::setClockPhase, pos, defs::ADC_CLOCK, value,
                    false);
}

Result<int> Detector::getMaxADCPhaseShift(Positions pos) const {
    return pimpl->Parallel(&Module::getMaxClockPhaseShift, pos,
                           defs::ADC_CLOCK);
}

Result<int> Detector::getADCPhaseInDegrees(Positions pos) const {
    return pimpl->Parallel(&Module::getClockPhase, pos, defs::ADC_CLOCK,
                           true);
}

void Detector::setADCPhaseInDegrees(int value, Positions pos) {
    pimpl->Parallel(&Module::setClockPhase, pos, defs::ADC_CLOCK, value,
                    true);
}

Result<int> Detector::getDBITPhase(Positions pos) const {
    return pimpl->Parallel(&Module::getClockPhase, pos, defs::DBIT_CLOCK,
                           false);
}

void Detector::setDBITPhase(int value, Positions pos) {
    pimpl->Parallel(&Module::setClockPhase, pos, defs::DBIT_CLOCK, value,
                    false);
}

Result<int> Detector::getMaxDBITPhaseShift(Positions pos) const {
    return pimpl->Parallel(&Module::getMaxClockPhaseShift, pos,
                           defs::DBIT_CLOCK);
}

Result<int> Detector::getDBITPhaseInDegrees(Positions pos) const {
    return pimpl->Parallel(&Module::getClockPhase, pos, defs::DBIT_CLOCK,
                           true);
}

void Detector::setDBITPhaseInDegrees(int value, Positions pos) {
    pimpl->Parallel(&Module::setClockPhase, pos, defs::DBIT_CLOCK, value,
                    true);
}

Result<int> Detector::getClockFrequency(int clkIndex, Positions pos) {
    return pimpl->Parallel(&Module::getClockFrequency, pos, clkIndex);
}

void Detector::setClockFrequency(int clkIndex, int value, Positions pos) {
    pimpl->Parallel(&Module::setClockFrequency, pos, clkIndex, value);
}

Result<int> Detector::getClockPhase(int clkIndex, Positions pos) {
    return pimpl->Parallel(&Module::getClockPhase, pos, clkIndex, false);
}

void Detector::setClockPhase(int clkIndex, int value, Positions pos) {
    pimpl->Parallel(&Module::setClockPhase, pos, clkIndex, value, false);
}

Result<int> Detector::getMaxClockPhaseShift(int clkIndex, Positions pos) {
    return pimpl->Parallel(&Module::getMaxClockPhaseShift, pos, clkIndex);
}

Result<int> Detector::getClockPhaseinDegrees(int clkIndex, Positions pos) {
    return pimpl->Parallel(&Module::getClockPhase, pos, clkIndex, true);
}

void Detector::setClockPhaseinDegrees(int clkIndex, int value, Positions pos) {
    pimpl->Parallel(&Module::setClockPhase, pos, clkIndex, value, true);
}

Result<int> Detector::getClockDivider(int clkIndex, Positions pos) {
    return pimpl->Parallel(&Module::getClockDivider, pos, clkIndex);
}

void Detector::setClockDivider(int clkIndex, int value, Positions pos) {
    pimpl->Parallel(&Module::setClockDivider, pos, clkIndex, value);
}

Result<int> Detector::getHighVoltage(Positions pos) const {
    return pimpl->Parallel(&Module::setDAC, pos, -1, defs::HIGH_VOLTAGE,
                           0);
}

void Detector::setHighVoltage(int value, Positions pos) {
    pimpl->Parallel(&Module::setDAC, pos, value, defs::HIGH_VOLTAGE, 0);
}

Result<bool> Detector::getPowerChip(Positions pos) const {
    return pimpl->Parallel(&Module::powerChip, pos, -1);
}

void Detector::setPowerChip(bool on, Positions pos) {
    if ((pos.empty() || pos[0] == -1) && on && pimpl->size() > 3) {
        for (int i = 0; i != pimpl->size(); ++i) {
            pimpl->Parallel(&Module::powerChip, {i}, static_cast<int>(on));
            usleep(1000 * 1000);
        }
    } else {
        pimpl->Parallel(&Module::powerChip, pos, static_cast<int>(on));
    }
}

Result<int> Detector::getTemperature(defs::dacIndex index,
                                     Positions pos) const {
    switch (index) {
    case defs::TEMPERATURE_ADC:
    case defs::TEMPERATURE_FPGA:
    case defs::TEMPERATURE_FPGAEXT:
    case defs::TEMPERATURE_10GE:
    case defs::TEMPERATURE_DCDC:
    case defs::TEMPERATURE_SODL:
    case defs::TEMPERATURE_SODR:
    case defs::TEMPERATURE_FPGA2:
    case defs::TEMPERATURE_FPGA3:
    case defs::SLOW_ADC_TEMP:
        break;
    default:
        throw RuntimeError("Unknown Temperature Index");
    }
    auto res = pimpl->Parallel(&Module::getADC, pos, index);
    switch (getDetectorType().squash()) {
    case defs::EIGER:
    case defs::JUNGFRAU:
        for (auto &it : res) {
            it /= 1000;
        }
        break;
    default:
        break;
    }
    return res;
}

Result<int> Detector::getDAC(defs::dacIndex index, bool mV,
                             Positions pos) const {
    return pimpl->Parallel(&Module::setDAC, pos, -1, index, mV);
}

void Detector::setDAC(defs::dacIndex index, int value, bool mV, Positions pos) {
    pimpl->Parallel(&Module::setDAC, pos, value, index, mV);
}

Result<int> Detector::getOnChipDAC(defs::dacIndex index, int chipIndex,
                                   Positions pos) const {
    return pimpl->Parallel(&Module::getOnChipDAC, pos, index, chipIndex);
}

void Detector::setOnChipDAC(defs::dacIndex index, int chipIndex, int value,
                            Positions pos) {
    pimpl->Parallel(&Module::setOnChipDAC, pos, index, chipIndex, value);
}

// Acquisition

void Detector::acquire() { pimpl->acquire(); }

void Detector::clearAcquiringFlag() { pimpl->setAcquiringFlag(0); }

void Detector::startReceiver() {
    pimpl->Parallel(&Module::startReceiver, {});
}

void Detector::stopReceiver() {
    pimpl->Parallel(&Module::stopReceiver, {});
}

void Detector::startDetector() {
    if (getDetectorType().squash() == defs::EIGER) {
        pimpl->Parallel(&Module::prepareAcquisition, {});
    }
    pimpl->Parallel(&Module::startAcquisition, {});
}

void Detector::stopDetector() {
    pimpl->Parallel(&Module::stopAcquisition, {});
}

Result<defs::runStatus> Detector::getDetectorStatus(Positions pos) const {
    return pimpl->Parallel(&Module::getRunStatus, pos);
}

Result<defs::runStatus> Detector::getReceiverStatus(Positions pos) const {
    return pimpl->Parallel(&Module::getReceiverStatus, pos);
}

Result<int64_t> Detector::getFramesCaught(Positions pos) const {
    return pimpl->Parallel(&Module::getFramesCaughtByReceiver, pos);
}

Result<std::vector<uint64_t>> Detector::getNumMissingPackets(Positions pos) const {
    return pimpl->Parallel(&Module::getNumMissingPackets, pos);
}

Result<uint64_t> Detector::getStartingFrameNumber(Positions pos) const {
    return pimpl->Parallel(&Module::getStartingFrameNumber, pos);
}

void Detector::setStartingFrameNumber(uint64_t value, Positions pos) {
    pimpl->Parallel(&Module::setStartingFrameNumber, pos, value);
}

void Detector::sendSoftwareTrigger(Positions pos) {
    pimpl->Parallel(&Module::sendSoftwareTrigger, pos);
}

// Network Configuration (Detector<->Receiver)

Result<int> Detector::getNumberofUDPInterfaces(Positions pos) const {
    return pimpl->Parallel(&Module::getNumberofUDPInterfaces, pos);
}

void Detector::setNumberofUDPInterfaces(int n, Positions pos) {
    int previouslyClientStreaming = pimpl->enableDataStreamingToClient();
    bool useReceiver = getUseReceiverFlag().squash(false);
    bool previouslyReceiverStreaming = false;
    if (useReceiver) {
        previouslyReceiverStreaming = getRxZmqDataStream(pos).squash(true);
    }
    pimpl->Parallel(&Module::setNumberofUDPInterfaces, pos, n);
    // ensure receiver zmq socket ports are multiplied by 2 (2 interfaces)
    if (getUseReceiverFlag().squash(false) && size()) {
        int startingPort = getRxZmqPort({0}).squash(0);
        setRxZmqPort(startingPort, -1);
    }
    // redo the zmq sockets if enabled
    if (previouslyClientStreaming != 0) {
        pimpl->enableDataStreamingToClient(0);
        pimpl->enableDataStreamingToClient(1);
    }
    if (previouslyReceiverStreaming) {
        setRxZmqDataStream(false, pos);
        setRxZmqDataStream(true, pos);
    }
}

Result<int> Detector::getSelectedUDPInterface(Positions pos) const {
    return pimpl->Parallel(&Module::getSelectedUDPInterface, pos);
}

void Detector::selectUDPInterface(int interface, Positions pos) {
    pimpl->Parallel(&Module::selectUDPInterface, pos, interface);
}

Result<IpAddr> Detector::getSourceUDPIP(Positions pos) const {
    return pimpl->Parallel(&Module::getSourceUDPIP, pos);
}

void Detector::setSourceUDPIP(const IpAddr ip, Positions pos) {
    pimpl->Parallel(&Module::setSourceUDPIP, pos, ip);
}

Result<IpAddr> Detector::getSourceUDPIP2(Positions pos) const {
    return pimpl->Parallel(&Module::getSourceUDPIP2, pos);
}

void Detector::setSourceUDPIP2(const IpAddr ip, Positions pos) {
    pimpl->Parallel(&Module::setSourceUDPIP2, pos, ip);
}

Result<MacAddr> Detector::getSourceUDPMAC(Positions pos) const {
    return pimpl->Parallel(&Module::getSourceUDPMAC, pos);
}

void Detector::setSourceUDPMAC(const MacAddr mac, Positions pos) {
    pimpl->Parallel(&Module::setSourceUDPMAC, pos, mac);
}

Result<MacAddr> Detector::getSourceUDPMAC2(Positions pos) const {
    return pimpl->Parallel(&Module::getSourceUDPMAC2, pos);
}

void Detector::setSourceUDPMAC2(const MacAddr mac, Positions pos) {
    pimpl->Parallel(&Module::setSourceUDPMAC2, pos, mac);
}

Result<IpAddr> Detector::getDestinationUDPIP(Positions pos) const {
    return pimpl->Parallel(&Module::getDestinationUDPIP, pos);
}

void Detector::setDestinationUDPIP(const IpAddr ip, Positions pos) {
    pimpl->Parallel(&Module::setDestinationUDPIP, pos, ip);
}

Result<IpAddr> Detector::getDestinationUDPIP2(Positions pos) const {
    return pimpl->Parallel(&Module::getDestinationUDPIP2, pos);
}

void Detector::setDestinationUDPIP2(const IpAddr ip, Positions pos) {
    pimpl->Parallel(&Module::setDestinationUDPIP2, pos, ip);
}

Result<MacAddr> Detector::getDestinationUDPMAC(Positions pos) const {
    return pimpl->Parallel(&Module::getDestinationUDPMAC, pos);
}

void Detector::setDestinationUDPMAC(const MacAddr mac, Positions pos) {
    pimpl->Parallel(&Module::setDestinationUDPMAC, pos, mac);
}

Result<MacAddr> Detector::getDestinationUDPMAC2(Positions pos) const {
    return pimpl->Parallel(&Module::getDestinationUDPMAC2, pos);
}

void Detector::setDestinationUDPMAC2(const MacAddr mac, Positions pos) {
    pimpl->Parallel(&Module::setDestinationUDPMAC2, pos, mac);
}

Result<int> Detector::getDestinationUDPPort(Positions pos) const {
    return pimpl->Parallel(&Module::getDestinationUDPPort, pos);
}

void Detector::setDestinationUDPPort(int port, int module_id) {
    if (module_id == -1) {
        std::vector<int> port_list = getPortNumbers(port);
        for (int idet = 0; idet < size(); ++idet) {
            pimpl->Parallel(&Module::setDestinationUDPPort, {idet},
                            port_list[idet]);
        }
    } else {
        pimpl->Parallel(&Module::setDestinationUDPPort, {module_id}, port);
    }
}

Result<int> Detector::getDestinationUDPPort2(Positions pos) const {
    return pimpl->Parallel(&Module::getDestinationUDPPort2, pos);
}

void Detector::setDestinationUDPPort2(int port, int module_id) {
    if (module_id == -1) {
        std::vector<int> port_list = getPortNumbers(port);
        for (int idet = 0; idet < size(); ++idet) {
            pimpl->Parallel(&Module::setDestinationUDPPort2, {idet},
                            port_list[idet]);
        }
    } else {
        pimpl->Parallel(&Module::setDestinationUDPPort2, {module_id},
                        port);
    }
}

Result<std::string> Detector::printRxConfiguration(Positions pos) const {
    return pimpl->Parallel(&Module::printReceiverConfiguration, pos);
}

Result<bool> Detector::getTenGiga(Positions pos) const {
    return pimpl->Parallel(&Module::enableTenGigabitEthernet, pos, -1);
}

void Detector::setTenGiga(bool value, Positions pos) {
    pimpl->Parallel(&Module::enableTenGigabitEthernet, pos,
                    static_cast<int>(value));
}

Result<bool> Detector::getTenGigaFlowControl(Positions pos) const {
    return pimpl->Parallel(&Module::getTenGigaFlowControl, pos);
}

void Detector::setTenGigaFlowControl(bool enable, Positions pos) {
    pimpl->Parallel(&Module::setTenGigaFlowControl, pos, enable);
}

Result<int> Detector::getTransmissionDelayFrame(Positions pos) const {
    return pimpl->Parallel(&Module::getTransmissionDelayFrame, pos);
}

void Detector::setTransmissionDelayFrame(int value, Positions pos) {
    pimpl->Parallel(&Module::setTransmissionDelayFrame, pos, value);
}

Result<int> Detector::getTransmissionDelayLeft(Positions pos) const {
    return pimpl->Parallel(&Module::getTransmissionDelayLeft, pos);
}

void Detector::setTransmissionDelayLeft(int value, Positions pos) {
    pimpl->Parallel(&Module::setTransmissionDelayLeft, pos, value);
}

Result<int> Detector::getTransmissionDelayRight(Positions pos) const {
    return pimpl->Parallel(&Module::getTransmissionDelayRight, pos);
}

void Detector::setTransmissionDelayRight(int value, Positions pos) {
    pimpl->Parallel(&Module::setTransmissionDelayRight, pos, value);
}

// Receiver

Result<bool> Detector::getUseReceiverFlag(Positions pos) const {
    return pimpl->Parallel(&Module::getUseReceiverFlag, pos);
}

Result<std::string> Detector::getRxHostname(Positions pos) const {
    return pimpl->Parallel(&Module::getReceiverHostname, pos);
}

void Detector::setRxHostname(const std::string &receiver, Positions pos) {
    pimpl->Parallel(&Module::setReceiverHostname, pos, receiver);
}

Result<int> Detector::getRxPort(Positions pos) const {
    return pimpl->Parallel(&Module::getReceiverPort, pos);
}

void Detector::setRxPort(int port, int module_id) {
    if (module_id == -1) {
        std::vector<int> port_list(size());
        for (auto &it : port_list) {
            it = port++;
        }
        for (int idet = 0; idet < size(); ++idet) {
            pimpl->Parallel(&Module::setReceiverPort, {idet},
                            port_list[idet]);
        }
    } else {
        pimpl->Parallel(&Module::setReceiverPort, {module_id}, port);
    }
}

Result<int> Detector::getRxFifoDepth(Positions pos) const {
    return pimpl->Parallel(&Module::setReceiverFifoDepth, pos, -1);
}

void Detector::setRxFifoDepth(int nframes, Positions pos) {
    pimpl->Parallel(&Module::setReceiverFifoDepth, pos, nframes);
}

Result<bool> Detector::getRxSilentMode(Positions pos) const {
    return pimpl->Parallel(&Module::setReceiverSilentMode, pos, -1);
}

void Detector::setRxSilentMode(bool value, Positions pos) {
    pimpl->Parallel(&Module::setReceiverSilentMode, pos,
                    static_cast<int>(value));
}

Result<defs::frameDiscardPolicy>
Detector::getRxFrameDiscardPolicy(Positions pos) const {
    return pimpl->Parallel(&Module::setReceiverFramesDiscardPolicy, pos,
                           defs::GET_FRAME_DISCARD_POLICY);
}

void Detector::setRxFrameDiscardPolicy(defs::frameDiscardPolicy f,
                                       Positions pos) {
    pimpl->Parallel(&Module::setReceiverFramesDiscardPolicy, pos, f);
}

Result<bool> Detector::getPartialFramesPadding(Positions pos) const {
    return pimpl->Parallel(&Module::getPartialFramesPadding, pos);
}

void Detector::setPartialFramesPadding(bool value, Positions pos) {
    pimpl->Parallel(&Module::setPartialFramesPadding, pos, value);
}

Result<int64_t> Detector::getRxUDPSocketBufferSize(Positions pos) const {
    return pimpl->Parallel(&Module::getReceiverUDPSocketBufferSize, pos);
}

void Detector::setRxUDPSocketBufferSize(int64_t udpsockbufsize, Positions pos) {
    pimpl->Parallel(&Module::setReceiverUDPSocketBufferSize, pos,
                    udpsockbufsize);
}

Result<int64_t> Detector::getRxRealUDPSocketBufferSize(Positions pos) const {
    return pimpl->Parallel(&Module::getReceiverRealUDPSocketBufferSize,
                           pos);
}

Result<bool> Detector::getRxLock(Positions pos) {
    return pimpl->Parallel(&Module::lockReceiver, pos, -1);
}

void Detector::setRxLock(bool value, Positions pos) {
    pimpl->Parallel(&Module::lockReceiver, pos, static_cast<int>(value));
}

Result<sls::IpAddr> Detector::getRxLastClientIP(Positions pos) const {
    return pimpl->Parallel(&Module::getReceiverLastClientIP, pos);
}

// File

Result<defs::fileFormat> Detector::getFileFormat(Positions pos) const {
    return pimpl->Parallel(&Module::getFileFormat, pos);
}

void Detector::setFileFormat(defs::fileFormat f, Positions pos) {
    pimpl->Parallel(&Module::setFileFormat, pos, f);
}

Result<std::string> Detector::getFilePath(Positions pos) const {
    return pimpl->Parallel(&Module::getFilePath, pos);
}

void Detector::setFilePath(const std::string &fpath, Positions pos) {
    pimpl->Parallel(&Module::setFilePath, pos, fpath);
}

Result<std::string> Detector::getFileNamePrefix(Positions pos) const {
    return pimpl->Parallel(&Module::getFileName, pos);
}

void Detector::setFileNamePrefix(const std::string &fname, Positions pos) {
    pimpl->Parallel(&Module::setFileName, pos, fname);
}

Result<int64_t> Detector::getAcquisitionIndex(Positions pos) const {
    return pimpl->Parallel(&Module::getFileIndex, pos);
}

void Detector::setAcquisitionIndex(int64_t i, Positions pos) {
    pimpl->Parallel(&Module::setFileIndex, pos, i);
}

Result<bool> Detector::getFileWrite(Positions pos) const {
    return pimpl->Parallel(&Module::getFileWrite, pos);
}

void Detector::setFileWrite(bool value, Positions pos) {
    pimpl->Parallel(&Module::setFileWrite, pos, value);
}

void Detector::setMasterFileWrite(bool value, Positions pos) {
    pimpl->Parallel(&Module::setMasterFileWrite, pos, value);
}

Result<bool> Detector::getMasterFileWrite(Positions pos) const {
    return pimpl->Parallel(&Module::getMasterFileWrite, pos);
}

Result<bool> Detector::getFileOverWrite(Positions pos) const {
    return pimpl->Parallel(&Module::getFileOverWrite, pos);
}

void Detector::setFileOverWrite(bool value, Positions pos) {
    pimpl->Parallel(&Module::setFileOverWrite, pos, value);
}

Result<int> Detector::getFramesPerFile(Positions pos) const {
    return pimpl->Parallel(&Module::getFramesPerFile, pos);
}

void Detector::setFramesPerFile(int n, Positions pos) {
    pimpl->Parallel(&Module::setFramesPerFile, pos, n);
}

// Zmq Streaming (Receiver<->Client)

Result<bool> Detector::getRxZmqDataStream(Positions pos) const {
    return pimpl->Parallel(&Module::enableDataStreamingFromReceiver, pos,
                           -1);
}

void Detector::setRxZmqDataStream(bool value, Positions pos) {
    pimpl->Parallel(&Module::enableDataStreamingFromReceiver, pos,
                    static_cast<int>(value));
}

Result<int> Detector::getRxZmqFrequency(Positions pos) const {
    return pimpl->Parallel(&Module::getReceiverStreamingFrequency, pos);
}

void Detector::setRxZmqFrequency(int freq, Positions pos) {
    pimpl->Parallel(&Module::setReceiverStreamingFrequency, pos, freq);
}

Result<int> Detector::getRxZmqTimer(Positions pos) const {
    return pimpl->Parallel(&Module::setReceiverStreamingTimer, pos, -1);
}

void Detector::setRxZmqTimer(int time_in_ms, Positions pos) {
    pimpl->Parallel(&Module::setReceiverStreamingTimer, pos, time_in_ms);
}

Result<int> Detector::getRxZmqPort(Positions pos) const {
    return pimpl->Parallel(&Module::getReceiverStreamingPort, pos);
}

void Detector::setRxZmqPort(int port, int module_id) {
    if (module_id == -1) {
        std::vector<int> port_list = getPortNumbers(port);
        for (int idet = 0; idet < size(); ++idet) {
            pimpl->Parallel(&Module::setReceiverStreamingPort, {idet},
                            port_list[idet]);
        }
    } else {
        pimpl->Parallel(&Module::setReceiverStreamingPort, {module_id},
                        port);
    }
}

Result<IpAddr> Detector::getRxZmqIP(Positions pos) const {
    return pimpl->Parallel(&Module::getReceiverStreamingIP, pos);
}

void Detector::setRxZmqIP(const IpAddr ip, Positions pos) {
    bool previouslyReceiverStreaming = getRxZmqDataStream(pos).squash(false);
    pimpl->Parallel(&Module::setReceiverStreamingIP, pos, ip);
    if (previouslyReceiverStreaming) {
        setRxZmqDataStream(false, pos);
        setRxZmqDataStream(true, pos);
    }
}

Result<int> Detector::getClientZmqPort(Positions pos) const {
    return pimpl->Parallel(&Module::getClientStreamingPort, pos);
}

void Detector::setClientZmqPort(int port, int module_id) {
    if (module_id == -1) {
        std::vector<int> port_list = getPortNumbers(port);
        for (int idet = 0; idet < size(); ++idet) {
            pimpl->Parallel(&Module::setClientStreamingPort, {idet},
                            port_list[idet]);
        }
    } else {
        pimpl->Parallel(&Module::setClientStreamingPort, {module_id},
                        port);
    }
}

Result<IpAddr> Detector::getClientZmqIp(Positions pos) const {
    return pimpl->Parallel(&Module::getClientStreamingIP, pos);
}

void Detector::setClientZmqIp(const IpAddr ip, Positions pos) {
    int previouslyClientStreaming = pimpl->enableDataStreamingToClient(-1);
    pimpl->Parallel(&Module::setClientStreamingIP, pos, ip);
    if (previouslyClientStreaming != 0) {
        pimpl->enableDataStreamingToClient(0);
        pimpl->enableDataStreamingToClient(1);
    }
}

// Eiger Specific

Result<int> Detector::getDynamicRange(Positions pos) const {
    return pimpl->Parallel(&Module::setDynamicRange, pos, -1);
}

void Detector::setDynamicRange(int value) {
    pimpl->Parallel(&Module::setDynamicRange, {}, value);
}

Result<ns> Detector::getSubExptime(Positions pos) const {
    return pimpl->Parallel(&Module::getSubExptime, pos);
}

void Detector::setSubExptime(ns t, Positions pos) {
    pimpl->Parallel(&Module::setSubExptime, pos, t.count());
}

Result<ns> Detector::getSubDeadTime(Positions pos) const {
    return pimpl->Parallel(&Module::getSubDeadTime, pos);
}

void Detector::setSubDeadTime(ns value, Positions pos) {
    pimpl->Parallel(&Module::setSubDeadTime, pos, value.count());
}

Result<int> Detector::getThresholdEnergy(Positions pos) const {
    return pimpl->Parallel(&Module::getThresholdEnergy, pos);
}

void Detector::setThresholdEnergy(int threshold_ev,
                                  defs::detectorSettings settings,
                                  bool trimbits, Positions pos) {
    pimpl->Parallel(&Module::setThresholdEnergy, pos, threshold_ev,
                    settings, static_cast<int>(trimbits));
}

Result<std::string> Detector::getSettingsPath(Positions pos) const {
    return pimpl->Parallel(&Module::getSettingsDir, pos);
}

void Detector::setSettingsPath(const std::string &value, Positions pos) {
    pimpl->Parallel(&Module::setSettingsDir, pos, value);
}

void Detector::loadTrimbits(const std::string &fname, Positions pos) {
    pimpl->Parallel(&Module::loadSettingsFile, pos, fname);
}

Result<bool> Detector::getRxAddGapPixels(Positions pos) const {
    return pimpl->Parallel(&Module::enableGapPixels, pos, -1);
}

void Detector::setRxAddGapPixels(bool enable) {
    pimpl->setGapPixelsinReceiver(enable);
}

Result<bool> Detector::getParallelMode(Positions pos) const {
    return pimpl->Parallel(&Module::getParallelMode, pos);
}

void Detector::setParallelMode(bool value, Positions pos) {
    pimpl->Parallel(&Module::setParallelMode, pos, value);
}

Result<bool> Detector::getOverFlowMode(Positions pos) const {
    return pimpl->Parallel(&Module::getOverFlowMode, pos);
}

void Detector::setOverFlowMode(bool value, Positions pos) {
    pimpl->Parallel(&Module::setOverFlowMode, pos, value);
}

Result<bool> Detector::getStoreInRamMode(Positions pos) const {
    return pimpl->Parallel(&Module::getStoreInRamMode, pos);
}

void Detector::setStoreInRamMode(bool value, Positions pos) {
    pimpl->Parallel(&Module::setStoreInRamMode, pos, value);
}

Result<bool> Detector::getBottom(Positions pos) const {
    return pimpl->Parallel(&Module::getFlippedDataX, pos);
}

void Detector::setBottom(bool value, Positions pos) {
    pimpl->Parallel(&Module::setFlippedDataX, pos, value);
}

Result<int> Detector::getAllTrimbits(Positions pos) const {
    return pimpl->Parallel(&Module::setAllTrimbits, pos, -1);
}

void Detector::setAllTrimbits(int value, Positions pos) {
    pimpl->Parallel(&Module::setAllTrimbits, pos, value);
}

Result<std::vector<int>> Detector::getTrimEnergies(Positions pos) const {
    return pimpl->Parallel(&Module::getTrimEn, pos);
}

void Detector::setTrimEnergies(std::vector<int> energies, Positions pos) {
    pimpl->Parallel(&Module::setTrimEn, pos, energies);
}

Result<ns> Detector::getRateCorrection(Positions pos) const {
    return pimpl->Parallel(&Module::getRateCorrection, pos);
}

void Detector::setDefaultRateCorrection(Positions pos) {
    pimpl->Parallel(&Module::setDefaultRateCorrection, pos);
}

void Detector::setRateCorrection(ns dead_time, Positions pos) {
    pimpl->Parallel(&Module::setRateCorrection, pos, dead_time.count());
}

Result<int> Detector::getPartialReadout(Positions pos) const {
    return pimpl->Parallel(&Module::getReadNLines, pos);
}

void Detector::setPartialReadout(const int lines, Positions pos) {
    pimpl->Parallel(&Module::setReadNLines, pos, lines);
}

Result<bool> Detector::getInterruptSubframe(Positions pos) const {
    return pimpl->Parallel(&Module::getInterruptSubframe, pos);
}

void Detector::setInterruptSubframe(const bool enable, Positions pos) {
    pimpl->Parallel(&Module::setInterruptSubframe, pos, enable);
}

Result<ns> Detector::getMeasuredPeriod(Positions pos) const {
    return pimpl->Parallel(&Module::getMeasuredPeriod, pos);
}

Result<ns> Detector::getMeasuredSubFramePeriod(Positions pos) const {
    return pimpl->Parallel(&Module::getMeasuredSubFramePeriod, pos);
}

Result<bool> Detector::getActive(Positions pos) const {
    return pimpl->Parallel(&Module::activate, pos, -1);
}

void Detector::setActive(bool active, Positions pos) {
    pimpl->Parallel(&Module::activate, pos, static_cast<int>(active));
}

Result<bool> Detector::getRxPadDeactivatedMode(Positions pos) const {
    return pimpl->Parallel(&Module::setDeactivatedRxrPaddingMode, pos, -1);
}

void Detector::setRxPadDeactivatedMode(bool pad, Positions pos) {
    pimpl->Parallel(&Module::setDeactivatedRxrPaddingMode, pos,
                    static_cast<int>(pad));
}

Result<bool> Detector::getPartialReset(Positions pos) const {
    auto res = pimpl->Parallel(&Module::setCounterBit, pos, -1);
    Result<bool> t(res.size());
    for (unsigned int i = 0; i < res.size(); ++i) {
        t[i] = !res[i];
    }
    return t;
}

void Detector::setPartialReset(bool value, Positions pos) {
    pimpl->Parallel(&Module::setCounterBit, pos, !value);
}

void Detector::pulsePixel(int n, defs::xy pixel, Positions pos) {
    pimpl->Parallel(&Module::pulsePixel, pos, n, pixel.x, pixel.y);
}

void Detector::pulsePixelNMove(int n, defs::xy pixel, Positions pos) {
    pimpl->Parallel(&Module::pulsePixelNMove, pos, n, pixel.x, pixel.y);
}

void Detector::pulseChip(int n, Positions pos) {
    pimpl->Parallel(&Module::pulseChip, pos, n);
}

Result<bool> Detector::getQuad(Positions pos) const {
    return pimpl->Parallel(&Module::getQuad, pos);
}

void Detector::setQuad(const bool enable) {
    if (enable && size() > 1) {
        throw RuntimeError("Cannot set Quad type as it is available only for 1 "
                           "Eiger Quad Half module.");
    }
    pimpl->Parallel(&Module::setQuad, {}, enable);
}

// Jungfrau Specific

Result<int> Detector::getThresholdTemperature(Positions pos) const {
    auto res = pimpl->Parallel(&Module::setThresholdTemperature, pos, -1);
    for (auto &it : res) {
        it /= 1000;
    }
    return res;
}

void Detector::setThresholdTemperature(int temp, Positions pos) {
    pimpl->Parallel(&Module::setThresholdTemperature, pos, temp * 1000);
}

Result<bool> Detector::getTemperatureControl(Positions pos) const {
    return pimpl->Parallel(&Module::setTemperatureControl, pos, -1);
}

void Detector::setTemperatureControl(bool enable, Positions pos) {
    pimpl->Parallel(&Module::setTemperatureControl, pos,
                    static_cast<int>(enable));
}

Result<int> Detector::getTemperatureEvent(Positions pos) const {
    return pimpl->Parallel(&Module::setTemperatureEvent, pos, -1);
}

void Detector::resetTemperatureEvent(Positions pos) {
    pimpl->Parallel(&Module::setTemperatureEvent, pos, 0);
}

Result<bool> Detector::getAutoCompDisable(Positions pos) const {
    return pimpl->Parallel(&Module::setAutoComparatorDisableMode, pos, -1);
}

void Detector::setAutoCompDisable(bool value, Positions pos) {
    pimpl->Parallel(&Module::setAutoComparatorDisableMode, pos,
                    static_cast<int>(value));
}

Result<int> Detector::getNumberOfAdditionalStorageCells(Positions pos) const {
    return pimpl->Parallel(&Module::getNumberOfAdditionalStorageCells,
                           pos);
}

void Detector::setNumberOfAdditionalStorageCells(int value) {
    pimpl->Parallel(&Module::setNumberOfAdditionalStorageCells, {}, value);
}

Result<int> Detector::getStorageCellStart(Positions pos) const {
    return pimpl->Parallel(&Module::setStoragecellStart, pos, -1);
}

void Detector::setStoragecellStart(int cell, Positions pos) {
    pimpl->Parallel(&Module::setStoragecellStart, pos, cell);
}

Result<ns> Detector::getStorageCellDelay(Positions pos) const {
    return pimpl->Parallel(&Module::getStorageCellDelay, pos);
}

void Detector::setStorageCellDelay(ns value, Positions pos) {
    pimpl->Parallel(&Module::setStorageCellDelay, pos, value.count());
}

// Gotthard Specific

Result<defs::ROI> Detector::getROI(Positions pos) const {
    return pimpl->Parallel(&Module::getROI, pos);
}

void Detector::setROI(defs::ROI value, int module_id) {
    if (module_id < 0 && size() > 1) {
        throw RuntimeError("Cannot set ROI for all modules simultaneously");
    }
    pimpl->Parallel(&Module::setROI, {module_id}, value);
}

void Detector::clearROI(Positions pos) {
    pimpl->Parallel(&Module::clearROI, pos);
}

Result<ns> Detector::getExptimeLeft(Positions pos) const {
    return pimpl->Parallel(&Module::getExptimeLeft, pos);
}

Result<defs::externalSignalFlag>
Detector::getExternalSignalFlags(Positions pos) const {
    return pimpl->Parallel(&Module::setExternalSignalFlags, pos,
                           defs::GET_EXTERNAL_SIGNAL_FLAG);
}

void Detector::setExternalSignalFlags(defs::externalSignalFlag value,
                                      Positions pos) {
    pimpl->Parallel(&Module::setExternalSignalFlags, pos, value);
}

Result<int> Detector::getImageTestMode(Positions pos) {
    return pimpl->Parallel(&Module::getImageTestMode, pos);
}

void Detector::setImageTestMode(int value, Positions pos) {
    pimpl->Parallel(&Module::setImageTestMode, pos, value);
}

// Gotthard2 Specific

Result<int64_t> Detector::getNumberOfBursts(Positions pos) const {
    return pimpl->Parallel(&Module::getNumberOfBursts, pos);
}

void Detector::setNumberOfBursts(int64_t value) {
    pimpl->Parallel(&Module::setNumberOfBursts, {}, value);
}

Result<ns> Detector::getBurstPeriod(Positions pos) const {
    return pimpl->Parallel(&Module::getBurstPeriod, pos);
}

void Detector::setBurstPeriod(ns value, Positions pos) {
    pimpl->Parallel(&Module::setBurstPeriod, pos, value.count());
}

Result<std::array<int, 2>> Detector::getInjectChannel(Positions pos) {
    return pimpl->Parallel(&Module::getInjectChannel, pos);
}

void Detector::setInjectChannel(const int offsetChannel, const int incrementChannel, Positions pos) {
    pimpl->Parallel(&Module::setInjectChannel, pos, offsetChannel, incrementChannel);
}

Result<std::vector<int>> Detector::getVetoPhoton(const int chipIndex, Positions pos) {
    return pimpl->Parallel(&Module::getVetoPhoton, pos, chipIndex);
}

void Detector::setVetoPhoton(const int chipIndex, const int numPhotons, const int energy, const std::string& fname,  Positions pos) {
    pimpl->Parallel(&Module::setVetoPhoton, pos, chipIndex, numPhotons, energy, fname);
}    

void Detector::setVetoReference(const int gainIndex, const int value, Positions pos) {
    pimpl->Parallel(&Module::setVetoReference, pos, gainIndex, value);
} 

Result<defs::burstMode> Detector::getBurstMode(Positions pos) {
    return pimpl->Parallel(&Module::getBurstMode, pos);
}

void Detector::setBurstMode(defs::burstMode value, Positions pos) {
    pimpl->Parallel(&Module::setBurstMode, pos, value);
}

Result<bool> Detector::getCurrentSource(Positions pos) const {
    return pimpl->Parallel(&Module::getCurrentSource, pos);
}

void Detector::setCurrentSource(bool value, Positions pos) {
    pimpl->Parallel(&Module::setCurrentSource, pos, value);
}

Result<defs::timingSourceType> Detector::getTimingSource(Positions pos) const {
    return pimpl->Parallel(&Module::getTimingSource, pos);
}

void Detector::setTimingSource(defs::timingSourceType value, Positions pos) {
    pimpl->Parallel(&Module::setTimingSource, pos, value);
}

// Mythen3 Specific

Result<uint32_t> Detector::getCounterMask(Positions pos) const {
    return pimpl->Parallel(&Module::getCounterMask, pos);
}

void Detector::setCounterMask(uint32_t countermask, Positions pos) {
    pimpl->Parallel(&Module::setCounterMask, pos, countermask);
}

// CTB/ Moench Specific

Result<int> Detector::getNumberOfAnalogSamples(Positions pos) const {
    return pimpl->Parallel(&Module::getNumberOfAnalogSamples, pos);
}

void Detector::setNumberOfAnalogSamples(int value, Positions pos) {
    pimpl->Parallel(&Module::setNumberOfAnalogSamples, pos, value);
}


Result<int> Detector::getADCClock(Positions pos) const {
    return pimpl->Parallel(&Module::getClockFrequency, pos,
                           defs::ADC_CLOCK);
}

void Detector::setADCClock(int value_in_MHz, Positions pos) {
    pimpl->Parallel(&Module::setClockFrequency, pos, defs::ADC_CLOCK,
                    value_in_MHz);
}

Result<int> Detector::getRUNClock(Positions pos) const {
    return pimpl->Parallel(&Module::getClockFrequency, pos,
                           defs::RUN_CLOCK);
}

void Detector::setRUNClock(int value_in_MHz, Positions pos) {
    pimpl->Parallel(&Module::setClockFrequency, pos, defs::RUN_CLOCK,
                    value_in_MHz);
}

Result<int> Detector::getSYNCClock(Positions pos) const {
    return pimpl->Parallel(&Module::getClockFrequency, pos,
                           defs::SYNC_CLOCK);
}

Result<int> Detector::getADCPipeline(Positions pos) const {
    return pimpl->Parallel(&Module::getPipeline, pos, defs::ADC_CLOCK);
}

void Detector::setADCPipeline(int value, Positions pos) {
    pimpl->Parallel(&Module::setPipeline, pos, defs::ADC_CLOCK, value);
}

Result<int> Detector::getDBITPipeline(Positions pos) const {
    return pimpl->Parallel(&Module::getPipeline, pos, defs::DBIT_CLOCK);
}

Result<int> Detector::getVoltage(defs::dacIndex index, Positions pos) const {
    switch (index) {
    case defs::V_LIMIT:
    case defs::V_POWER_A:
    case defs::V_POWER_B:
    case defs::V_POWER_C:
    case defs::V_POWER_D:
    case defs::V_POWER_IO:
    case defs::V_POWER_CHIP:
        break;
    default:
        throw RuntimeError("Unknown Voltage Index");
    }
    return pimpl->Parallel(&Module::setDAC, pos, -1, index, 1);
}

void Detector::setVoltage(defs::dacIndex index, int value, Positions pos) {
    switch (index) {
    case defs::V_LIMIT:
    case defs::V_POWER_A:
    case defs::V_POWER_B:
    case defs::V_POWER_C:
    case defs::V_POWER_D:
    case defs::V_POWER_IO:
    case defs::V_POWER_CHIP:
        break;
    default:
        throw RuntimeError("Unknown Voltage Index");
    }
    pimpl->Parallel(&Module::setDAC, pos, value, index, 1);
}

Result<uint32_t> Detector::getADCEnableMask(Positions pos) const {
    return pimpl->Parallel(&Module::getADCEnableMask, pos);
}

void Detector::setADCEnableMask(uint32_t mask, Positions pos) {
    pimpl->Parallel(&Module::setADCEnableMask, pos, mask);
}

Result<uint32_t> Detector::getTenGigaADCEnableMask(Positions pos) const {
    return pimpl->Parallel(&Module::getTenGigaADCEnableMask, pos);
}

void Detector::setTenGigaADCEnableMask(uint32_t mask, Positions pos) {
    pimpl->Parallel(&Module::setTenGigaADCEnableMask, pos, mask);
}

// CTB Specific


Result<int> Detector::getNumberOfDigitalSamples(Positions pos) const {
    return pimpl->Parallel(&Module::getNumberOfDigitalSamples, pos);
}

void Detector::setNumberOfDigitalSamples(int value, Positions pos) {
    pimpl->Parallel(&Module::setNumberOfDigitalSamples, pos, value);
}

Result<defs::readoutMode> Detector::getReadoutMode(Positions pos) const {
    return pimpl->Parallel(&Module::getReadoutMode, pos);
}

void Detector::setReadoutMode(defs::readoutMode value, Positions pos) {
    pimpl->Parallel(&Module::setReadoutMode, pos, value);
}

Result<int> Detector::getDBITClock(Positions pos) const {
    return pimpl->Parallel(&Module::getClockFrequency, pos,
                           defs::DBIT_CLOCK);
}

void Detector::setDBITClock(int value_in_MHz, Positions pos) {
    pimpl->Parallel(&Module::setClockFrequency, pos, defs::DBIT_CLOCK,
                    value_in_MHz);
}

void Detector::setDBITPipeline(int value, Positions pos) {
    pimpl->Parallel(&Module::setPipeline, pos, defs::DBIT_CLOCK, value);
}

Result<int> Detector::getMeasuredVoltage(defs::dacIndex index,
                                         Positions pos) const {
    switch (index) {
    case defs::V_POWER_A:
    case defs::V_POWER_B:
    case defs::V_POWER_C:
    case defs::V_POWER_D:
    case defs::V_POWER_IO:
    case defs::V_POWER_CHIP:
        break;
    default:
        throw RuntimeError("Unknown Voltage Index");
    }
    return pimpl->Parallel(&Module::getADC, pos, index);
}

Result<int> Detector::getMeasuredCurrent(defs::dacIndex index,
                                         Positions pos) const {
    switch (index) {
    case defs::I_POWER_A:
    case defs::I_POWER_B:
    case defs::I_POWER_C:
    case defs::I_POWER_D:
    case defs::I_POWER_IO:
        break;
    default:
        throw RuntimeError("Unknown Current Index");
    }
    return pimpl->Parallel(&Module::getADC, pos, index);
}

Result<int> Detector::getSlowADC(defs::dacIndex index, Positions pos) const {
    if (index < defs::SLOW_ADC0 || index > defs::SLOW_ADC7) {
        throw RuntimeError("Unknown Slow ADC Index");
    }
    return pimpl->Parallel(&Module::getADC, pos, index);
}

Result<int> Detector::getExternalSamplingSource(Positions pos) const {
    return pimpl->Parallel(&Module::getExternalSamplingSource, pos);
}

void Detector::setExternalSamplingSource(int value, Positions pos) {
    pimpl->Parallel(&Module::setExternalSamplingSource, pos, value);
}

Result<int> Detector::getExternalSampling(Positions pos) const {
    return pimpl->Parallel(&Module::getExternalSampling, pos);
}

void Detector::setExternalSampling(bool value, Positions pos) {
    pimpl->Parallel(&Module::setExternalSampling, pos, value);
}

Result<std::vector<int>> Detector::getRxDbitList(Positions pos) const {
    return pimpl->Parallel(&Module::getReceiverDbitList, pos);
}

void Detector::setRxDbitList(const std::vector<int>& list, Positions pos) {
    pimpl->Parallel(&Module::setReceiverDbitList, pos, list);
}

Result<int> Detector::getRxDbitOffset(Positions pos) const {
    return pimpl->Parallel(&Module::getReceiverDbitOffset, pos);
}

void Detector::setRxDbitOffset(int value, Positions pos) {
    pimpl->Parallel(&Module::setReceiverDbitOffset, pos, value);
}

void Detector::setDigitalIODelay(uint64_t pinMask, int delay, Positions pos) {
    pimpl->Parallel(&Module::setDigitalIODelay, pos, pinMask, delay);
}

Result<bool> Detector::getLEDEnable(Positions pos) const {
    return pimpl->Parallel(&Module::setLEDEnable, pos, -1);
}

void Detector::setLEDEnable(bool enable, Positions pos) {
    pimpl->Parallel(&Module::setLEDEnable, pos, static_cast<int>(enable));
}

// Pattern

void Detector::savePattern(const std::string &fname) {
    std::ofstream outfile;
    outfile.open(fname.c_str(), std::ios_base::out);
    if (!outfile.is_open()) {
        throw RuntimeError("Could not create file to save pattern");
    }
    // get pattern limits
    auto r = pimpl->Parallel(&Module::setPatternLoopAddresses, {}, -1, -1, -1)
                 .tsquash("Inconsistent pattern limits");

    CmdProxy proxy(this);
    // pattern words
    for (int i = r[0]; i <= r[1]; ++i) {
        std::ostringstream os;
        os << "0x" << std::hex << i;
        auto addr = os.str();
        proxy.Call("patword", {addr}, -1, defs::GET_ACTION, outfile);
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
        proxy.Call(cmd, {}, -1, defs::GET_ACTION, outfile);
}

void Detector::setPattern(const std::string &fname, Positions pos) {
    pimpl->Parallel(&Module::setPattern, pos, fname);
}

Result<uint64_t> Detector::getPatternIOControl(Positions pos) const {
    return pimpl->Parallel(&Module::setPatternIOControl, pos, -1);
}

void Detector::setPatternIOControl(uint64_t word, Positions pos) {
    pimpl->Parallel(&Module::setPatternIOControl, pos, word);
}

Result<uint64_t> Detector::getPatternClockControl(Positions pos) const {
    return pimpl->Parallel(&Module::setPatternClockControl, pos, -1);
}

void Detector::setPatternClockControl(uint64_t word, Positions pos) {
    pimpl->Parallel(&Module::setPatternClockControl, pos, word);
}

Result<uint64_t> Detector::getPatternWord(int addr, Positions pos) {
    return pimpl->Parallel(&Module::setPatternWord, pos, addr, -1);
}

void Detector::setPatternWord(int addr, uint64_t word, Positions pos) {
    pimpl->Parallel(&Module::setPatternWord, pos, addr, word);
}

Result<std::array<int, 2>>
Detector::getPatternLoopAddresses(int level, Positions pos) const {
    return pimpl->Parallel(&Module::setPatternLoopAddresses, pos, level,
                           -1, -1);
}

void Detector::setPatternLoopAddresses(int level, int start, int stop,
                                       Positions pos) {
    pimpl->Parallel(&Module::setPatternLoopAddresses, pos, level, start,
                    stop);
}

Result<int> Detector::getPatternLoopCycles(int level, Positions pos) const {
    return pimpl->Parallel(&Module::setPatternLoopCycles, pos, level, -1);
}

void Detector::setPatternLoopCycles(int level, int n, Positions pos) {
    pimpl->Parallel(&Module::setPatternLoopCycles, pos, level, n);
}

Result<int> Detector::getPatternWaitAddr(int level, Positions pos) const {
    return pimpl->Parallel(&Module::setPatternWaitAddr, pos, level, -1);
}

void Detector::setPatternWaitAddr(int level, int addr, Positions pos) {
    pimpl->Parallel(&Module::setPatternWaitAddr, pos, level, addr);
}

Result<uint64_t> Detector::getPatternWaitTime(int level, Positions pos) const {
    return pimpl->Parallel(&Module::setPatternWaitTime, pos, level, -1);
}

void Detector::setPatternWaitTime(int level, uint64_t t, Positions pos) {
    pimpl->Parallel(&Module::setPatternWaitTime, pos, level, t);
}

Result<uint64_t> Detector::getPatternMask(Positions pos) {
    return pimpl->Parallel(&Module::getPatternMask, pos);
}

void Detector::setPatternMask(uint64_t mask, Positions pos) {
    pimpl->Parallel(&Module::setPatternMask, pos, mask);
}

Result<uint64_t> Detector::getPatternBitMask(Positions pos) const {
    return pimpl->Parallel(&Module::getPatternBitMask, pos);
}

void Detector::setPatternBitMask(uint64_t mask, Positions pos) {
    pimpl->Parallel(&Module::setPatternBitMask, pos, mask);
}

// Moench

Result<std::string> Detector::getAdditionalJsonHeader(Positions pos) const {
    return pimpl->Parallel(&Module::getAdditionalJsonHeader, pos);
}

void Detector::setAdditionalJsonHeader(const std::string &jsonheader,
                                       Positions pos) {
    pimpl->Parallel(&Module::setAdditionalJsonHeader, pos, jsonheader);
}

Result<std::string> Detector::getAdditionalJsonParameter(const std::string &key,
                                                         Positions pos) const {
    return pimpl->Parallel(&Module::getAdditionalJsonParameter, pos, key);
}

void Detector::setAdditionalJsonParameter(const std::string &key,
                                          const std::string &value,
                                          Positions pos) {
    pimpl->Parallel(&Module::setAdditionalJsonParameter, pos, key, value);
}

Result<int> Detector::getDetectorMinMaxEnergyThreshold(const bool isEmax,
                                                       Positions pos) const {
    auto res = pimpl->Parallel(&Module::getAdditionalJsonParameter, pos,
                               isEmax ? "emax" : "emin");
    Result<int> intResult(res.size());
    try {
        for (unsigned int i = 0; i < res.size(); ++i) {
            intResult[i] = stoi(res[i]);
        }
    } catch (...) {
        throw RuntimeError(
            "Cannot find or convert emin/emax string to integer");
    }
    return intResult;
}

void Detector::setDetectorMinMaxEnergyThreshold(const bool isEmax,
                                                const int value,
                                                Positions pos) {
    pimpl->Parallel(&Module::setAdditionalJsonParameter, pos,
                    isEmax ? "emax" : "emin", std::to_string(value));
}

Result<defs::frameModeType> Detector::getFrameMode(Positions pos) const {
    auto res = pimpl->Parallel(&Module::getAdditionalJsonParameter, pos,
                               "frameMode");
    Result<defs::frameModeType> intResult(res.size());
    try {
        for (unsigned int i = 0; i < res.size(); ++i) {
            intResult[i] =
                sls::StringTo<slsDetectorDefs::frameModeType>(res[i]);
        }
    } catch (...) {
        throw RuntimeError(
            "Cannot find or convert frameMode string to integer");
    }
    return intResult;
}

void Detector::setFrameMode(defs::frameModeType value, Positions pos) {
    pimpl->Parallel(&Module::setAdditionalJsonParameter, pos, "frameMode",
                    sls::ToString(value));
}

Result<defs::detectorModeType> Detector::getDetectorMode(Positions pos) const {
    auto res = pimpl->Parallel(&Module::getAdditionalJsonParameter, pos,
                               "detectorMode");
    Result<defs::detectorModeType> intResult(res.size());
    try {
        for (unsigned int i = 0; i < res.size(); ++i) {
            intResult[i] =
                sls::StringTo<slsDetectorDefs::detectorModeType>(res[i]);
        }
    } catch (...) {
        throw RuntimeError(
            "Cannot find or convert detectorMode string to integer");
    }
    return intResult;
}

void Detector::setDetectorMode(defs::detectorModeType value, Positions pos) {
    pimpl->Parallel(&Module::setAdditionalJsonParameter, pos,
                    "detectorMode", sls::ToString(value));
}

// Advanced

void Detector::programFPGA(const std::string &fname, Positions pos) {
    std::vector<char> buffer = pimpl->readProgrammingFile(fname);
    pimpl->Parallel(&Module::programFPGA, pos, buffer);
}

void Detector::resetFPGA(Positions pos) {
    pimpl->Parallel(&Module::resetFPGA, pos);
}

void Detector::copyDetectorServer(const std::string &fname,
                                  const std::string &hostname, Positions pos) {
    pimpl->Parallel(&Module::copyDetectorServer, pos, fname, hostname);
    rebootController(pos);
}

void Detector::rebootController(Positions pos) {
    pimpl->Parallel(&Module::rebootController, pos);
}

void Detector::updateFirmwareAndServer(const std::string &sname,
                                       const std::string &hostname,
                                       const std::string &fname,
                                       Positions pos) {
    pimpl->Parallel(&Module::copyDetectorServer, pos, sname, hostname);
    programFPGA(fname, pos);
    rebootController(pos);
}

Result<uint32_t> Detector::readRegister(uint32_t addr, Positions pos) const {
    return pimpl->Parallel(&Module::readRegister, pos, addr);
}

void Detector::writeRegister(uint32_t addr, uint32_t val, Positions pos) {
    pimpl->Parallel(&Module::writeRegister, pos, addr, val);
}

void Detector::setBit(uint32_t addr, int bitnr, Positions pos) {
    pimpl->Parallel(&Module::setBit, pos, addr, bitnr);
}

void Detector::clearBit(uint32_t addr, int bitnr, Positions pos) {
    pimpl->Parallel(&Module::clearBit, pos, addr, bitnr);
}

void Detector::executeFirmwareTest(Positions pos) {
    pimpl->Parallel(&Module::executeFirmwareTest, pos);
}

void Detector::executeBusTest(Positions pos) {
    pimpl->Parallel(&Module::executeBusTest, pos);
}

void Detector::writeAdcRegister(uint32_t addr, uint32_t value, Positions pos) {
    pimpl->Parallel(&Module::writeAdcRegister, pos, addr, value);
}

bool Detector::getInitialChecks() const {
    return pimpl->getInitialChecks();
}

void Detector::setInitialChecks(const bool value) {
    pimpl->setInitialChecks(value);
}

Result<uint32_t> Detector::getADCInvert(Positions pos) const {
    return pimpl->Parallel(&Module::getADCInvert, pos);
}

void Detector::setADCInvert(uint32_t value, Positions pos) {
    pimpl->Parallel(&Module::setADCInvert, pos, value);
}

// Insignificant

Result<int> Detector::getControlPort(Positions pos) const {
    return pimpl->Parallel(&Module::getControlPort, pos);
}

void Detector::setControlPort(int value, Positions pos) {
    pimpl->Parallel(&Module::setControlPort, pos, value);
}

Result<int> Detector::getStopPort(Positions pos) const {
    return pimpl->Parallel(&Module::getStopPort, pos);
}

void Detector::setStopPort(int value, Positions pos) {
    pimpl->Parallel(&Module::setStopPort, pos, value);
}

Result<bool> Detector::getDetectorLock(Positions pos) const {
    return pimpl->Parallel(&Module::lockServer, pos, -1);
}

void Detector::setDetectorLock(bool lock, Positions pos) {
    pimpl->Parallel(&Module::lockServer, pos, static_cast<int>(lock));
}

Result<sls::IpAddr> Detector::getLastClientIP(Positions pos) const {
    return pimpl->Parallel(&Module::getLastClientIP, pos);
}

void Detector::executeCommand(const std::string &value, Positions pos) {
    pimpl->Parallel(&Module::execCommand, pos, value);
}

Result<int64_t> Detector::getNumberOfFramesFromStart(Positions pos) const {
    return pimpl->Parallel(&Module::getNumberOfFramesFromStart, pos);
}

Result<ns> Detector::getActualTime(Positions pos) const {
    return pimpl->Parallel(&Module::getActualTime, pos);
}

Result<ns> Detector::getMeasurementTime(Positions pos) const {
    return pimpl->Parallel(&Module::getMeasurementTime, pos);
}

std::string Detector::getUserDetails() const { return pimpl->getUserDetails(); }

Result<uint64_t> Detector::getRxCurrentFrameIndex(Positions pos) const {
    return pimpl->Parallel(&Module::getReceiverCurrentFrameIndex, pos);
}

std::vector<int> Detector::getPortNumbers(int start_port) {
    int num_sockets_per_detector = 1;
    switch (getDetectorType().squash()) {
    case defs::EIGER:
        num_sockets_per_detector *= 2;
        break;
    case defs::JUNGFRAU:
        if (getNumberofUDPInterfaces().squash() == 2) {
            num_sockets_per_detector *= 2;
        }
        break;
    default:
        break;
    }
    std::vector<int> res;
    res.reserve(size());
    for (int idet = 0; idet < size(); ++idet) {
        res.push_back(start_port + (idet * num_sockets_per_detector));
    }
    return res;
}

} // namespace sls