#include "Detector.h"
#include "CmdParser.h"
#include "CmdProxy.h"
#include "container_utils.h"
#include "detectorData.h"
#include "logger.h"
#include "multiSlsDetector.h"
#include "slsDetector.h"
#include "sls_detector_defs.h"

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
    : pimpl(sls::make_unique<multiSlsDetector>(shm_id)) {}

Detector::~Detector() = default;

// Configuration
void Detector::freeSharedMemory() { pimpl->freeSharedMemory(); }

void Detector::loadConfig(const std::string &fname) {
    int shm_id = getShmId();
    freeSharedMemory();
    pimpl = sls::make_unique<multiSlsDetector>(shm_id);
    FILE_LOG(logINFO) << "Loading configuration file: " << fname;
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
        FILE_LOG(logDEBUG1)
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
    return pimpl->Parallel(&slsDetector::getHostname, pos);
}

void Detector::setHostname(const std::vector<std::string> &hostname) {
    pimpl->setHostname(hostname);
}

void Detector::setVirtualDetectorServers(int numServers, int startingPort) {
    pimpl->setVirtualDetectorServers(numServers, startingPort);
}

int Detector::getShmId() const { return pimpl->getMultiId(); }

std::string Detector::getPackageVersion() const {
    return pimpl->getPackageVersion();
}

int64_t Detector::getClientVersion() const {
    return pimpl->getClientSoftwareVersion();
}

Result<int64_t> Detector::getFirmwareVersion(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getFirmwareVersion, pos);
}

Result<int64_t> Detector::getDetectorServerVersion(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getDetectorServerVersion, pos);
}

Result<int64_t> Detector::getSerialNumber(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getSerialNumber, pos);
}

Result<int64_t> Detector::getReceiverVersion(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverSoftwareVersion, pos);
}

Result<defs::detectorType> Detector::getDetectorType(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getDetectorType, pos);
}

int Detector::size() const { return pimpl->size(); }

bool Detector::empty() const { return pimpl->size() == 0; }

defs::xy Detector::getModuleGeometry() const {
    return pimpl->getNumberOfDetectors();
}

Result<defs::xy> Detector::getModuleSize(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getNumberOfChannels, pos);
}

defs::xy Detector::getDetectorSize() const {
    return pimpl->getNumberOfChannels();
}

void Detector::setDetectorSize(const defs::xy value) {
    pimpl->setNumberOfChannels(value);
}

Result<defs::detectorSettings> Detector::getSettings(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getSettings, pos);
}

void Detector::setSettings(defs::detectorSettings value, Positions pos) {
    pimpl->Parallel(&slsDetector::setSettings, pos, value);
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
    return pimpl->Parallel(&slsDetector::getNumberOfFrames, pos);
}

void Detector::setNumberOfFrames(int64_t value) {
    pimpl->Parallel(&slsDetector::setNumberOfFrames, {}, value);
}

Result<int64_t> Detector::getNumberOfTriggers(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getNumberOfTriggers, pos);
}

void Detector::setNumberOfTriggers(int64_t value) {
    pimpl->Parallel(&slsDetector::setNumberOfTriggers, {}, value);
}

Result<ns> Detector::getExptime(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getExptime, pos);
}

void Detector::setExptime(ns t, Positions pos) {
    pimpl->Parallel(&slsDetector::setExptime, pos, t.count());
}

Result<ns> Detector::getPeriod(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getPeriod, pos);
}

void Detector::setPeriod(ns t, Positions pos) {
    pimpl->Parallel(&slsDetector::setPeriod, pos, t.count());
}

Result<ns> Detector::getDelayAfterTrigger(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getDelayAfterTrigger, pos);
}

void Detector::setDelayAfterTrigger(ns value, Positions pos) {
    pimpl->Parallel(&slsDetector::setDelayAfterTrigger, pos, value.count());
}

Result<int64_t> Detector::getNumberOfFramesLeft(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getNumberOfFramesLeft, pos);
}

Result<int64_t> Detector::getNumberOfTriggersLeft(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getNumberOfTriggersLeft, pos);
}

Result<ns> Detector::getDelayAfterTriggerLeft(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getDelayAfterTriggerLeft, pos);
}

Result<ns> Detector::getPeriodLeft(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getPeriodLeft, pos);
}

Result<defs::timingMode> Detector::getTimingMode(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setTimingMode, pos,
                           defs::GET_TIMING_MODE);
}

void Detector::setTimingMode(defs::timingMode value, Positions pos) {
    pimpl->Parallel(&slsDetector::setTimingMode, pos, value);
}

Result<defs::speedLevel> Detector::getSpeed(Positions pos) const {
    auto res =
        pimpl->Parallel(&slsDetector::getClockDivider, pos, defs::RUN_CLOCK);
    Result<defs::speedLevel> speedResult(res.size());
    for (unsigned int i = 0; i < res.size(); ++i) {
        speedResult[i] = static_cast<defs::speedLevel>(res[i]);
    }
    return speedResult;
}

void Detector::setSpeed(defs::speedLevel value, Positions pos) {
    pimpl->Parallel(&slsDetector::setClockDivider, pos, defs::RUN_CLOCK,
                    static_cast<int>(value));
}

Result<int> Detector::getADCPhase(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getClockPhase, pos, defs::ADC_CLOCK,
                           false);
}

void Detector::setADCPhase(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setClockPhase, pos, defs::ADC_CLOCK, value,
                    false);
}

Result<int> Detector::getMaxADCPhaseShift(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getMaxClockPhaseShift, pos,
                           defs::ADC_CLOCK);
}

Result<int> Detector::getADCPhaseInDegrees(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getClockPhase, pos, defs::ADC_CLOCK,
                           true);
}

void Detector::setADCPhaseInDegrees(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setClockPhase, pos, defs::ADC_CLOCK, value,
                    true);
}

Result<int> Detector::getClockFrequency(int clkIndex, Positions pos) {
    return pimpl->Parallel(&slsDetector::getClockFrequency, pos, clkIndex);
}

void Detector::setClockFrequency(int clkIndex, int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setClockFrequency, pos, clkIndex, value);
}

