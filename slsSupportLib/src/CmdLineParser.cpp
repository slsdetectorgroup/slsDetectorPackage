
#include "CmdLineParser.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <iterator>
#include <sstream>
//printing function for debugging
void CmdLineParser::Print() {
    std::cout << "\nCmdLineParser::Print()\n";
    std::cout << "\tmulti_id: " << multi_id_ << ", detector_id: " << detector_id_ << std::endl;
    std::cout << "\texecutable: " << executable_ << '\n';
    std::cout << "\tcommand: " << command_ << '\n';
    std::cout << "\tn_arguments: " << n_arguments() << '\n';
    std::cout << "\targuments: ";
    for (const auto &argument : arguments_) {
        std::cout << argument << " ";
    }
    std::cout << "\n\n";
};

void CmdLineParser::Parse(int argc, char *argv[]) {
    //first element of argv is the command used to call the executable ->skipping
    //and if this is the only command skip all
    executable_ = argv[0];
    if (argc > 1) {
        //second element is cmd string that needs to be decoded
        DecodeIdAndPosition(argv[1]);
        //The rest of the arguments goes into a vector for later processing
        for (int i = 2; i < argc; ++i) {
            arguments_.emplace_back(std::string(argv[i]));
        }
    }
};

void CmdLineParser::Parse(const std::string &s) {
    std::istringstream iss(s);
    auto it = std::istream_iterator<std::string>(iss);
    //read the first element and increment
    command_ = *it++;
    arguments_ = std::vector<std::string>(it, std::istream_iterator<std::string>());
    ;
    DecodeIdAndPosition(command_.c_str());
}

void CmdLineParser::DecodeIdAndPosition(const char *c) {
    bool contains_id = std::strchr(c, '-') != nullptr;
    bool contains_pos = std::strchr(c, ':') != nullptr;
    char tmp[100];

    if (contains_id && contains_pos) {
        int r = sscanf(c, "%d-%d:%s", &multi_id_, &detector_id_, tmp);
        if (r != 3) {
            throw(std::invalid_argument("Cannot decode client or detector id from: \"" + std::string(c) + "\"\n"));
        }
        command_ = tmp;
    } else if (contains_id && !contains_pos) {
        int r = sscanf(c, "%d-%s", &multi_id_, tmp);
        if (r != 2) {
            throw(std::invalid_argument("Cannot decode client id from: \"" + std::string(c) + "\"\n"));
        }
        command_ = tmp;
    } else if (!contains_id && contains_pos) {
        int r = sscanf(c, "%d:%s", &detector_id_, tmp);
        if (r != 2) {
            throw(std::invalid_argument("Cannot decode detector id from: \"" + std::string(c) + "\"\n"));
        }
        command_ = tmp;
    } else {
        command_ = c;
    }
}

std::vector<char *> CmdLineParser::argv() {
    std::vector<char *> vec;
    if (command_.empty()!=true){
        vec.push_back(&command_.front());
    }
    
    for (auto &arg : arguments_) {
        vec.push_back(&arg.front());
    }
    return vec;
}
