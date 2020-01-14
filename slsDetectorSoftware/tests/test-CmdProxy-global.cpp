#include "test-CmdProxy-global.h"
#include "CmdProxy.h"
#include "Detector.h"
#include "catch.hpp"
#include "tests/globals.h"

using sls::CmdProxy;
using sls::Detector;
using test::GET;
using test::PUT;


void test_dac(defs::dacIndex index, const std::string &dacname, int dacvalue) {
    Detector det;
    CmdProxy proxy(&det);
    std::ostringstream oss_set, oss_get;
    auto dacstr = std::to_string(dacvalue);
    auto previous = det.getDAC(index, false);
    proxy.Call(dacname, {dacstr}, -1, PUT, oss_set);
    REQUIRE(oss_set.str() == dacname + " " + dacstr + "\n");
    proxy.Call(dacname, {}, -1, GET, oss_get);
    REQUIRE(oss_get.str() == dacname + " " + dacstr + "\n");
    // Reset all dacs to previous value
    for (int i = 0; i != det.size(); ++i) {
        det.setDAC(index, previous[i], false, {i});
    }
}
