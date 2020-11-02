#pragma once
#include "sls/sls_detector_defs.h"

void test_dac(slsDetectorDefs::dacIndex index, const std::string &dacname,
              int dacvalue);
void test_onchip_dac(slsDetectorDefs::dacIndex index,
                     const std::string &dacname, int dacvalue);
