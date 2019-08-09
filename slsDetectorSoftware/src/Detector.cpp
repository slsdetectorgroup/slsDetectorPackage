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

int Detector::getTotalNumberOfDetectors() const {
    return pimpl->getNumberOfDetectors();
}

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
    return pimpl->Parallel(&slsDetector::getTimeLeft, pos, defs::ACQUISITION_TIME);
}

Result<ns> Detector::getPeriodLeft(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getTimeLeft, pos, defs::FRAME_PERIOD);
}

Result<ns> Detector::getDelayAfterTriggerLeft(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getTimeLeft, pos, defs::DELAY_AFTER_TRIGGER);
}

Result<int64_t> Detector::getNumberOfFramesFromStart(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getTimeLeft, pos, defs::FRAMES_FROM_START);
}

Result<ns> Detector::getActualTime(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getTimeLeft, pos, defs::ACTUAL_TIME);
}

Result<ns> Detector::getMeasurementTime(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getTimeLeft, pos, defs::MEASUREMENT_TIME);
}

Result<ns> Detector::getMeasuredPeriod(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getTimeLeft, pos, defs::MEASURED_PERIOD);
};

Result<ns> Detector::getMeasuredSubFramePeriod(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getTimeLeft, pos, defs::MEASURED_SUBPERIOD);
};

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
    if (on && pimpl->getNumberOfDetectors() > 3) {
        for (int i = 0; i != pimpl->getNumberOfDetectors(); ++i) {
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

} // namespace sls