Result<int> Detector::getClockPhase(int clkIndex, Positions pos) {
    return pimpl->Parallel(&slsDetector::getClockPhase, pos, clkIndex, false);
}

void Detector::setClockPhase(int clkIndex, int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setClockPhase, pos, clkIndex, value, false);
}

Result<int> Detector::getMaxClockPhaseShift(int clkIndex, Positions pos) {
    return pimpl->Parallel(&slsDetector::getMaxClockPhaseShift, pos, clkIndex);
}

Result<int> Detector::getClockPhaseinDegrees(int clkIndex, Positions pos) {
    return pimpl->Parallel(&slsDetector::getClockPhase, pos, clkIndex, true);
}

void Detector::setClockPhaseinDegrees(int clkIndex, int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setClockPhase, pos, clkIndex, value, true);
}

Result<int> Detector::getClockDivider(int clkIndex, Positions pos) {
    return pimpl->Parallel(&slsDetector::getClockDivider, pos, clkIndex);
}

void Detector::setClockDivider(int clkIndex, int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setClockDivider, pos, clkIndex, value);
}

Result<int> Detector::getHighVoltage(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setDAC, pos, -1, defs::HIGH_VOLTAGE,
                           0);
}

void Detector::setHighVoltage(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setDAC, pos, value, defs::HIGH_VOLTAGE, 0);
}

Result<bool> Detector::getPowerChip(Positions pos) const {
    return pimpl->Parallel(&slsDetector::powerChip, pos, -1);
}

void Detector::setPowerChip(bool on, Positions pos) {
    if ((pos.empty() || pos[0] == -1) && on && pimpl->size() > 3) {
        for (int i = 0; i != pimpl->size(); ++i) {
            pimpl->Parallel(&slsDetector::powerChip, {i}, static_cast<int>(on));
            usleep(1000 * 1000);
        }
    } else {
        pimpl->Parallel(&slsDetector::powerChip, pos, static_cast<int>(on));
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
    auto res = pimpl->Parallel(&slsDetector::getADC, pos, index);
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
    return pimpl->Parallel(&slsDetector::setDAC, pos, -1, index, mV);
}

void Detector::setDAC(defs::dacIndex index, int value, bool mV, Positions pos) {
    pimpl->Parallel(&slsDetector::setDAC, pos, value, index, mV);
}

Result<int> Detector::getOnChipDAC(defs::dacIndex index, int chipIndex,
                                   Positions pos) const {
    return pimpl->Parallel(&slsDetector::getOnChipDAC, pos, index, chipIndex);
}

void Detector::setOnChipDAC(defs::dacIndex index, int chipIndex, int value,
                            Positions pos) {
    pimpl->Parallel(&slsDetector::setOnChipDAC, pos, index, chipIndex, value);
}

// Acquisition

void Detector::acquire() { pimpl->acquire(); }

void Detector::clearAcquiringFlag() { pimpl->setAcquiringFlag(0); }

void Detector::startReceiver() {
    pimpl->Parallel(&slsDetector::startReceiver, {});
}

void Detector::stopReceiver() {
    pimpl->Parallel(&slsDetector::stopReceiver, {});
}

void Detector::startDetector() {
    if (getDetectorType().squash() == defs::EIGER) {
        pimpl->Parallel(&slsDetector::prepareAcquisition, {});
    }
    pimpl->Parallel(&slsDetector::startAcquisition, {});
}

void Detector::stopDetector() {
    pimpl->Parallel(&slsDetector::stopAcquisition, {});
}

Result<defs::runStatus> Detector::getDetectorStatus(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getRunStatus, pos);
}

Result<defs::runStatus> Detector::getReceiverStatus(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverStatus, pos);
}

Result<int64_t> Detector::getFramesCaught(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getFramesCaughtByReceiver, pos);
}

Result<std::vector<uint64_t>> Detector::getNumMissingPackets(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getNumMissingPackets, pos);
}

Result<uint64_t> Detector::getStartingFrameNumber(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getStartingFrameNumber, pos);
}

void Detector::setStartingFrameNumber(uint64_t value, Positions pos) {
    pimpl->Parallel(&slsDetector::setStartingFrameNumber, pos, value);
}

void Detector::sendSoftwareTrigger(Positions pos) {
    pimpl->Parallel(&slsDetector::sendSoftwareTrigger, pos);
}

// Network Configuration (Detector<->Receiver)

Result<int> Detector::getNumberofUDPInterfaces(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getNumberofUDPInterfaces, pos);
}

void Detector::setNumberofUDPInterfaces(int n, Positions pos) {
    int previouslyClientStreaming = pimpl->enableDataStreamingToClient();
    bool previouslyReceiverStreaming = getRxZmqDataStream(pos).squash(true);
    pimpl->Parallel(&slsDetector::setNumberofUDPInterfaces, pos, n);
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
    return pimpl->Parallel(&slsDetector::getSelectedUDPInterface, pos);
}

void Detector::selectUDPInterface(int interface, Positions pos) {
    pimpl->Parallel(&slsDetector::selectUDPInterface, pos, interface);
}

Result<IpAddr> Detector::getSourceUDPIP(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getSourceUDPIP, pos);
}

void Detector::setSourceUDPIP(const IpAddr ip, Positions pos) {
    pimpl->Parallel(&slsDetector::setSourceUDPIP, pos, ip);
}

Result<IpAddr> Detector::getSourceUDPIP2(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getSourceUDPIP2, pos);
}

void Detector::setSourceUDPIP2(const IpAddr ip, Positions pos) {
    pimpl->Parallel(&slsDetector::setSourceUDPIP2, pos, ip);
}

Result<MacAddr> Detector::getSourceUDPMAC(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getSourceUDPMAC, pos);
}

void Detector::setSourceUDPMAC(const MacAddr mac, Positions pos) {
    pimpl->Parallel(&slsDetector::setSourceUDPMAC, pos, mac);
}

Result<MacAddr> Detector::getSourceUDPMAC2(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getSourceUDPMAC2, pos);
}

