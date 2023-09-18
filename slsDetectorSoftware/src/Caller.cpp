#include "Caller.h"
#include <iostream>
#include "sls/string_utils.h"
namespace sls {

// enum { GET_ACTION, PUT_ACTION, READOUT_ACTION, HELP_ACTION };

void Caller::call(const CmdParser &parser, int action, std::ostream &os) {

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

std::string Caller::frames(int action) {

std::ostringstream os;
// infer action based on number of arguments
if (action == -1) {

if (args.size() == 0) {

std::cout << "inferred action: GET" << std::endl;
action = slsDetectorDefs::GET_ACTION;
}

else if (args.size() == 1) {

std::cout << "inferred action: PUT" << std::endl;
action = slsDetectorDefs::PUT_ACTION;
}

else {

throw RuntimeError("Could not infer action: Wrong number of arguments");
}

}

// check if action and arguments are valid
if (action == slsDetectorDefs::GET_ACTION) {

if (args.size() != 0) {

throw RuntimeError("Wrong number of arguments for action GET");
}

}

if (action == slsDetectorDefs::PUT_ACTION) {

if (args.size() != 1) {

throw RuntimeError("Wrong number of arguments for action PUT");
}

try {
StringTo<int>(args[0]);
} catch (...) {
  throw RuntimeError("Could not convert argument 0 to int");
}
}

// generate code for each action
if (action == slsDetectorDefs::GET_ACTION) {

auto t = det->getNumberOfFrames(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}

if (action == slsDetectorDefs::PUT_ACTION) {

auto arg0 = StringTo<int>(args[0]);
det->setNumberOfFrames(arg0);
os << args.front() << '\n';
}

return os.str();
}


}