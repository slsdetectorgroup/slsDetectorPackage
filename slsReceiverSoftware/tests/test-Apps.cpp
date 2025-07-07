// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "catch.hpp"
#include "CommandLineOptions.h"
#include "sls/versionAPI.h"

namespace sls {

    TEST_CASE("CommandLineOption construction", "[.rxcmdcall]") {
        CommandLineOptions s(AppType::SingleReceiver);
        REQUIRE(s.getTypeString() == "slsReceiver");
        REQUIRE(s.getVersion() == std::string("slsReceiver Version: ") + APIRECEIVER);
        REQUIRE_NOTHROW(s.getHelpMessage());

        CommandLineOptions m(AppType::MultiReceiver);
        REQUIRE(m.getTypeString() == "slsMultiReceiver");
        REQUIRE(m.getVersion() == std::string("slsMultiReceiver Version: ") + APIRECEIVER);
        REQUIRE_NOTHROW(m.getHelpMessage());

        CommandLineOptions f(AppType::FrameSynchronizer);
        REQUIRE(f.getTypeString() == "slsFrameSynchronizer");
        REQUIRE(f.getVersion() == std::string("slsFrameSynchronizer Version: ") + APIRECEIVER);
        REQUIRE_NOTHROW(f.getHelpMessage());
    }

    TEST_CASE("Validate slsReceiver options", "[.rxcmdcall]") {
        CommandLineOptions s(AppType::SingleReceiver);
        // valid options
        REQUIRE_NOTHROW(s.parse({}));
        REQUIRE_NOTHROW(s.parse({"", "-v"}));
        REQUIRE_NOTHROW(s.parse({"", "-h"}));
        REQUIRE_NOTHROW(s.parse({"", "-h", "gdfg"})); // ignored extra args
        REQUIRE_NOTHROW(s.parse({"", "-p", "1955"}));
        REQUIRE_NOTHROW(s.parse({"", "-u", "1001"}));
        REQUIRE_NOTHROW(s.parse({"", "-p", "1234", "-u", "1001"}));
        REQUIRE_NOTHROW(s.parse({"", "-t", "1955"}));
        // invalid options
        REQUIRE_THROWS(s.parse({"", "-c"}));
        REQUIRE_THROWS(s.parse({"", "-n", "2"}));
        REQUIRE_THROWS(s.parse({"", "-m", "2"}));
    }

    TEST_CASE("Validate slsMultiReceiver options", "[.rxcmdcall]") {
        CommandLineOptions s(AppType::MultiReceiver);
        // valid options
        REQUIRE_NOTHROW(s.parse({}));
        REQUIRE_NOTHROW(s.parse({"", "-v"}));
        REQUIRE_NOTHROW(s.parse({"", "-h"}));
        REQUIRE_NOTHROW(s.parse({"", "-h", "gdfg"})); // ignored extra args
        REQUIRE_NOTHROW(s.parse({"", "-p", "1955"}));
        REQUIRE_NOTHROW(s.parse({"", "-u", "1001"}));
        REQUIRE_NOTHROW(s.parse({"", "-p", "1234", "-u", "1001"}));
        REQUIRE_NOTHROW(s.parse({"", "-c"}));
        REQUIRE_NOTHROW(s.parse({"", "-n", "2"}));
        REQUIRE_NOTHROW(s.parse({"", "-p", "1234", "-u", "1001", "-c", "-n", "2"}));
        // invalid options
        REQUIRE_THROWS(s.parse({"", "-t", "1955"}));
        REQUIRE_THROWS(s.parse({"", "-m", "2"}));
    }