void Detector::setSourceUDPMAC2(const MacAddr mac, Positions pos) {
    pimpl->Parallel(&slsDetector::setSourceUDPMAC2, pos, mac);
}

Result<IpAddr> Detector::getDestinationUDPIP(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getDestinationUDPIP, pos);
}

void Detector::setDestinationUDPIP(const IpAddr ip, Positions pos) {
    pimpl->Parallel(&slsDetector::setDestinationUDPIP, pos, ip);
}

Result<IpAddr> Detector::getDestinationUDPIP2(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getDestinationUDPIP2, pos);
}

void Detector::setDestinationUDPIP2(const IpAddr ip, Positions pos) {
    pimpl->Parallel(&slsDetector::setDestinationUDPIP2, pos, ip);
}

Result<MacAddr> Detector::getDestinationUDPMAC(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getDestinationUDPMAC, pos);
}

void Detector::setDestinationUDPMAC(const MacAddr mac, Positions pos) {
    pimpl->Parallel(&slsDetector::setDestinationUDPMAC, pos, mac);
}

Result<MacAddr> Detector::getDestinationUDPMAC2(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getDestinationUDPMAC2, pos);
}

void Detector::setDestinationUDPMAC2(const MacAddr mac, Positions pos) {
    pimpl->Parallel(&slsDetector::setDestinationUDPMAC2, pos, mac);
}

Result<int> Detector::getDestinationUDPPort(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getDestinationUDPPort, pos);
}

void Detector::setDestinationUDPPort(int port, int module_id) {
    if (module_id == -1) {
        std::vector<int> port_list = getPortNumbers(port);
        for (int idet = 0; idet < size(); ++idet) {
            pimpl->Parallel(&slsDetector::setDestinationUDPPort, {idet},
                            port_list[idet]);
        }
    } else {
        pimpl->Parallel(&slsDetector::setDestinationUDPPort, {module_id}, port);
    }
}

Result<int> Detector::getDestinationUDPPort2(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getDestinationUDPPort2, pos);
}

void Detector::setDestinationUDPPort2(int port, int module_id) {
    if (module_id == -1) {
        std::vector<int> port_list = getPortNumbers(port);
        for (int idet = 0; idet < size(); ++idet) {
            pimpl->Parallel(&slsDetector::setDestinationUDPPort2, {idet},
                            port_list[idet]);
        }
    } else {
        pimpl->Parallel(&slsDetector::setDestinationUDPPort2, {module_id},
                        port);
    }
}

Result<std::string> Detector::printRxConfiguration(Positions pos) const {
    return pimpl->Parallel(&slsDetector::printReceiverConfiguration, pos);
}

Result<bool> Detector::getTenGiga(Positions pos) const {
    return pimpl->Parallel(&slsDetector::enableTenGigabitEthernet, pos, -1);
}

void Detector::setTenGiga(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::enableTenGigabitEthernet, pos,
                    static_cast<int>(value));
}

Result<bool> Detector::getTenGigaFlowControl(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getTenGigaFlowControl, pos);
}

void Detector::setTenGigaFlowControl(bool enable, Positions pos) {
    pimpl->Parallel(&slsDetector::setTenGigaFlowControl, pos, enable);
}

Result<int> Detector::getTransmissionDelayFrame(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getTransmissionDelayFrame, pos);
}

void Detector::setTransmissionDelayFrame(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setTransmissionDelayFrame, pos, value);
}

Result<int> Detector::getTransmissionDelayLeft(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getTransmissionDelayLeft, pos);
}

void Detector::setTransmissionDelayLeft(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setTransmissionDelayLeft, pos, value);
}

Result<int> Detector::getTransmissionDelayRight(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getTransmissionDelayRight, pos);
}

void Detector::setTransmissionDelayRight(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setTransmissionDelayRight, pos, value);
}

// Receiver

Result<bool> Detector::getUseReceiverFlag(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getUseReceiverFlag, pos);
}

Result<std::string> Detector::getRxHostname(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverHostname, pos);
}

void Detector::setRxHostname(const std::string &receiver, Positions pos) {
    pimpl->Parallel(&slsDetector::setReceiverHostname, pos, receiver);
}

Result<int> Detector::getRxPort(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverPort, pos);
}

void Detector::setRxPort(int port, int module_id) {
    if (module_id == -1) {
        std::vector<int> port_list(size());
        for (auto &it : port_list) {
            it = port++;
        }
        for (int idet = 0; idet < size(); ++idet) {
            pimpl->Parallel(&slsDetector::setReceiverPort, {idet},
                            port_list[idet]);
        }
    } else {
        pimpl->Parallel(&slsDetector::setReceiverPort, {module_id}, port);
    }
}

Result<int> Detector::getRxFifoDepth(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setReceiverFifoDepth, pos, -1);
}

void Detector::setRxFifoDepth(int nframes, Positions pos) {
    pimpl->Parallel(&slsDetector::setReceiverFifoDepth, pos, nframes);
}

Result<bool> Detector::getRxSilentMode(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setReceiverSilentMode, pos, -1);
}

void Detector::setRxSilentMode(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::setReceiverSilentMode, pos,
                    static_cast<int>(value));
}

Result<defs::frameDiscardPolicy>
Detector::getRxFrameDiscardPolicy(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setReceiverFramesDiscardPolicy, pos,
                           defs::GET_FRAME_DISCARD_POLICY);
}

void Detector::setRxFrameDiscardPolicy(defs::frameDiscardPolicy f,
                                       Positions pos) {
    pimpl->Parallel(&slsDetector::setReceiverFramesDiscardPolicy, pos, f);
}

Result<bool> Detector::getPartialFramesPadding(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getPartialFramesPadding, pos);
}

void Detector::setPartialFramesPadding(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::setPartialFramesPadding, pos, value);
}

Result<int64_t> Detector::getRxUDPSocketBufferSize(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverUDPSocketBufferSize, pos);
}

void Detector::setRxUDPSocketBufferSize(int64_t udpsockbufsize, Positions pos) {
    pimpl->Parallel(&slsDetector::setReceiverUDPSocketBufferSize, pos,
                    udpsockbufsize);
}

