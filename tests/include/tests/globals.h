#pragma once
#include "sls_detector_defs.h"
using dt = slsDetectorDefs::detectorType;
using di = slsDetectorDefs::dacIndex;
using defs = slsDetectorDefs;

namespace test {
extern std::string hostname;
extern std::string detector_type;
extern dt type;
extern std::string my_ip;
extern decltype(defs::GET_ACTION) GET;
extern decltype(defs::PUT_ACTION) PUT;


} // namespace test
