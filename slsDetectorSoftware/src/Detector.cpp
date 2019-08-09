#include "Detector.h"
#include "container_utils.h"
#include "logger.h"
#include "multiSlsDetector.h"
#include "slsDetector.h"
#include "sls_detector_defs.h"
namespace sls {

using defs = slsDetectorDefs;

Detector::Detector(int multi_id)
    : pimpl(sls::make_unique<multiSlsDetector>(multi_id)) {}
Detector::~Detector() = default;

// Acquisition
void Detector::acquire() { pimpl->acquire(); }

void Detector::startReceiver(Positions pos) {
    pimpl->Parallel(&slsDetector::startReceiver, pos);
}
void Detector::stopReceiver(Positions pos) {
    pimpl->Parallel(&slsDetector::stopReceiver, pos);
}

Result<defs::runStatus> Detector::getReceiverStatus(Positions pos) {
    return pimpl->Parallel(&slsDetector::getReceiverStatus, pos);
}

bool Detector::getAcquiringFlag() const { return pimpl->getAcquiringFlag(); }

void Detector::setAcquiringFlag(bool value) { pimpl->setAcquiringFlag(value); }

// Configuration
Result<std::string> Detector::getHostname(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getHostname, pos);
}

void Detector::freeSharedMemory() { pimpl->freeSharedMemory(); }

void Detector::setConfig(const std::string &fname) {
    pimpl->readConfigurationFile(fname);
}

void Detector::clearBit(uint32_t addr, int bitnr, Positions pos) {
    pimpl->Parallel(&slsDetector::clearBit, pos, addr, bitnr);
}
void Detector::setBit(uint32_t addr, int bitnr, Positions pos) {
    pimpl->Parallel(&slsDetector::setBit, pos, addr, bitnr);
}
Result<uint32_t> Detector::getRegister(uint32_t addr, Positions pos) {
    return pimpl->Parallel(&slsDetector::readRegister, pos, addr);
}

Result<uint64_t> Detector::getStartingFrameNumber(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getStartingFrameNumber, pos);
}
void Detector::setStartingFrameNumber(uint64_t value, Positions pos) {
    pimpl->Parallel(&slsDetector::setStartingFrameNumber, pos, value);
}

// File
void Detector::setFileName(const std::string &fname, Positions pos) {
    pimpl->Parallel(&slsDetector::setFileName, pos, fname);
}
Result<std::string> Detector::getFileName(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setFileName, pos, "");
}

void Detector::setFilePath(const std::string &fpath, Positions pos) {
    pimpl->Parallel(&slsDetector::setFilePath, pos, fpath);
}
Result<std::string> Detector::getFilePath(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getFilePath, pos);
}

void Detector::setFileWrite(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::setFileWrite, pos, value);
}

Result<bool> Detector::getFileWrite(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getFileWrite, pos);
}

void Detector::setFileOverWrite(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::setFileOverWrite, pos, value);
}

Result<bool> Detector::getFileOverWrite(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getFileOverWrite, pos);
}

// dhanya
int Detector::getMultiId() const { return pimpl->getMultiId(); }

void Detector::checkDetectorVersionCompatibility(Positions pos) const {
    pimpl->Parallel(&slsDetector::checkDetectorVersionCompatibility, pos);
}

void Detector::checkReceiverVersionCompatibility(Positions pos) const {
    pimpl->Parallel(&slsDetector::checkReceiverVersionCompatibility, pos);
}

Result<int64_t> Detector::getDetectorFirmwareVersion(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getId, pos,
                           defs::DETECTOR_FIRMWARE_VERSION);
}

Result<int64_t> Detector::getDetectorServerVersion(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getId, pos,
                           defs::DETECTOR_SOFTWARE_VERSION);
}

Result<int64_t> Detector::getDetectorSerialNumber(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getId, pos,
                           defs::DETECTOR_SERIAL_NUMBER);
}

int64_t Detector::getClientSoftwareVersion() const {
    return pimpl->getClientSoftwareVersion();
}

Result<int64_t> Detector::getReceiverSoftwareVersion(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverSoftwareVersion, pos);
}

std::string Detector::getUserDetails() const { return pimpl->getUserDetails(); }

void Detector::setHostname(const std::vector<std::string> &value) {
    pimpl->setHostname(value);
}

defs::detectorType Detector::getDetectorType() const {
    return pimpl->getDetectorTypeAsEnum();
}

Result<defs::detectorType>
Detector::getDetectorTypeAsEnum(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getDetectorTypeAsEnum, pos);
}

Result<std::string> Detector::getDetectorTypeAsString(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getDetectorTypeAsString, pos);
}

int Detector::size() const { return pimpl->size(); }

defs::coordinates Detector::getNumberOfDetectors() const {
    defs::coordinates coord;
    coord.x = pimpl->getNumberOfDetectors(defs::X);
    coord.y = pimpl->getNumberOfDetectors(defs::Y);
    return coord;
}

Result<defs::coordinates> Detector::getNumberOfChannels(Positions pos) const {
    if (pos.empty() || (pos.size() == 1 && pos[0] == -1)) {
        return {pimpl->getNumberOfChannels()};
    }
    return pimpl->Parallel(&slsDetector::getNumberOfChannels, pos);
}

Result<defs::coordinates>
Detector::getNumberOfChannelsInclGapPixels(Positions pos) const {
    if (pos.empty() || (pos.size() == 1 && pos[0] == -1)) {
        return {pimpl->getTotalNumberOfChannelsInclGapPixels()};
    }
    return pimpl->Parallel(&slsDetector::getNumberOfChannelsInclGapPixels, pos);
}