Result<int64_t> Detector::getRxRealUDPSocketBufferSize(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverRealUDPSocketBufferSize,
                           pos);
}

Result<bool> Detector::getRxLock(Positions pos) {
    return pimpl->Parallel(&slsDetector::lockReceiver, pos, -1);
}

void Detector::setRxLock(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::lockReceiver, pos, static_cast<int>(value));
}

Result<sls::IpAddr> Detector::getRxLastClientIP(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverLastClientIP, pos);
}

// File

Result<defs::fileFormat> Detector::getFileFormat(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getFileFormat, pos);
}

void Detector::setFileFormat(defs::fileFormat f, Positions pos) {
    pimpl->Parallel(&slsDetector::setFileFormat, pos, f);
}

Result<std::string> Detector::getFilePath(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getFilePath, pos);
}

void Detector::setFilePath(const std::string &fpath, Positions pos) {
    pimpl->Parallel(&slsDetector::setFilePath, pos, fpath);
}

Result<std::string> Detector::getFileNamePrefix(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getFileName, pos);
}

void Detector::setFileNamePrefix(const std::string &fname, Positions pos) {
    pimpl->Parallel(&slsDetector::setFileName, pos, fname);
}

Result<int64_t> Detector::getAcquisitionIndex(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getFileIndex, pos);
}

void Detector::setAcquisitionIndex(int64_t i, Positions pos) {
    pimpl->Parallel(&slsDetector::setFileIndex, pos, i);
}

Result<bool> Detector::getFileWrite(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getFileWrite, pos);
}

void Detector::setFileWrite(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::setFileWrite, pos, value);
}

void Detector::setMasterFileWrite(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::setMasterFileWrite, pos, value);
}

Result<bool> Detector::getMasterFileWrite(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getMasterFileWrite, pos);
}

Result<bool> Detector::getFileOverWrite(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getFileOverWrite, pos);
}

void Detector::setFileOverWrite(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::setFileOverWrite, pos, value);
}

Result<int> Detector::getFramesPerFile(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getFramesPerFile, pos);
}

void Detector::setFramesPerFile(int n, Positions pos) {
    pimpl->Parallel(&slsDetector::setFramesPerFile, pos, n);
}

// Zmq Streaming (Receiver<->Client)

Result<bool> Detector::getRxZmqDataStream(Positions pos) const {
    return pimpl->Parallel(&slsDetector::enableDataStreamingFromReceiver, pos,
                           -1);
}

void Detector::setRxZmqDataStream(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::enableDataStreamingFromReceiver, pos,
                    static_cast<int>(value));
}

Result<int> Detector::getRxZmqFrequency(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setReceiverStreamingFrequency, pos,
                           -1);
}

void Detector::setRxZmqFrequency(int freq, Positions pos) {
    pimpl->Parallel(&slsDetector::setReceiverStreamingFrequency, pos, freq);
}

Result<int> Detector::getRxZmqTimer(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setReceiverStreamingTimer, pos, -1);
}

void Detector::setRxZmqTimer(int time_in_ms, Positions pos) {
    pimpl->Parallel(&slsDetector::setReceiverStreamingTimer, pos, time_in_ms);
}

Result<int> Detector::getRxZmqPort(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverStreamingPort, pos);
}

void Detector::setRxZmqPort(int port, int module_id) {
    if (module_id == -1) {
        std::vector<int> port_list = getPortNumbers(port);
        for (int idet = 0; idet < size(); ++idet) {
            pimpl->Parallel(&slsDetector::setReceiverStreamingPort, {idet},
                            port_list[idet]);
        }
    } else {
        pimpl->Parallel(&slsDetector::setReceiverStreamingPort, {module_id},
                        port);
    }
}

Result<IpAddr> Detector::getRxZmqIP(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverStreamingIP, pos);
}

void Detector::setRxZmqIP(const IpAddr ip, Positions pos) {
    bool previouslyReceiverStreaming = getRxZmqDataStream(pos).squash(false);
    pimpl->Parallel(&slsDetector::setReceiverStreamingIP, pos, ip);
    if (previouslyReceiverStreaming) {
        setRxZmqDataStream(false, pos);
        setRxZmqDataStream(true, pos);
    }
}

Result<int> Detector::getClientZmqPort(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getClientStreamingPort, pos);
}

void Detector::setClientZmqPort(int port, int module_id) {
    if (module_id == -1) {
        std::vector<int> port_list = getPortNumbers(port);
        for (int idet = 0; idet < size(); ++idet) {
            pimpl->Parallel(&slsDetector::setClientStreamingPort, {idet},
                            port_list[idet]);
        }
    } else {
        pimpl->Parallel(&slsDetector::setClientStreamingPort, {module_id},
                        port);
    }
}

Result<IpAddr> Detector::getClientZmqIp(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getClientStreamingIP, pos);
}

void Detector::setClientZmqIp(const IpAddr ip, Positions pos) {
    int previouslyClientStreaming = pimpl->enableDataStreamingToClient(-1);
    pimpl->Parallel(&slsDetector::setClientStreamingIP, pos, ip);
    if (previouslyClientStreaming != 0) {
        pimpl->enableDataStreamingToClient(0);
        pimpl->enableDataStreamingToClient(1);
    }
}

// Eiger Specific

Result<int> Detector::getDynamicRange(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setDynamicRange, pos, -1);
}

void Detector::setDynamicRange(int value) {
    pimpl->Parallel(&slsDetector::setDynamicRange, {}, value);
}

Result<ns> Detector::getSubExptime(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getSubExptime, pos);
}

void Detector::setSubExptime(ns t, Positions pos) {
    pimpl->Parallel(&slsDetector::setSubExptime, pos, t.count());
}

Result<ns> Detector::getSubDeadTime(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getSubDeadTime, pos);
}

void Detector::setSubDeadTime(ns value, Positions pos) {
    pimpl->Parallel(&slsDetector::setSubDeadTime, pos, value.count());
}

Result<int> Detector::getThresholdEnergy(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getThresholdEnergy, pos);
}

