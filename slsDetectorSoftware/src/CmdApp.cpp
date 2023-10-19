#include "sls/Detector.h"
#include "CmdParser.h"
#include "Caller.h"
#include  "sls/logger.h"
#include "inferAction.h"

#include <iostream>
int main(int argc, char *argv[]){
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

#ifdef HELP
    int action = slsDetectorDefs::HELP_ACTION;
#endif
#ifdef INFER
    int action = -1;
#endif
    std::cout << "Experimental command parsing\n";

    sls::CmdParser parser;
    parser.Parse(argc, argv);
    if (action == slsDetectorDefs::READOUT_ACTION)
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
    sls::Detector d(parser.multi_id());
    sls::Caller c(&d);
    sls::InferAction inferAction = sls::InferAction();


    try
    {
        if (action == -1){
            action = inferAction.infer(parser);
            std::string actionString= (action==slsDetectorDefs::GET_ACTION) ? "GET" : "PUT";
            std::cout << "inferred action: " << actionString << std::endl;
        }

        c.call(parser, action);
    }
    catch(sls::RuntimeError& e){}
    catch ( const std::exception& e ){ LOG(sls::logERROR) << e.what() << std::endl;}




}