#include "sls/Detector.h"
#include "CmdParser.h"
#include "Caller.h"
#include  "sls/logger.h"

#include <iostream>
int main(int argc, char *argv[]){

    std::cout << "Experimental command parsing\n";

    sls::CmdParser parser;
    parser.Parse(argc, argv);
    
    int default_action = -1;
    sls::Detector d(parser.multi_id());
    sls::Caller c(&d);

    std::cout << "call with action get\n";
    try
    {
    c.call(parser, slsDetectorDefs::GET_ACTION);
    }
    catch(sls::RuntimeError& e){}
    catch ( const std::exception& e ){ LOG(sls::logERROR) << e.what() << std::endl;}
    std::cout << "\ncall with action put\n";
    try
    {
    c.call(parser, slsDetectorDefs::PUT_ACTION);
    }
    catch(sls::RuntimeError& e){}
    catch ( const std::exception& e ){LOG(sls::logERROR)<< e.what() << std::endl;}

    std::cout << "\ncall with 'infer action'\n";
    try
    {
    c.call(parser, default_action);
    }
    catch(sls::RuntimeError& e){}
    catch ( const std::exception& e ){LOG(sls::logERROR)  << e.what() << std::endl;}




}