defs::coordinates Detector::getMaxNumberOfChannels() const {
    return pimpl->getMaxNumberOfChannels();
}

void Detector::setMaxNumberOfChannels(const defs::coordinates value) {
    pimpl->setMaxNumberOfChannels(value);
}

Result<defs::coordinates> Detector::getDetectorOffsets(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getDetectorOffsets, pos);
}

void Detector::setDetectorOffsets(defs::coordinates value, Positions pos) {
    pimpl->Parallel(&slsDetector::setDetectorOffsets, pos, value);
    // pimpl->Parallel<defs::coordinates>(&slsDetector::setDetectorOffset, pos,
    // value);
}

Result<bool> Detector::getQuad(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getQuad, pos);
}

void Detector::setQuad(const bool value, Positions pos) {
    pimpl->setQuad(value, 0);
}

Result<int> Detector::getReadNLines(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReadNLines, pos);
}

void Detector::setReadNLines(const int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setReadNLines, pos, value);
}

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

Result<int> Detector::getReceiverPort(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverPort, pos);
}

void Detector::setReceiverPort(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setReceiverPort, pos, value);
}

Result<bool> Detector::getLockServer(Positions pos) const {
    return pimpl->Parallel(&slsDetector::lockServer, pos, -1);
}

void Detector::setLockServer(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::lockServer, pos, static_cast<int>(value));
}

Result<std::string> Detector::getLastClientIP(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getLastClientIP, pos);
}

void Detector::exitServer(Positions pos) {
    pimpl->Parallel(&slsDetector::exitServer, pos);
}

void Detector::execCommand(const std::string &value, Positions pos) {
    pimpl->Parallel(&slsDetector::execCommand, pos, value);
}

void Detector::writeConfigurationFile(const std::string &value) {
    pimpl->writeConfigurationFile(value);
}

Result<defs::detectorSettings> Detector::getSettings(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getSettings, pos);
}

void Detector::setSettings(defs::detectorSettings value, Positions pos) {
    pimpl->Parallel(&slsDetector::setSettings, pos, value);
}

Result<int> Detector::getThresholdEnergy(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getThresholdEnergy, pos);
}

void Detector::setThresholdEnergy(int value, defs::detectorSettings sett,
                                  int tb, Positions pos) {
    pimpl->Parallel(&slsDetector::setThresholdEnergy, pos, value, sett, tb);
}

Result<std::string> Detector::getSettingsDir(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getSettingsDir, pos);
}

void Detector::setSettingsDir(const std::string &value, Positions pos) {
    pimpl->Parallel(&slsDetector::setSettingsDir, pos, value);
}

void Detector::loadSettingsFile(const std::string &value, Positions pos) {
    pimpl->Parallel(&slsDetector::loadSettingsFile, pos, value);
}

void Detector::saveSettingsFile(const std::string &value, Positions pos) {
    pimpl->Parallel(&slsDetector::saveSettingsFile, pos, value);
}

Result<defs::runStatus> Detector::getRunStatus(Positions pos) {
    return pimpl->Parallel(&slsDetector::getRunStatus, pos);
}

void Detector::prepareAcquisition() {
    pimpl->Parallel(&slsDetector::prepareAcquisition, {});
}

void Detector::startAcquisition() {
    if (getDetectorType() == defs::EIGER) {
        prepareAcquisition();
    }
    pimpl->Parallel(&slsDetector::startAcquisition, {});
}

void Detector::stopAcquisition() { pimpl->stopAcquisition(); }

void Detector::sendSoftwareTrigger(Positions pos) {
    pimpl->Parallel(&slsDetector::sendSoftwareTrigger, pos);
}

void Detector::startAndReadAll() {
    if (getDetectorType() == defs::EIGER) {
        prepareAcquisition();
    }
    pimpl->Parallel(&slsDetector::startAndReadAll, {});
}

void Detector::startReadOut() {
    pimpl->Parallel(&slsDetector::startReadOut, {});
}

void Detector::readAll() { pimpl->Parallel(&slsDetector::readAll, {}); }

void Detector::configureMAC(Positions pos) {
    pimpl->Parallel(&slsDetector::configureMAC, pos);
}

Result<int64_t> Detector::getNumberOfFrames() const {
    return pimpl->Parallel(&slsDetector::setTimer, {}, defs::FRAME_NUMBER, -1);
}

void Detector::setNumberOfFrames(int64_t value) {
    pimpl->Parallel(&slsDetector::setTimer, {}, defs::FRAME_NUMBER, value);
}

Result<int64_t> Detector::getNumberOfCycles() const {
    return pimpl->Parallel(&slsDetector::setTimer, {}, defs::CYCLES_NUMBER, -1);
}

void Detector::setNumberOfCycles(int64_t value) {
    pimpl->Parallel(&slsDetector::setTimer, {}, defs::CYCLES_NUMBER, value);
}

Result<int64_t> Detector::getNumberOfStorageCells() const {
    return pimpl->Parallel(&slsDetector::setTimer, {},
                           defs::STORAGE_CELL_NUMBER, -1);
}

void Detector::setNumberOfStorageCells(int64_t value) {
    pimpl->Parallel(&slsDetector::setTimer, {}, defs::STORAGE_CELL_NUMBER,
                    value);
}

Result<int64_t> Detector::getNumberOfAnalogSamples(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setTimer, pos, defs::ANALOG_SAMPLES,
                           -1);
}

