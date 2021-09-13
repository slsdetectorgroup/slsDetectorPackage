
#include "CmdParser.h"
#include "sls/sls_detector_defs.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <iterator>
#include <sstream>

namespace sls {

void CmdParser::Print() {
    std::cout << "\nCmdParser::Print()\n";
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

void CmdParser::Parse(int argc, const char *const argv[]) {
    Reset();
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

void CmdParser::Parse(std::string s) {
    //taking s by value we can modify it. 
    Reset();

    //Are we looking at -h --help?
    

    auto pos = s.find_first_not_of("0123456789:- ");
    if (pos!=0){
        auto pre = s.substr(0, pos);
        pre.erase(std::remove(pre.begin(), pre.end(), ' '), pre.end());
        s.erase(0, pos);
        DecodeIdAndPosition(pre.c_str());
    }
    

    //Load string 
    std::istringstream iss(s);
    auto it = std::istream_iterator<std::string>(iss);
    command_ = *it++; //First arg is the comand to run

    arguments_ =
        std::vector<std::string>(it, std::istream_iterator<std::string>());
    auto old_size = arguments_.size();
    arguments_.erase(std::remove_if(begin(arguments_), end(arguments_),
                                    [](const std::string &item) {
                                        return (item == "-h" ||
                                                item == "--help");
                                    }),
                     end(arguments_));
    if (old_size - arguments_.size() > 0)
        help_ = true;
    // if (!arguments_.empty()) {
    //     command_ = arguments_[0];
    //     arguments_.erase(begin(arguments_));
    // }
    //allow comma sep
    for (auto& arg : arguments_){
        if (arg.back() == ',')
            arg.pop_back();
    }

    // DecodeIdAndPosition(command_.c_str());
}

void CmdParser::DecodeIdAndPosition(const char *c) {
    bool contains_id = std::strchr(c, '-') != nullptr;
    bool contains_pos = std::strchr(c, ':') != nullptr;

    // if (!isdigit(c[0])){
    //     // The first char is not a digit which means we have a command.
    //     // or at least a candidate, calling could still fail 
    //     command_ = c;
    //     return;
    // }

    if (contains_id && contains_pos) {
        int r = sscanf(c, "%d-%d:", &multi_id_, &detector_id_);
        if (r != 2) {
            throw(sls::RuntimeError(
                "Cannot decode client or detector id from: \"" +
                std::string(c) + "\"\n"));
        }
        // command_ = tmp;
    } else if (contains_id && !contains_pos) {
        int r = sscanf(c, "%d-", &multi_id_);
        if (r != 1) {
            throw(sls::RuntimeError("Cannot decode client id from: \"" +
                                    std::string(c) + "\"\n"));
        }
        // command_ = tmp;
    } else if (!contains_id && contains_pos) {
        int r = sscanf(c, "%d:", &detector_id_);
        if (r != 1) {
            throw(sls::RuntimeError("Cannot decode detector id from: \"" +
                                    std::string(c) + "\"\n"));
        }
        // command_ = tmp;
    } else {
        // command_ = c;
    }
}

// void CmdParser::DecodeIdAndPosition(const char *c) {
//     bool contains_id = std::strchr(c, '-') != nullptr;
//     bool contains_pos = std::strchr(c, ':') != nullptr;
//     char tmp[100];

//     if (contains_id && contains_pos) {
//         int r = sscanf(c, "%d-%d:%s", &multi_id_, &detector_id_, tmp);
//         if (r != 3) {
//             throw(sls::RuntimeError(
//                 "Cannot decode client or detector id from: \"" +
//                 std::string(c) + "\"\n"));
//         }
//         command_ = tmp;
//     } else if (contains_id && !contains_pos) {
//         int r = sscanf(c, "%d-%s", &multi_id_, tmp);
//         if (r != 2) {
//             throw(sls::RuntimeError("Cannot decode client id from: \"" +
//                                     std::string(c) + "\"\n"));
//         }
//         command_ = tmp;
//     } else if (!contains_id && contains_pos) {
//         int r = sscanf(c, "%d:%s", &detector_id_, tmp);
//         if (r != 2) {
//             throw(sls::RuntimeError("Cannot decode detector id from: \"" +
//                                     std::string(c) + "\"\n"));
//         }
//         command_ = tmp;
//     } else {
//         command_ = c;
//     }
// }

std::vector<const char *> CmdParser::argv() const {
    std::vector<const char *> vec;
    if (!command_.empty()) {
        vec.push_back(&command_.front());
    }
    for (auto &arg : arguments_) {
        vec.push_back(&arg.front());
    }
    return vec;
}

std::string CmdParser::cli_line() const {
    std::ostringstream os;
    os << command_;
    for (const auto &arg : arguments_)
        os << " " << arg;
    return os.str();
}

void CmdParser::Reset() {
    multi_id_ = 0;
    detector_id_ = -1;
    help_ = false;
    command_.clear();
    executable_.clear();
    arguments_.clear();
}

} // namespace sls