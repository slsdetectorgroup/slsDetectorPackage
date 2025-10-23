// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <cstdint>
#include <getopt.h>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

enum class AppType { MultiReceiver, SingleReceiver, FrameSynchronizer };

struct CommonOptions {
    uint16_t port = -1;
    uid_t userid = -1;
    bool versionRequested = false;
    bool helpRequested = false;
};

struct MultiReceiverOptions : CommonOptions {
    uint16_t numReceivers = 1;
    bool callbackEnabled = false;
};

struct FrameSyncOptions : CommonOptions {
    uint16_t numReceivers = 1;
    bool printHeaders = false;
};

using ParsedOptions =
    std::variant<CommonOptions, MultiReceiverOptions, FrameSyncOptions>;

class CommandLineOptions {
  public:
    explicit CommandLineOptions(AppType app);
    ParsedOptions parse(const std::vector<std::string> &args); // for testing
    ParsedOptions parse(int argc, char *argv[]);
    std::string getTypeString() const;
    std::string getVersion() const;
    std::string getHelpMessage() const;
    static void setEffectiveUID(uid_t uid);
    static std::tuple<uint16_t, uint16_t, bool>
    ParseDeprecated(const std::vector<std::string> &args);

  private:
    AppType appType_;
    std::string optString_;
    std::vector<option> longOptions_;
    std::vector<option> buildOptionList() const;
    std::string buildOptString() const;

    static uint16_t parsePort(const char *optarg);
    static uint16_t parseNumReceivers(const char *optarg);
    static uid_t parseUID(const char *optarg);
    void handleCommonOption(int opt, const char *optarg, CommonOptions &base);
    void handleAppSpecificOption(int opt, const char *optarg,
                                 CommonOptions &base,
                                 MultiReceiverOptions &multi,
                                 FrameSyncOptions &frame);

    static constexpr uint16_t MAX_RECEIVERS = 1000;
};
