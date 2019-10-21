#include "multiSlsDetectorClient.h"
#include "CmdProxy.h"
#include "Detector.h"
#include "multiSlsDetector.h"
#include "slsDetectorCommand.h"
#include "sls_detector_exceptions.h"

#include <memory>

multiSlsDetectorClient::multiSlsDetectorClient(int argc, char *argv[],
                                               int action,
                                               multiSlsDetector *myDetector,
                                               std::ostream &output)
    : action_(action), detPtr(myDetector), os(output) {
    parser.Parse(argc, argv);
    runCommand();
}

multiSlsDetectorClient::multiSlsDetectorClient(const std::string &args,
                                               int action,
                                               multiSlsDetector *myDetector,
                                               std::ostream &output)
    : action_(action), detPtr(myDetector), os(output) {
    parser.Parse(args);
    runCommand();
}

void multiSlsDetectorClient::runCommand() {
    if (parser.isHelp())
        action_ = slsDetectorDefs::HELP_ACTION;
    bool verify = true;
    bool update = true;
    if (action_ == slsDetectorDefs::PUT_ACTION && parser.command().empty()) {
        os << "Wrong usage - should be: " << parser.executable()
           << "[id-][pos:]channel arg" << std::endl;
        os << std::endl;
        return;
    };
    if (action_ == slsDetectorDefs::GET_ACTION && parser.command().empty()) {
        os << "Wrong usage - should be: " << parser.executable()
           << "[id-][pos:]channel arg" << std::endl;
        os << std::endl;
        return;
    };

    if (action_ == slsDetectorDefs::READOUT_ACTION &&
        parser.detector_id() != -1) {
        os << "detector_id: " << parser.detector_id()
           << " ,readout of individual detectors is not allowed!" << std::endl;
        return;
    }

    // special commands
    if (parser.command() == "free") {
        multiSlsDetector::freeSharedMemory(parser.multi_id(),
                                           parser.detector_id());
        return;
    } // get user details without verify sharedMultiSlsDetector version
    else if ((parser.command() == "user") &&
             (action_ == slsDetectorDefs::GET_ACTION)) {
        verify = false;
        update = false;
    }

    // create multiSlsDetector class if required
    std::unique_ptr<multiSlsDetector> localDet;
    if (detPtr == nullptr) {
        try {
            localDet = sls::make_unique<multiSlsDetector>(parser.multi_id(),
                                                          verify, update);
            detPtr = localDet.get();
        } catch (const sls::RuntimeError &e) {
            /*os << e.GetMessage() << std::endl;*/
            return;
        } catch (...) {
            os << " caught exception\n";
            return;
        }
    }

    if (parser.detector_id() >= static_cast<int>(detPtr->size())) {
        os << "position " << parser.detector_id() << " is out of bounds (max " << detPtr->size() << ").\n";
        return;
    }

    // Call CmdProxy which execute the command if it exists, on success
    // returns an empty string If the command is not in CmdProxy but
    // deprecated the new command is returned
    if (action_ != slsDetectorDefs::READOUT_ACTION) {
        int multi_id = 0;
        if (detPtr != nullptr)
            multi_id = detPtr->getMultiId();
        sls::Detector d(multi_id);
        sls::CmdProxy proxy(&d);
        auto cmd = proxy.Call(parser.command(), parser.arguments(),
                              parser.detector_id(), action_, os);
        if (cmd.empty()) {
            return;
        } else {
            parser.setCommand(cmd);
        }
    }

    // call multi detector command line
    slsDetectorCommand myCmd(detPtr);
    std::string answer =
        myCmd.executeLine(parser.n_arguments() + 1, parser.argv().data(),
                          action_, parser.detector_id());

    if (parser.multi_id() != 0)
        os << parser.multi_id() << '-';
    if (parser.detector_id() != -1)
        os << parser.detector_id() << ':';

    if (action_ != slsDetectorDefs::READOUT_ACTION) {
        os << parser.command() << " ";
    }
    os << answer << std::endl;
}