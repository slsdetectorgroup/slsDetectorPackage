#include "Caller.h"
#include "CmdParser.h"
#include "inferAction.h"
#include "sls/Detector.h"
#include "sls/logger.h"
#include "sls/versionAPI.h"

#include <iostream>
int main(int argc, char *argv[]) {
    // To genereate sepereate binaries for put, get, acquire and help
#ifdef PUT
    int action = slsDetectorDefs::PUT_ACTION;
#endif

#ifdef GET
    int action = slsDetectorDefs::GET_ACTION;
#endif

#ifdef READOUT
    int action = slsDetectorDefs::READOUT_ACTION;
#endif

#ifdef READOUTZMQ
    int action = slsDetectorDefs::READOUT_ZMQ_ACTION;
#endif

#ifdef HELP
    int action = slsDetectorDefs::HELP_ACTION;
#endif
#ifdef INFER
    int action = -1;
#endif

    // Check for --version in the arguments
    for (int i = 1; i < argc; ++i) {
        if (!(strcmp(argv[i], "--version")) || !(strcmp(argv[i], "-v"))) {
            std::cout << argv[0] << " " << APILIB << std::endl;
            return 0;
        }
    }

    sls::CmdParser parser;
    parser.Parse(argc, argv);

    if (action == slsDetectorDefs::READOUT_ACTION ||
        action == slsDetectorDefs::READOUT_ZMQ_ACTION)
        parser.setCommand("acquire");

    if (parser.isHelp())
        action = slsDetectorDefs::HELP_ACTION;
    else {
        // Free shared memory should work also without a detector
        // if we have an option for verify in the detector constructor
        // we could avoid this but clutter the code
        if (parser.command() == "free") {
            if (parser.detector_id() != -1)
                std::cout << "Cannot free shared memory of sub-detector\n";
            else
                sls::freeSharedMemory(parser.multi_id());
            return 0;
        }
    }

    if (parser.command() == "config" && action == slsDetectorDefs::PUT_ACTION) {
        sls::freeSharedMemory(parser.multi_id());
    }

    sls::InferAction inferAction = sls::InferAction();

    try {
        if (action == -1) {
            action = inferAction.infer(parser);
            std::string actionString =
                (action == slsDetectorDefs::GET_ACTION) ? "GET" : "PUT";
            std::cout << "inferred action: " << actionString << std::endl;
        }

        std::unique_ptr<sls::Detector> d{nullptr};
        if (action != slsDetectorDefs::HELP_ACTION) {
            d = sls::make_unique<sls::Detector>(parser.multi_id());
        }
        sls::Caller c(d.get());

        c.call(parser.command(), parser.arguments(), parser.detector_id(),
               action, std::cout, parser.receiver_id());
    } catch (sls::RuntimeError &e) {
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}