void Detector::setNumberOfAnalogSamples(int64_t value, Positions pos) {
    pimpl->Parallel(&slsDetector::setTimer, pos, defs::ANALOG_SAMPLES, value);
}

Result<int64_t> Detector::getNumberOfDigitalSamples(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setTimer, pos, defs::DIGITAL_SAMPLES,
                           -1);
}

void Detector::setNumberOfDigitalSamples(int64_t value, Positions pos) {
    pimpl->Parallel(&slsDetector::setTimer, pos, defs::DIGITAL_SAMPLES, value);
}

Result<ns> Detector::getExptime(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setTimer, pos, defs::ACQUISITION_TIME,
                           -1);
}

void Detector::setExptime(ns t, Positions pos) {
    pimpl->Parallel(&slsDetector::setTimer, pos, defs::ACQUISITION_TIME,
                    t.count());
}

Result<ns> Detector::getPeriod(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setTimer, pos, defs::FRAME_PERIOD, -1);
}

void Detector::setPeriod(ns t, Positions pos) {
    pimpl->Parallel(&slsDetector::setTimer, pos, defs::FRAME_PERIOD, t.count());
}

Result<ns> Detector::getDelayAfterTrigger(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setTimer, pos,
                           defs::DELAY_AFTER_TRIGGER, -1);
}

void Detector::setDelayAfterTrigger(ns value, Positions pos) {
    pimpl->Parallel(&slsDetector::setTimer, pos, defs::DELAY_AFTER_TRIGGER,
                    value.count());
}

Result<ns> Detector::getSubExptime(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setTimer, pos,
                           defs::SUBFRAME_ACQUISITION_TIME, -1);
}

void Detector::setSubExptime(ns t, Positions pos) {
    pimpl->Parallel(&slsDetector::setTimer, pos,
                    defs::SUBFRAME_ACQUISITION_TIME, t.count());
}

Result<ns> Detector::getSubDeadTime(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setTimer, pos, defs::SUBFRAME_DEADTIME,
                           -1);
}

void Detector::setSubDeadTime(ns value, Positions pos) {
    pimpl->Parallel(&slsDetector::setTimer, pos, defs::SUBFRAME_DEADTIME,
                    value.count());
}

Result<ns> Detector::getStorageCellDelay(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setTimer, pos,
                           defs::STORAGE_CELL_DELAY, -1);
}

void Detector::setStorageCellDelay(ns value, Positions pos) {
    pimpl->Parallel(&slsDetector::setTimer, pos, defs::STORAGE_CELL_DELAY,
                    value.count());
}

Result<int64_t> Detector::getNumberOfFramesLeft(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getTimeLeft, pos, defs::FRAME_NUMBER);
}

Result<int64_t> Detector::getNumberOfCyclesLeft(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getTimeLeft, pos, defs::CYCLES_NUMBER);
}

Result<ns> Detector::getExptimeLeft(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getTimeLeft, pos,
                           defs::ACQUISITION_TIME);
}

Result<ns> Detector::getPeriodLeft(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getTimeLeft, pos, defs::FRAME_PERIOD);
}

Result<ns> Detector::getDelayAfterTriggerLeft(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getTimeLeft, pos,
                           defs::DELAY_AFTER_TRIGGER);
}

Result<int64_t> Detector::getNumberOfFramesFromStart(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getTimeLeft, pos,
                           defs::FRAMES_FROM_START);
}

Result<ns> Detector::getActualTime(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getTimeLeft, pos, defs::ACTUAL_TIME);
}

Result<ns> Detector::getMeasurementTime(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getTimeLeft, pos,
                           defs::MEASUREMENT_TIME);
}

Result<ns> Detector::getMeasuredPeriod(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getTimeLeft, pos,
                           defs::MEASURED_PERIOD);
}

Result<ns> Detector::getMeasuredSubFramePeriod(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getTimeLeft, pos,
                           defs::MEASURED_SUBPERIOD);
}

Result<int> Detector::getSpeed(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setSpeed, pos, defs::CLOCK_DIVIDER, -1,
                           0);
}

void Detector::setSpeed(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setSpeed, pos, defs::CLOCK_DIVIDER, value, 0);
}

Result<int> Detector::getADCPhase(bool inDeg, Positions pos) const {
    return pimpl->Parallel(&slsDetector::setSpeed, pos, defs::ADC_PHASE, -1,
                           inDeg);
}

void Detector::setADCPhase(int value, bool inDeg, Positions pos) {
    pimpl->Parallel(&slsDetector::setSpeed, pos, defs::ADC_PHASE, value, inDeg);
}

Result<int> Detector::getMaxADCPhaseShift(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setSpeed, pos,
                           defs::MAX_ADC_PHASE_SHIFT, -1, 0);
}

Result<int> Detector::getDBITPhase(bool inDeg, Positions pos) const {
    return pimpl->Parallel(&slsDetector::setSpeed, pos, defs::DBIT_PHASE, -1,
                           inDeg);
}

void Detector::setDBITPhase(int value, bool inDeg, Positions pos) {
    pimpl->Parallel(&slsDetector::setSpeed, pos, defs::DBIT_PHASE, value,
                    inDeg);
}

Result<int> Detector::getMaxDBITPhaseShift(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setSpeed, pos,
                           defs::MAX_DBIT_PHASE_SHIFT, -1, 0);
}

Result<int> Detector::getADCClock(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setSpeed, pos, defs::ADC_CLOCK, -1, 0);
}

void Detector::setADCClock(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setSpeed, pos, defs::ADC_CLOCK, value, 0);
}

