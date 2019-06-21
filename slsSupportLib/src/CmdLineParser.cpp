
#include "CmdLineParser.h"
#include "sls_detector_defs.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <iterator>
#include <sstream>

void CmdLineParser::Print() {
    std::cout << "\nCmdLineParser::Print()\n";
    std::cout << "\tmulti_id: " << multi_id_
              << ", detector_id: " << detector_id_ << std::endl;
    std::cout << "\texecutable: " << executable_ << '\n';
    std::cout << "\tcommand: " << command_ << '\n';
    std::cout << "\tn_arguments: " << n_arguments() << '\n';
    std::cout << "\targuments: ";
    for (const auto &argument : arguments_) {
        std::cout << argument << " ";
    }
    std::cout << "\n\n";
};

void CmdLineParser::Parse(int argc, const char *const argv[]) {
    executable_ = argv[0]; // first arg is calling binary
    if (argc > 1) {
        std::string s = argv[1];
        for (int i = 2; i < argc; ++i) {
            s += " ";
            s += argv[i];
        }
        Parse(s);
    }
}

void CmdLineParser::Parse(const std::string &s) {
    std::istringstream iss(s);
    auto it = std::istream_iterator<std::string>(iss);
    arguments_ =
        std::vector<std::string>(it, std::istream_iterator<std::string>());
    auto old_size = arguments_.size();
    arguments_.erase(std::remove_if(begin(arguments_), end(arguments_),
                                     [](const std::string &item) {
                                         if (item == "-h" || item == "--help")
                                             return true;
                                         return false;
                                     }),
                      end(arguments_));
    if (old_size - arguments_.size() > 0)
        help_ = true;
    if(!arguments_.empty()){
        command_ = arguments_[0];
        arguments_.erase(begin(arguments_));
    }
    DecodeIdAndPosition(command_.c_str());
}

void CmdLineParser::DecodeIdAndPosition(const char *c) {
    bool contains_id = std::strchr(c, '-') != nullptr;
    bool contains_pos = std::strchr(c, ':') != nullptr;
    char tmp[100];

    if (contains_id && contains_pos) {
        int r = sscanf(c, "%d-%d:%s", &multi_id_, &detector_id_, tmp);
        if (r != 3) {
            throw(sls::RuntimeError(
                "Cannot decode client or detector id from: \"" +
                std::string(c) + "\"\n"));
        }
        command_ = tmp;
    } else if (contains_id && !contains_pos) {
        int r = sscanf(c, "%d-%s", &multi_id_, tmp);
        if (r != 2) {
            throw(sls::RuntimeError("Cannot decode client id from: \"" +
                                    std::string(c) + "\"\n"));
        }
        command_ = tmp;
    } else if (!contains_id && contains_pos) {
        int r = sscanf(c, "%d:%s", &detector_id_, tmp);
        if (r != 2) {
            throw(sls::RuntimeError("Cannot decode detector id from: \"" +
                                    std::string(c) + "\"\n"));
        }
        command_ = tmp;
    } else {
        command_ = c;
    }
}

std::vector<const char *> CmdLineParser::argv() const {
    std::vector<const char *> vec;
    if (command_.empty() != true) {
        vec.push_back(&command_.front());
    }
    for (auto &arg : arguments_) {
        vec.push_back(&arg.front());
    }
    return vec;
}
