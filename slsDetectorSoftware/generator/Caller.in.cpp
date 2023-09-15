#include "Caller.h"
#include <iostream>
#include "sls/string_utils.h"
namespace sls {

// enum { GET_ACTION, PUT_ACTION, READOUT_ACTION, HELP_ACTION };

void Caller::call(const CmdParser &parser, int action, std::ostream &os) {
  std::cout << "Caller received:\n";
  std::cout << "cmd: " << parser.command() << "\nargs: ";
  for (auto &arg : parser.arguments()) {
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

// THIS COMMENT IS GOING TO BE REPLACED BY THE ACTUAL CODE

}