Result<int> Detector::getDBITClock(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setSpeed, pos, defs::DBIT_CLOCK, -1,
                           0);
}

void Detector::setDBITClock(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setSpeed, pos, defs::DBIT_CLOCK, value, 0);
}

Result<int> Detector::getRUNClock(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setSpeed, pos, defs::CLOCK_DIVIDER, -1,
                           0);
}

void Detector::setRUNClock(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setSpeed, pos, defs::CLOCK_DIVIDER, value, 0);
}

Result<int> Detector::getSYNCClock(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setSpeed, pos, defs::SYNC_CLOCK, -1,
                           0);
}

Result<int> Detector::getADCPipeline(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setSpeed, pos, defs::ADC_PIPELINE, -1,
                           0);
}

void Detector::setADCPipeline(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setSpeed, pos, defs::ADC_PIPELINE, value, 0);
}

Result<int> Detector::getDBITPipeline(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setSpeed, pos, defs::DBIT_PIPELINE, -1,
                           0);
}

void Detector::setDBITPipeline(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setSpeed, pos, defs::DBIT_PIPELINE, value, 0);
}

Result<int> Detector::getDynamicRange(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setDynamicRange, pos, -1);
}

void Detector::setDynamicRange(int value) { pimpl->setDynamicRange(value); }

Result<int> Detector::getHighVoltage(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setDAC, pos, -1, defs::HIGH_VOLTAGE,
                           0);
}

void Detector::setHighVoltage(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setDAC, pos, value, defs::HIGH_VOLTAGE, 0);
}

Result<int> Detector::getIODelay(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setDAC, pos, -1, defs::IO_DELAY, 0);
}

void Detector::setIODelay(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setDAC, pos, value, defs::IO_DELAY, 0);
}

Result<int> Detector::getTemp(defs::dacIndex index, Positions pos) const {
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
    switch (getDetectorType()) {
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

Result<int> Detector::getVrefVoltage(bool mV, Positions pos) const {
    return pimpl->Parallel(&slsDetector::setDAC, pos, -1, defs::ADC_VPP, mV);
}

void Detector::setVrefVoltage(int value, bool mV, Positions pos) {
    pimpl->Parallel(&slsDetector::setDAC, pos, value, defs::ADC_VPP, mV);
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

void Detector::setVoltage(int value, defs::dacIndex index, Positions pos) {
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

Result<int> Detector::getDAC(defs::dacIndex index, bool mV,
                             Positions pos) const {
    return pimpl->Parallel(&slsDetector::setDAC, pos, -1, index, mV);
}

void Detector::setDAC(int value, defs::dacIndex index, bool mV, Positions pos) {
    pimpl->Parallel(&slsDetector::setDAC, pos, value, index, mV);
}

    Result<defs::externalCommunicationMode> Detector::getTimingMode(Positions pos) const {
        return pimpl->Parallel(&slsDetector::setExternalCommunicationMode, pos, defs::GET_EXTERNAL_COMMUNICATION_MODE);
    }

    void Detector::setTimingMode(defs::externalCommunicationMode value, Positions pos) {
        pimpl->Parallel(&slsDetector::setExternalCommunicationMode, pos, value);
    }

    Result<defs::externalSignalFlag> Detector::getExternalSignalFlags(Positions pos) const {
        return pimpl->Parallel(&slsDetector::setExternalSignalFlags, pos, defs::GET_EXTERNAL_SIGNAL_FLAG);
    }

    void Detector::setExternalSignalFlags(defs::externalSignalFlag value, Positions pos) {
        pimpl->Parallel(&slsDetector::setExternalSignalFlags, pos, value);
    }

    Result<bool> Detector::getParallelMode(Positions pos) const {
        auto res = pimpl->Parallel(&slsDetector::setReadOutFlags, pos, defs::GET_READOUT_FLAGS);
        Result <bool> booleanRes; 
        for (unsigned int i = 0; i < res.size(); ++i) {
            booleanRes[i] = (res[i] & defs::PARALLEL) ? true : false;
        }
        return booleanRes;
    }

    void Detector::setParallelMode(bool value, Positions pos) {
        pimpl->Parallel(&slsDetector::setReadOutFlags, pos, value ? defs::PARALLEL : defs::NONPARALLEL);
    }

    Result<bool> Detector::getOverFlowMode(Positions pos) const {
        auto res = pimpl->Parallel(&slsDetector::setReadOutFlags, pos, defs::GET_READOUT_FLAGS);
        Result <bool> booleanRes; 
        for (unsigned int i = 0; i < res.size(); ++i) {
            booleanRes[i] = (res[i] & defs::SHOW_OVERFLOW) ? true : false;
        }
        return booleanRes;
    }

    void Detector::setOverFlowMode(bool value, Positions pos) {
        pimpl->Parallel(&slsDetector::setReadOutFlags, pos, value ? defs::SHOW_OVERFLOW : defs::NOOVERFLOW);
    }

    Result<int> Detector::getSignalType(Positions pos) const {
        auto res = pimpl->Parallel(&slsDetector::setReadOutFlags, pos, defs::GET_READOUT_FLAGS);
        for (auto &it : res) {
            if (it & defs::ANALOG_AND_DIGITAL) {
                it = 2;
            } else if (it & defs::DIGITAL_ONLY) {
                it = 1;
            } else if (it == defs::NORMAL_READOUT) {
                it = 0;
            } else {
                 throw RuntimeError("Unknown Signal Type");
            }
        }
        return res;
    }

    void Detector::setSignalType(int value, Positions pos) {
        defs::readOutFlags flag;
        switch (value) {
            case 0:
                flag = defs::NORMAL_READOUT;
                break;
            case 1:
                flag = defs::DIGITAL_ONLY;
                break;
            case 2:
                flag = defs::ANALOG_AND_DIGITAL;
                break;
            default:
                 throw RuntimeError("Unknown Signal Type");
        }
        pimpl->Parallel(&slsDetector::setReadOutFlags, pos, flag);
    }

// Erik
Result<int> Detector::getFramesCaughtByReceiver(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getFramesCaughtByReceiver, pos);
}

Result<uint64_t> Detector::getReceiverCurrentFrameIndex(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverCurrentFrameIndex, pos);
}

void Detector::resetFramesCaught(Positions pos) {
    pimpl->Parallel(&slsDetector::resetFramesCaught, pos);
}

void Detector::setMasterFileWrite(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::setMasterFileWrite, pos, value);
}

Result<bool> Detector::getMasterFileWrite(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getMasterFileWrite, pos);
}

