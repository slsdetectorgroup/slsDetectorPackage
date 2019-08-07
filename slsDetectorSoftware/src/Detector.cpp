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

bool Detector::getAcquiringFlag() const{
    return pimpl->getAcquiringFlag();
}

void Detector::setAcquiringFlag(bool value){
    pimpl->setAcquiringFlag(value);
}

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
    auto r = pimpl->Parallel(&slsDetector::setTimer, pos,
                             defs::ACQUISITION_TIME, -1);
    return Result<ns>(begin(r), end(r));
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
    auto r = pimpl->Parallel(&slsDetector::setTimer, pos,
                             defs::SUBFRAME_ACQUISITION_TIME, -1);
    return Result<ns>(begin(r), end(r));
}

void Detector::setSubExptime(ns t, Positions pos) {
    pimpl->Parallel(&slsDetector::setTimer, pos,
                    defs::SUBFRAME_ACQUISITION_TIME, t.count());
}

Result<ns> Detector::getPeriod(Positions pos) const {
    auto r =
        pimpl->Parallel(&slsDetector::setTimer, pos, defs::FRAME_PERIOD, -1);
    return Result<ns>(begin(r), end(r));
}

void Detector::setPeriod(ns t, Positions pos) {
    pimpl->Parallel(&slsDetector::setTimer, pos, defs::FRAME_PERIOD, t.count());
}

// File
void Detector::setFileName(const std::string &fname) {
    pimpl->Parallel(&slsDetector::setFileName, Positions{}, fname);
}
Result<std::string> Detector::getFileName() const {
    return pimpl->Parallel(&slsDetector::setFileName, Positions{}, "");
}

void Detector::setFilePath(const std::string &fpath) {
    pimpl->Parallel(&slsDetector::setFilePath, Positions{}, fpath);
}
Result<std::string> Detector::getFilePath() const {
    return pimpl->Parallel(&slsDetector::getFilePath, Positions{});
}

void Detector::setFileWrite(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::setFileWrite, Positions{}, value);
}

Result<bool> Detector::getFileWrite(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getFileWrite, Positions{});
}

void Detector::setFileOverWrite(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::setFileOverWrite, Positions{}, value);
}

Result<bool> Detector::getFileOverWrite(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getFileOverWrite, Positions{});
}

} // namespace sls