void Detector::setThresholdEnergy(int threshold_ev,
                                  defs::detectorSettings settings,
                                  bool trimbits, Positions pos) {
    pimpl->Parallel(&slsDetector::setThresholdEnergy, pos, threshold_ev,
                    settings, static_cast<int>(trimbits));
}

Result<std::string> Detector::getSettingsPath(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getSettingsDir, pos);
}

void Detector::setSettingsPath(const std::string &value, Positions pos) {
    pimpl->Parallel(&slsDetector::setSettingsDir, pos, value);
}

void Detector::loadTrimbits(const std::string &fname, Positions pos) {
    pimpl->Parallel(&slsDetector::loadSettingsFile, pos, fname);
}

Result<bool> Detector::getRxAddGapPixels(Positions pos) const {
    return pimpl->Parallel(&slsDetector::enableGapPixels, pos, -1);
}

void Detector::setRxAddGapPixels(bool enable) {
    pimpl->setGapPixelsinReceiver(enable);
}

Result<bool> Detector::getParallelMode(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getParallelMode, pos);
}

void Detector::setParallelMode(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::setParallelMode, pos, value);
}

Result<bool> Detector::getOverFlowMode(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getOverFlowMode, pos);
}

void Detector::setOverFlowMode(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::setOverFlowMode, pos, value);
}

Result<bool> Detector::getStoreInRamMode(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getStoreInRamMode, pos);
}

void Detector::setStoreInRamMode(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::setStoreInRamMode, pos, value);
}

Result<bool> Detector::getBottom(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getFlippedDataX, pos);
}

void Detector::setBottom(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::setFlippedDataX, pos,
                    static_cast<int>(value));
}

Result<int> Detector::getAllTrimbits(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setAllTrimbits, pos, -1);
}

void Detector::setAllTrimbits(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setAllTrimbits, pos, value);
}

Result<std::vector<int>> Detector::getTrimEnergies(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getTrimEn, pos);
}

void Detector::setTrimEnergies(std::vector<int> energies, Positions pos) {
    pimpl->Parallel(&slsDetector::setTrimEn, pos, energies);
}

Result<ns> Detector::getRateCorrection(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getRateCorrection, pos);
}

void Detector::setDefaultRateCorrection(Positions pos) {
    pimpl->Parallel(&slsDetector::setDefaultRateCorrection, pos);
}

void Detector::setRateCorrection(ns dead_time, Positions pos) {
    pimpl->Parallel(&slsDetector::setRateCorrection, pos, dead_time.count());
}

Result<int> Detector::getPartialReadout(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReadNLines, pos);
}

void Detector::setPartialReadout(const int lines, Positions pos) {
    pimpl->Parallel(&slsDetector::setReadNLines, pos, lines);
}

Result<bool> Detector::getInterruptSubframe(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getInterruptSubframe, pos);
}

void Detector::setInterruptSubframe(const bool enable, Positions pos) {
    pimpl->Parallel(&slsDetector::setInterruptSubframe, pos, enable);
}

Result<ns> Detector::getMeasuredPeriod(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getMeasuredPeriod, pos);
}

Result<ns> Detector::getMeasuredSubFramePeriod(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getMeasuredSubFramePeriod, pos);
}

Result<bool> Detector::getActive(Positions pos) const {
    return pimpl->Parallel(&slsDetector::activate, pos, -1);
}

void Detector::setActive(bool active, Positions pos) {
    pimpl->Parallel(&slsDetector::activate, pos, static_cast<int>(active));
}

Result<bool> Detector::getRxPadDeactivatedMode(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setDeactivatedRxrPaddingMode, pos, -1);
}

void Detector::setRxPadDeactivatedMode(bool pad, Positions pos) {
    pimpl->Parallel(&slsDetector::setDeactivatedRxrPaddingMode, pos,
                    static_cast<int>(pad));
}

Result<bool> Detector::getPartialReset(Positions pos) const {
    auto res = pimpl->Parallel(&slsDetector::setCounterBit, pos, -1);
    Result<bool> t(res.size());
    for (unsigned int i = 0; i < res.size(); ++i) {
        t[i] = !res[i];
    }
    return t;
}

void Detector::setPartialReset(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::setCounterBit, pos, !value);
}

void Detector::pulsePixel(int n, defs::xy pixel, Positions pos) {
    pimpl->Parallel(&slsDetector::pulsePixel, pos, n, pixel.x, pixel.y);
}

void Detector::pulsePixelNMove(int n, defs::xy pixel, Positions pos) {
    pimpl->Parallel(&slsDetector::pulsePixelNMove, pos, n, pixel.x, pixel.y);
}

void Detector::pulseChip(int n, Positions pos) {
    pimpl->Parallel(&slsDetector::pulseChip, pos, n);
}

Result<bool> Detector::getQuad(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getQuad, pos);
}

void Detector::setQuad(const bool enable) {
    if (enable && size() > 1) {
        throw RuntimeError("Cannot set Quad type as it is available only for 1 "
                           "Eiger Quad Half module.");
    }
    pimpl->Parallel(&slsDetector::setQuad, {}, enable);
}

// Jungfrau Specific

Result<int> Detector::getThresholdTemperature(Positions pos) const {
    auto res = pimpl->Parallel(&slsDetector::setThresholdTemperature, pos, -1);
    for (auto &it : res) {
        it /= 1000;
    }
    return res;
}

void Detector::setThresholdTemperature(int temp, Positions pos) {
    pimpl->Parallel(&slsDetector::setThresholdTemperature, pos, temp * 1000);
}

Result<bool> Detector::getTemperatureControl(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setTemperatureControl, pos, -1);
}

void Detector::setTemperatureControl(bool enable, Positions pos) {
    pimpl->Parallel(&slsDetector::setTemperatureControl, pos,
                    static_cast<int>(enable));
}

Result<int> Detector::getTemperatureEvent(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setTemperatureEvent, pos, -1);
}

void Detector::resetTemperatureEvent(Positions pos) {
    pimpl->Parallel(&slsDetector::setTemperatureEvent, pos, 0);
}

