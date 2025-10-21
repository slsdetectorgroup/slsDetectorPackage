// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "CommandLineOptions.h"
#include "catch.hpp"
#include "sls/logger.h"
#include "sls/versionAPI.h"

#include <unistd.h>

namespace sls {

template <typename T, typename U> constexpr bool is_type() {
    return std::is_same_v<std::decay_t<U>, T>;
}

TEST_CASE("CommandLineOption construction", "[detector]") {
    CommandLineOptions s(AppType::SingleReceiver);
    REQUIRE(s.getTypeString() == "slsReceiver");
    REQUIRE(s.getVersion() ==
            std::string("slsReceiver Version: ") + APIRECEIVER);
    REQUIRE_NOTHROW(s.getHelpMessage());

    CommandLineOptions m(AppType::MultiReceiver);
    REQUIRE(m.getTypeString() == "slsMultiReceiver");
    REQUIRE(m.getVersion() ==
            std::string("slsMultiReceiver Version: ") + APIRECEIVER);
    REQUIRE_NOTHROW(m.getHelpMessage());

    CommandLineOptions f(AppType::FrameSynchronizer);
    REQUIRE(f.getTypeString() == "slsFrameSynchronizer");
    REQUIRE(f.getVersion() ==
            std::string("slsFrameSynchronizer Version: ") + APIRECEIVER);
    REQUIRE_NOTHROW(f.getHelpMessage());
}

TEST_CASE("Parse Help", "[detector]") {
    for (auto app : {AppType::SingleReceiver, AppType::MultiReceiver,
                     AppType::FrameSynchronizer}) {
        CommandLineOptions s(app);
        ParsedOptions opts = s.parse({"", "-h"});
        if (app == AppType::SingleReceiver) {
            REQUIRE_NOTHROW(std::get<CommonOptions>(opts).helpRequested);
        } else if (app == AppType::MultiReceiver) {
            REQUIRE_NOTHROW(std::get<MultiReceiverOptions>(opts).helpRequested);
        } else if (app == AppType::FrameSynchronizer) {
            REQUIRE_NOTHROW(std::get<FrameSyncOptions>(opts).helpRequested);
        }
    }
}

TEST_CASE("Validate common options", "[detector]") {
    std::string uidStr = std::to_string(getuid());

    for (auto app : {AppType::SingleReceiver, AppType::MultiReceiver,
                     AppType::FrameSynchronizer}) {
        CommandLineOptions s(app);
        REQUIRE_NOTHROW(s.parse({}));
        REQUIRE_NOTHROW(s.parse({"", "-v"}));
        REQUIRE_NOTHROW(s.parse({"", "-h"}));
        REQUIRE_NOTHROW(s.parse({"", "-h", "gdfg"})); // ignored extra args
        REQUIRE_NOTHROW(s.parse({"", "-p", "1955"}));
        REQUIRE_NOTHROW(s.parse({"", "-u", uidStr}));
        REQUIRE_NOTHROW(s.parse({"", "-p", "1234", "-u", uidStr}));
    }
}

TEST_CASE("Validate specific options", "[detector]") {
    std::string uidStr = std::to_string(getuid());

    CommandLineOptions s(AppType::SingleReceiver);
    REQUIRE_NOTHROW(s.parse({"", "-t", "1955"}));
    REQUIRE_THROWS(s.parse({"", "-c"}));
    REQUIRE_THROWS(s.parse({"", "-n", "2"}));
    REQUIRE_THROWS(s.parse({"", "-m", "2"}));

    for (auto app : {AppType::MultiReceiver, AppType::FrameSynchronizer}) {
        CommandLineOptions m(app);
        REQUIRE_NOTHROW(m.parse({"", "-c"}));
        REQUIRE_NOTHROW(m.parse({"", "-n", "2"}));
        REQUIRE_NOTHROW(
            m.parse({"", "-p", "1234", "-u", uidStr, "-c", "-n", "2"}));
        REQUIRE_THROWS(m.parse({"", "-t", "1955"}));
        REQUIRE_THROWS(m.parse({"", "-m", "2"}));
    }
}

TEST_CASE("Parse version and help", "[detector]") {
    for (auto app : {AppType::SingleReceiver, AppType::MultiReceiver,
                     AppType::FrameSynchronizer}) {
        CommandLineOptions s(app);
        auto opts = s.parse({});
        std::visit(
            [](const auto &o) {
                REQUIRE(o.versionRequested == false); // default
                REQUIRE(o.helpRequested == false);    // default
            },
            opts);

        opts = s.parse({"", "-v"});
        std::visit(
            [](const auto &o) {
                REQUIRE(o.versionRequested == true);
                REQUIRE(o.helpRequested == false);
            },
            opts);

        opts = s.parse({"", "-h"});
        std::visit(
            [](const auto &o) {
                REQUIRE(o.versionRequested == false);
                REQUIRE(o.helpRequested == true);
            },
            opts);

        opts = s.parse({"", "-h", "-v"});
        std::visit(
            [](const auto &o) {
                REQUIRE(o.versionRequested == true);
                REQUIRE(o.helpRequested == true);
            },
            opts);

        opts = s.parse({"", "-v", "-h"});
        std::visit(
            [](const auto &o) {
                REQUIRE(o.helpRequested == true);
                REQUIRE(o.versionRequested == true);
            },
            opts);

        opts = s.parse({"", "-v", "-h", "sdfsf"}); // ignores extra args
        std::visit(
            [](const auto &o) {
                REQUIRE(o.helpRequested == true);
                REQUIRE(o.versionRequested == true);
            },
            opts);
    }
}

TEST_CASE("Parse port and uid", "[detector]") {
    uid_t uid = getuid();
    std::string uidStr = std::to_string(uid);
    uid_t invalidUid = uid + 1000;
    std::string invalidUidStr = std::to_string(invalidUid);

    for (auto app : {AppType::SingleReceiver, AppType::MultiReceiver,
                     AppType::FrameSynchronizer}) {
        CommandLineOptions s(app);
        REQUIRE_THROWS(
            s.parse({"", "-p", "1234", "-u", invalidUidStr})); // invalid uid
        REQUIRE_THROWS(s.parse({"", "-p", "500"}));            // invalid port

        auto opts = s.parse({"", "-p", "1234", "-u", uidStr});
        std::visit(
            [&](const auto &o) {
                REQUIRE(o.port == 1234);
                REQUIRE(o.userid == uid);
            },
            opts);

        opts = s.parse({"", "-p", "5678"});
        std::visit(
            [](const auto &o) {
                REQUIRE(o.port == 5678);
                REQUIRE(o.userid == static_cast<uid_t>(-1)); // default
            },
            opts);

        opts = s.parse({});
        std::visit(
            [](const auto &o) {
                REQUIRE(o.port == 1954);                     // default
                REQUIRE(o.userid == static_cast<uid_t>(-1)); // default
            },
            opts);
    }
}

TEST_CASE("Parse num receivers and opt arg (Specific opt)", "[detector]") {
    for (auto app : {AppType::MultiReceiver, AppType::FrameSynchronizer}) {
        CommandLineOptions s(app);

        REQUIRE_THROWS(s.parse({"", "-n", "0"})); // invalid number of receivers
        REQUIRE_THROWS(s.parse({"", "-n", "1001"})); // exceeds max receivers
        REQUIRE_NOTHROW(s.parse({"", "-n", "10"}));  // valid

        auto opts = s.parse({""});
        std::visit(
            [](const auto &o) {
                using T = decltype(o);
                if constexpr (is_type<MultiReceiverOptions, T>()) {
                    REQUIRE(o.numReceivers == 1); // default
                    REQUIRE(o.callbackEnabled == false);

                } else if constexpr (is_type<FrameSyncOptions, T>()) {
                    REQUIRE(o.numReceivers == 1); // default
                    REQUIRE(o.printHeaders == false);
                }
            },
            opts);

        opts = s.parse({"", "-n", "5"});
        std::visit(
            [](const auto &o) {
                using T = decltype(o);
                if constexpr (is_type<MultiReceiverOptions, T>()) {
                    REQUIRE(o.numReceivers == 5);
                    REQUIRE(o.callbackEnabled == false); // default

                } else if constexpr (is_type<FrameSyncOptions, T>()) {
                    REQUIRE(o.numReceivers == 5);
                    REQUIRE(o.printHeaders == false); // default
                }
            },
            opts);

        opts = s.parse({"", "-c", "-n", "3"});
        std::visit(
            [](const auto &o) {
                using T = decltype(o);
                if constexpr (is_type<MultiReceiverOptions, T>()) {
                    REQUIRE(o.numReceivers == 3);
                    REQUIRE(o.callbackEnabled == true);

                } else if constexpr (is_type<FrameSyncOptions, T>()) {
                    REQUIRE(o.numReceivers == 3);
                    REQUIRE(o.printHeaders == true);
                }
            },
            opts);
    }
}

TEST_CASE("Parse deprecated options", "[detector]") {
    for (auto app : {AppType::SingleReceiver, AppType::MultiReceiver,
                     AppType::FrameSynchronizer}) {
        CommandLineOptions s(app);
        // argc 3 or 4, invalid
        REQUIRE_THROWS(s.parse({"", "1954"}));
        REQUIRE_THROWS(s.parse({
            "",
            "1954",
        }));
        // argc 3 or 4
        if (app == AppType::SingleReceiver) {
            REQUIRE_THROWS(
                s.parse({"", "1954", "1"})); // deprecated unsupported
        } else {
            REQUIRE_THROWS(s.parse({"", "1954", "1", "1", "-p",
                                    "1954"})); // mix deprecated and current
            REQUIRE_THROWS(
                s.parse({"", "1954", "1", "-c"})); // mix deprecated and current
            REQUIRE_THROWS(s.parse(
                {"", "1954", "1", "-n", "34"})); // mix deprecated and current
            REQUIRE_THROWS(s.parse({"", "110", "1954"})); // mix order
            REQUIRE_THROWS(s.parse({"", "1023", "10"}));  // privileged port
            REQUIRE_THROWS(s.parse({"", "2000", "0"})); // invalid num receivers
            REQUIRE_THROWS(
                s.parse({"", "2000", "1001"})); // invalid num receivers
            REQUIRE_THROWS(s.parse({"", "1954", "1", "2"})); // invalid 3rd opt

            REQUIRE_NOTHROW(s.parse({""}));
            REQUIRE_NOTHROW(s.parse({"", "1954", "1"}));
            REQUIRE_NOTHROW(s.parse({"", "1954", "1", "0"}));
            REQUIRE_NOTHROW(s.parse({"", "1954", "1", "1"}));

            // default
            auto opts = s.parse({""});
            std::visit(
                [](const auto &o) {
                    using T = decltype(o);
                    if constexpr (is_type<MultiReceiverOptions, T>()) {
                        REQUIRE(o.port == 1954);
                        REQUIRE(o.numReceivers == 1);
                        REQUIRE(o.callbackEnabled == false);

                    } else if constexpr (is_type<FrameSyncOptions, T>()) {
                        REQUIRE(o.port == 1954);
                        REQUIRE(o.numReceivers == 1);
                        REQUIRE(o.printHeaders == false); // default
                    }
                },
                opts);

            opts = s.parse({"", "1958", "10"});
            std::visit(
                [](const auto &o) {
                    using T = decltype(o);
                    if constexpr (is_type<MultiReceiverOptions, T>()) {
                        REQUIRE(o.port == 1958);
                        REQUIRE(o.numReceivers == 10);
                        REQUIRE(o.callbackEnabled == false); // default

                    } else if constexpr (is_type<FrameSyncOptions, T>()) {
                        REQUIRE(o.port == 1958);
                        REQUIRE(o.numReceivers == 10);
                        REQUIRE(o.printHeaders == false); // default
                    }
                },
                opts);

            opts = s.parse({"", "1958", "10", "1"});
            std::visit(
                [](const auto &o) {
                    using T = decltype(o);
                    if constexpr (is_type<MultiReceiverOptions, T>()) {
                        REQUIRE(o.port == 1958);
                        REQUIRE(o.numReceivers == 10);
                        REQUIRE(o.callbackEnabled == true); // default

                    } else if constexpr (is_type<FrameSyncOptions, T>()) {
                        REQUIRE(o.port == 1958);
                        REQUIRE(o.numReceivers == 10);
                        REQUIRE(o.printHeaders == true); // default
                    }
                },
                opts);
        }
    }

    // test function directly
    // nargs can be 1, 3 or 4
    REQUIRE_THROWS(CommandLineOptions::ParseDeprecated({"", ""}));
    REQUIRE_THROWS(CommandLineOptions::ParseDeprecated({"", "", "", "", ""}));

    // default
    auto [p, n, o] = CommandLineOptions::ParseDeprecated({""});
    REQUIRE(p == 1954);
    REQUIRE(n == 1);
    REQUIRE(o == false);

    std::tie(p, n, o) = CommandLineOptions::ParseDeprecated({"", "1955", "6"});
    REQUIRE(p == 1955);
    REQUIRE(n == 6);
    REQUIRE(o == false);

    std::tie(p, n, o) =
        CommandLineOptions::ParseDeprecated({"", "1955", "6", "1"});
    REQUIRE(p == 1955);
    REQUIRE(n == 6);
    REQUIRE(o == true);
}
} // namespace sls