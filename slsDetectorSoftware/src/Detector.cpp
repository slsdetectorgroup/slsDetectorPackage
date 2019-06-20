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

void Detector::freeSharedMemory() { pimpl->freeSharedMemory(); }

void Detector::acquire() { pimpl->acquire(); }

void Detector::setConfig(const std::string &fname) {
    pimpl->readConfigurationFile(fname);
}

std::vector<ns> Detector::getExptime(Positions pos) const {
    auto r = pimpl->Parallel(&slsDetector::setTimer, pos,
                             defs::ACQUISITION_TIME, -1);
    return std::vector<ns>(begin(r), end(r));
}

void Detector::setExptime(ns t, Positions pos) {
    pimpl->Parallel(&slsDetector::setTimer, pos, defs::ACQUISITION_TIME,
                    t.count());
}

std::vector<ns> Detector::getSubExptime(Positions pos) const {
    auto r = pimpl->Parallel(&slsDetector::setTimer, pos,
                             defs::SUBFRAME_ACQUISITION_TIME, -1);
    return std::vector<ns>(begin(r), end(r));
}

void Detector::setSubExptime(ns t, Positions pos) {
    pimpl->Parallel(&slsDetector::setTimer, pos,
                    defs::SUBFRAME_ACQUISITION_TIME, t.count());
}

std::vector<ns> Detector::getPeriod(Positions pos) const {
    auto r =
        pimpl->Parallel(&slsDetector::setTimer, pos, defs::FRAME_PERIOD, -1);
    return std::vector<ns>(begin(r), end(r));
}

void Detector::setPeriod(ns t, Positions pos) {
    pimpl->Parallel(&slsDetector::setTimer, pos, defs::FRAME_PERIOD, t.count());
}

void Detector::setFname(const std::string &fname) {
    pimpl->Parallel(&slsDetector::setFileName, Positions{}, fname);
}
std::vector<std::string> Detector::getFname() const {
    return pimpl->Parallel(&slsDetector::setFileName, Positions{}, "");
}

void Detector::setFwrite(bool value, Positions pos) {
    pimpl->Parallel(&slsDetector::setFileWrite, Positions{}, value);
}

std::vector<bool> Detector::getFwrite(Positions pos) const {
    return pimpl->Parallel(&slsDetector::getFileWrite, Positions{});
}

} // namespace sls