#include "sls/Detector.h"
#include "CmdParser.h"
#include "Caller.h"

#include <iostream>
int main(int argc, char *argv[]){

    std::cout << "Experimental command parsing\n";

    sls::CmdParser parser;
    parser.Parse(argc, argv);

    std::cout << "cmd: " << parser.command() << "\narguments: ";
    for (auto& arg : parser.arguments()){
        std::cout << arg << " ";
    }
    std::cout << "\n\n";
    
    int default_action = -1;
    sls::Detector d(parser.multi_id());
    sls::Caller c(&d);
    c.call(parser, default_action);
}