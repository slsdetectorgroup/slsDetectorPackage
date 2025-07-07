// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <cstdint>
#include <string>
#include <variant>

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

ParsedOptions parseCommandLine(AppType app, int argc, char* argv[]);
int GetDeprecatedCommandLineOptions(int argc, char *argv[], uint16_t &startPort, uint16_t &numReceivers, bool &optionalArg);
void setEffectiveUID(uid_t uid);
std::string getTypeString(const AppType app);
std::string getVersion(AppType app);
std::string getHelpMessage(AppType app);
void setupSignalHandler(int signal, void (*handler)(int));
