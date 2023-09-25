#include "sls/Detector.h"
#include "CmdParser.h"
#include "Caller.h"
#include  "sls/logger.h"

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
    std::cout << "Experimental command parsing\n";

    sls::CmdParser parser;
    parser.Parse(argc, argv);
    
//    int default_action = -1;
    sls::Detector d(parser.multi_id());
    sls::Caller c(&d);


    std::cout << "call with action get\n"<<std::endl;
    try
    {
    c.call(parser, action);
    }
    catch(sls::RuntimeError& e){}
    catch ( const std::exception& e ){ LOG(sls::logERROR) << e.what() << std::endl;}

//
//    std::cout << "\ncall with 'infer action'\n";
//    try
//    {
//    c.call(parser, default_action);
//    }
//    catch(sls::RuntimeError& e){}
//    catch ( const std::exception& e ){LOG(sls::logERROR)  << e.what() << std::endl;}




}