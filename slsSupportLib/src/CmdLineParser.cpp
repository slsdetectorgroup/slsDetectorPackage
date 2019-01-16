
#include "CmdLineParser.h"
#include <cstdio>
#include <cstring>
#include <sstream>
#include <iterator>
#include <iostream>
//printing function for debugging
void CmdLineParser::Print()
{
    std::cout << "\nCmdLineParser::Print()\n";
    std::cout << "\tmulti_id: " << multi_id_ << ", detector_id: " << detector_id_ << std::endl;
    std::cout << "\tcommand: " << command_ << std::endl;
    std::cout << "\targuments: ";
    for (size_t i = 0; i < arguments_.size(); ++i) {
        std::cout << arguments_[i] << " ";
    }
    std::cout << "\n\n";
};

void CmdLineParser::Parse(int argc, char* argv[])
{
    //first element of argv is the command used to call the executable ->skipping
    //and if this is the only command skip all
    if (argc > 1) {
        //second element is cmd string that needs to be decoded
        DecodeIdAndPosition(argv[1]);
        //The rest of the arguments goes into a vector for later processing
        for (int i = 2; i < argc; ++i)
            arguments_.push_back(std::string(argv[i]));
    }
};

void CmdLineParser::Parse(std::string s){
  std::istringstream iss(s);
  auto it =  std::istream_iterator<std::string>(iss);
  //read the first element and increment
  command_ = *it++;
  arguments_ =  std::vector<std::string>(it, std::istream_iterator<std::string>());;
  DecodeIdAndPosition(command_.c_str());
}

void CmdLineParser::DecodeIdAndPosition(const char* c)
{
    bool contains_id  = std::strchr(c, '-');
    bool contains_pos = std::strchr(c, ':');
    char tmp[100];

    if (contains_id && contains_pos) {
        int r = sscanf(c, "%d-%d:%s", &multi_id_, &detector_id_, tmp);
        if (r != 3)
            throw(std::invalid_argument("Cannot decode client or detector id from: \"" + std::string(c) + "\"\n"));
        command_ = tmp;
    } else if (contains_id && !contains_pos) {
        int r = sscanf(c, "%d-%s", &multi_id_, tmp);
        if (r != 2)
            throw(std::invalid_argument("Cannot decode client id from: \"" + std::string(c) + "\"\n"));
        command_ = tmp;
    } else if (!contains_id && contains_pos) {
        int r = sscanf(c, "%d:%s", &detector_id_, tmp);
        if (r != 2)
            throw(std::invalid_argument("Cannot decode detector id from: \"" + std::string(c) + "\"\n"));
        command_ = tmp;
    } else {
        command_ = c;
    }
}