Result<bool> Detector::getAutoCompDisable(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setAutoComparatorDisableMode, pos, -1);
}

void Detector::setAutoCompDisable(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::setAutoComparatorDisableMode, pos,
                    static_cast<int>(value));
}

Result<int> Detector::getNumberOfAdditionalStorageCells(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getNumberOfAdditionalStorageCells,
                           pos);
}

void Detector::setNumberOfAdditionalStorageCells(int value) {
    pimpl->Parallel(&slsDetector::setNumberOfAdditionalStorageCells, {}, value);
}

Result<int> Detector::getStorageCellStart(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setStoragecellStart, pos, -1);
}

void Detector::setStoragecellStart(int cell, Positions pos) {
    pimpl->Parallel(&slsDetector::setStoragecellStart, pos, cell);
}

Result<ns> Detector::getStorageCellDelay(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getStorageCellDelay, pos);
}

void Detector::setStorageCellDelay(ns value, Positions pos) {
    pimpl->Parallel(&slsDetector::setStorageCellDelay, pos, value.count());
}

// Gotthard Specific

Result<defs::ROI> Detector::getROI(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getROI, pos);
}

void Detector::setROI(defs::ROI value, int module_id) {
    if (module_id < 0 && size() > 1) {
        throw RuntimeError("Cannot set ROI for all modules simultaneously");
    }
    pimpl->Parallel(&slsDetector::setROI, {module_id}, value);
}

void Detector::clearROI(Positions pos) {
    pimpl->Parallel(&slsDetector::clearROI, pos);
}

Result<ns> Detector::getExptimeLeft(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getExptimeLeft, pos);
}

Result<defs::externalSignalFlag>
Detector::getExternalSignalFlags(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setExternalSignalFlags, pos,
                           defs::GET_EXTERNAL_SIGNAL_FLAG);
}

void Detector::setExternalSignalFlags(defs::externalSignalFlag value,
                                      Positions pos) {
    pimpl->Parallel(&slsDetector::setExternalSignalFlags, pos, value);
}

Result<int> Detector::getImageTestMode(Positions pos) {
    return pimpl->Parallel(&slsDetector::getImageTestMode, pos);
}

void Detector::setImageTestMode(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setImageTestMode, pos, value);
}

// Gotthard2 Specific

Result<std::array<int, 2>> Detector::getInjectChannel(Positions pos) {
    return pimpl->Parallel(&slsDetector::getInjectChannel, pos);
}

void Detector::setInjectChannel(const int offsetChannel, const int incrementChannel, Positions pos) {
    pimpl->Parallel(&slsDetector::setInjectChannel, pos, offsetChannel, incrementChannel);
}

Result<std::vector<int>> Detector::getVetoPhoton(const int chipIndex, Positions pos) {
    return pimpl->Parallel(&slsDetector::getVetoPhoton, pos, chipIndex);
}

void Detector::setVetoPhoton(const int chipIndex, const int numPhotons, const int energy, const std::string& fname,  Positions pos) {
    pimpl->Parallel(&slsDetector::setVetoPhoton, pos, chipIndex, numPhotons, energy, fname);
}    

void Detector::setVetoReference(const int gainIndex, const int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setVetoReference, pos, gainIndex, value);
} 

void Detector::setBurstMode(bool enable, Positions pos) {
    pimpl->Parallel(&slsDetector::setBurstMode, pos, enable);
}

Result<bool> Detector::getBurstMode(Positions pos) {
    return pimpl->Parallel(&slsDetector::getBurstMode, pos);
}    

// Mythen3 Specific

Result<uint32_t> Detector::getCounterMask(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getCounterMask, pos);
}

void Detector::setCounterMask(uint32_t countermask, Positions pos) {
    pimpl->Parallel(&slsDetector::setCounterMask, pos, countermask);
}

// CTB Specific

Result<int> Detector::getNumberOfAnalogSamples(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getNumberOfAnalogSamples, pos);
}

void Detector::setNumberOfAnalogSamples(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setNumberOfAnalogSamples, pos, value);
}

Result<int> Detector::getNumberOfDigitalSamples(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getNumberOfDigitalSamples, pos);
}

void Detector::setNumberOfDigitalSamples(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setNumberOfDigitalSamples, pos, value);
}

Result<defs::readoutMode> Detector::getReadoutMode(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReadoutMode, pos);
}

void Detector::setReadoutMode(defs::readoutMode value, Positions pos) {
    pimpl->Parallel(&slsDetector::setReadoutMode, pos, value);
}

Result<int> Detector::getDBITPhase(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getClockPhase, pos, defs::DBIT_CLOCK,
                           false);
}

void Detector::setDBITPhase(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setClockPhase, pos, defs::DBIT_CLOCK, value,
                    false);
}

Result<int> Detector::getMaxDBITPhaseShift(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getMaxClockPhaseShift, pos,
                           defs::DBIT_CLOCK);
}

Result<int> Detector::getDBITPhaseInDegrees(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getClockPhase, pos, defs::DBIT_CLOCK,
                           true);
}

void Detector::setDBITPhaseInDegrees(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setClockPhase, pos, defs::DBIT_CLOCK, value,
                    true);
}

Result<int> Detector::getADCClock(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getClockFrequency, pos,
                           defs::ADC_CLOCK);
}

void Detector::setADCClock(int value_in_MHz, Positions pos) {
    pimpl->Parallel(&slsDetector::setClockFrequency, pos, defs::ADC_CLOCK,
                    value_in_MHz);
}

Result<int> Detector::getDBITClock(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getClockFrequency, pos,
                           defs::DBIT_CLOCK);
}

void Detector::setDBITClock(int value_in_MHz, Positions pos) {
    pimpl->Parallel(&slsDetector::setClockFrequency, pos, defs::DBIT_CLOCK,
                    value_in_MHz);
}

Result<int> Detector::getRUNClock(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getClockFrequency, pos,
                           defs::RUN_CLOCK);
}

void Detector::setRUNClock(int value_in_MHz, Positions pos) {
    pimpl->Parallel(&slsDetector::setClockFrequency, pos, defs::RUN_CLOCK,
                    value_in_MHz);
}