void Detector::setReceiverStreamingTimer(int time_in_ms, Positions pos) {
    pimpl->Parallel(&slsDetector::setReceiverStreamingTimer, pos, time_in_ms);
}

Result<int> Detector::getReceiverStreamingTimer(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setReceiverStreamingTimer, pos, -1);
}

void Detector::enableDataStreamingFromReceiver(bool enable, Positions pos) {
    pimpl->Parallel(&slsDetector::enableDataStreamingFromReceiver, pos,
                    static_cast<int>(enable));
}

void Detector::setTenGigaEnabled(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::enableTenGigabitEthernet, pos,
                    static_cast<int>(value));
}

Result<bool> Detector::getTenGigaEnabled(Positions pos) const {
    return pimpl->Parallel(&slsDetector::enableTenGigabitEthernet, pos, -1);
}

Result<int> Detector::getReceiverFifoDepth(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setReceiverFifoDepth, pos, -1);
}

void Detector::setReceiverFifoDepth(int nframes, Positions pos) {
    pimpl->Parallel(&slsDetector::setReceiverFifoDepth, pos, nframes);
}

Result<bool> Detector::getReceiverSilentMode(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setReceiverSilentMode, pos, -1);
}

void Detector::setReceiverSilentMode(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::setReceiverSilentMode, pos,
                    static_cast<int>(value));
}

void Detector::setPattern(const std::string &fname, Positions pos) {
    pimpl->Parallel(&slsDetector::setPattern, pos, fname);
}

void Detector::setPatternIOControl(uint64_t word, Positions pos) {
    pimpl->Parallel(&slsDetector::setPatternIOControl, pos, word);
}

Result<uint64_t> Detector::getPatternIOControl(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setPatternIOControl, pos, -1);
}

void Detector::setPatternClockControl(uint64_t word, Positions pos) {
    pimpl->Parallel(&slsDetector::setPatternClockControl, pos, word);
}

Result<uint64_t> Detector::getPatternClockControl(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setPatternClockControl, pos, -1);
}

void Detector::setPatternWord(int addr, uint64_t word, Positions pos) {
    pimpl->Parallel(&slsDetector::setPatternWord, pos, addr, word);
}

void Detector::setPatternLoops(int level, int start, int stop, int n,
                               Positions pos) {
    pimpl->Parallel(&slsDetector::setPatternLoops, pos, level, start, stop, n);
}

Result<std::array<int, 3>> Detector::getPatternLoops(int level,
                                                     Positions pos) const {
    return pimpl->Parallel(&slsDetector::setPatternLoops, pos, level, -1, -1,
                           -1);
}

void Detector::setPatternWaitAddr(int level, int addr, Positions pos) {
    pimpl->Parallel(&slsDetector::setPatternWaitAddr, pos, level, addr);
}

Result<int> Detector::getPatternWaitAddr(int level, Positions pos) const {
    return pimpl->Parallel(&slsDetector::setPatternWaitAddr, pos, level, -1);
}

void Detector::setPatternWaitTime(int level, uint64_t t, Positions pos) {
    pimpl->Parallel(&slsDetector::setPatternWaitTime, pos, level, t);
}

Result<uint64_t> Detector::getPatternWaitTime(int level, Positions pos) const {
    return pimpl->Parallel(&slsDetector::setPatternWaitTime, pos, level, -1);
}

Result<uint64_t> Detector::getPatternMask(Positions pos) {
    return pimpl->Parallel(&slsDetector::getPatternMask, pos);
}

void Detector::setPatternBitMask(uint64_t mask, Positions pos) {
    pimpl->Parallel(&slsDetector::setPatternBitMask, pos, mask);
}

Result<uint64_t> Detector::getPatternBitMask(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getPatternBitMask, pos);
}

void Detector::setLEDEnable(bool enable, Positions pos) {
    pimpl->Parallel(&slsDetector::setLEDEnable, pos, static_cast<int>(enable));
}

Result<bool> Detector::getLEDEnable(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setLEDEnable, pos, -1);
}

void Detector::setDigitalIODelay(uint64_t pinMask, int delay, Positions pos) {
    pimpl->Parallel(&slsDetector::setDigitalIODelay, pos, pinMask, delay);
}

Result<int> Detector::getFileIndex(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getFileIndex, pos);
}

void Detector::setFileIndex(int i, Positions pos) {
    pimpl->Parallel(&slsDetector::setFileIndex, pos, i);
}

