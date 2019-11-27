
/*
This file is used to generate the command line binaries
(sls_detector_get/put/acquire/help). By defines in CMake 
we get the different files. 

*/
#include "CmdParser.h"
#include "CmdProxy.h"
#include "Detector.h"
#include "sls_detector_defs.h"
#include "versionAPI.h"
#include <cstring> //strcmp
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

#ifdef HELP
    int action = slsDetectorDefs::HELP_ACTION;
#endif

    // Check for --version in the arguments
    for (int i = 1; i < argc; ++i) {
        if (!(strcmp(argv[i], "--version")) || !(strcmp(argv[i], "-v"))) {
            int64_t tempval = APILIB;
            std::cout << argv[0] << " " << GITBRANCH << " (0x" << std::hex
                      << tempval << ")" << std::endl;
            return 0;
        }
    }

    sls::CmdParser parser;
    parser.Parse(argc, argv);

    // If we called sls_detector_acquire, add the acquire command
    if (action == slsDetectorDefs::READOUT_ACTION)
        parser.setCommand("acquire");

    if (parser.isHelp())
        action = slsDetectorDefs::HELP_ACTION;

    // Free shared memory should work also without a detector
    // if we have an option for verify in the detector constructor
    // we could avoid this but clutter the code
    if (parser.command() == "free" && action != slsDetectorDefs::HELP_ACTION) {
        if (parser.detector_id() != -1)
            std::cout << "Cannot free shared memory of sub-detector\n";
        else
            sls::freeSharedMemory(parser.multi_id());
        return 0;
    }

    try {
        // How big should this try block be?
        sls::Detector det(parser.multi_id());
        sls::CmdProxy proxy(&det);
        proxy.Call(parser.command(), parser.arguments(), parser.detector_id(),
                   action);
    } catch (const sls::RuntimeError &e) {
        // OK to catch and do nothing since this will print the error message
        // and command line app will anyway exit
    }
}