Result<int> Detector::getSYNCClock(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getClockFrequency, pos,
                           defs::SYNC_CLOCK);
}

Result<int> Detector::getADCPipeline(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getPipeline, pos, defs::ADC_CLOCK);
}

void Detector::setADCPipeline(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setPipeline, pos, defs::ADC_CLOCK, value);
}

Result<int> Detector::getDBITPipeline(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getPipeline, pos, defs::DBIT_CLOCK);
}

void Detector::setDBITPipeline(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setPipeline, pos, defs::DBIT_CLOCK, value);
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
    return pimpl->Parallel(&slsDetector::setDAC, pos, -1, index, 1);
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
    pimpl->Parallel(&slsDetector::setDAC, pos, value, index, 1);
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
    return pimpl->Parallel(&slsDetector::getADC, pos, index);
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
    return pimpl->Parallel(&slsDetector::getADC, pos, index);
}

Result<int> Detector::getSlowADC(defs::dacIndex index, Positions pos) const {
    if (index < defs::SLOW_ADC0 || index > defs::SLOW_ADC7) {
        throw RuntimeError("Unknown Slow ADC Index");
    }
    return pimpl->Parallel(&slsDetector::getADC, pos, index);
}

Result<uint32_t> Detector::getADCEnableMask(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getADCEnableMask, pos);
}

void Detector::setADCEnableMask(uint32_t mask, Positions pos) {
    pimpl->Parallel(&slsDetector::setADCEnableMask, pos, mask);
}

Result<uint32_t> Detector::getTenGigaADCEnableMask(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getTenGigaADCEnableMask, pos);
}

void Detector::setTenGigaADCEnableMask(uint32_t mask, Positions pos) {
    pimpl->Parallel(&slsDetector::setTenGigaADCEnableMask, pos, mask);
}

Result<uint32_t> Detector::getADCInvert(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getADCInvert, pos);
}

void Detector::setADCInvert(uint32_t value, Positions pos) {
    pimpl->Parallel(&slsDetector::setADCInvert, pos, value);
}

Result<int> Detector::getExternalSamplingSource(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getExternalSamplingSource, pos);
}

void Detector::setExternalSamplingSource(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setExternalSamplingSource, pos, value);
}

Result<int> Detector::getExternalSampling(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getExternalSampling, pos);
}

void Detector::setExternalSampling(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::setExternalSampling, pos, value);
}

Result<std::vector<int>> Detector::getRxDbitList(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverDbitList, pos);
}

void Detector::setRxDbitList(const std::vector<int>& list, Positions pos) {
    pimpl->Parallel(&slsDetector::setReceiverDbitList, pos, list);
}

Result<int> Detector::getRxDbitOffset(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverDbitOffset, pos);
}

void Detector::setRxDbitOffset(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setReceiverDbitOffset, pos, value);
}

void Detector::setDigitalIODelay(uint64_t pinMask, int delay, Positions pos) {
    pimpl->Parallel(&slsDetector::setDigitalIODelay, pos, pinMask, delay);
}

Result<bool> Detector::getLEDEnable(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setLEDEnable, pos, -1);
}

void Detector::setLEDEnable(bool enable, Positions pos) {
    pimpl->Parallel(&slsDetector::setLEDEnable, pos, static_cast<int>(enable));
}

// Pattern

void Detector::savePattern(const std::string &fname) {
    std::ofstream outfile;
    outfile.open(fname.c_str(), std::ios_base::out);
    if (!outfile.is_open()) {
        throw RuntimeError("Could not create file to save pattern");
    }
    // get pattern limits
    auto r = pimpl->Parallel(&slsDetector::setPatternLoopAddresses, {}, -1, -1, -1)
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
    pimpl->Parallel(&slsDetector::setPattern, pos, fname);
}

Result<uint64_t> Detector::getPatternIOControl(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setPatternIOControl, pos, -1);
}

void Detector::setPatternIOControl(uint64_t word, Positions pos) {
    pimpl->Parallel(&slsDetector::setPatternIOControl, pos, word);
}

Result<uint64_t> Detector::getPatternClockControl(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setPatternClockControl, pos, -1);
}

void Detector::setPatternClockControl(uint64_t word, Positions pos) {
    pimpl->Parallel(&slsDetector::setPatternClockControl, pos, word);
}

Result<uint64_t> Detector::getPatternWord(int addr, Positions pos) {
    return pimpl->Parallel(&slsDetector::setPatternWord, pos, addr, -1);
}

void Detector::setPatternWord(int addr, uint64_t word, Positions pos) {
    pimpl->Parallel(&slsDetector::setPatternWord, pos, addr, word);
}

Result<std::array<int, 2>>
Detector::getPatternLoopAddresses(int level, Positions pos) const {
    return pimpl->Parallel(&slsDetector::setPatternLoopAddresses, pos, level,
                           -1, -1);
}

void Detector::setPatternLoopAddresses(int level, int start, int stop,
                                       Positions pos) {
    pimpl->Parallel(&slsDetector::setPatternLoopAddresses, pos, level, start,
                    stop);
}

Result<int> Detector::getPatternLoopCycles(int level, Positions pos) const {
    return pimpl->Parallel(&slsDetector::setPatternLoopCycles, pos, level, -1);
}

void Detector::setPatternLoopCycles(int level, int n, Positions pos) {
    pimpl->Parallel(&slsDetector::setPatternLoopCycles, pos, level, n);
}

Result<int> Detector::getPatternWaitAddr(int level, Positions pos) const {
    return pimpl->Parallel(&slsDetector::setPatternWaitAddr, pos, level, -1);
}

void Detector::setPatternWaitAddr(int level, int addr, Positions pos) {
    pimpl->Parallel(&slsDetector::setPatternWaitAddr, pos, level, addr);
}

Result<uint64_t> Detector::getPatternWaitTime(int level, Positions pos) const {
    return pimpl->Parallel(&slsDetector::setPatternWaitTime, pos, level, -1);
}

