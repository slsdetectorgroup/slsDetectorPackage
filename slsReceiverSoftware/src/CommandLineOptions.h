// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <cstdint>
#include <getopt.h>
#include <string>
#include <variant>
#include <vector>

enum class AppType {
    MultiReceiver,
    SingleReceiver,
    FrameSynchronizer
};


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

using ParsedOptions = std::variant<CommonOptions, MultiReceiverOptions, FrameSyncOptions>;

class CommandLineOptions {
  public:
    constexpr explicit CommandLineOptions(AppType app) : appType_(app) {}
    ParsedOptions parse(int argc, char *argv[]);
    std::string getTypeString();
    std::string getVersion();
    std::string getHelpMessage();
    static void setupSignalHandler(int signal, void (*handler)(int));
    static void setEffectiveUID(uid_t uid);

  private:
    AppType appType_;
    std::vector<option> buildOptionList() const;
    std::string buildOptString() const;

    void handleCommonOption(int opt, const char *optarg, CommonOptions &base);
    void handleAppSpecificOption(int opt, const char *optarg,
                                 CommonOptions &base,
                                 MultiReceiverOptions &multi,
                                 FrameSyncOptions &frame);

    static int GetDeprecated(int argc, char *argv[], uint16_t &startPort,
                             uint16_t &numReceivers, bool &optionalArg);

    static constexpr uint16_t MAX_RECEIVERS = 1000;
};
