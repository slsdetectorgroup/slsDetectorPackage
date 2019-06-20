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

std::vector<ns> Detector::getExptime(Positions pos) {
    auto t = pimpl->getTimer(defs::ACQUISITION_TIME, pos);
}
} // namespace sls