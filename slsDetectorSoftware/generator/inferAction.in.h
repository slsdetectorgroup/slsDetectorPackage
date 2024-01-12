#include "CmdParser.h"
#include <iostream>
#include <map>
#include <vector>

namespace sls {
class InferAction {
  public:
    InferAction() {}
    int infer(sls::CmdParser &parser, std::ostream &os = std::cout);
    std::vector<std::string> args;
    std::string cmd;

    // generated functions
    // THIS COMMENT TO BE REPLACED BY THE ACTUAL CODE (1) - DO NOT REMOVE
    //     int frames();

  private:
    using FunctionMap = std::map<std::string, int (InferAction::*)()>;
    FunctionMap functions{
        // generated functions

        // THIS COMMENT TO BE REPLACED BY THE ACTUAL CODE (2) - DO NOT REMOVE

        //        {"frames",&InferAction::frames}

    };
};

} // namespace sls