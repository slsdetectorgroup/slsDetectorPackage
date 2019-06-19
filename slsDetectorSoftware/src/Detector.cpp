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

void Detector::acquire() { pimpl->acquire(); }

std::vector<ns> Detector::getExptime(int det_id) const {
    if (det_id < 0) {
        auto t = pimpl->parallelCall(&slsDetector::setTimer,
                                     defs::ACQUISITION_TIME, -1);
        return std::vector<ns>(begin(t), end(t));
    } else
        return std::vector<ns>{0};
}
} // namespace sls