Result<defs::fileFormat> Detector::getFileFormat(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getFileFormat, pos);
}

void Detector::setFileFormat(defs::fileFormat f, Positions pos) {
    pimpl->Parallel(&slsDetector::setFileFormat, pos, f);
}

Result<bool> Detector::getPartialFramesPadding(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getPartialFramesPadding, pos);
}

void Detector::setPartialFramesPadding(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::setPartialFramesPadding, pos, value);
}

void Detector::setReceiverFrameDiscardPolicy(defs::frameDiscardPolicy f,
                                             Positions pos) {
    pimpl->Parallel(&slsDetector::setReceiverFramesDiscardPolicy, pos, f);
}

Result<defs::frameDiscardPolicy>
Detector::getReceiverFrameDiscardPolicy(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setReceiverFramesDiscardPolicy, pos,
                           defs::GET_FRAME_DISCARD_POLICY);
}

void Detector::setFramesPerFile(int n, Positions pos) {
    pimpl->Parallel(&slsDetector::setFramesPerFile, pos, n);
}

Result<int> Detector::getFramesPerFile(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getFramesPerFile, pos);
}

Result<std::string> Detector::getReceiverLastClientIP(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverLastClientIP, pos);
}

void Detector::setReceiverLock(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::lockReceiver, pos, static_cast<int>(value));
}

Result<bool> Detector::getReceiverLock(Positions pos) {
    return pimpl->Parallel(&slsDetector::lockReceiver, pos, -1);
}

Result<bool> Detector::getUseReceiverFlag(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getUseReceiverFlag, pos);
}

void Detector::printReceiverConfiguration(Positions pos) const {
    pimpl->Parallel(&slsDetector::printReceiverConfiguration, pos,
                    TLogLevel::logINFO);
}

Result<int64_t> Detector::getRateCorrection(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getRateCorrection, pos);
}

void Detector::setRateCorrection(int64_t dead_time_ns, Positions pos) {
    pimpl->Parallel(&slsDetector::setRateCorrection, pos, dead_time_ns);
}

void Detector::setAutoCompDisable(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::setAutoComparatorDisableMode, pos,
                    static_cast<int>(value));
}

Result<bool> Detector::getAutoCompDisable(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setAutoComparatorDisableMode, pos, -1);
}

void Detector::setPowerChip(bool on, Positions pos) {
    if (on && pimpl->size() > 3) {
        for (unsigned int i = 0; i != pimpl->size(); ++i) {
            pimpl->powerChip(static_cast<int>(on), i);
            usleep(1000 * 1000);
        }
    } else {
        pimpl->Parallel(&slsDetector::powerChip, pos, static_cast<int>(on));
    }
}

Result<bool> Detector::getPowerChip(Positions pos) const {
    return pimpl->Parallel(&slsDetector::powerChip, pos, -1);
}

void Detector::updateFirmwareAndServer(const std::string &sname,
                                       const std::string &hostname,
                                       const std::string &fname,
                                       Positions pos) {
    copyDetectorServer(fname, hostname, pos);
    programFPGA(fname, pos);
}

void Detector::rebootController(Positions pos) {
    pimpl->Parallel(&slsDetector::rebootController, pos);
}

void Detector::copyDetectorServer(const std::string &fname,
                                  const std::string &hostname, Positions pos) {
    pimpl->Parallel(&slsDetector::copyDetectorServer, pos, fname, hostname);
}

void Detector::resetFPGA(Positions pos) {
    pimpl->Parallel(&slsDetector::resetFPGA, pos);
}

void Detector::programFPGA(const std::string &fname, Positions pos) {
    FILE_LOG(logINFO) << "This can take awhile. Please be patient...";
    std::vector<char> buffer = pimpl->readPofFile(fname);
    pimpl->Parallel(&slsDetector::programFPGA, pos, buffer);
}

void Detector::setStoragecellStart(int cell, Positions pos) {
    pimpl->Parallel(&slsDetector::setStoragecellStart, pos, cell);
}

Result<int> Detector::getStorageCellStart(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setStoragecellStart, pos, -1);
}

void Detector::setTemperatureEvent(int val, Positions pos) {
    pimpl->Parallel(&slsDetector::setTemperatureEvent, pos, val);
}

Result<int> Detector::getTemperatureEvent(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setTemperatureEvent, pos, -1);
}

void Detector::setTemperatureControl(bool enable, Positions pos) {
    pimpl->Parallel(&slsDetector::setTemperatureControl, pos,
                    static_cast<int>(enable));
}

Result<bool> Detector::getTemperatureControl(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setTemperatureControl, pos, -1);
}

void Detector::setThresholdTemperature(int temp, Positions pos) {
    pimpl->Parallel(&slsDetector::setThresholdTemperature, pos, temp);
}

Result<int> Detector::getThresholdTemperature(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setThresholdTemperature, pos, -1);
}

void Detector::pulseChip(int n, Positions pos) {
    pimpl->Parallel(&slsDetector::pulseChip, pos, n);
}

void Detector::pulsePixelNMove(int n, int x, int y, Positions pos) {
    pimpl->Parallel(&slsDetector::pulsePixelNMove, pos, n, x, y);
}

void Detector::pulsePixel(int n, int x, int y, Positions pos) {
    pimpl->Parallel(&slsDetector::pulsePixel, pos, n, x, y);
}

Result<std::vector<int>> Detector::getTrimEn(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getTrimEn, pos);
}

void Detector::setTrimEn(std::vector<int> energies, Positions pos) {
    pimpl->Parallel(&slsDetector::setTrimEn, pos, energies);
}

