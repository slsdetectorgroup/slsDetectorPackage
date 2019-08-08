#include "Detector.h"
#include "container_utils.h"
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

Result<ns> Detector::getExptime(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setTimer, pos, defs::ACQUISITION_TIME,
                           -1);
}

Result<uint64_t> Detector::getStartingFrameNumber(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getStartingFrameNumber, pos);
}
void Detector::setStartingFrameNumber(uint64_t value, Positions pos) {
    pimpl->Parallel(&slsDetector::setStartingFrameNumber, pos, value);
}

void Detector::setExptime(ns t, Positions pos) {
    pimpl->Parallel(&slsDetector::setTimer, pos, defs::ACQUISITION_TIME,
                    t.count());
}

Result<ns> Detector::getSubExptime(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setTimer, pos,
                           defs::SUBFRAME_ACQUISITION_TIME, -1);
}

void Detector::setSubExptime(ns t, Positions pos) {
    pimpl->Parallel(&slsDetector::setTimer, pos,
                    defs::SUBFRAME_ACQUISITION_TIME, t.count());
}

Result<ns> Detector::getPeriod(Positions pos) const {
    return pimpl->Parallel(&slsDetector::setTimer, pos, defs::FRAME_PERIOD, -1);
}

void Detector::setPeriod(ns t, Positions pos) {
    pimpl->Parallel(&slsDetector::setTimer, pos, defs::FRAME_PERIOD, t.count());
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

Result<int64_t> Detector::getClientSoftwareVersion() const {
    return pimpl->getClientSoftwareVersion();
}

Result<int64_t> Detector::getReceiverSoftwareVersion(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getReceiverSoftwareVersion, pos);
}

std::string Detector::getUserDetails() const { return pimpl->getUserDetails(); }

void Detector::setHostname(const std::vector<std::string> &name) {
    pimpl->setHostname(name);
}

defs::detectorType Detector::getDetectorTypeAsEnum() const {
    return pimpl->getDetectorTypeAsEnum();
}

Result<defs::detectorType>
Detector::getDetectorTypeAsEnum(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getDetectorTypeAsEnum, pos);
}

Result<std::string> Detector::getDetectorTypeAsString(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getDetectorTypeAsString, pos);
}

// Erik


void Detector::setPatternClockControl(uint64_t word, Positions pos){
    pimpl->Parallel(&slsDetector::setPatternClockControl, pos, word);
}

Result<uint64_t> Detector::getPatternClockControl(Positions pos) const{
    return pimpl->Parallel(&slsDetector::setPatternClockControl, pos, -1);
}

void Detector::setPatternWord(int addr, uint64_t word, Positions pos){
    pimpl->Parallel(&slsDetector::setPatternWord, pos, addr, word);
}



void Detector::setPatternLoops(int level, int start, int stop, int n, Positions pos){
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

} // namespace sls