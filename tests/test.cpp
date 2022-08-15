// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
// tests-main.cpp
// #define CATCH_CONFIG_MAIN
// #include "catch.hpp"

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include "sls/ToString.h"
#include "sls/sls_detector_defs.h"
#include "tests/config.h"
#include <string>

namespace sls {
// using namespace Catch::clara;
using Opt = Catch::clara::Opt;
using dt = slsDetectorDefs::detectorType;

namespace test {
std::string hostname;
std::string detector_type;
std::string my_ip;
dt type;
auto GET = slsDetectorDefs::GET_ACTION;
auto PUT = slsDetectorDefs::PUT_ACTION;
} // namespace test
} // namespace sls

int main(int argc, char *argv[]) {
    sls::test::my_ip = "undefined";

    Catch::Session session;
    auto cli = session.cli() |
               sls::Opt(sls::test::hostname, "hostname")["-hn"]["--hostname"](
                   "Detector hostname for integration tests") |
               sls::Opt(sls::test::detector_type,
                        "detector_type")["-dt"]["--detector_type"](
                   "Detector type for integration tests") |
               sls::Opt(sls::test::my_ip,
                        "my_ip")["-hip"]["--host_ip"]("Host ip address");

    session.cli(cli);

    auto ret = session.applyCommandLine(argc, argv);
    if (ret) {
        return ret;
    }

    sls::test::type = slsDetectorDefs::GENERIC;
    if (!sls::test::detector_type.empty()) {
        sls::test::type = sls::StringTo<slsDetectorDefs::detectorType>(
            sls::test::detector_type);
    }

    return session.run();
}