void Detector::setGapPixelsEnable(bool enable, Positions pos) {
    pimpl->setGapPixelsEnable(enable, pos);
}

Result<bool> Detector::getGapPixelEnable(Positions pos) const {
    return pimpl->Parallel(&slsDetector::enableGapPixels, pos, -1);
}

void Detector::setAllTrimbits(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setAllTrimbits, pos, value);
}

void Detector::setFlippedData(defs::dimension d, bool flipped, Positions pos) {
    pimpl->Parallel(&slsDetector::setFlippedData, pos, d,
                    static_cast<int>(flipped));
}

Result<bool> Detector::getFlippedData(defs::dimension d, Positions pos) const {
    return pimpl->Parallel(&slsDetector::getFlippedData, pos, d);
}

void Detector::setRxPadDeactivatedMod(bool pad, Positions pos) {
    pimpl->Parallel(&slsDetector::setDeactivatedRxrPaddingMode, pos,
                    static_cast<int>(pad));
}

Result<bool> Detector::getRxPadDeactivatedMod(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setDeactivatedRxrPaddingMode, pos, -1);
}

void Detector::setActive(bool active, Positions pos) {
    pimpl->Parallel(&slsDetector::activate, pos, static_cast<int>(active));
}

Result<bool> Detector::getActive(Positions pos) const {
    return pimpl->Parallel(&slsDetector::activate, pos, -1);
}

void Detector::writeAdcRegister(uint32_t addr, uint32_t value, Positions pos) {
    pimpl->Parallel(&slsDetector::writeAdcRegister, pos, addr, value);
}

Result<int> Detector::getReceiverDbitOffset(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverDbitOffset, pos);
}

void Detector::setReceiverDbitOffset(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setReceiverDbitOffset, pos, value);
}

Result<std::vector<int>> Detector::getReceiverDbitList(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverDbitList, pos);
}

void Detector::setReceiverDbitList(std::vector<int> list, Positions pos) {
    pimpl->Parallel(&slsDetector::setReceiverDbitList, pos, list);
}

Result<int> Detector::getExternalSampling(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getExternalSampling, pos);
}

void Detector::setExternalSampling(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::setExternalSampling, pos, value);
}

Result<int> Detector::getExternalSamplingSource(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getExternalSamplingSource, pos);
}

void Detector::setExternalSamplingSource(int value, Positions pos) {
    pimpl->Parallel(&slsDetector::setExternalSamplingSource, pos, value);
}

uint32_t Detector::getADCInvert() const {
    return pimpl->Parallel(&slsDetector::getADCInvert, {})
        .tsquash("Different Values for function getADCInvert");
}

void Detector::setADCInvert(uint32_t value) {
    pimpl->Parallel(&slsDetector::setADCInvert, {}, value);
}

uint32_t Detector::getADCEnableMask() const {
    return pimpl->Parallel(&slsDetector::getADCEnableMask, {})
        .tsquash("Values of ADC enable mask cannot be different");
}

void Detector::setADCEnableMask(uint32_t mask) {
    pimpl->Parallel(&slsDetector::setADCEnableMask, {}, mask);
}

Result<int> Detector::getCounterBit(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setCounterBit, pos, -1);
}

void Detector::setCounterBit(int i, Positions pos) {
    pimpl->Parallel(&slsDetector::setCounterBit, pos, i);
}

void Detector::resetCounterBlock(int startACQ, Positions pos) {
    pimpl->Parallel(&slsDetector::resetCounterBlock, pos, startACQ);
}

void Detector::loadImageToDetector(defs::imageType index,
                                   const std::string &fname, Positions pos) {
    // TODO! optimize away multiple loads
    pimpl->Parallel(&slsDetector::loadImageToDetector, pos, index, fname);
}

Result<int> Detector::digitalTest(defs::digitalTestMode mode, int ival,
                                  Positions pos) {
    return pimpl->Parallel(&slsDetector::digitalTest, pos, mode, ival);
}

void Detector::setFlowControl10G(bool enable, Positions pos) {
    pimpl->Parallel(&slsDetector::setDetectorNetworkParameter, pos,
                    defs::FLOW_CONTROL_10G, static_cast<int>(enable));
}

Result<bool> Detector::getFlowControl10G(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setDetectorNetworkParameter, pos,
                           defs::FLOW_CONTROL_10G, -1);
}

Result<int64_t>
Detector::getReceiverRealUDPSocketBufferSize(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverRealUDPSocketBufferSize,
                           pos);
}

Result<int64_t> Detector::getReceiverUDPSocketBufferSize(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverUDPSocketBufferSize, pos);
}

void Detector::setReceiverUDPSocketBufferSize(int64_t udpsockbufsize,
                                              Positions pos) {
    pimpl->Parallel(&slsDetector::setReceiverUDPSocketBufferSize, pos,
                    udpsockbufsize);
}

int Detector::setDetectorMode(defs::detectorModeType value) {
    return pimpl->setDetectorMode(value);
}
int Detector::setFrameMode(defs::frameModeType value) {
    return pimpl->setFrameMode(value);
}
int Detector::setDetectorMinMaxEnergyThreshold(const int index, int value) {
    return pimpl->setDetectorMinMaxEnergyThreshold(index, value);
}

void Detector::setAdditionalJsonHeader(const std::string &jsonheader,
                                       Positions pos) {
    pimpl->Parallel(&slsDetector::setAdditionalJsonHeader, pos, jsonheader);
}

