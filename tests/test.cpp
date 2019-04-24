// tests-main.cpp
// #define CATCH_CONFIG_MAIN
// #include "catch.hpp"

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include "sls_detector_defs.h"
#include "tests/config.h"
#include <string>

// using namespace Catch::clara;
using Opt = Catch::clara::Opt;
using dt = slsDetectorDefs::detectorType;

std::string hostname;
std::string detector_type;
dt type;

int main(int argc, char *argv[]) {
    Catch::Session session;

    auto cli = session.cli() |
               Opt(hostname, "hostname")["-hn"]["--hostname"](
                   "Detector hostname for integration tests") |
               Opt(detector_type, "detector_type")["-dt"]["--detector_type"](
                   "Detector type for integration tests");

    session.cli(cli);

    auto ret = session.applyCommandLine(argc, argv);
    if (ret) {
        return ret;
    }

    type = slsDetectorDefs::detectorTypeToEnum(detector_type);

    return session.run();
}