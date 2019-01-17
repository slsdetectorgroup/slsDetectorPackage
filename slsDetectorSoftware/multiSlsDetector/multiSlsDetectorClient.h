#pragma once
#include <iostream>
#include <string>

#include "CmdLineParser.h"
#include "container_utils.h"
#include "string_utils.h"
#include "multiSlsDetector.h"
#include "slsDetectorCommand.h"
#include "sls_detector_exceptions.h"

#include <cstdlib>
#include <memory>

inline int dummyCallback(detectorData *d, int p, void *) {
    std::cout << "got data " << p << std::endl;
    return 0;
};

class multiSlsDetectorClient {
  public:
    multiSlsDetectorClient(int argc, char *argv[], int action, multiSlsDetector *myDetector = nullptr):
    action_(action),
    detPtr(myDetector){
        parser.Parse(argc, argv);
        runCommand();

    }
    multiSlsDetectorClient(const std::string& args, int action, multiSlsDetector *myDetector = nullptr):
    action_(action),
    detPtr(myDetector){
        parser.Parse(args);
        runCommand();
    }
    private:
     int action_;
     CmdLineParser parser;
     multiSlsDetector* detPtr = nullptr;

    void runCommand(){
        bool verify = true;
        bool update = true;
        if (action_ == slsDetectorDefs::PUT_ACTION && parser.n_arguments() == 0) {
            std::cout << "Wrong usage - should be: " << parser.executable() << "[id-][pos:]channel arg" << std::endl;
            std::cout << std::endl;
            return;
        };
        if (action_ == slsDetectorDefs::GET_ACTION && parser.command().empty()) {
            std::cout << "Wrong usage - should be: " << parser.executable() << "[id-][pos:]channel arg" << std::endl;
            std::cout << std::endl;
            return;
        };

        if (action_ == slsDetectorDefs::READOUT_ACTION && parser.detector_id() != -1) {
            std::cout << "detector_id: " << parser.detector_id() << " ,readout of individual detectors is not allowed!" << std::endl;
            return;
        }

        // special commands
        if (parser.command() == "free") {
            multiSlsDetector::freeSharedMemory(parser.multi_id(), parser.detector_id());
            return;
        } // get user details without verify sharedMultiSlsDetector version
        else if ((parser.command() == "user") && (action_ == slsDetectorDefs::GET_ACTION)) {
            verify = false;
            update = false;
        }

        //std::cout<<"id:"<<id<<" pos:"<<pos<<std::endl;
        // create multiSlsDetector class if required
        std::unique_ptr<multiSlsDetector> localDet;
        if (detPtr == nullptr) {
            try {
                localDet = sls::make_unique<multiSlsDetector>(parser.multi_id(), verify, update);
                detPtr = localDet.get();
            } catch (const SlsDetectorPackageExceptions &e) {
                /*std::cout << e.GetMessage() << std::endl;*/
                return;
            } catch (...) {
                std::cout << " caught exception\n";
                return;
            }
        }
        if (parser.detector_id() >= detPtr->getNumberOfDetectors()) {
            std::cout << "position is out of bounds.\n";
            return;
        }

        // call multi detector command line
        slsDetectorCommand myCmd(detPtr);
        std::string answer = myCmd.executeLine(parser.n_arguments()+1, parser.argv().data(), action_, parser.detector_id());

        if (parser.multi_id()!=0)
            std::cout << parser.multi_id() << '-';
        if (parser.detector_id() != -1)
            std::cout << parser.detector_id() << ':';

        if (action_ != slsDetectorDefs::READOUT_ACTION) {
            std::cout << parser.command() << " ";
        }
        std::cout << answer << std::endl;
    }
};
