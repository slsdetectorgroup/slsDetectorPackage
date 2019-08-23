#pragma once
#include <iostream>
#include "CmdLineParser.h"

class multiSlsDetector;

class multiSlsDetectorClient {
  public:
    multiSlsDetectorClient(int argc, char *argv[], int action,
                           multiSlsDetector *myDetector = nullptr,
                           std::ostream &output = std::cout);
    multiSlsDetectorClient(const std::string &args, int action,
                           multiSlsDetector *myDetector = nullptr,
                           std::ostream &output = std::cout);

  private:
    int action_;
    CmdLineParser parser;
    multiSlsDetector *detPtr = nullptr;
    std::ostream &os;

    void runCommand();
};
