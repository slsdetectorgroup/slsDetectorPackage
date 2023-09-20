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

std::string Caller::exptime(int action) {

  std::ostringstream os;
  // infer action based on number of arguments
  if (action == -1) {

    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {

    if (args.size() != 0 && args.size() != 1) {

      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

    if (args.size() == 1) {
    }

  } else if (action == slsDetectorDefs::PUT_ACTION) {

    if (args.size() != 1 && args.size() != 2) {

      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 1) {

      try {
        std::string tmp_time(args[0]);
        std::string unit = RemoveUnit(tmp_time);
        auto converted_time = StringTo<time::ns>(tmp_time, unit);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument to time::ns");
      }
    }

    if (args.size() == 2) {

      try {
        StringTo<time::ns>(args[0], args[1]);
      }
      catch (...) {
        throw RuntimeError("Could not convert arguments to time::ns");
      }
    }

  } else {

    throw RuntimeError("Invalid action");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {

    if (detector_type == defs::MYTHEN3) {

      if (args.size() == 0) {

        auto t = det->getExptimeForAllGates(std::vector<int>{ det_id });
        os << OutString(t) << '\n';
      }

      if (args.size() == 1) {

        auto t = det->getExptimeForAllGates(std::vector<int>{ det_id });
        os << OutString(t, args[0]) << '\n';
      }

    } else {

      if (args.size() == 0) {

        auto t = det->getExptime(std::vector<int>{ det_id });
        os << OutString(t) << '\n';
      }

      if (args.size() == 1) {

        auto t = det->getExptime(std::vector<int>{ det_id });
        os << OutString(t, args[0]) << '\n';
      }
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {

    if (args.size() == 1) {

      std::string tmp_time(args[0]);
      std::string unit = RemoveUnit(tmp_time);
      auto converted_time = StringTo<time::ns>(tmp_time, unit);
      det->setExptime(converted_time, std::vector<int>{ det_id });
      os << args[0] << '\n';
    }

    if (args.size() == 2) {

      auto converted_time = StringTo<time::ns>(args[0], args[1]);
      det->setExptime(converted_time, std::vector<int>{ det_id });
      os << args[0] << args[1] << '\n';
    }
  }

  return os.str();
}

std::string Caller::frames(int action) {

  std::ostringstream os;
  // infer action based on number of arguments
  if (action == -1) {

    if (args.size() == 0) {

      std::cout << "inferred action: GET" << std::endl;
      action = slsDetectorDefs::GET_ACTION;
    } else if (args.size() == 1) {

      std::cout << "inferred action: PUT" << std::endl;
      action = slsDetectorDefs::PUT_ACTION;
    } else {

      throw RuntimeError("Could not infer action: Wrong number of arguments");
    }
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {

    if (args.size() != 0) {

      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else if (action == slsDetectorDefs::PUT_ACTION) {

    if (args.size() != 1) {

      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 1) {

      try {
        StringTo<int>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 0 to int");
      }
    }

  } else {

    throw RuntimeError("Invalid action");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {

    if (args.size() == 0) {

      auto t = det->getNumberOfFrames(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {

    if (args.size() == 1) {

      auto arg0 = StringTo<int>(args[0]);
      det->setNumberOfFrames(arg0);
      os << args[0] << '\n';
    }
  }

  return os.str();
}
}