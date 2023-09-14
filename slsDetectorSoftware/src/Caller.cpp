// WARNING this file is auto generated
// changes might be overwritten at any time

#include "Caller.h"
#include <iostream>
#include "sls/string_utils.h"
namespace sls{

// enum { GET_ACTION, PUT_ACTION, READOUT_ACTION, HELP_ACTION };

void Caller::call(const CmdParser& parser, int action, std::ostream &os){
    std::cout << "Caller received:\n";
    std::cout << "cmd: " << parser.command() << "\nargs: ";
    for (auto& arg : parser.arguments()){
        std::cout << arg << " ";
    }
    std::cout << '\n';
    std::cout << "action: " << action << "\n\n";

    args = parser.arguments();
    cmd = parser.command();
    det_id = parser.detector_id();
    auto it = functions.find(parser.command());
    if (it != functions.end()) {
        os << ((*this).*(it->second))(action);
    } else {
        throw RuntimeError(parser.command() +
                           " Unknown command, use list to list all commands");
    }
}

std::string Caller::frames(int action){
    std::cout << "Caller::frames\n";
    std::ostringstream os;
    os << cmd << ' ';  
    if(action == -1){
        if(args.size()==0)
            action = slsDetectorDefs::GET_ACTION;
        else if(args.size()==1)
            action = slsDetectorDefs::PUT_ACTION;
        else{
            throw RuntimeError("Wrong number of arguments");
        }
    }

    if(action == slsDetectorDefs::GET_ACTION){
        auto t = det->getNumberOfFrames(std::vector<int>{det_id});  
        os << OutString(t) << '\n';        
    }else if(action == slsDetectorDefs::PUT_ACTION){
        int n_frames = StringTo<int>(args[0]);
        det->setNumberOfFrames(n_frames);
        os << args.front() << '\n';   
    }


    return os.str();
}



}