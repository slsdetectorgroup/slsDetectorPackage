
#include "CmdLineParser.h"
#include "CmdProxy.h"
#include "Detector.h"
#include "sls_detector_defs.h"
#include "versionAPI.h"
#include <cstring> //strcmp
#include <iostream>
int main(int argc, char *argv[]) {
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

    sls::CmdLineParser parser;
    parser.Parse(argc, argv);

    //If we called sls_detector_aquire, add the command
    if (action == slsDetectorDefs::READOUT_ACTION)
        parser.setCommand("acquire2");

    if (parser.isHelp())
        action = slsDetectorDefs::HELP_ACTION;

    if (parser.command() == "free" && action != slsDetectorDefs::HELP_ACTION){
        sls::freeSharedMemory(parser.multi_id(), parser.detector_id());
        return 0;
    }
    //TODO fix help

    sls::Detector det(parser.multi_id()); //This might fail on shmversion?
    sls::CmdProxy proxy(&det);

    try {
        auto cmd = proxy.Call(parser.command(), parser.arguments(),
                              parser.detector_id(), action);
        // TODO! move this check into CmdProxy
        if (!cmd.empty()) {
            std::cout << cmd
                      << " Unknown command, use list to list all commands\n";
        }
    } catch (const sls::RuntimeError &e) {
    }
}