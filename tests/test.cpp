// tests-main.cpp
// #define CATCH_CONFIG_MAIN
// #include "catch.hpp"

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include "sls_detector_defs.h"
#include "ToString.h"
#include "tests/config.h"
#include <string>

// using namespace Catch::clara;
using Opt = Catch::clara::Opt;
using dt = slsDetectorDefs::detectorType;

namespace test {
std::string hostname;
std::string detector_type;
std::string my_ip;
dt type;
} // namespace test

int main(int argc, char *argv[]) {
    test::my_ip = "undefined";

    Catch::Session session;
    auto cli =
        session.cli() |
        Opt(test::hostname, "hostname")["-hn"]["--hostname"](
            "Detector hostname for integration tests") |
        Opt(test::detector_type, "detector_type")["-dt"]["--detector_type"](
            "Detector type for integration tests") |
        Opt(test::my_ip, "my_ip")["-hip"]["--host_ip"]("Host ip address");

    session.cli(cli);

    auto ret = session.applyCommandLine(argc, argv);
    if (ret) {
        return ret;
    }

    test::type = sls::StringTo<slsDetectorDefs::detectorType>(test::detector_type);

    return session.run();
}