#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

#include "CmdProxy.h"
#include "Detector.h"
#include "sls_detector_defs.h"

int main(){

    std::cout << "Generating command line documentation!!!!!!!!!!!!!!!!\n";
    
    sls::CmdProxy<sls::Detector> proxy(nullptr);
    auto commands = proxy.GetAllCommands();

    std::ofstream fs("commands.rst");
    fs << ".. glossary::\n";

    std::vector<std::string> extra{"exptime2", "period2", "subexptime2"};
    for (const auto& cmd : extra){
        std::ostringstream os;
        proxy.Call(cmd, {}, -1, slsDetectorDefs::HELP_ACTION, os);
        fs << '\t' << cmd << "\n\t\t" << os.str().erase(0, cmd.size()) << '\n';
    }

    // for (const auto& cmd : commands)
    //     fs << '\t' << cmd << '\n';


}