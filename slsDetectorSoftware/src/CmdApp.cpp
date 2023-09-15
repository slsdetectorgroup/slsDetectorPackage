#include "sls/Detector.h"
#include "CmdParser.h"
#include "Caller.h"

#include <iostream>
int main(int argc, char *argv[]){

    std::cout << "Experimental command parsing\n";

    sls::CmdParser parser;
    parser.Parse(argc, argv);
    
    int default_action = -1;
    sls::Detector d(parser.multi_id());
    sls::Caller c(&d);

    std::cout << "call with action get\n";
    c.call(parser, default_action);

    std::cout << "\ncall with action put\n";
    c.call(parser, slsDetectorDefs::PUT_ACTION);


}