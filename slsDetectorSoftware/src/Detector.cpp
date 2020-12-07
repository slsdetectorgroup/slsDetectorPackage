#include "sls/Detector.h"
#include "sls/detectorData.h"

#include "CmdParser.h"
#include "CmdProxy.h"
#include "DetectorImpl.h"
#include "Module.h"
#include "sls/Pattern.h"
#include "sls/container_utils.h"
#include "sls/logger.h"
#include "sls/sls_detector_defs.h"
#include "sls/versionAPI.h"

#include <chrono>
#include <fstream>
#include <thread>

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
    std::ifstream input_file(fname);
    if (!input_file) {
        throw RuntimeError("Could not open configuration file " + fname +
                           " for reading");
    }
    std::vector<std::string> parameters;
    for (std::string line; std::getline(input_file, line);) {
        if (line.find('#') != std::string::npos) {
            line.erase(line.find('#'));
        }
        if (line.length() > 1) {
            parameters.push_back(line);
        }
    }
    loadParameters(parameters);
}

void Detector::loadParameters(const std::vector<std::string> &parameters) {
    CmdProxy proxy(this);
    CmdParser parser;
    for (const auto &current_line : parameters) {
        parser.Parse(current_line);
        proxy.Call(parser.command(), parser.arguments(), parser.detector_id(),
                   defs::PUT_ACTION);
    }
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

std::string Detector::getPackageVersion() const { return GITBRANCH; }

int64_t Detector::getClientVersion() const { return APILIB; }

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

std::vector<defs::detectorSettings> Detector::getSettingsList() const {
    switch (getDetectorType().squash()) {
    case defs::EIGER:
        return std::vector<defs::detectorSettings>{
            defs::STANDARD, defs::HIGHGAIN, defs::LOWGAIN, defs::VERYHIGHGAIN,
            defs::VERYLOWGAIN};
    case defs::GOTTHARD:
        return std::vector<defs::detectorSettings>{
            defs::HIGHGAIN, defs::DYNAMICGAIN, defs::LOWGAIN, defs::MEDIUMGAIN,
            defs::VERYHIGHGAIN};
    case defs::JUNGFRAU:
        return std::vector<defs::detectorSettings>{
            defs::DYNAMICGAIN, defs::DYNAMICHG0,    defs::FIXGAIN1,
            defs::FIXGAIN2,    defs::FORCESWITCHG1, defs::FORCESWITCHG2};
    case defs::GOTTHARD2:
        return std::vector<defs::detectorSettings>{
            defs::DYNAMICGAIN, defs::DYNAMICHG0, defs::FIXGAIN1,
            defs::FIXGAIN2};
    case defs::MOENCH:
        return std::vector<defs::detectorSettings>{
            defs::G1_HIGHGAIN,         defs::G1_LOWGAIN,
            defs::G2_HIGHCAP_HIGHGAIN, defs::G2_HIGHCAP_LOWGAIN,
            defs::G2_LOWCAP_HIGHGAIN,  defs::G2_LOWCAP_LOWGAIN,
            defs::G4_HIGHGAIN,         defs::G4_LOWGAIN};
    case defs::CHIPTESTBOARD:
    case defs::MYTHEN3:
        throw RuntimeError("Settings not implemented for this detector");
    default:
        throw RuntimeError("Unknown detector type");
    }
}

Result<defs::detectorSettings> Detector::getSettings(Positions pos) const {
    return pimpl->Parallel(&Module::getSettings, pos);
}

void Detector::setSettings(const defs::detectorSettings value, Positions pos) {
    pimpl->Parallel(&Module::setSettings, pos, value);
}

void Detector::loadTrimbits(const std::string &fname, Positions pos) {
    pimpl->Parallel(&Module::loadSettingsFile, pos, fname);
}

Result<int> Detector::getAllTrimbits(Positions pos) const {
    return pimpl->Parallel(&Module::getAllTrimbits, pos);
}

void Detector::setAllTrimbits(int value, Positions pos) {
    pimpl->Parallel(&Module::setAllTrimbits, pos, value);
}

bool Detector::getGapPixelsinCallback() const {
    return pimpl->getGapPixelsinCallback();
}

void Detector::setGapPixelsinCallback(bool enable) {
    pimpl->setGapPixelsinCallback(enable);
}

Result<bool> Detector::isVirtualDetectorServer(Positions pos) const {
    return pimpl->Parallel(&Module::isVirtualDetectorServer, pos);
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
    return pimpl->Parallel(&Module::getExptime, pos, -1);
}

void Detector::setExptime(ns t, Positions pos) {
    pimpl->Parallel(&Module::setExptime, pos, -1, t.count());
    updateRxRateCorrections();
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

Result<int> Detector::getDynamicRange(Positions pos) const {
    return pimpl->Parallel(&Module::getDynamicRange, pos);
}

void Detector::setDynamicRange(int value) {
    pimpl->Parallel(&Module::setDynamicRange, {}, value);
    updateRxRateCorrections();
}

std::vector<int> Detector::getDynamicRangeList() const {
    switch (getDetectorType().squash()) {
    case defs::EIGER:
        return std::vector<int>{4, 8, 16, 32};
    case defs::MYTHEN3:
        return std::vector<int>{8, 16, 32};
    default:
        return std::vector<int>{16};
    }
}

Result<defs::timingMode> Detector::getTimingMode(Positions pos) const {
    return pimpl->Parallel(&Module::getTimingMode, pos);
}

void Detector::setTimingMode(defs::timingMode value, Positions pos) {
    pimpl->Parallel(&Module::setTimingMode, pos, value);
}

std::vector<defs::timingMode> Detector::getTimingModeList() const {
    switch (getDetectorType().squash()) {
    case defs::EIGER:
        return std::vector<defs::timingMode>{defs::AUTO_TIMING,
                                             defs::TRIGGER_EXPOSURE,
                                             defs::GATED, defs::BURST_TRIGGER};
    case defs::MYTHEN3:
        return std::vector<defs::timingMode>{defs::AUTO_TIMING,
                                             defs::TRIGGER_EXPOSURE,
                                             defs::GATED, defs::TRIGGER_GATED};
    default:
        return std::vector<defs::timingMode>{defs::AUTO_TIMING,
                                             defs::TRIGGER_EXPOSURE};
    }
}

Result<defs::speedLevel> Detector::getSpeed(Positions pos) const {
    auto res = pimpl->Parallel(&Module::getClockDivider, pos, defs::RUN_CLOCK);
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
    return pimpl->Parallel(&Module::getClockPhase, pos, defs::ADC_CLOCK, false);
}

void Detector::setADCPhase(int value, Positions pos) {
    pimpl->Parallel(&Module::setClockPhase, pos, defs::ADC_CLOCK, value, false);
}

Result<int> Detector::getMaxADCPhaseShift(Positions pos) const {
    return pimpl->Parallel(&Module::getMaxClockPhaseShift, pos,
                           defs::ADC_CLOCK);
}

Result<int> Detector::getADCPhaseInDegrees(Positions pos) const {
    return pimpl->Parallel(&Module::getClockPhase, pos, defs::ADC_CLOCK, true);
}

void Detector::setADCPhaseInDegrees(int value, Positions pos) {
    pimpl->Parallel(&Module::setClockPhase, pos, defs::ADC_CLOCK, value, true);
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
    return pimpl->Parallel(&Module::getClockPhase, pos, defs::DBIT_CLOCK, true);
}

void Detector::setDBITPhaseInDegrees(int value, Positions pos) {
    pimpl->Parallel(&Module::setClockPhase, pos, defs::DBIT_CLOCK, value, true);
}

Result<int> Detector::getClockFrequency(int clkIndex, Positions pos) {
    return pimpl->Parallel(&Module::getClockFrequency, pos, clkIndex);
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
    return pimpl->Parallel(&Module::getDAC, pos, defs::HIGH_VOLTAGE, false);
}

void Detector::setHighVoltage(int value, Positions pos) {
    pimpl->Parallel(&Module::setDAC, pos, value, defs::HIGH_VOLTAGE, false);
}

Result<bool> Detector::getPowerChip(Positions pos) const {
    return pimpl->Parallel(&Module::getPowerChip, pos);
}

void Detector::setPowerChip(bool on, Positions pos) {
    if ((pos.empty() || pos[0] == -1) && on && pimpl->size() > 3) {
        for (int i = 0; i != pimpl->size(); ++i) {
            pimpl->Parallel(&Module::setPowerChip, {i}, on);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } else {
        pimpl->Parallel(&Module::setPowerChip, pos, on);
    }
}

Result<int> Detector::getImageTestMode(Positions pos) {
    return pimpl->Parallel(&Module::getImageTestMode, pos);
}

void Detector::setImageTestMode(int value, Positions pos) {
    pimpl->Parallel(&Module::setImageTestMode, pos, value);
}

std::vector<defs::dacIndex> Detector::getTemperatureList() const {
    std::vector<defs::dacIndex> retval;
    switch (getDetectorType().squash()) {
    case defs::CHIPTESTBOARD:
        return std::vector<defs::dacIndex>{defs::SLOW_ADC_TEMP};
    case defs::JUNGFRAU:
    case defs::GOTTHARD:
        return std::vector<defs::dacIndex>{defs::TEMPERATURE_ADC,
                                           defs::TEMPERATURE_FPGA};
    case defs::EIGER:
        return std::vector<defs::dacIndex>{
            defs::TEMPERATURE_FPGA,  defs::TEMPERATURE_FPGAEXT,
            defs::TEMPERATURE_10GE,  defs::TEMPERATURE_DCDC,
            defs::TEMPERATURE_SODL,  defs::TEMPERATURE_SODR,
            defs::TEMPERATURE_FPGA2, defs::TEMPERATURE_FPGA3};
    default:
        return std::vector<defs::dacIndex>{};
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

std::vector<defs::dacIndex> Detector::getDacList() const {
    std::vector<defs::dacIndex> retval;
    switch (getDetectorType().squash()) {
    case defs::EIGER:
        return std::vector<defs::dacIndex>{
            defs::VSVP,      defs::VTRIM,   defs::VRPREAMP, defs::VRSHAPER,
            defs::VSVN,      defs::VTGSTV,  defs::VCMP_LL,  defs::VCMP_LR,
            defs::VCAL,      defs::VCMP_RL, defs::RXB_RB,   defs::RXB_LB,
            defs::VCMP_RR,   defs::VCP,     defs::VCN,      defs::VISHAPER,
            defs::VTHRESHOLD};
    case defs::GOTTHARD:
        return std::vector<defs::dacIndex>{
            defs::VREF_DS,   defs::VCASCN_PB, defs::VCASCP_PB, defs::VOUT_CM,
            defs::VCASC_OUT, defs::VIN_CM,    defs::VREF_COMP, defs::IB_TESTC};
    case defs::JUNGFRAU:
        return std::vector<defs::dacIndex>{
            defs::VB_COMP,   defs::VDD_PROT, defs::VIN_COM, defs::VREF_PRECH,
            defs::VB_PIXBUF, defs::VB_DS,    defs::VREF_DS, defs::VREF_COMP};
    case defs::GOTTHARD2:
        return std::vector<defs::dacIndex>{
            defs::VREF_H_ADC,   defs::VB_COMP_FE,  defs::VB_COMP_ADC,
            defs::VCOM_CDS,     defs::VREF_RSTORE, defs::VB_OPA_1ST,
            defs::VREF_COMP_FE, defs::VCOM_ADC1,   defs::VREF_PRECH,
            defs::VREF_L_ADC,   defs::VREF_CDS,    defs::VB_CS,
            defs::VB_OPA_FD,    defs::VCOM_ADC2};
    case defs::MYTHEN3:
        return std::vector<defs::dacIndex>{
            defs::VCASSH,    defs::VTH2,     defs::VRSHAPER, defs::VRSHAPER_N,
            defs::VIPRE_OUT, defs::VTH3,     defs::VTH1,     defs::VICIN,
            defs::VCAS,      defs::VRPREAMP, defs::VCAL_N,   defs::VIPRE,
            defs::VISHAPER,  defs::VCAL_P,   defs::VTRIM,    defs::VDCSH,
            defs::VTHRESHOLD};
    case defs::MOENCH:
        return std::vector<defs::dacIndex>{
            defs::VBP_COLBUF, defs::VIPRE,   defs::VIN_CM,    defs::VB_SDA,
            defs::VCASC_SFP,  defs::VOUT_CM, defs::VIPRE_CDS, defs::IBIAS_SFP};
    case defs::CHIPTESTBOARD:
        for (int i = 0; i != 18; ++i) {
            retval.push_back(static_cast<defs::dacIndex>(i));
        }
        break;
    default:
        throw RuntimeError("Unknown detector type");
    }
    return retval;
}

void Detector::setDefaultDacs(Positions pos) {
    pimpl->Parallel(&Module::setDefaultDacs, pos);
}

Result<int> Detector::getDAC(defs::dacIndex index, bool mV,
                             Positions pos) const {
    return pimpl->Parallel(&Module::getDAC, pos, index, mV);
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

Result<defs::externalSignalFlag>
Detector::getExternalSignalFlags(int signalIndex, Positions pos) const {
    return pimpl->Parallel(&Module::getExternalSignalFlags, pos, signalIndex);
}

void Detector::setExternalSignalFlags(int signalIndex,
                                      defs::externalSignalFlag value,
                                      Positions pos) {
    pimpl->Parallel(&Module::setExternalSignalFlags, pos, signalIndex, value);
}

Result<bool> Detector::getParallelMode(Positions pos) const {
    return pimpl->Parallel(&Module::getParallelMode, pos);
}

void Detector::setParallelMode(bool value, Positions pos) {
    pimpl->Parallel(&Module::setParallelMode, pos, value);
}

// Acquisition

void Detector::acquire() { pimpl->acquire(); }

void Detector::clearAcquiringFlag() { pimpl->setAcquiringFlag(0); }

void Detector::startReceiver() { pimpl->Parallel(&Module::startReceiver, {}); }

void Detector::stopReceiver() { pimpl->Parallel(&Module::stopReceiver, {}); }

void Detector::startDetector() {
    pimpl->Parallel(&Module::startAcquisition, {});
}

void Detector::startDetectorReadout() {
    pimpl->Parallel(&Module::startReadout, {});
}

void Detector::stopDetector() { pimpl->Parallel(&Module::stopAcquisition, {}); }

Result<defs::runStatus> Detector::getDetectorStatus(Positions pos) const {
    return pimpl->Parallel(&Module::getRunStatus, pos);
}

Result<defs::runStatus> Detector::getReceiverStatus(Positions pos) const {
    return pimpl->Parallel(&Module::getReceiverStatus, pos);
}

Result<int64_t> Detector::getFramesCaught(Positions pos) const {
    return pimpl->Parallel(&Module::getFramesCaughtByReceiver, pos);
}

Result<std::vector<uint64_t>>
Detector::getNumMissingPackets(Positions pos) const {
    return pimpl->Parallel(&Module::getNumMissingPackets, pos);
}

Result<uint64_t> Detector::getNextFrameNumber(Positions pos) const {
    return pimpl->Parallel(&Module::getNextFrameNumber, pos);
}

void Detector::setNextFrameNumber(uint64_t value, Positions pos) {
    pimpl->Parallel(&Module::setNextFrameNumber, pos, value);
}

void Detector::sendSoftwareTrigger(Positions pos) {
    pimpl->Parallel(&Module::sendSoftwareTrigger, pos);
}

Result<defs::scanParameters> Detector::getScan(Positions pos) const {
    return pimpl->Parallel(&Module::getScan, pos);
}

void Detector::setScan(const defs::scanParameters t) {
    pimpl->Parallel(&Module::setScan, {}, t);
}

Result<std::string> Detector::getScanErrorMessage(Positions pos) const {
    return pimpl->Parallel(&Module::getScanErrorMessage, pos);
}

// Network Configuration (Detector<->Receiver)

Result<int> Detector::getNumberofUDPInterfaces(Positions pos) const {
    return pimpl->Parallel(&Module::getNumberofUDPInterfaces, pos);
}

void Detector::setNumberofUDPInterfaces(int n, Positions pos) {
    bool previouslyClientStreaming = pimpl->getDataStreamingToClient();
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
    if (previouslyClientStreaming) {
        pimpl->setDataStreamingToClient(false);
        pimpl->setDataStreamingToClient(true);
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
        pimpl->Parallel(&Module::setDestinationUDPPort2, {module_id}, port);
    }
}

void Detector::reconfigureUDPDestination(Positions pos) {
    pimpl->Parallel(&Module::reconfigureUDPDestination, pos);
}

void Detector::validateUDPConfiguration(Positions pos) {
    pimpl->Parallel(&Module::validateUDPConfiguration, pos);
}

Result<std::string> Detector::printRxConfiguration(Positions pos) const {
    return pimpl->Parallel(&Module::printReceiverConfiguration, pos);
}

Result<bool> Detector::getTenGiga(Positions pos) const {
    return pimpl->Parallel(&Module::getTenGiga, pos);
}

void Detector::setTenGiga(bool value, Positions pos) {
    pimpl->Parallel(&Module::setTenGiga, pos, value);
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
    updateRxRateCorrections();
}

void Detector::setRxHostname(const std::vector<std::string> &name) {
    // set all to same rx_hostname
    if (name.size() == 1) {
        pimpl->Parallel(&Module::setReceiverHostname, {}, name[0]);
    } else {
        if ((int)name.size() != size()) {
            throw RuntimeError(
                "Receiver hostnames size " + std::to_string(name.size()) +
                " does not match detector size " + std::to_string(size()));
        }
        // set each rx_hostname
        for (int idet = 0; idet < size(); ++idet) {
            pimpl->Parallel(&Module::setReceiverHostname, {idet}, name[idet]);
        }
    }
    updateRxRateCorrections();
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
            pimpl->Parallel(&Module::setReceiverPort, {idet}, port_list[idet]);
        }
    } else {
        pimpl->Parallel(&Module::setReceiverPort, {module_id}, port);
    }
}

Result<int> Detector::getRxFifoDepth(Positions pos) const {
    return pimpl->Parallel(&Module::getReceiverFifoDepth, pos);
}

void Detector::setRxFifoDepth(int nframes, Positions pos) {
    pimpl->Parallel(&Module::setReceiverFifoDepth, pos, nframes);
}

Result<bool> Detector::getRxSilentMode(Positions pos) const {
    return pimpl->Parallel(&Module::getReceiverSilentMode, pos);
}

void Detector::setRxSilentMode(bool value, Positions pos) {
    pimpl->Parallel(&Module::setReceiverSilentMode, pos, value);
}

Result<defs::frameDiscardPolicy>
Detector::getRxFrameDiscardPolicy(Positions pos) const {
    return pimpl->Parallel(&Module::getReceiverFramesDiscardPolicy, pos);
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

Result<int> Detector::getRxUDPSocketBufferSize(Positions pos) const {
    return pimpl->Parallel(&Module::getReceiverUDPSocketBufferSize, pos);
}

void Detector::setRxUDPSocketBufferSize(int udpsockbufsize, Positions pos) {
    pimpl->Parallel(&Module::setReceiverUDPSocketBufferSize, pos,
                    udpsockbufsize);
}

Result<int> Detector::getRxRealUDPSocketBufferSize(Positions pos) const {
    return pimpl->Parallel(&Module::getReceiverRealUDPSocketBufferSize, pos);
}

Result<bool> Detector::getRxLock(Positions pos) {
    return pimpl->Parallel(&Module::getReceiverLock, pos);
}

void Detector::setRxLock(bool value, Positions pos) {
    pimpl->Parallel(&Module::setReceiverLock, pos, value);
}

Result<sls::IpAddr> Detector::getRxLastClientIP(Positions pos) const {
    return pimpl->Parallel(&Module::getReceiverLastClientIP, pos);
}

Result<std::array<pid_t, NUM_RX_THREAD_IDS>>
Detector::getRxThreadIds(Positions pos) const {
    return pimpl->Parallel(&Module::getReceiverThreadIds, pos);
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

void Detector::setMasterFileWrite(bool value) {
    pimpl->Parallel(&Module::setMasterFileWrite, {0}, value);
}

bool Detector::getMasterFileWrite() const {
    return pimpl->Parallel(&Module::getMasterFileWrite, {0})[0];
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
    return pimpl->Parallel(&Module::getReceiverStreaming, pos);
}

void Detector::setRxZmqDataStream(bool value, Positions pos) {
    pimpl->Parallel(&Module::setReceiverStreaming, pos, value);
}

Result<int> Detector::getRxZmqFrequency(Positions pos) const {
    return pimpl->Parallel(&Module::getReceiverStreamingFrequency, pos);
}

void Detector::setRxZmqFrequency(int freq, Positions pos) {
    pimpl->Parallel(&Module::setReceiverStreamingFrequency, pos, freq);
}

Result<int> Detector::getRxZmqTimer(Positions pos) const {
    return pimpl->Parallel(&Module::getReceiverStreamingTimer, pos);
}

void Detector::setRxZmqTimer(int time_in_ms, Positions pos) {
    pimpl->Parallel(&Module::setReceiverStreamingTimer, pos, time_in_ms);
}

Result<int> Detector::getRxZmqStartingFrame(Positions pos) const {
    return pimpl->Parallel(&Module::getReceiverStreamingStartingFrame, pos);
}

void Detector::setRxZmqStartingFrame(int fnum, Positions pos) {
    pimpl->Parallel(&Module::setReceiverStreamingStartingFrame, pos, fnum);
}

Result<int> Detector::getRxZmqPort(Positions pos) const {
    return pimpl->Parallel(&Module::getReceiverStreamingPort, pos);
}

void Detector::setRxZmqPort(int port, int module_id) {
    bool previouslyReceiverStreaming =
        getRxZmqDataStream(std::vector<int>{module_id}).squash(false);
    if (module_id == -1) {
        std::vector<int> port_list = getPortNumbers(port);
        for (int idet = 0; idet < size(); ++idet) {
            pimpl->Parallel(&Module::setReceiverStreamingPort, {idet},
                            port_list[idet]);
        }
    } else {
        pimpl->Parallel(&Module::setReceiverStreamingPort, {module_id}, port);
    }
    if (previouslyReceiverStreaming) {
        setRxZmqDataStream(false, std::vector<int>{module_id});
        setRxZmqDataStream(true, std::vector<int>{module_id});
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
    bool previouslyClientStreaming = pimpl->getDataStreamingToClient();
    if (module_id == -1) {
        std::vector<int> port_list = getPortNumbers(port);
        for (int idet = 0; idet < size(); ++idet) {
            pimpl->Parallel(&Module::setClientStreamingPort, {idet},
                            port_list[idet]);
        }
    } else {
        pimpl->Parallel(&Module::setClientStreamingPort, {module_id}, port);
    }
    if (previouslyClientStreaming) {
        pimpl->setDataStreamingToClient(false);
        pimpl->setDataStreamingToClient(true);
    }
}

Result<IpAddr> Detector::getClientZmqIp(Positions pos) const {
    return pimpl->Parallel(&Module::getClientStreamingIP, pos);
}

void Detector::setClientZmqIp(const IpAddr ip, Positions pos) {
    bool previouslyClientStreaming = pimpl->getDataStreamingToClient();
    pimpl->Parallel(&Module::setClientStreamingIP, pos, ip);
    if (previouslyClientStreaming) {
        pimpl->setDataStreamingToClient(false);
        pimpl->setDataStreamingToClient(true);
    }
}

int Detector::getClientZmqHwm() const { return pimpl->getClientStreamingHwm(); }

void Detector::setClientZmqHwm(const int limit) {
    pimpl->setClientStreamingHwm(limit);
}

Result<int> Detector::getRxZmqHwm(Positions pos) const {
    return pimpl->Parallel(&Module::getReceiverStreamingHwm, pos);
}

void Detector::setRxZmqHwm(const int limit) {
    bool previouslyReceiverStreaming = getRxZmqDataStream().squash(false);
    pimpl->Parallel(&Module::setReceiverStreamingHwm, {}, limit);
    if (previouslyReceiverStreaming) {
        setRxZmqDataStream(false, {});
        setRxZmqDataStream(true, {});
    }
}

// Eiger Specific

Result<ns> Detector::getSubExptime(Positions pos) const {
    return pimpl->Parallel(&Module::getSubExptime, pos);
}

void Detector::setSubExptime(ns t, Positions pos) {
    pimpl->Parallel(&Module::setSubExptime, pos, t.count());
    updateRxRateCorrections();
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
    pimpl->Parallel(&Module::setThresholdEnergy, pos, threshold_ev, settings,
                    static_cast<int>(trimbits));
}

Result<std::string> Detector::getSettingsPath(Positions pos) const {
    return pimpl->Parallel(&Module::getSettingsDir, pos);
}

void Detector::setSettingsPath(const std::string &value, Positions pos) {
    pimpl->Parallel(&Module::setSettingsDir, pos, value);
}

Result<bool> Detector::getOverFlowMode(Positions pos) const {
    return pimpl->Parallel(&Module::getOverFlowMode, pos);
}

void Detector::setOverFlowMode(bool value, Positions pos) {
    pimpl->Parallel(&Module::setOverFlowMode, pos, value);
}

Result<bool> Detector::getBottom(Positions pos) const {
    return pimpl->Parallel(&Module::getFlippedDataX, pos);
}

void Detector::setBottom(bool value, Positions pos) {
    pimpl->Parallel(&Module::setFlippedDataX, pos, value);
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
    updateRxRateCorrections();
}

void Detector::setRateCorrection(ns dead_time, Positions pos) {
    pimpl->Parallel(&Module::setRateCorrection, pos, dead_time.count());
    updateRxRateCorrections();
}

void Detector::updateRxRateCorrections() {
    // get tau from all modules and send to Rx index 0
    if (getDetectorType().squash() == defs::EIGER) {
        if (getUseReceiverFlag().squash(false)) {
            std::vector<int64_t> dead_times;
            for (auto item : getRateCorrection())
                dead_times.push_back(item.count());
            pimpl->Parallel(&Module::sendReceiverRateCorrections, {0},
                            dead_times);
        }
    }
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
    return pimpl->Parallel(&Module::getActivate, pos);
}

void Detector::setActive(const bool active, Positions pos) {
    pimpl->Parallel(&Module::setActivate, pos, active);
}

Result<bool> Detector::getRxPadDeactivatedMode(Positions pos) const {
    return pimpl->Parallel(&Module::getDeactivatedRxrPaddingMode, pos);
}

void Detector::setRxPadDeactivatedMode(bool pad, Positions pos) {
    pimpl->Parallel(&Module::setDeactivatedRxrPaddingMode, pos, pad);
}

Result<bool> Detector::getPartialReset(Positions pos) const {
    return pimpl->Parallel(&Module::getCounterBit, pos);
}

void Detector::setPartialReset(bool value, Positions pos) {
    pimpl->Parallel(&Module::setCounterBit, pos, value);
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
    return pimpl->Parallel(&Module::getThresholdTemperature, pos);
}

void Detector::setThresholdTemperature(int temp, Positions pos) {
    pimpl->Parallel(&Module::setThresholdTemperature, pos, temp);
}

Result<bool> Detector::getTemperatureControl(Positions pos) const {
    return pimpl->Parallel(&Module::getTemperatureControl, pos);
}

void Detector::setTemperatureControl(bool enable, Positions pos) {
    pimpl->Parallel(&Module::setTemperatureControl, pos, enable);
}

Result<int> Detector::getTemperatureEvent(Positions pos) const {
    return pimpl->Parallel(&Module::getTemperatureEvent, pos);
}

void Detector::resetTemperatureEvent(Positions pos) {
    pimpl->Parallel(&Module::resetTemperatureEvent, pos);
}

Result<bool> Detector::getAutoCompDisable(Positions pos) const {
    return pimpl->Parallel(&Module::getAutoComparatorDisableMode, pos);
}

void Detector::setAutoCompDisable(bool value, Positions pos) {
    pimpl->Parallel(&Module::setAutoComparatorDisableMode, pos, value);
}

Result<int> Detector::getNumberOfAdditionalStorageCells(Positions pos) const {
    return pimpl->Parallel(&Module::getNumberOfAdditionalStorageCells, pos);
}

void Detector::setNumberOfAdditionalStorageCells(int value) {
    pimpl->Parallel(&Module::setNumberOfAdditionalStorageCells, {}, value);
}

Result<int> Detector::getStorageCellStart(Positions pos) const {
    return pimpl->Parallel(&Module::getStorageCellStart, pos);
}

void Detector::setStorageCellStart(int cell, Positions pos) {
    pimpl->Parallel(&Module::setStorageCellStart, pos, cell);
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

Result<int64_t> Detector::getNumberOfBurstsLeft(Positions pos) const {
    return pimpl->Parallel(&Module::getNumberOfBurstsLeft, pos);
}

Result<std::array<int, 2>> Detector::getInjectChannel(Positions pos) {
    return pimpl->Parallel(&Module::getInjectChannel, pos);
}

void Detector::setInjectChannel(const int offsetChannel,
                                const int incrementChannel, Positions pos) {
    pimpl->Parallel(&Module::setInjectChannel, pos, offsetChannel,
                    incrementChannel);
}

void Detector::getVetoPhoton(const int chipIndex, const std::string &fname,
                             Positions pos) {
    pimpl->Parallel(&Module::getVetoPhoton, pos, chipIndex, fname);
}

void Detector::setVetoPhoton(const int chipIndex, const int numPhotons,
                             const int energy, const std::string &fname,
                             Positions pos) {
    pimpl->Parallel(&Module::setVetoPhoton, pos, chipIndex, numPhotons, energy,
                    fname);
}

void Detector::setVetoReference(const int gainIndex, const int value,
                                Positions pos) {
    pimpl->Parallel(&Module::setVetoReference, pos, gainIndex, value);
}

void Detector::setVetoFile(const int chipIndex, const std::string &fname,
                           Positions pos) {
    pimpl->Parallel(&Module::setVetoFile, pos, chipIndex, fname);
}

Result<defs::burstMode> Detector::getBurstMode(Positions pos) {
    return pimpl->Parallel(&Module::getBurstMode, pos);
}

void Detector::setBurstMode(defs::burstMode value, Positions pos) {
    pimpl->Parallel(&Module::setBurstMode, pos, value);
}

Result<bool> Detector::getCDSGain(Positions pos) const {
    return pimpl->Parallel(&Module::getCDSGain, pos);
}

void Detector::setCDSGain(bool value, Positions pos) {
    pimpl->Parallel(&Module::setCDSGain, pos, value);
}

Result<int> Detector::getFilter(Positions pos) const {
    return pimpl->Parallel(&Module::getFilter, pos);
}

void Detector::setFilter(int value, Positions pos) {
    pimpl->Parallel(&Module::setFilter, pos, value);
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

Result<bool> Detector::getVeto(Positions pos) const {
    return pimpl->Parallel(&Module::getVeto, pos);
}

void Detector::setVeto(bool enable, Positions pos) {
    pimpl->Parallel(&Module::setVeto, pos, enable);
}

Result<int> Detector::getADCConfiguration(const int chipIndex,
                                          const int adcIndex,
                                          Positions pos) const {
    return pimpl->Parallel(&Module::getADCConfiguration, pos, chipIndex,
                           adcIndex);
}

void Detector::setADCConfiguration(const int chipIndex, const int adcIndex,
                                   const int value, Positions pos) {
    pimpl->Parallel(&Module::setADCConfiguration, pos, chipIndex, adcIndex,
                    value);
}

void Detector::getBadChannels(const std::string &fname, Positions pos) const {
    pimpl->Parallel(&Module::getBadChannels, pos, fname);
}

void Detector::setBadChannels(const std::string &fname, Positions pos) {
    pimpl->Parallel(&Module::setBadChannels, pos, fname);
}

// Mythen3 Specific

Result<uint32_t> Detector::getCounterMask(Positions pos) const {
    return pimpl->Parallel(&Module::getCounterMask, pos);
}

void Detector::setCounterMask(uint32_t countermask, Positions pos) {
    pimpl->Parallel(&Module::setCounterMask, pos, countermask);
}

Result<int> Detector::getNumberOfGates(Positions pos) const {
    return pimpl->Parallel(&Module::getNumberOfGates, pos);
}

void Detector::setNumberOfGates(int value, Positions pos) {
    pimpl->Parallel(&Module::setNumberOfGates, pos, value);
}

Result<ns> Detector::getExptime(int gateIndex, Positions pos) const {
    return pimpl->Parallel(&Module::getExptime, pos, gateIndex);
}

void Detector::setExptime(int gateIndex, ns t, Positions pos) {
    pimpl->Parallel(&Module::setExptime, pos, gateIndex, t.count());
}

Result<std::array<ns, 3>> Detector::getExptimeForAllGates(Positions pos) const {
    return pimpl->Parallel(&Module::getExptimeForAllGates, pos);
}

Result<ns> Detector::getGateDelay(int gateIndex, Positions pos) const {
    return pimpl->Parallel(&Module::getGateDelay, pos, gateIndex);
}

void Detector::setGateDelay(int gateIndex, ns t, Positions pos) {
    pimpl->Parallel(&Module::setGateDelay, pos, gateIndex, t.count());
}

Result<std::array<ns, 3>>
Detector::getGateDelayForAllGates(Positions pos) const {
    return pimpl->Parallel(&Module::getGateDelayForAllGates, pos);
}

// CTB/ Moench Specific

Result<int> Detector::getNumberOfAnalogSamples(Positions pos) const {
    return pimpl->Parallel(&Module::getNumberOfAnalogSamples, pos);
}

void Detector::setNumberOfAnalogSamples(int value, Positions pos) {
    pimpl->Parallel(&Module::setNumberOfAnalogSamples, pos, value);
}

Result<int> Detector::getADCClock(Positions pos) const {
    return pimpl->Parallel(&Module::getClockFrequency, pos, defs::ADC_CLOCK);
}

void Detector::setADCClock(int value_in_MHz, Positions pos) {
    pimpl->Parallel(&Module::setClockFrequency, pos, defs::ADC_CLOCK,
                    value_in_MHz);
}

Result<int> Detector::getRUNClock(Positions pos) const {
    return pimpl->Parallel(&Module::getClockFrequency, pos, defs::RUN_CLOCK);
}

void Detector::setRUNClock(int value_in_MHz, Positions pos) {
    pimpl->Parallel(&Module::setClockFrequency, pos, defs::RUN_CLOCK,
                    value_in_MHz);
}

Result<int> Detector::getSYNCClock(Positions pos) const {
    return pimpl->Parallel(&Module::getClockFrequency, pos, defs::SYNC_CLOCK);
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
    return pimpl->Parallel(&Module::getDAC, pos, index, true);
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
    pimpl->Parallel(&Module::setDAC, pos, value, index, true);
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

/** Options: ANALOG_ONLY, DIGITAL_ONLY, ANALOG_AND_DIGITAL \n
 * Default: ANALOG_ONLY */
void Detector::setReadoutMode(defs::readoutMode value, Positions pos) {
    pimpl->Parallel(&Module::setReadoutMode, pos, value);
}

Result<int> Detector::getDBITClock(Positions pos) const {
    return pimpl->Parallel(&Module::getClockFrequency, pos, defs::DBIT_CLOCK);
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

Result<bool> Detector::getExternalSampling(Positions pos) const {
    return pimpl->Parallel(&Module::getExternalSampling, pos);
}

void Detector::setExternalSampling(bool value, Positions pos) {
    pimpl->Parallel(&Module::setExternalSampling, pos, value);
}

Result<std::vector<int>> Detector::getRxDbitList(Positions pos) const {
    return pimpl->Parallel(&Module::getReceiverDbitList, pos);
}

void Detector::setRxDbitList(const std::vector<int> &list, Positions pos) {
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
    return pimpl->Parallel(&Module::getLEDEnable, pos);
}

void Detector::setLEDEnable(bool enable, Positions pos) {
    pimpl->Parallel(&Module::setLEDEnable, pos, enable);
}

// Pattern

void Detector::setPattern(const std::string &fname, Positions pos) {
    Pattern pat;
    pat.load(fname);
    pat.validate();
    setPattern(pat, pos);
}

void Detector::setPattern(const Pattern &pat, Positions pos) {
    pat.validate();
    pimpl->Parallel(&Module::setPattern, pos, pat);
}

void Detector::savePattern(const std::string &fname) {
    auto t = pimpl->Parallel(&Module::getPattern, {});
    auto pat = t.tsquash("Inconsistent pattern parameters between modules");
    pat.validate();
    pat.save(fname);
}

void Detector::loadDefaultPattern(Positions pos) {
    pimpl->Parallel(&Module::loadDefaultPattern, pos);
}

Result<uint64_t> Detector::getPatternIOControl(Positions pos) const {
    return pimpl->Parallel(&Module::getPatternIOControl, pos);
}

void Detector::setPatternIOControl(uint64_t word, Positions pos) {
    pimpl->Parallel(&Module::setPatternIOControl, pos, word);
}

Result<uint64_t> Detector::getPatternWord(int addr, Positions pos) {
    return pimpl->Parallel(&Module::getPatternWord, pos, addr);
}

void Detector::setPatternWord(int addr, uint64_t word, Positions pos) {
    pimpl->Parallel(&Module::setPatternWord, pos, addr, word);
}

Result<std::array<int, 2>>
Detector::getPatternLoopAddresses(int level, Positions pos) const {
    return pimpl->Parallel(&Module::getPatternLoopAddresses, pos, level);
}

void Detector::setPatternLoopAddresses(int level, int start, int stop,
                                       Positions pos) {
    pimpl->Parallel(&Module::setPatternLoopAddresses, pos, level, start, stop);
}

Result<int> Detector::getPatternLoopCycles(int level, Positions pos) const {
    return pimpl->Parallel(&Module::getPatternLoopCycles, pos, level);
}

void Detector::setPatternLoopCycles(int level, int n, Positions pos) {
    pimpl->Parallel(&Module::setPatternLoopCycles, pos, level, n);
}

Result<int> Detector::getPatternWaitAddr(int level, Positions pos) const {
    return pimpl->Parallel(&Module::getPatternWaitAddr, pos, level);
}

void Detector::setPatternWaitAddr(int level, int addr, Positions pos) {
    pimpl->Parallel(&Module::setPatternWaitAddr, pos, level, addr);
}

Result<uint64_t> Detector::getPatternWaitTime(int level, Positions pos) const {
    return pimpl->Parallel(&Module::getPatternWaitTime, pos, level);
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

void Detector::startPattern(Positions pos) {
    pimpl->Parallel(&Module::startPattern, pos);
}

// Moench

Result<std::map<std::string, std::string>>
Detector::getAdditionalJsonHeader(Positions pos) const {
    return pimpl->Parallel(&Module::getAdditionalJsonHeader, pos);
}

void Detector::setAdditionalJsonHeader(
    const std::map<std::string, std::string> &jsonHeader, Positions pos) {
    pimpl->Parallel(&Module::setAdditionalJsonHeader, pos, jsonHeader);
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

Result<int> Detector::getBit(uint32_t addr, int bitnr, Positions pos) {
    return pimpl->Parallel(&Module::getBit, pos, addr, bitnr);
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

bool Detector::getInitialChecks() const { return pimpl->getInitialChecks(); }

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
    return pimpl->Parallel(&Module::getLockDetector, pos);
}

void Detector::setDetectorLock(bool lock, Positions pos) {
    pimpl->Parallel(&Module::setLockDetector, pos, lock);
}

Result<sls::IpAddr> Detector::getLastClientIP(Positions pos) const {
    return pimpl->Parallel(&Module::getLastClientIP, pos);
}

Result<std::string> Detector::executeCommand(const std::string &value,
                                             Positions pos) {
    return pimpl->Parallel(&Module::execCommand, pos, value);
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