Result<std::string> Detector::getAdditionalJsonHeader(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getAdditionalJsonHeader, pos);
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

Result<std::string> Detector::getReceiverStreamingIP(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverStreamingIP, pos);
}

void Detector::setReceiverDataStreamingOutIP(const std::string &ip,
                                             Positions pos) {
    // TODO! probably in one call
    pimpl->Parallel(&slsDetector::setReceiverStreamingIP, pos, ip);
    enableDataStreamingFromReceiver(false, pos);
    enableDataStreamingFromReceiver(true, pos);
}

Result<std::string> Detector::getClientStreamingIP(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getClientStreamingIP, pos);
}

void Detector::setClientDataStreamingInIP(const std::string &ip,
                                          Positions pos) {
    // TODO! probably in one call
    pimpl->Parallel(&slsDetector::setClientStreamingIP, pos, ip);
    pimpl->enableDataStreamingToClient(0);
    pimpl->enableDataStreamingToClient(1);
}

Result<int> Detector::getReceiverStreamingPort(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverStreamingPort, pos);
}

void Detector::setReceiverDataStreamingOutPort(int port, int module_id) {
    pimpl->setReceiverDataStreamingOutPort(port, module_id);
}

Result<int> Detector::getClientStreamingPort(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getClientStreamingPort, pos);
}

void Detector::setClientDataStreamingInPort(int port, int module_id) {
    pimpl->setClientDataStreamingInPort(port, module_id);
}

Result<int> Detector::getSelectedUDPInterface(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getSelectedUDPInterface, pos);
}

void Detector::selectUDPInterface(int interface, Positions pos) {
    pimpl->Parallel(&slsDetector::selectUDPInterface, pos, interface);
}

Result<int> Detector::getNumberofUDPInterfaces(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getNumberofUDPInterfaces, pos);
}

void Detector::setNumberofUDPInterfaces(int n, Positions pos) {
    pimpl->Parallel(&slsDetector::setNumberofUDPInterfaces, pos, n);

    pimpl->enableDataStreamingToClient(0);
    pimpl->enableDataStreamingToClient(1);
    enableDataStreamingFromReceiver(false, pos);
    enableDataStreamingFromReceiver(true, pos);
}

Result<int> Detector::getReceiverUDPPort2(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverUDPPort2, pos);
}

void Detector::setReceiverUDPPort2(int udpport, Positions pos) {
    pimpl->Parallel(&slsDetector::setReceiverUDPPort2, pos, udpport);
}

Result<int> Detector::getReceiverUDPPort(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverUDPPort, pos);
}

void Detector::setReceiverUDPPort(int udpport, Positions pos) {
    pimpl->Parallel(&slsDetector::setReceiverUDPPort, pos, udpport);
}

Result<MacAddr> Detector::getReceiverUDPMAC2(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverUDPMAC2, pos);
}

void Detector::setReceiverUDPMAC2(const std::string &udpmac, Positions pos) {
    pimpl->Parallel(&slsDetector::setReceiverUDPMAC2, pos, udpmac);
}

Result<MacAddr> Detector::getReceiverUDPMAC(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverUDPMAC, pos);
}

void Detector::setReceiverUDPMAC(const std::string &udpmac, Positions pos) {
    pimpl->Parallel(&slsDetector::setReceiverUDPMAC, pos, udpmac);
}

Result<IpAddr> Detector::getReceiverUDPIP2(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverUDPIP2, pos);
}

void Detector::setReceiverUDPIP2(const std::string &udpip, Positions pos) {
    pimpl->Parallel(&slsDetector::setReceiverUDPIP2, pos, udpip);
}

Result<IpAddr> Detector::getReceiverUDPIP(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverUDPIP, pos);
}

void Detector::setReceiverUDPIP(const std::string &udpip, Positions pos) {
    pimpl->Parallel(&slsDetector::setReceiverUDPIP, pos, udpip);
}

Result<std::string> Detector::getReceiverHostname(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverHostname, pos);
}

void Detector::setReceiverHostname(const std::string &receiver, Positions pos) {
    pimpl->Parallel(&slsDetector::setReceiverHostname, pos, receiver);
}

Result<IpAddr> Detector::getDetectorIP2(Positions pos) const{
    return pimpl->Parallel(&slsDetector::getDetectorIP2, pos);
}

void Detector::setDetectorIP2(const std::string &detectorIP, Positions pos){
    pimpl->Parallel(&slsDetector::setDetectorIP2, pos, detectorIP);
}

Result<IpAddr> Detector::getDetectorIP(Positions pos) const{
    return pimpl->Parallel(&slsDetector::getDetectorIP, pos);
}

void Detector::setDetectorIP(const std::string &detectorIP, Positions pos){
    pimpl->Parallel(&slsDetector::setDetectorIP, pos, detectorIP);
}

Result<MacAddr> Detector::getDetectorMAC(Positions pos) const{
    return pimpl->Parallel(&slsDetector::getDetectorMAC, pos);
}
void Detector::setDetectorMAC(const std::string &detectorMAC, Positions pos){
    pimpl->Parallel(&slsDetector::setDetectorMAC, pos, detectorMAC);
}

Result<MacAddr> Detector::getDetectorMAC2(Positions pos) const{
    return pimpl->Parallel(&slsDetector::getDetectorMAC2, pos);
}
void Detector::setDetectorMAC2(const std::string &detectorMAC, Positions pos){
    pimpl->Parallel(&slsDetector::setDetectorMAC2, pos, detectorMAC);
}

} // namespace sls