void Detector::setPatternWaitTime(int level, uint64_t t, Positions pos) {
    pimpl->Parallel(&slsDetector::setPatternWaitTime, pos, level, t);
}

Result<uint64_t> Detector::getPatternMask(Positions pos) {
    return pimpl->Parallel(&slsDetector::getPatternMask, pos);
}

void Detector::setPatternMask(uint64_t mask, Positions pos) {
    pimpl->Parallel(&slsDetector::setPatternMask, pos, mask);
}

Result<uint64_t> Detector::getPatternBitMask(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getPatternBitMask, pos);
}

void Detector::setPatternBitMask(uint64_t mask, Positions pos) {
    pimpl->Parallel(&slsDetector::setPatternBitMask, pos, mask);
}

// Moench

Result<std::string> Detector::getAdditionalJsonHeader(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getAdditionalJsonHeader, pos);
}

void Detector::setAdditionalJsonHeader(const std::string &jsonheader,
                                       Positions pos) {
    pimpl->Parallel(&slsDetector::setAdditionalJsonHeader, pos, jsonheader);
}

Result<std::string> Detector::getAdditionalJsonParameter(const std::string &key,
                                                         Positions pos) const {
    return pimpl->Parallel(&slsDetector::getAdditionalJsonParameter, pos, key);
}

void Detector::setAdditionalJsonParameter(const std::string &key,
                                          const std::string &value,
                                          Positions pos) {
    pimpl->Parallel(&slsDetector::setAdditionalJsonParameter, pos, key, value);
}

Result<int> Detector::getDetectorMinMaxEnergyThreshold(const bool isEmax,
                                                       Positions pos) const {
    auto res = pimpl->Parallel(&slsDetector::getAdditionalJsonParameter, pos,
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
    pimpl->Parallel(&slsDetector::setAdditionalJsonParameter, pos,
                    isEmax ? "emax" : "emin", std::to_string(value));
}

Result<defs::frameModeType> Detector::getFrameMode(Positions pos) const {
    auto res = pimpl->Parallel(&slsDetector::getAdditionalJsonParameter, pos,
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
    pimpl->Parallel(&slsDetector::setAdditionalJsonParameter, pos, "frameMode",
                    sls::ToString(value));
}

Result<defs::detectorModeType> Detector::getDetectorMode(Positions pos) const {
    auto res = pimpl->Parallel(&slsDetector::getAdditionalJsonParameter, pos,
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
    pimpl->Parallel(&slsDetector::setAdditionalJsonParameter, pos,
                    "detectorMode", sls::ToString(value));
}

// Advanced

void Detector::programFPGA(const std::string &fname, Positions pos) {
    FILE_LOG(logINFO)
        << "Updating Firmware. This can take awhile. Please be patient...";
    std::vector<char> buffer = pimpl->readPofFile(fname);
    pimpl->Parallel(&slsDetector::programFPGA, pos, buffer);
}

void Detector::resetFPGA(Positions pos) {
    pimpl->Parallel(&slsDetector::resetFPGA, pos);
}

void Detector::copyDetectorServer(const std::string &fname,
                                  const std::string &hostname, Positions pos) {
    pimpl->Parallel(&slsDetector::copyDetectorServer, pos, fname, hostname);
    rebootController(pos);
}

void Detector::rebootController(Positions pos) {
    pimpl->Parallel(&slsDetector::rebootController, pos);
}

void Detector::updateFirmwareAndServer(const std::string &sname,
                                       const std::string &hostname,
                                       const std::string &fname,
                                       Positions pos) {
    pimpl->Parallel(&slsDetector::copyDetectorServer, pos, sname, hostname);
    programFPGA(fname, pos);
    rebootController(pos);
}

Result<uint32_t> Detector::readRegister(uint32_t addr, Positions pos) const {
    return pimpl->Parallel(&slsDetector::readRegister, pos, addr);
}

void Detector::writeRegister(uint32_t addr, uint32_t val, Positions pos) {
    pimpl->Parallel(&slsDetector::writeRegister, pos, addr, val);
}

void Detector::setBit(uint32_t addr, int bitnr, Positions pos) {
    pimpl->Parallel(&slsDetector::setBit, pos, addr, bitnr);
}

void Detector::clearBit(uint32_t addr, int bitnr, Positions pos) {
    pimpl->Parallel(&slsDetector::clearBit, pos, addr, bitnr);
}

void Detector::executeFirmwareTest(Positions pos) {
    pimpl->Parallel(&slsDetector::executeFirmwareTest, pos);
}

void Detector::executeBusTest(Positions pos) {
    pimpl->Parallel(&slsDetector::executeBusTest, pos);
}

void Detector::writeAdcRegister(uint32_t addr, uint32_t value, Positions pos) {
    pimpl->Parallel(&slsDetector::writeAdcRegister, pos, addr, value);
}

// Insignificant

Result<int> Detector::getControlPort(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getControlPort, pos);
}

void Detector::setControlPort(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setControlPort, pos, value);
}

Result<int> Detector::getStopPort(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getStopPort, pos);
}

void Detector::setStopPort(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setStopPort, pos, value);
}

Result<bool> Detector::getDetectorLock(Positions pos) const {
    return pimpl->Parallel(&slsDetector::lockServer, pos, -1);
}

void Detector::setDetectorLock(bool lock, Positions pos) {
    pimpl->Parallel(&slsDetector::lockServer, pos, static_cast<int>(lock));
}

Result<sls::IpAddr> Detector::getLastClientIP(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getLastClientIP, pos);
}

void Detector::executeCommand(const std::string &value, Positions pos) {
    pimpl->Parallel(&slsDetector::execCommand, pos, value);
}

Result<int64_t> Detector::getNumberOfFramesFromStart(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getNumberOfFramesFromStart, pos);
}

Result<ns> Detector::getActualTime(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getActualTime, pos);
}

Result<ns> Detector::getMeasurementTime(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getMeasurementTime, pos);
}

std::string Detector::getUserDetails() const { return pimpl->getUserDetails(); }

Result<uint64_t> Detector::getRxCurrentFrameIndex(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverCurrentFrameIndex, pos);
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