// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
/*

This class parses command line input or string input to extract the
multi_id, detector_id, command and arguments. It's used in the command
line binaries (sls_detector_get/put) to parse commands and in the
slsDetectorShared library to parse input from config files.

This class is fully internal to the project and NO guarantees are given
on the stability of the interface or implementation. This is also the
reason that the header file is not exposed.


*/

#ifndef CMD_LINE_PARSER_H
#define CMD_LINE_PARSER_H
#include <string>
#include <vector>

namespace sls {
class CmdParser {
  public:
    void Parse(int argc, const char *const argv[]);
    void Parse(std::string s);

    int multi_id() const noexcept { return multi_id_; };
    int detector_id() const noexcept { return detector_id_; };
    int receiver_id() const noexcept { return receiver_id_; };
    int n_arguments() const noexcept { return arguments_.size(); }
    const std::string &command() const noexcept { return command_; }
    void setCommand(std::string cmd) { command_ = cmd; }
    bool isHelp() const noexcept { return help_; }

    const std::string &executable() const noexcept { return executable_; }
    const std::vector<std::string> &arguments() const noexcept {
        return arguments_;
    };

  private:
    void DecodeIdAndPosition(std::string pre);
    void Reset(); // reset all private variables
    int multi_id_ = 0;
    int detector_id_ = -1;
    int receiver_id_ = -1;
    bool help_ = false;
    std::string command_;
    std::string executable_;
    std::vector<std::string> arguments_;
};

} // namespace sls
#endif // CMD_LINE_PARSER_H