    TEST_CASE("Validate slsFrameSynchronizer options", "[.rxcmdcall]") {
        CommandLineOptions s(AppType::FrameSynchronizer);
        // valid options
        REQUIRE_NOTHROW(s.parse({}));
        REQUIRE_NOTHROW(s.parse({"", "-v"}));
        REQUIRE_NOTHROW(s.parse({"", "-h"}));
        REQUIRE_NOTHROW(s.parse({"", "-h", "gdfg"})); // ignored extra args
        REQUIRE_NOTHROW(s.parse({"", "-p", "1955"}));
        REQUIRE_NOTHROW(s.parse({"", "-u", "1001"}));
        REQUIRE_NOTHROW(s.parse({"", "-p", "1234", "-u", "1001"}));
        REQUIRE_NOTHROW(s.parse({"", "-c"}));
        REQUIRE_NOTHROW(s.parse({"", "-n", "2"}));
        REQUIRE_NOTHROW(s.parse({"", "-p", "1234", "-u", "1001", "-c", "-n", "2"}));
        // invalid options
        REQUIRE_THROWS(s.parse({"", "-t", "1955"}));
        REQUIRE_THROWS(s.parse({"", "-m", "2"}));
    }

    TEST_CASE("Parse version and help", "[.rxcmdcall]") {
        CommandLineOptions s(AppType::SingleReceiver);
        auto opts = s.parse({});
        auto &o = std::get<CommonOptions>(opts);
        REQUIRE(o.versionRequested == false);
        REQUIRE(o.helpRequested == false);

        opts = s.parse({"", "-v"});
        o = std::get<CommonOptions>(opts);
        REQUIRE(o.versionRequested == true);
        REQUIRE(o.helpRequested == false);

        opts = s.parse({"", "-h"});
        o = std::get<CommonOptions>(opts);
        REQUIRE(o.versionRequested == false);
        REQUIRE(o.helpRequested == true);

        opts = s.parse({"", "-h", "-v"});
        o = std::get<CommonOptions>(opts);
        REQUIRE(o.versionRequested == false); // because it exits after help
        REQUIRE(o.helpRequested == true);

        opts = s.parse({"", "-v", "-h"});
        o = std::get<CommonOptions>(opts);
        REQUIRE(o.helpRequested == false); // because it exits after version
        REQUIRE(o.versionRequested == true);

        opts = s.parse({"", "-v", "-h", "sdfsf"}); // should ignore extra args
        o = std::get<CommonOptions>(opts);
        REQUIRE(o.helpRequested == false); // because it exits after version
        REQUIRE(o.versionRequested == true);
    }

    TEST_CASE("Parse port and uid", "[.rxcmdcall]") {
        for (auto app : {AppType::SingleReceiver, AppType::MultiReceiver, AppType::FrameSynchronizer}) {
            CommandLineOptions s(app);
            REQUIRE_THROWS(s.parse({"", "-p", "1234", "-u", "1000"})); // invalid uid
            REQUIRE_THROWS(s.parse({"", "-p", "500"})); // invalid port
            
            auto opts = s.parse({"", "-p", "1234", "-u", "1001"});
            auto &o = std::get<CommonOptions>(opts);
            REQUIRE(o.port == 1234);
            REQUIRE(o.userid == 1001);

            opts = s.parse({"", "-p", "5678"});
            o = std::get<CommonOptions>(opts);
            REQUIRE(o.port == 5678);
            REQUIRE(o.userid == static_cast<uid_t>(-1)); // default value

            opts = s.parse({});
            o = std::get<CommonOptions>(opts);
            REQUIRE(o.port == 1954); // default value
            REQUIRE(o.userid == static_cast<uid_t>(-1)); // default value
        }
    }

    TEST_CASE("Parse number of receivers and optional arg", "[.rxcmdcall]") {
        CommandLineOptions s(AppType::MultiReceiver);
        // invalid options
        REQUIRE_THROWS(s.parse({"", "-n", "0"})); // invalid number of receivers
        REQUIRE_THROWS(s.parse({"", "-n", "1001"})); // exceeds max receivers
        // valid options
        REQUIRE_NOTHROW(s.parse({"", "-n", "10"})); // valid number of receivers

        auto opts = s.parse({"", "-n", "5"});
        auto &m = std::get<MultiReceiverOptions>(opts);
        REQUIRE(m.numReceivers == 5);
        REQUIRE(m.callbackEnabled == false); // default value

        opts = s.parse({"", "-c", "-n", "3"});
        m = std::get<MultiReceiverOptions>(opts);
        REQUIRE(m.numReceivers == 3);
        REQUIRE(m.callbackEnabled == true);
    }
}