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

std::string Caller::list(int action) {
  std::string ret;
  for (auto &f : functions) {
    ret += f.first + "\n";
  }
  return ret;
}

std::string Caller::activate(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: activate" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getActive(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setActive(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::adcclk(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: adcclk" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getADCClock(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setADCClock(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::adcenable(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: adcenable" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
        StringTo<uint32_t>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 0 to uint32_t");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getADCEnableMask(std::vector<int>{ det_id });
      os << OutStringHex(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<uint32_t>(args[0]);
      det->setADCEnableMask(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::adcenable10g(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: adcenable10g" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
        StringTo<uint32_t>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 0 to uint32_t");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getTenGigaADCEnableMask(std::vector<int>{ det_id });
      os << OutStringHex(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<uint32_t>(args[0]);
      det->setTenGigaADCEnableMask(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::adcindex(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: adcindex" << std::endl;
    os << R"V0G0N([name] 
		[ChipTestBoard] Get the adc index for the given name. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 1) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 1) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 1) {
      if (det->getDetectorType().squash() != defs::CHIPTESTBOARD) {
        throw RuntimeError(cmd + " only allowed for CTB.");
      }
      if (det_id != -1) {
        throw RuntimeError("Cannot execute adcindex at module level");
      }
      auto t = det->getAdcIndex(args[0]);
      os << static_cast<int>(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::adcinvert(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: adcinvert" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
        StringTo<uint32_t>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 0 to uint32_t");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getADCInvert(std::vector<int>{ det_id });
      os << OutStringHex(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<uint32_t>(args[0]);
      det->setADCInvert(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::adcname(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: adcname" << std::endl;
    os << R"V0G0N([0-31][name] 
		[ChipTestBoard] Set the adc at the given position to the given name. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 1) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 1) {
      try {
        StringTo<int>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 0 to int");
      }
    }

  } else if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 2) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 2) {
      try {
        StringTo<int>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 0 to int");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 1) {
      if (det->getDetectorType().squash() != defs::CHIPTESTBOARD) {
        throw RuntimeError(cmd + " only allowed for CTB.");
      }
      if (det_id != -1) {
        throw RuntimeError("Cannot execute adcname at module level");
      }
      auto arg0 = StringTo<int>(args[0]);
      auto t = det->getAdcName(arg0);
      os << args[0] << ' ' << t << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 2) {
      if (det->getDetectorType().squash() != defs::CHIPTESTBOARD) {
        throw RuntimeError(cmd + " only allowed for CTB.");
      }
      if (det_id != -1) {
        throw RuntimeError("Cannot execute adcname at module level");
      }
      auto arg0 = StringTo<int>(args[0]);
      det->setAdcName(arg0, args[1]);
      os << ToString(args) << '\n';
    }
  }

  return os.str();
}

std::string Caller::adcpipeline(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: adcpipeline" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getADCPipeline(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setADCPipeline(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::apulse(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: apulse" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getAnalogPulsing(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setAnalogPulsing(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::asamples(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: asamples" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getNumberOfAnalogSamples(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setNumberOfAnalogSamples(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::autocompdisable(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: autocompdisable" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getAutoComparatorDisable(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setAutoComparatorDisable(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::burstperiod(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: burstperiod" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getBurstPeriod(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }

    if (args.size() == 1) {
      auto t = det->getBurstPeriod(std::vector<int>{ det_id });
      os << OutString(t, args[0]) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      std::string tmp_time(args[0]);
      std::string unit = RemoveUnit(tmp_time);
      auto converted_time = StringTo<time::ns>(tmp_time, unit);
      det->setBurstPeriod(converted_time, std::vector<int>{ det_id });
      os << args[0] << '\n';
    }

    if (args.size() == 2) {
      auto converted_time = StringTo<time::ns>(args[0], args[1]);
      det->setBurstPeriod(converted_time, std::vector<int>{ det_id });
      os << args[0] << args[1] << '\n';
    }
  }

  return os.str();
}

std::string Caller::bursts(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: bursts" << std::endl;
    os << R"V0G0N([n_bursts]
	[Gotthard2] Number of bursts per aquire. Only in auto timing mode and burst mode. Use timing command to set timing mode and burstmode command to set burst mode. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getNumberOfBursts(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute bursts at module level");
      }
      auto arg0 = StringTo<int>(args[0]);
      det->setNumberOfBursts(arg0);
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::burstsl(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: burstsl" << std::endl;
    os << R"V0G0N(
	[Gotthard2] Number of bursts left in acquisition. Only in burst auto mode. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getNumberOfBurstsLeft(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::bustest(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: bustest" << std::endl;
    os << R"V0G0N(
	[Jungfrau][Moench][Gotthard][Mythen3][Gotthard2][Ctb] Bus test, ie. Writes different values in a R/W register and confirms the writes to check bus.
	Advanced User function! )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 0) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute bustest at module level");
      }
      det->executeBusTest(std::vector<int>{ det_id });
      os << "successful" << '\n';
    }
  }

  return os.str();
}

std::string Caller::cdsgain(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: cdsgain" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
        StringTo<bool>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 0 to bool");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getCDSGain(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<bool>(args[0]);
      det->setCDSGain(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::chipversion(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: chipversion" << std::endl;
    os << R"V0G0N(
	[Jungfrau] Returns chip version. Can be 1.0 or 1.1 )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getChipVersion(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::clearbusy(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: clearbusy" << std::endl;
    os << R"V0G0N(
	If acquisition aborted during acquire command, use this to clear acquiring flag in shared memory before starting next acquisition )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 0) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute clearbusy at module level");
      }
      det->clearAcquiringFlag();
      os << "successful" << '\n';
    }
  }

  return os.str();
}

std::string Caller::clearroi(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: clearroi" << std::endl;
    os << R"V0G0N([Gotthard] Resets Region of interest in detector. All )V0G0N"
          R"V0G0N(channels enabled. Default is all channels enabled. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 0) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute clearroi at module level");
      }
      det->clearROI(std::vector<int>{ det_id });
      os << "successful" << '\n';
    }
  }

  return os.str();
}

std::string Caller::column(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: column" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getColumn(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setColumn(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::compdisabletime(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: compdisabletime" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getComparatorDisableTime(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }

    if (args.size() == 1) {
      auto t = det->getComparatorDisableTime(std::vector<int>{ det_id });
      os << OutString(t, args[0]) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      std::string tmp_time(args[0]);
      std::string unit = RemoveUnit(tmp_time);
      auto converted_time = StringTo<time::ns>(tmp_time, unit);
      det->setComparatorDisableTime(converted_time, std::vector<int>{ det_id });
      os << args[0] << '\n';
    }

    if (args.size() == 2) {
      auto converted_time = StringTo<time::ns>(args[0], args[1]);
      det->setComparatorDisableTime(converted_time, std::vector<int>{ det_id });
      os << args[0] << args[1] << '\n';
    }
  }

  return os.str();
}

std::string Caller::config(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: config" << std::endl;
    os << R"V0G0N(
	Frees shared memory before loading configuration file. Set up once. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 1) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 1) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute config at module level");
      }
      det->loadConfig(args[0]);
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::dacindex(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: dacindex" << std::endl;
    os << R"V0G0N([name] 
		[ChipTestBoard] Get the dac index for the given name. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 1) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 1) {
      defs::dacIndex index = defs::DAC_0;
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 1) {
      defs::dacIndex index = defs::DAC_0;
      if (det->getDetectorType().squash() != defs::CHIPTESTBOARD) {
        throw RuntimeError(cmd + " only allowed for CTB.");
      }
      if (det_id != -1) {
        throw RuntimeError("Cannot execute dacindex at module level");
      }
      auto t = det->getDacIndex(args[0]);
      os << ToString(static_cast<int>(t) - index) << '\n';
    }
  }

  return os.str();
}

std::string Caller::dacname(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: dacname" << std::endl;
    os << R"V0G0N([0-17][name] 
		[ChipTestBoard] Set the dac at the given position to the given name. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 1) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 1) {
      defs::dacIndex index = defs::DAC_0;
    }

  } else if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 2) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 2) {
      defs::dacIndex index = defs::DAC_0;
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 1) {
      defs::dacIndex index = defs::DAC_0;
      if (det->getDetectorType().squash() != defs::CHIPTESTBOARD) {
        throw RuntimeError(cmd + " only allowed for CTB.");
      }
      if (det_id != -1) {
        throw RuntimeError("Cannot execute dacname at module level");
      }
      auto t = det->getDacName(
          static_cast<defs::dacIndex>(StringTo<int>(args[0]) + index));
      os << args[0] << ' ' << t << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 2) {
      defs::dacIndex index = defs::DAC_0;
      if (det->getDetectorType().squash() != defs::CHIPTESTBOARD) {
        throw RuntimeError(cmd + " only allowed for CTB.");
      }
      if (det_id != -1) {
        throw RuntimeError("Cannot execute dacname at module level");
      }
      det->setDacName(
          static_cast<defs::dacIndex>(StringTo<int>(args[0]) + index), args[1]);
      os << ToString(args) << '\n';
    }
  }

  return os.str();
}

std::string Caller::dbitclk(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: dbitclk" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getDBITClock(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setDBITClock(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::dbitpipeline(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: dbitpipeline" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getDBITPipeline(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setDBITPipeline(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::defaultpattern(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: defaultpattern" << std::endl;
    os << R"V0G0N(
	[Mythen3] Loads and runs default pattern in pattern generator. It is to go back to initial settings. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 0) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute defaultpattern at module level");
      }
      det->loadDefaultPattern(std::vector<int>{ det_id });
      os << "successful" << '\n';
    }
  }

  return os.str();
}

std::string Caller::delay(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: delay" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getDelayAfterTrigger(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }

    if (args.size() == 1) {
      auto t = det->getDelayAfterTrigger(std::vector<int>{ det_id });
      os << OutString(t, args[0]) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      std::string tmp_time(args[0]);
      std::string unit = RemoveUnit(tmp_time);
      auto converted_time = StringTo<time::ns>(tmp_time, unit);
      det->setDelayAfterTrigger(converted_time, std::vector<int>{ det_id });
      os << args[0] << '\n';
    }

    if (args.size() == 2) {
      auto converted_time = StringTo<time::ns>(args[0], args[1]);
      det->setDelayAfterTrigger(converted_time, std::vector<int>{ det_id });
      os << args[0] << args[1] << '\n';
    }
  }

  return os.str();
}

std::string Caller::delayl(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: delayl" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

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

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getDelayAfterTriggerLeft(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }

    if (args.size() == 1) {
      auto t = det->getDelayAfterTriggerLeft(std::vector<int>{ det_id });
      os << OutString(t, args[0]) << '\n';
    }
  }

  return os.str();
}

std::string Caller::detectorserverversion(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: detectorserverversion" << std::endl;
    os << R"V0G0N(
	On-board detector server software version )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getDetectorServerVersion(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::dpulse(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: dpulse" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getDigitalPulsing(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setDigitalPulsing(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::dr(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: dr" << std::endl;
    os << R"V0G0N([value]
	Dynamic Range or number of bits per pixel in detector.
	[Eiger] Options: 4, 8, 12, 16, 32. If set to 32, also sets clkdivider to 2, else to 0.
	[Mythen3] Options: 8, 16, 32
	[Jungfrau][Moench][Gotthard][Ctb][Mythen3][Gotthard2] 16 )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getDynamicRange(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute dr at module level");
      }
      auto arg0 = StringTo<int>(args[0]);
      det->setDynamicRange(arg0);
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::drlist(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: drlist" << std::endl;
    os << R"V0G0N(
	Gets the list of dynamic ranges for this detector. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getDynamicRangeList();
      os << ToString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::dsamples(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: dsamples" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getNumberOfDigitalSamples(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setNumberOfDigitalSamples(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::exptime(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: exptime" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
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

std::string Caller::exptime1(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: exptime1" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

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
      int gateIndex = 0;
    }

    if (args.size() == 1) {
      int gateIndex = 0;
    }

  } else if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 1 && args.size() != 2) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 1) {
      int gateIndex = 0;
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
      int gateIndex = 0;
      try {
        StringTo<time::ns>(args[0], args[1]);
      }
      catch (...) {
        throw RuntimeError("Could not convert arguments to time::ns");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      int gateIndex = 0;
      auto t = det->getExptime(gateIndex, std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }

    if (args.size() == 1) {
      int gateIndex = 0;
      auto t = det->getExptime(gateIndex, std::vector<int>{ det_id });
      os << OutString(t, args[0]) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (detector_type == defs::MYTHEN3) {
      if (args.size() == 1) {
        int gateIndex = 0;
        std::string tmp_time(args[0]);
        std::string unit = RemoveUnit(tmp_time);
        auto converted_time = StringTo<time::ns>(tmp_time, unit);
        det->setExptime(gateIndex, converted_time, std::vector<int>{ det_id });
        os << args[0] << '\n';
      }

      if (args.size() == 2) {
        int gateIndex = 0;
        auto converted_time = StringTo<time::ns>(args[0], args[1]);
        det->setExptime(gateIndex, converted_time, std::vector<int>{ det_id });
        os << args[0] << args[1] << '\n';
      }

    } else {

      if (args.size() == 1) {
        int gateIndex = 0;
        std::string tmp_time(args[0]);
        std::string unit = RemoveUnit(tmp_time);
        auto converted_time = StringTo<time::ns>(tmp_time, unit);
        det->setExptime(converted_time, std::vector<int>{ det_id });
        os << args[0] << '\n';
      }

      if (args.size() == 2) {
        int gateIndex = 0;
        auto converted_time = StringTo<time::ns>(args[0], args[1]);
        det->setExptime(converted_time, std::vector<int>{ det_id });
        os << args[0] << args[1] << '\n';
      }
    }
  }

  return os.str();
}

std::string Caller::exptime2(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: exptime2" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

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
      int gateIndex = 1;
    }

    if (args.size() == 1) {
      int gateIndex = 1;
    }

  } else if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 1 && args.size() != 2) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 1) {
      int gateIndex = 1;
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
      int gateIndex = 1;
      try {
        StringTo<time::ns>(args[0], args[1]);
      }
      catch (...) {
        throw RuntimeError("Could not convert arguments to time::ns");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      int gateIndex = 1;
      auto t = det->getExptime(gateIndex, std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }

    if (args.size() == 1) {
      int gateIndex = 1;
      auto t = det->getExptime(gateIndex, std::vector<int>{ det_id });
      os << OutString(t, args[0]) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (detector_type == defs::MYTHEN3) {
      if (args.size() == 1) {
        int gateIndex = 0;
        std::string tmp_time(args[0]);
        std::string unit = RemoveUnit(tmp_time);
        auto converted_time = StringTo<time::ns>(tmp_time, unit);
        det->setExptime(gateIndex, converted_time, std::vector<int>{ det_id });
        os << args[0] << '\n';
      }

      if (args.size() == 2) {
        int gateIndex = 0;
        auto converted_time = StringTo<time::ns>(args[0], args[1]);
        det->setExptime(gateIndex, converted_time, std::vector<int>{ det_id });
        os << args[0] << args[1] << '\n';
      }

    } else {

      if (args.size() == 1) {
        int gateIndex = 1;
        std::string tmp_time(args[0]);
        std::string unit = RemoveUnit(tmp_time);
        auto converted_time = StringTo<time::ns>(tmp_time, unit);
        det->setExptime(converted_time, std::vector<int>{ det_id });
        os << args[0] << '\n';
      }

      if (args.size() == 2) {
        int gateIndex = 1;
        auto converted_time = StringTo<time::ns>(args[0], args[1]);
        det->setExptime(converted_time, std::vector<int>{ det_id });
        os << args[0] << args[1] << '\n';
      }
    }
  }

  return os.str();
}

std::string Caller::exptime3(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: exptime3" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

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
      int gateIndex = 2;
    }

    if (args.size() == 1) {
      int gateIndex = 2;
    }

  } else if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 1 && args.size() != 2) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 1) {
      int gateIndex = 2;
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
      int gateIndex = 2;
      try {
        StringTo<time::ns>(args[0], args[1]);
      }
      catch (...) {
        throw RuntimeError("Could not convert arguments to time::ns");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      int gateIndex = 2;
      auto t = det->getExptime(gateIndex, std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }

    if (args.size() == 1) {
      int gateIndex = 2;
      auto t = det->getExptime(gateIndex, std::vector<int>{ det_id });
      os << OutString(t, args[0]) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (detector_type == defs::MYTHEN3) {
      if (args.size() == 1) {
        int gateIndex = 0;
        std::string tmp_time(args[0]);
        std::string unit = RemoveUnit(tmp_time);
        auto converted_time = StringTo<time::ns>(tmp_time, unit);
        det->setExptime(gateIndex, converted_time, std::vector<int>{ det_id });
        os << args[0] << '\n';
      }

      if (args.size() == 2) {
        int gateIndex = 0;
        auto converted_time = StringTo<time::ns>(args[0], args[1]);
        det->setExptime(gateIndex, converted_time, std::vector<int>{ det_id });
        os << args[0] << args[1] << '\n';
      }

    } else {

      if (args.size() == 1) {
        int gateIndex = 2;
        std::string tmp_time(args[0]);
        std::string unit = RemoveUnit(tmp_time);
        auto converted_time = StringTo<time::ns>(tmp_time, unit);
        det->setExptime(converted_time, std::vector<int>{ det_id });
        os << args[0] << '\n';
      }

      if (args.size() == 2) {
        int gateIndex = 2;
        auto converted_time = StringTo<time::ns>(args[0], args[1]);
        det->setExptime(converted_time, std::vector<int>{ det_id });
        os << args[0] << args[1] << '\n';
      }
    }
  }

  return os.str();
}

std::string Caller::exptimel(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: exptimel" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

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

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getExptimeLeft(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }

    if (args.size() == 1) {
      auto t = det->getExptimeLeft(std::vector<int>{ det_id });
      os << OutString(t, args[0]) << '\n';
    }
  }

  return os.str();
}

std::string Caller::extrastoragecells(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: extrastoragecells" << std::endl;
    os << R"V0G0N([0-15]
	[Jungfrau] Only for chipv1.0. Number of additional storage cells. Default is 0. For advanced users only. 
	The #images = #frames x #triggers x (#extrastoragecells + 1). )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t =
          det->getNumberOfAdditionalStorageCells(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute extrastoragecells at module level");
      }
      auto arg0 = StringTo<int>(args[0]);
      det->setNumberOfAdditionalStorageCells(arg0);
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::extsampling(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: extsampling" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getExternalSampling(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setExternalSampling(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::extsamplingsrc(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: extsamplingsrc" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getExternalSamplingSource(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setExternalSamplingSource(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::fformat(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: fformat" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
        StringTo<slsDetectorDefs::fileFormat>(args[0]);
      }
      catch (...) {
        throw RuntimeError(
            "Could not convert argument 0 to slsDetectorDefs::fileFormat");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getFileFormat(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<slsDetectorDefs::fileFormat>(args[0]);
      det->setFileFormat(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::filtercells(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: filtercells" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getNumberOfFilterCells(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setNumberOfFilterCells(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::filterresistor(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: filterresistor" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getFilterResistor(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setFilterResistor(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::findex(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: findex" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
        StringTo<uint64_t>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 0 to uint64_t");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getAcquisitionIndex(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<uint64_t>(args[0]);
      det->setAcquisitionIndex(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::firmwaretest(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: firmwaretest" << std::endl;
    os << R"V0G0N(
	[Jungfrau][Moench][Gotthard][Mythen3][Gotthard2][Ctb] Firmware test, ie. reads a read fixed pattern from a register. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 0) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute firmwaretest at module level");
      }
      det->executeFirmwareTest(std::vector<int>{ det_id });
      os << "successful" << '\n';
    }
  }

  return os.str();
}

std::string Caller::fliprows(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: fliprows" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getFlipRows(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setFlipRows(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::flowcontrol10g(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: flowcontrol10g" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getTenGigaFlowControl(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setTenGigaFlowControl(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::fmaster(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: fmaster" << std::endl;
    os << R"V0G0N([0, 1]
	Enable or disable receiver master file. Default is 1. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute fmaster at module level");
      }
      auto t = det->getMasterFileWrite();
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute fmaster at module level");
      }
      auto arg0 = StringTo<int>(args[0]);
      det->setMasterFileWrite(arg0);
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::fname(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: fname" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getFileNamePrefix(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      det->setFileNamePrefix(args[0], std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::foverwrite(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: foverwrite" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getFileOverWrite(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setFileOverWrite(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::fpath(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: fpath" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getFilePath(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      det->setFilePath(args[0], std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::framecounter(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: framecounter" << std::endl;
    os << R"V0G0N(
	[Jungfrau][Moench][Mythen3][Gotthard2][CTB] Number of frames from start run control.
	[Gotthard2] only in continuous mode. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getNumberOfFramesFromStart(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::frames(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: frames" << std::endl;
    os << R"V0G0N([n_frames]
	Number of frames per acquisition. In trigger mode, number of frames per trigger. 
	Cannot be set in modular level. 
	In scan mode, number of frames is set to number of steps.
	[Gotthard2] Burst mode has a maximum of 2720 frames. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
        StringTo<int64_t>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 0 to int64_t");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
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
      if (det_id != -1) {
        throw RuntimeError("Cannot execute frames at module level");
      }
      auto arg0 = StringTo<int64_t>(args[0]);
      det->setNumberOfFrames(arg0);
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::framesl(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: framesl" << std::endl;
    os << R"V0G0N(
	[Gotthard][Jungfrau][Moench][Mythen3][Gotthard2][CTB] Number of frames left in acquisition. 
	[Gotthard2] only in continuous auto mode. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getNumberOfFramesLeft(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::frametime(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: frametime" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

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

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getMeasurementTime(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }

    if (args.size() == 1) {
      auto t = det->getMeasurementTime(std::vector<int>{ det_id });
      os << OutString(t, args[0]) << '\n';
    }
  }

  return os.str();
}

std::string Caller::fwrite(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: fwrite" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getFileWrite(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setFileWrite(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::gainmode(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: gainmode" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
        StringTo<slsDetectorDefs::gainMode>(args[0]);
      }
      catch (...) {
        throw RuntimeError(
            "Could not convert argument 0 to slsDetectorDefs::gainMode");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getGainMode(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<slsDetectorDefs::gainMode>(args[0]);
      det->setGainMode(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::gates(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: gates" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getNumberOfGates(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setNumberOfGates(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::hardwareversion(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: hardwareversion" << std::endl;
    os << R"V0G0N(
	[Jungfrau][Gotthard2][Myhten3][Gotthard][Ctb][Moench] Hardware version of detector. 
	[Eiger] Hardware version of front FPGA on detector. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getHardwareVersion(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::highvoltage(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: highvoltage" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getHighVoltage(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setHighVoltage(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::im_a(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: im_a" << std::endl;
    os << R"V0G0N(
	[Ctb] Measured current of power supply a in mA. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t =
          det->getMeasuredCurrent(defs::I_POWER_A, std::vector<int>{ det_id });
      os << OutString(t) << " °C" << '\n';
    }
  }

  return os.str();
}

std::string Caller::im_b(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: im_b" << std::endl;
    os << R"V0G0N(
	[Ctb] Measured current of power supply b in mA. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t =
          det->getMeasuredCurrent(defs::I_POWER_B, std::vector<int>{ det_id });
      os << OutString(t) << " °C" << '\n';
    }
  }

  return os.str();
}

std::string Caller::im_c(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: im_c" << std::endl;
    os << R"V0G0N(
	[Ctb] Measured current of power supply c in mA. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t =
          det->getMeasuredCurrent(defs::I_POWER_C, std::vector<int>{ det_id });
      os << OutString(t) << " °C" << '\n';
    }
  }

  return os.str();
}

std::string Caller::im_d(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: im_d" << std::endl;
    os << R"V0G0N(
	[Ctb] Measured current of power supply d in mA. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t =
          det->getMeasuredCurrent(defs::I_POWER_D, std::vector<int>{ det_id });
      os << OutString(t) << " °C" << '\n';
    }
  }

  return os.str();
}

std::string Caller::im_io(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: im_io" << std::endl;
    os << R"V0G0N(
	[Ctb] Measured current of power supply io in mA. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t =
          det->getMeasuredCurrent(defs::I_POWER_IO, std::vector<int>{ det_id });
      os << OutString(t) << " °C" << '\n';
    }
  }

  return os.str();
}

std::string Caller::imagetest(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: imagetest" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getImageTestMode(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setImageTestMode(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::interpolation(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: interpolation" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getInterpolation(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setInterpolation(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::interruptsubframe(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: interruptsubframe" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getInterruptSubframe(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setInterruptSubframe(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::kernelversion(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: kernelversion" << std::endl;
    os << R"V0G0N(
	Get kernel version on the detector including time and date. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getKernelVersion(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::lastclient(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: lastclient" << std::endl;
    os << R"V0G0N(
	Client IP Address that last communicated with the detector. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getLastClientIP(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::led(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: led" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getLEDEnable(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setLEDEnable(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::lock(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: lock" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getDetectorLock(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setDetectorLock(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::master(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: master" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getMaster(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setMaster(arg0, det_id);
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::maxadcphaseshift(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: maxadcphaseshift" << std::endl;
    os << R"V0G0N(
	[Jungfrau][Moench][CTB] Absolute maximum Phase shift of ADC clock. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getMaxADCPhaseShift(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::maxdbitphaseshift(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: maxdbitphaseshift" << std::endl;
    os << R"V0G0N(
	[CTB][Jungfrau] Absolute maximum Phase shift of of the clock to latch digital bits. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getMaxDBITPhaseShift(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::measuredperiod(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: measuredperiod" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

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

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getMeasuredPeriod(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }

    if (args.size() == 1) {
      auto t = det->getMeasuredPeriod(std::vector<int>{ det_id });
      os << OutString(t, args[0]) << '\n';
    }
  }

  return os.str();
}

std::string Caller::measuredsubperiod(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: measuredsubperiod" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

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

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getMeasuredSubFramePeriod(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }

    if (args.size() == 1) {
      auto t = det->getMeasuredSubFramePeriod(std::vector<int>{ det_id });
      os << OutString(t, args[0]) << '\n';
    }
  }

  return os.str();
}

std::string Caller::moduleid(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: moduleid" << std::endl;
    os << R"V0G0N(
	[Gotthard2][Eiger][Mythen3][Jungfrau][Moench] 16 bit value (ideally unique) that is streamed out in the UDP header of the detector. Picked up from a file on the module. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getModuleId(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::nextframenumber(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: nextframenumber" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
        StringTo<uint64_t>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 0 to uint64_t");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getNextFrameNumber(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<uint64_t>(args[0]);
      det->setNextFrameNumber(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::nmod(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: nmod" << std::endl;
    os << R"V0G0N(
	Number of modules in shared memory. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->size();
      os << ToString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::numinterfaces(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: numinterfaces" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getNumberofUDPInterfaces(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setNumberofUDPInterfaces(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::overflow(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: overflow" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getOverFlowMode(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setOverFlowMode(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::parallel(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: parallel" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getParallelMode(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setParallelMode(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::parameters(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: parameters" << std::endl;
    os << R"V0G0N(
	Sets detector measurement parameters to those contained in fname. Set up per measurement. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 1) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 1) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute parameters at module level");
      }
      det->loadParameters(args[0]);
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::partialreset(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: partialreset" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getPartialReset(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setPartialReset(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::patfname(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: patfname" << std::endl;
    os << R"V0G0N(
	[Ctb][Mythen3] Gets the pattern file name including path of the last pattern uploaded. Returns an empty if nothing was uploaded or via a server default file )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getPatterFileName(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::patioctrl(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: patioctrl" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
        StringTo<uint64_t>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 0 to uint64_t");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getPatternIOControl(std::vector<int>{ det_id });
      os << OutStringHex(t, 16) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<uint64_t>(args[0]);
      det->setPatternIOControl(arg0, std::vector<int>{ det_id });
      os << ToStringHex(args[0], 16) << '\n';
    }
  }

  return os.str();
}

std::string Caller::patmask(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: patmask" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
        StringTo<uint64_t>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 0 to uint64_t");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getPatternMask(std::vector<int>{ det_id });
      os << OutStringHex(t, 16) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<uint64_t>(args[0]);
      det->setPatternMask(arg0, std::vector<int>{ det_id });
      os << ToStringHex(args[0], 16) << '\n';
    }
  }

  return os.str();
}

std::string Caller::patsetbit(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: patsetbit" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
        StringTo<uint64_t>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 0 to uint64_t");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getPatternBitMask(std::vector<int>{ det_id });
      os << OutStringHex(t, 16) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<uint64_t>(args[0]);
      det->setPatternBitMask(arg0, std::vector<int>{ det_id });
      os << ToStringHex(args[0], 16) << '\n';
    }
  }

  return os.str();
}

std::string Caller::patternstart(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: patternstart" << std::endl;
    os << R"V0G0N(
	[Mythen3] Starts Pattern )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 0) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute patternstart at module level");
      }
      det->startPattern(std::vector<int>{ det_id });
      os << "successful" << '\n';
    }
  }

  return os.str();
}

std::string Caller::period(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: period" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getPeriod(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }

    if (args.size() == 1) {
      auto t = det->getPeriod(std::vector<int>{ det_id });
      os << OutString(t, args[0]) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      std::string tmp_time(args[0]);
      std::string unit = RemoveUnit(tmp_time);
      auto converted_time = StringTo<time::ns>(tmp_time, unit);
      det->setPeriod(converted_time, std::vector<int>{ det_id });
      os << args[0] << '\n';
    }

    if (args.size() == 2) {
      auto converted_time = StringTo<time::ns>(args[0], args[1]);
      det->setPeriod(converted_time, std::vector<int>{ det_id });
      os << args[0] << args[1] << '\n';
    }
  }

  return os.str();
}

std::string Caller::periodl(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: periodl" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

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

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getPeriodLeft(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }

    if (args.size() == 1) {
      auto t = det->getPeriodLeft(std::vector<int>{ det_id });
      os << OutString(t, args[0]) << '\n';
    }
  }

  return os.str();
}

std::string Caller::polarity(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: polarity" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
        StringTo<defs::polarity>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 0 to defs::polarity");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getPolarity(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<defs::polarity>(args[0]);
      det->setPolarity(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::port(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: port" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getControlPort(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setControlPort(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::powerchip(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: powerchip" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getPowerChip(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setPowerChip(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::pumpprobe(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: pumpprobe" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getPumpProbe(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setPumpProbe(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::readnrows(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: readnrows" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getReadNRows(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setReadNRows(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::readout(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: readout" << std::endl;
    os << R"V0G0N(
	[Mythen3] Starts detector readout. Status changes to TRANSMITTING and automatically returns to idle at the end of readout. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 0) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute readout at module level");
      }
      det->startDetectorReadout();
      os << "successful" << '\n';
    }
  }

  return os.str();
}

std::string Caller::readoutspeedlist(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: readoutspeedlist" << std::endl;
    os << R"V0G0N(
	List of readout speed levels implemented for this detector. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getReadoutSpeedList();
      os << ToString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::rebootcontroller(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rebootcontroller" << std::endl;
    os << R"V0G0N(
	[Jungfrau][Moench][Ctb][Gotthard][Mythen3][Gotthard2] Reboot controller of detector. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 0) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute rebootcontroller at module level");
      }
      det->rebootController(std::vector<int>{ det_id });
      os << "successful" << '\n';
    }
  }

  return os.str();
}

std::string Caller::resetfpga(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: resetfpga" << std::endl;
    os << R"V0G0N(
	[Jungfrau][Moench][Ctb] Reset FPGA. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 0) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute resetfpga at module level");
      }
      det->resetFPGA(std::vector<int>{ det_id });
      os << "successful" << '\n';
    }
  }

  return os.str();
}

std::string Caller::romode(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: romode" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
        StringTo<slsDetectorDefs::readoutMode>(args[0]);
      }
      catch (...) {
        throw RuntimeError(
            "Could not convert argument 0 to slsDetectorDefs::readoutMode");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getReadoutMode(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<slsDetectorDefs::readoutMode>(args[0]);
      det->setReadoutMode(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::row(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: row" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getRow(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setRow(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::runclk(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: runclk" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getRUNClock(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setRUNClock(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::runtime(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: runtime" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

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

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getActualTime(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }

    if (args.size() == 1) {
      auto t = det->getActualTime(std::vector<int>{ det_id });
      os << OutString(t, args[0]) << '\n';
    }
  }

  return os.str();
}

std::string Caller::rx_arping(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rx_arping" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getRxArping(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setRxArping(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::rx_clearroi(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rx_clearroi" << std::endl;
    os << R"V0G0N(Resets Region of interest in receiver. Default is all )V0G0N"
          R"V0G0N(channels/pixels enabled. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 0) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute rx_clearroi at module level");
      }
      det->clearRxROI();
      os << "successful" << '\n';
    }
  }

  return os.str();
}

std::string Caller::rx_dbitoffset(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rx_dbitoffset" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getRxDbitOffset(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setRxDbitOffset(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::rx_discardpolicy(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rx_discardpolicy" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
        StringTo<slsDetectorDefs::frameDiscardPolicy>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 0 to "
                           "slsDetectorDefs::frameDiscardPolicy");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getRxFrameDiscardPolicy(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<slsDetectorDefs::frameDiscardPolicy>(args[0]);
      det->setRxFrameDiscardPolicy(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::rx_fifodepth(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rx_fifodepth" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getRxFifoDepth(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setRxFifoDepth(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::rx_framecaught(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rx_framecaught" << std::endl;
    os << R"V0G0N(
	Number of frames caught by receiver. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getFramesCaught(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::rx_frameindex(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rx_frameindex" << std::endl;
    os << R"V0G0N(
	Current frame index received for receiver during acquisition. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getRxCurrentFrameIndex(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::rx_framesperfile(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rx_framesperfile" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getFramesPerFile(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setFramesPerFile(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::rx_lastclient(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rx_lastclient" << std::endl;
    os << R"V0G0N(
	Client IP Address that last communicated with the receiver. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getRxLastClientIP(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::rx_lock(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rx_lock" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getRxLock(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setRxLock(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::rx_missingpackets(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rx_missingpackets" << std::endl;
    os << R"V0G0N(
	Number of missing packets for receiver. If negative, they are packets in excess. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getNumMissingPackets(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::rx_padding(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rx_padding" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getPartialFramesPadding(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setPartialFramesPadding(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::rx_printconfig(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rx_printconfig" << std::endl;
    os << R"V0G0N(
	Prints the receiver configuration. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->printRxConfiguration(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::rx_realudpsocksize(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rx_realudpsocksize" << std::endl;
    os << R"V0G0N(
	Actual udp socket buffer size. Double the size of rx_udpsocksize due to kernel bookkeeping. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getRxRealUDPSocketBufferSize(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::rx_silent(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rx_silent" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getRxSilentMode(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setRxSilentMode(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::rx_start(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rx_start" << std::endl;
    os << R"V0G0N(
	Starts receiver listener for detector data packets and create a data file (if file write enabled). )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 0) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute rx_start at module level");
      }
      det->startReceiver();
      os << "successful" << '\n';
    }
  }

  return os.str();
}

std::string Caller::rx_stop(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rx_stop" << std::endl;
    os << R"V0G0N(
	Stops receiver listener for detector data packets and closes current data file (if file write enabled). )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 0) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute rx_stop at module level");
      }
      det->stopReceiver();
      os << "successful" << '\n';
    }
  }

  return os.str();
}

std::string Caller::rx_tcpport(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rx_tcpport" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getRxPort(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setRxPort(arg0, det_id);
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::rx_threads(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rx_threads" << std::endl;
    os << R"V0G0N(
	Get kernel thread ids from the receiver in order of [parent, tcp, listener 0, processor 0, streamer 0, listener 1, processor 1, streamer 1, arping]. If no streamer yet or there is no second interface, it gives 0 in its place. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getRxThreadIds(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::rx_udpsocksize(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rx_udpsocksize" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getRxUDPSocketBufferSize(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setRxUDPSocketBufferSize(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::rx_version(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rx_version" << std::endl;
    os << R"V0G0N(
	Receiver version )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getReceiverVersion(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::rx_zmqfreq(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rx_zmqfreq" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getRxZmqFrequency(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setRxZmqFrequency(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::rx_zmqhwm(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rx_zmqhwm" << std::endl;
    os << R"V0G0N([n_value]
	Receiver's zmq send high water mark. Default is the zmq library's default (1000). This is a high number and can be set to 2 for gui purposes. One must also set the client's receive high water mark to similar value. Final effect is sum of them. Also restarts receiver zmq streaming if enabled. Can set to -1 to set default value. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getRxZmqHwm(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute rx_zmqhwm at module level");
      }
      auto arg0 = StringTo<int>(args[0]);
      det->setRxZmqHwm(arg0);
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::rx_zmqip(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rx_zmqip" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getRxZmqIP(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      det->setRxZmqIP(IpAddr(args[0]), std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::rx_zmqport(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rx_zmqport" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getRxZmqPort(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setRxZmqPort(arg0, det_id);
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::rx_zmqstartfnum(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rx_zmqstartfnum" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getRxZmqStartingFrame(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setRxZmqStartingFrame(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::rx_zmqstream(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: rx_zmqstream" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getRxZmqDataStream(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setRxZmqDataStream(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::savepattern(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: savepattern" << std::endl;
    os << R"V0G0N(
	[Ctb][Mythen3] Saves pattern to file (ascii). 
	[Ctb] Also executes pattern. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 1) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 1) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute savepattern at module level");
      }
      det->savePattern(args[0]);
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::scanerrmsg(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: scanerrmsg" << std::endl;
    os << R"V0G0N(
	Gets Scan error message if scan ended in error for non blocking acquisitions. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getScanErrorMessage(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::selinterface(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: selinterface" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getSelectedUDPInterface(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->selectUDPInterface(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::serialnumber(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: serialnumber" << std::endl;
    os << R"V0G0N(
	[Jungfrau][Moench][Gotthard][Mythen3][Gotthard2][CTB]
Serial number of detector. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getSerialNumber(std::vector<int>{ det_id });
      os << OutStringHex(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::settings(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: settings" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
        StringTo<slsDetectorDefs::detectorSettings>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 0 to "
                           "slsDetectorDefs::detectorSettings");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getSettings(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<slsDetectorDefs::detectorSettings>(args[0]);
      det->setSettings(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::settingslist(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: settingslist" << std::endl;
    os << R"V0G0N(
	List of settings implemented for this detector. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getSettingsList();
      os << ToString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::settingspath(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: settingspath" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getSettingsPath(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      det->setSettingsPath(args[0], std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::signalindex(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: signalindex" << std::endl;
    os << R"V0G0N([name] 
		[ChipTestBoard] Get the signal index for the given name. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 1) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 1) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 1) {
      if (det->getDetectorType().squash() != defs::CHIPTESTBOARD) {
        throw RuntimeError(cmd + " only allowed for CTB.");
      }
      if (det_id != -1) {
        throw RuntimeError("Cannot execute signalindex at module level");
      }
      auto t = det->getSignalIndex(args[0]);
      os << static_cast<int>(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::signalname(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: signalname" << std::endl;
    os << R"V0G0N([0-63][name] 
		[ChipTestBoard] Set the signal at the given position to the given name. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 1) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 1) {
      try {
        StringTo<int>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 0 to int");
      }
    }

  } else if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 2) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 2) {
      try {
        StringTo<int>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 0 to int");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 1) {
      if (det->getDetectorType().squash() != defs::CHIPTESTBOARD) {
        throw RuntimeError(cmd + " only allowed for CTB.");
      }
      if (det_id != -1) {
        throw RuntimeError("Cannot execute signalname at module level");
      }
      auto arg0 = StringTo<int>(args[0]);
      auto t = det->getSignalName(arg0);
      os << args[0] << ' ' << t << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 2) {
      if (det->getDetectorType().squash() != defs::CHIPTESTBOARD) {
        throw RuntimeError(cmd + " only allowed for CTB.");
      }
      if (det_id != -1) {
        throw RuntimeError("Cannot execute signalname at module level");
      }
      auto arg0 = StringTo<int>(args[0]);
      det->setSignalName(arg0, args[1]);
      os << ToString(args) << '\n';
    }
  }

  return os.str();
}

std::string Caller::slowadcindex(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: slowadcindex" << std::endl;
    os << R"V0G0N([name] 
		[ChipTestBoard] Get the slowadc index for the given name. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 1) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 1) {
      defs::dacIndex index = defs::SLOW_ADC0;
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 1) {
      defs::dacIndex index = defs::SLOW_ADC0;
      if (det->getDetectorType().squash() != defs::CHIPTESTBOARD) {
        throw RuntimeError(cmd + " only allowed for CTB.");
      }
      if (det_id != -1) {
        throw RuntimeError("Cannot execute slowadcindex at module level");
      }
      auto t = det->getSlowADCIndex(args[0]);
      os << ToString(static_cast<int>(t) - index) << '\n';
    }
  }

  return os.str();
}

std::string Caller::slowadcname(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: slowadcname" << std::endl;
    os << R"V0G0N([0-7][name] 
		[ChipTestBoard] Set the slowadc at the given position to the given name. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 1) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 1) {
      defs::dacIndex index = defs::SLOW_ADC0;
    }

  } else if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 2) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 2) {
      defs::dacIndex index = defs::SLOW_ADC0;
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 1) {
      defs::dacIndex index = defs::SLOW_ADC0;
      if (det->getDetectorType().squash() != defs::CHIPTESTBOARD) {
        throw RuntimeError(cmd + " only allowed for CTB.");
      }
      if (det_id != -1) {
        throw RuntimeError("Cannot execute slowadcname at module level");
      }
      auto t = det->getSlowADCName(
          static_cast<defs::dacIndex>(StringTo<int>(args[0]) + index));
      os << args[0] << ' ' << t << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 2) {
      defs::dacIndex index = defs::SLOW_ADC0;
      if (det->getDetectorType().squash() != defs::CHIPTESTBOARD) {
        throw RuntimeError(cmd + " only allowed for CTB.");
      }
      if (det_id != -1) {
        throw RuntimeError("Cannot execute slowadcname at module level");
      }
      det->setSlowADCName(
          static_cast<defs::dacIndex>(StringTo<int>(args[0]) + index), args[1]);
      os << ToString(args) << '\n';
    }
  }

  return os.str();
}

std::string Caller::slowadcvalues(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: slowadcvalues" << std::endl;
    os << R"V0G0N([name] 
		[ChipTestBoard] Get values of all slow adcs. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {

      std::string suffix = " mV";
      auto t = det->getSlowADCList();
      auto names = det->getSlowADCNames();
      auto name_it = names.begin();
      os << '[';
      auto it = t.cbegin();
      os << ToString(*name_it++) << ' ';
      os << OutString(det->getSlowADC(*it++, std::vector<int>{ det_id }))
         << suffix;
      while (it != t.cend()) {
        os << ", " << ToString(*name_it++) << ' ';
        os << OutString(det->getSlowADC(*it++, std::vector<int>{ det_id }))
           << suffix;
      }
      os << "]\n";
    }
  }

  return os.str();
}

std::string Caller::start(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: start" << std::endl;
    os << R"V0G0N(
	Starts detector acquisition. Status changes to RUNNING or WAITING and automatically returns to idle at the end of acquisition. If the acquisition was abruptly stopped, some detectors come back to STOPPED. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 0) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute start at module level");
      }
      det->startDetector(std::vector<int>{ det_id });
      os << "successful" << '\n';
    }
  }

  return os.str();
}

std::string Caller::stop(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: stop" << std::endl;
    os << R"V0G0N(
	Abort detector acquisition. Status changes to IDLE or STOPPED. Goes to stop server. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 0) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute stop at module level");
      }
      det->stopDetector(std::vector<int>{ det_id });
      os << "successful" << '\n';
    }
  }

  return os.str();
}

std::string Caller::stopport(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: stopport" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getStopPort(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setStopPort(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::storagecell_delay(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: storagecell_delay" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getStorageCellDelay(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }

    if (args.size() == 1) {
      auto t = det->getStorageCellDelay(std::vector<int>{ det_id });
      os << OutString(t, args[0]) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      std::string tmp_time(args[0]);
      std::string unit = RemoveUnit(tmp_time);
      auto converted_time = StringTo<time::ns>(tmp_time, unit);
      det->setStorageCellDelay(converted_time, std::vector<int>{ det_id });
      os << args[0] << '\n';
    }

    if (args.size() == 2) {
      auto converted_time = StringTo<time::ns>(args[0], args[1]);
      det->setStorageCellDelay(converted_time, std::vector<int>{ det_id });
      os << args[0] << args[1] << '\n';
    }
  }

  return os.str();
}

std::string Caller::storagecell_start(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: storagecell_start" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getStorageCellStart(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setStorageCellStart(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::subdeadtime(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: subdeadtime" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getSubDeadTime(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }

    if (args.size() == 1) {
      auto t = det->getSubDeadTime(std::vector<int>{ det_id });
      os << OutString(t, args[0]) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      std::string tmp_time(args[0]);
      std::string unit = RemoveUnit(tmp_time);
      auto converted_time = StringTo<time::ns>(tmp_time, unit);
      det->setSubDeadTime(converted_time, std::vector<int>{ det_id });
      os << args[0] << '\n';
    }

    if (args.size() == 2) {
      auto converted_time = StringTo<time::ns>(args[0], args[1]);
      det->setSubDeadTime(converted_time, std::vector<int>{ det_id });
      os << args[0] << args[1] << '\n';
    }
  }

  return os.str();
}

std::string Caller::subexptime(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: subexptime" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getSubExptime(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }

    if (args.size() == 1) {
      auto t = det->getSubExptime(std::vector<int>{ det_id });
      os << OutString(t, args[0]) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      std::string tmp_time(args[0]);
      std::string unit = RemoveUnit(tmp_time);
      auto converted_time = StringTo<time::ns>(tmp_time, unit);
      det->setSubExptime(converted_time, std::vector<int>{ det_id });
      os << args[0] << '\n';
    }

    if (args.size() == 2) {
      auto converted_time = StringTo<time::ns>(args[0], args[1]);
      det->setSubExptime(converted_time, std::vector<int>{ det_id });
      os << args[0] << args[1] << '\n';
    }
  }

  return os.str();
}

std::string Caller::sync(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: sync" << std::endl;
    os << R"V0G0N([0, 1]
	[Jungfrau][Moench] Enables or disables synchronization between modules. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getSynchronization(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute sync at module level");
      }
      auto arg0 = StringTo<int>(args[0]);
      det->setSynchronization(arg0);
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::syncclk(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: syncclk" << std::endl;
    os << R"V0G0N([n_clk in MHz]
	[Ctb] Sync clock in MHz. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getSYNCClock(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::temp_10ge(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: temp_10ge" << std::endl;
    os << R"V0G0N([n_value]
	[Eiger]Temperature close to the 10GbE )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getTemperature(slsDetectorDefs::TEMPERATURE_10GE,
                                   std::vector<int>{ det_id });
      os << OutString(t) << " °C" << '\n';
    }
  }

  return os.str();
}

std::string Caller::temp_adc(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: temp_adc" << std::endl;
    os << R"V0G0N([n_value]
	[Jungfrau][Moench][Gotthard] ADC Temperature )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getTemperature(slsDetectorDefs::TEMPERATURE_ADC,
                                   std::vector<int>{ det_id });
      os << OutString(t) << " °C" << '\n';
    }
  }

  return os.str();
}

std::string Caller::temp_control(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: temp_control" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getTemperatureControl(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setTemperatureControl(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::temp_dcdc(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: temp_dcdc" << std::endl;
    os << R"V0G0N([n_value]
	[Eiger]Temperature close to the dc dc converter )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getTemperature(slsDetectorDefs::TEMPERATURE_DCDC,
                                   std::vector<int>{ det_id });
      os << OutString(t) << " °C" << '\n';
    }
  }

  return os.str();
}

std::string Caller::temp_fpga(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: temp_fpga" << std::endl;
    os << R"V0G0N([n_value]
	[Eiger][Jungfrau][Moench][Gotthard][Mythen3][Gotthard2] FPGA Temperature )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getTemperature(slsDetectorDefs::TEMPERATURE_FPGA,
                                   std::vector<int>{ det_id });
      os << OutString(t) << " °C" << '\n';
    }
  }

  return os.str();
}

std::string Caller::temp_fpgaext(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: temp_fpgaext" << std::endl;
    os << R"V0G0N([n_value]
	[Eiger]Temperature close to the FPGA )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getTemperature(slsDetectorDefs::TEMPERATURE_FPGAEXT,
                                   std::vector<int>{ det_id });
      os << OutString(t) << " °C" << '\n';
    }
  }

  return os.str();
}

std::string Caller::temp_fpgafl(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: temp_fpgafl" << std::endl;
    os << R"V0G0N([n_value]
	[Eiger]Temperature of the left front end board fpga. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getTemperature(slsDetectorDefs::TEMPERATURE_FPGA2,
                                   std::vector<int>{ det_id });
      os << OutString(t) << " °C" << '\n';
    }
  }

  return os.str();
}

std::string Caller::temp_fpgafr(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: temp_fpgafr" << std::endl;
    os << R"V0G0N([n_value]
	[Eiger]Temperature of the left front end board fpga. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getTemperature(slsDetectorDefs::TEMPERATURE_FPGA3,
                                   std::vector<int>{ det_id });
      os << OutString(t) << " °C" << '\n';
    }
  }

  return os.str();
}

std::string Caller::temp_slowadc(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: temp_slowadc" << std::endl;
    os << R"V0G0N([n_value]
	[Ctb]Temperature of the slow adc )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getTemperature(slsDetectorDefs::SLOW_ADC_TEMP,
                                   std::vector<int>{ det_id });
      os << OutString(t) << " °C" << '\n';
    }
  }

  return os.str();
}

std::string Caller::temp_sodl(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: temp_sodl" << std::endl;
    os << R"V0G0N([n_value]
	[Eiger]Temperature close to the left so-dimm memory )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getTemperature(slsDetectorDefs::TEMPERATURE_SODL,
                                   std::vector<int>{ det_id });
      os << OutString(t) << " °C" << '\n';
    }
  }

  return os.str();
}

std::string Caller::temp_sodr(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: temp_sodr" << std::endl;
    os << R"V0G0N([n_value]
	[Eiger]Temperature close to the right so-dimm memory )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getTemperature(slsDetectorDefs::TEMPERATURE_SODR,
                                   std::vector<int>{ det_id });
      os << OutString(t) << " °C" << '\n';
    }
  }

  return os.str();
}

std::string Caller::temp_threshold(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: temp_threshold" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getThresholdTemperature(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setThresholdTemperature(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::templist(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: templist" << std::endl;
    os << R"V0G0N(
	List of temperature commands implemented for this detector. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getTemperatureList();
      os << ToString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::tengiga(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: tengiga" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getTenGiga(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setTenGiga(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::timing(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: timing" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
        StringTo<slsDetectorDefs::timingMode>(args[0]);
      }
      catch (...) {
        throw RuntimeError(
            "Could not convert argument 0 to slsDetectorDefs::timingMode");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getTimingMode(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<slsDetectorDefs::timingMode>(args[0]);
      det->setTimingMode(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::timinglist(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: timinglist" << std::endl;
    os << R"V0G0N(
	Gets the list of timing modes for this detector. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getTimingModeList();
      os << ToString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::timingsource(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: timingsource" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
        StringTo<slsDetectorDefs::timingSourceType>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 0 to "
                           "slsDetectorDefs::timingSourceType");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getTimingSource(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<slsDetectorDefs::timingSourceType>(args[0]);
      det->setTimingSource(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::top(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: top" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getTop(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setTop(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::transceiverenable(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: transceiverenable" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
        StringTo<uint32_t>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 0 to uint32_t");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getTransceiverEnableMask(std::vector<int>{ det_id });
      os << OutStringHex(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<uint32_t>(args[0]);
      det->setTransceiverEnableMask(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::triggers(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: triggers" << std::endl;
    os << R"V0G0N([n_triggers]
	Number of triggers per aquire. Set timing mode to use triggers. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
        StringTo<int64_t>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 0 to int64_t");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getNumberOfTriggers(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute triggers at module level");
      }
      auto arg0 = StringTo<int64_t>(args[0]);
      det->setNumberOfTriggers(arg0);
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::triggersl(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: triggersl" << std::endl;
    os << R"V0G0N(
	[Gotthard][Jungfrau][Moench][Mythen3][Gotthard2][CTB] Number of triggers left in acquisition. Only when external trigger used. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getNumberOfTriggersLeft(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::trimval(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: trimval" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getAllTrimbits(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setAllTrimbits(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::tsamples(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: tsamples" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getNumberOfTransceiverSamples(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setNumberOfTransceiverSamples(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::txdelay_frame(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: txdelay_frame" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getTransmissionDelayFrame(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setTransmissionDelayFrame(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::txdelay_left(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: txdelay_left" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getTransmissionDelayLeft(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setTransmissionDelayLeft(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::txdelay_right(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: txdelay_right" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getTransmissionDelayRight(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setTransmissionDelayRight(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::type(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: type" << std::endl;
    os << R"V0G0N(
	Returns detector type. Can be Eiger, Jungfrau, Gotthard, Moench, Mythen3, Gotthard2, ChipTestBoard )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getDetectorType(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::udp_cleardst(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: udp_cleardst" << std::endl;
    os << R"V0G0N(
	Clears udp destination details on the detector. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 0) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute udp_cleardst at module level");
      }
      det->clearUDPDestinations(std::vector<int>{ det_id });
      os << "successful" << '\n';
    }
  }

  return os.str();
}

std::string Caller::udp_dstmac(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: udp_dstmac" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getDestinationUDPMAC(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      det->setDestinationUDPMAC(MacAddr(args[0]), std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::udp_dstmac2(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: udp_dstmac2" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getDestinationUDPMAC2(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      det->setDestinationUDPMAC2(MacAddr(args[0]), std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::udp_dstport(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: udp_dstport" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getDestinationUDPPort(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setDestinationUDPPort(arg0, det_id);
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::udp_dstport2(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: udp_dstport2" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getDestinationUDPPort2(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setDestinationUDPPort2(arg0, det_id);
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::udp_firstdst(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: udp_firstdst" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getFirstUDPDestination(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setFirstUDPDestination(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::udp_numdst(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: udp_numdst" << std::endl;
    os << R"V0G0N(
	[Jungfrau][Moench][Eiger][Mythen3][Gotthard2] One can enter upto 32 (64 for Mythen3) destinations that the detector will stream images out in a round robin fashion. This is get only command. Default: 1 )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getNumberofUDPDestinations(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  return os.str();
}

std::string Caller::udp_reconfigure(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: udp_reconfigure" << std::endl;
    os << R"V0G0N(
	Reconfigures Detector with UDP destination. More for debugging as the configuration is done automatically when the detector has sufficient UDP details. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 0) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute udp_reconfigure at module level");
      }
      det->reconfigureUDPDestination(std::vector<int>{ det_id });
      os << "successful" << '\n';
    }
  }

  return os.str();
}

std::string Caller::udp_srcmac(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: udp_srcmac" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getSourceUDPMAC(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      det->setSourceUDPMAC(MacAddr(args[0]), std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::udp_srcmac2(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: udp_srcmac2" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getSourceUDPMAC2(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      det->setSourceUDPMAC2(MacAddr(args[0]), std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::udp_validate(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: udp_validate" << std::endl;
    os << R"V0G0N(
	Validates that UDP configuration in the detector is valid. If not configured, it will throw with error message requesting missing udp information. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 0) {
      if (det_id != -1) {
        throw RuntimeError("Cannot execute udp_validate at module level");
      }
      det->validateUDPConfiguration(std::vector<int>{ det_id });
      os << "successful" << '\n';
    }
  }

  return os.str();
}

std::string Caller::updatemode(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: updatemode" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getUpdateMode(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setUpdateMode(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::v_a(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: v_a" << std::endl;
    os << R"V0G0N([n_value]
	[Ctb] Voltage supply a in mV. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
        throw RuntimeError("Could not convert argument 1 to int");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getVoltage(defs::V_POWER_A, std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg1 = StringTo<int>(args[0]);
      det->setVoltage(defs::V_POWER_A, arg1, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::v_b(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: v_b" << std::endl;
    os << R"V0G0N([n_value]
	[Ctb] Voltage supply b in mV. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
        throw RuntimeError("Could not convert argument 1 to int");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getVoltage(defs::V_POWER_B, std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg1 = StringTo<int>(args[0]);
      det->setVoltage(defs::V_POWER_B, arg1, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::v_c(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: v_c" << std::endl;
    os << R"V0G0N([n_value]
	[Ctb] Voltage supply c in mV. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
        throw RuntimeError("Could not convert argument 1 to int");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getVoltage(defs::V_POWER_C, std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg1 = StringTo<int>(args[0]);
      det->setVoltage(defs::V_POWER_C, arg1, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::v_chip(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: v_chip" << std::endl;
    os << R"V0G0N([n_value]
	[Ctb] Voltage supply chip in mV. Do not use it unless you are completely sure you will not fry the board. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
        throw RuntimeError("Could not convert argument 1 to int");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getVoltage(defs::V_POWER_CHIP, std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg1 = StringTo<int>(args[0]);
      det->setVoltage(defs::V_POWER_CHIP, arg1, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::v_d(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: v_d" << std::endl;
    os << R"V0G0N([n_value]
	[Ctb] Voltage supply d in mV. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
        throw RuntimeError("Could not convert argument 1 to int");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getVoltage(defs::V_POWER_D, std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg1 = StringTo<int>(args[0]);
      det->setVoltage(defs::V_POWER_D, arg1, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::v_io(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: v_io" << std::endl;
    os << R"V0G0N([n_value]
	[Ctb] Voltage supply io in mV. Minimum 1200 mV. Must be the first power regulator to be set after fpga reset (on-board detector server start up). )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
        throw RuntimeError("Could not convert argument 1 to int");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getVoltage(defs::V_POWER_IO, std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg1 = StringTo<int>(args[0]);
      det->setVoltage(defs::V_POWER_IO, arg1, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::v_limit(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: v_limit" << std::endl;
    os << R"V0G0N([n_value]
	[Ctb] Soft limit for power supplies (ctb only) and DACS in mV. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
        throw RuntimeError("Could not convert argument 1 to int");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getVoltage(defs::V_LIMIT, std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg1 = StringTo<int>(args[0]);
      det->setVoltage(defs::V_LIMIT, arg1, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::vchip_comp_adc(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: vchip_comp_adc" << std::endl;
    os << R"V0G0N([chip index 0-10, -1 for all][10 bit hex value] 
	[Gotthard2] On chip Dac for comparator current of ADC. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 1) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 1) {
      try {
        StringTo<int>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 1 to int");
      }
    }

  } else if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 2) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 2) {
      try {
        StringTo<int>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 1 to int");
      }
      try {
        StringTo<int>(args[1]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 2 to int");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 1) {
      auto arg1 = StringTo<int>(args[0]);
      auto t = det->getOnChipDAC(defs::VB_COMP_ADC, arg1,
                                 std::vector<int>{ det_id });
      os << args[0] << OutStringHex(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 2) {
      auto arg1 = StringTo<int>(args[0]);
      auto arg2 = StringTo<int>(args[1]);
      det->setOnChipDAC(defs::VB_COMP_ADC, arg1, arg2,
                        std::vector<int>{ det_id });
      os << args[0] << args[1] << '\n';
    }
  }

  return os.str();
}

std::string Caller::vchip_comp_fe(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: vchip_comp_fe" << std::endl;
    os << R"V0G0N([chip index 0-10, -1 for all][10 bit hex value] 
	[Gotthard2] On chip Dac for comparator current of analogue front end. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 1) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 1) {
      try {
        StringTo<int>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 1 to int");
      }
    }

  } else if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 2) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 2) {
      try {
        StringTo<int>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 1 to int");
      }
      try {
        StringTo<int>(args[1]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 2 to int");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 1) {
      auto arg1 = StringTo<int>(args[0]);
      auto t =
          det->getOnChipDAC(defs::VB_COMP_FE, arg1, std::vector<int>{ det_id });
      os << args[0] << OutStringHex(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 2) {
      auto arg1 = StringTo<int>(args[0]);
      auto arg2 = StringTo<int>(args[1]);
      det->setOnChipDAC(defs::VB_COMP_FE, arg1, arg2,
                        std::vector<int>{ det_id });
      os << args[0] << args[1] << '\n';
    }
  }

  return os.str();
}

std::string Caller::vchip_cs(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: vchip_cs" << std::endl;
    os << R"V0G0N([chip index 0-10, -1 for all][10 bit hex value] 
	[Gotthard2] On chip Dac for current injection into preamplifier. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 1) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 1) {
      try {
        StringTo<int>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 1 to int");
      }
    }

  } else if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 2) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 2) {
      try {
        StringTo<int>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 1 to int");
      }
      try {
        StringTo<int>(args[1]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 2 to int");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 1) {
      auto arg1 = StringTo<int>(args[0]);
      auto t = det->getOnChipDAC(defs::VB_CS, arg1, std::vector<int>{ det_id });
      os << args[0] << OutStringHex(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 2) {
      auto arg1 = StringTo<int>(args[0]);
      auto arg2 = StringTo<int>(args[1]);
      det->setOnChipDAC(defs::VB_CS, arg1, arg2, std::vector<int>{ det_id });
      os << args[0] << args[1] << '\n';
    }
  }

  return os.str();
}

std::string Caller::vchip_opa_1st(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: vchip_opa_1st" << std::endl;
    os << R"V0G0N([chip index 0-10, -1 for all][10 bit hex value] 
	[Gotthard2] On chip Dac for opa current for driving the other DACs in chip. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 1) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 1) {
      try {
        StringTo<int>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 1 to int");
      }
    }

  } else if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 2) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 2) {
      try {
        StringTo<int>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 1 to int");
      }
      try {
        StringTo<int>(args[1]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 2 to int");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 1) {
      auto arg1 = StringTo<int>(args[0]);
      auto t =
          det->getOnChipDAC(defs::VB_OPA_1ST, arg1, std::vector<int>{ det_id });
      os << args[0] << OutStringHex(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 2) {
      auto arg1 = StringTo<int>(args[0]);
      auto arg2 = StringTo<int>(args[1]);
      det->setOnChipDAC(defs::VB_OPA_1ST, arg1, arg2,
                        std::vector<int>{ det_id });
      os << args[0] << args[1] << '\n';
    }
  }

  return os.str();
}

std::string Caller::vchip_opa_fd(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: vchip_opa_fd" << std::endl;
    os << R"V0G0N([chip index 0-10, -1 for all][10 bit hex value] 
	[Gotthard2] On chip Dac current for CDS opa stage. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 1) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 1) {
      try {
        StringTo<int>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 1 to int");
      }
    }

  } else if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 2) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 2) {
      try {
        StringTo<int>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 1 to int");
      }
      try {
        StringTo<int>(args[1]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 2 to int");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 1) {
      auto arg1 = StringTo<int>(args[0]);
      auto t =
          det->getOnChipDAC(defs::VB_OPA_FD, arg1, std::vector<int>{ det_id });
      os << args[0] << OutStringHex(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 2) {
      auto arg1 = StringTo<int>(args[0]);
      auto arg2 = StringTo<int>(args[1]);
      det->setOnChipDAC(defs::VB_OPA_FD, arg1, arg2,
                        std::vector<int>{ det_id });
      os << args[0] << args[1] << '\n';
    }
  }

  return os.str();
}

std::string Caller::vchip_ref_comp_fe(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: vchip_ref_comp_fe" << std::endl;
    os << R"V0G0N([chip index 0-10, -1 for all][10 bit hex value] 
	[Gotthard2] On chip Dac for reference voltage of the comparator of analogue front end. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 1) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 1) {
      try {
        StringTo<int>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 1 to int");
      }
    }

  } else if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 2) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 2) {
      try {
        StringTo<int>(args[0]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 1 to int");
      }
      try {
        StringTo<int>(args[1]);
      }
      catch (...) {
        throw RuntimeError("Could not convert argument 2 to int");
      }
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 1) {
      auto arg1 = StringTo<int>(args[0]);
      auto t = det->getOnChipDAC(defs::VREF_COMP_FE, arg1,
                                 std::vector<int>{ det_id });
      os << args[0] << OutStringHex(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 2) {
      auto arg1 = StringTo<int>(args[0]);
      auto arg2 = StringTo<int>(args[1]);
      det->setOnChipDAC(defs::VREF_COMP_FE, arg1, arg2,
                        std::vector<int>{ det_id });
      os << args[0] << args[1] << '\n';
    }
  }

  return os.str();
}

std::string Caller::veto(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: veto" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getVeto(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setVeto(arg0, std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::vm_a(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: vm_a" << std::endl;
    os << R"V0G0N(
	[Ctb] Measured voltage of power supply a in mV. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t =
          det->getMeasuredVoltage(defs::V_POWER_A, std::vector<int>{ det_id });
      os << OutString(t) << " °C" << '\n';
    }
  }

  return os.str();
}

std::string Caller::vm_b(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: vm_b" << std::endl;
    os << R"V0G0N(
	[Ctb] Measured voltage of power supply b in mV. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t =
          det->getMeasuredVoltage(defs::V_POWER_B, std::vector<int>{ det_id });
      os << OutString(t) << " °C" << '\n';
    }
  }

  return os.str();
}

std::string Caller::vm_c(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: vm_c" << std::endl;
    os << R"V0G0N(
	[Ctb] Measured voltage of power supply c in mV. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t =
          det->getMeasuredVoltage(defs::V_POWER_C, std::vector<int>{ det_id });
      os << OutString(t) << " °C" << '\n';
    }
  }

  return os.str();
}

std::string Caller::vm_d(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: vm_d" << std::endl;
    os << R"V0G0N(
	[Ctb] Measured voltage of power supply d in mV. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t =
          det->getMeasuredVoltage(defs::V_POWER_D, std::vector<int>{ det_id });
      os << OutString(t) << " °C" << '\n';
    }
  }

  return os.str();
}

std::string Caller::vm_io(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: vm_io" << std::endl;
    os << R"V0G0N(
	[Ctb] Measured voltage of power supply io in mV. )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t =
          det->getMeasuredVoltage(defs::V_POWER_IO, std::vector<int>{ det_id });
      os << OutString(t) << " °C" << '\n';
    }
  }

  return os.str();
}

std::string Caller::voltageindex(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: voltageindex" << std::endl;
    os << R"V0G0N([name] 
		[ChipTestBoard] Get the voltage index for the given name. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 1) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 1) {
      defs::dacIndex index = defs::V_POWER_A;
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 1) {
      defs::dacIndex index = defs::V_POWER_A;
      if (det->getDetectorType().squash() != defs::CHIPTESTBOARD) {
        throw RuntimeError(cmd + " only allowed for CTB.");
      }
      if (det_id != -1) {
        throw RuntimeError("Cannot execute voltageindex at module level");
      }
      auto t = det->getVoltageIndex(args[0]);
      os << ToString(static_cast<int>(t) - index) << '\n';
    }
  }

  return os.str();
}

std::string Caller::voltagename(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: voltagename" << std::endl;
    os << R"V0G0N([0-4][name] 
		[ChipTestBoard] Set the voltage at the given position to the given name. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 1) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 1) {
      defs::dacIndex index = defs::V_POWER_A;
    }

  } else if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() != 2) {
      throw RuntimeError("Wrong number of arguments for action PUT");
    }

    if (args.size() == 2) {
      defs::dacIndex index = defs::V_POWER_A;
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 1) {
      defs::dacIndex index = defs::V_POWER_A;
      if (det->getDetectorType().squash() != defs::CHIPTESTBOARD) {
        throw RuntimeError(cmd + " only allowed for CTB.");
      }
      if (det_id != -1) {
        throw RuntimeError("Cannot execute voltagename at module level");
      }
      auto t = det->getVoltageName(
          static_cast<defs::dacIndex>(StringTo<int>(args[0]) + index));
      os << args[0] << ' ' << t << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 2) {
      defs::dacIndex index = defs::V_POWER_A;
      if (det->getDetectorType().squash() != defs::CHIPTESTBOARD) {
        throw RuntimeError(cmd + " only allowed for CTB.");
      }
      if (det_id != -1) {
        throw RuntimeError("Cannot execute voltagename at module level");
      }
      det->setVoltageName(
          static_cast<defs::dacIndex>(StringTo<int>(args[0]) + index), args[1]);
      os << ToString(args) << '\n';
    }
  }

  return os.str();
}

std::string Caller::voltagevalues(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: voltagevalues" << std::endl;
    os << R"V0G0N([name] 
		[ChipTestBoard] Get values of all voltages. )V0G0N"
       << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
  }

  // check if action and arguments are valid
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() != 0) {
      throw RuntimeError("Wrong number of arguments for action GET");
    }

    if (args.size() == 0) {
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {

      std::string suffix = " mV";
      auto t = det->getVoltageList();
      auto names = det->getVoltageNames();
      auto name_it = names.begin();
      os << '[';
      auto it = t.cbegin();
      os << ToString(*name_it++) << ' ';
      os << OutString(det->getVoltage(*it++, std::vector<int>{ det_id }))
         << suffix;
      while (it != t.cend()) {
        os << ", " << ToString(*name_it++) << ' ';
        os << OutString(det->getVoltage(*it++, std::vector<int>{ det_id }))
           << suffix;
      }
      os << "]\n";
    }
  }

  return os.str();
}

std::string Caller::zmqip(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: zmqip" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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
    }

  } else {

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getClientZmqIp(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      det->setClientZmqIp(IpAddr(args[0]), std::vector<int>{ det_id });
      os << args.front() << '\n';
    }
  }

  return os.str();
}

std::string Caller::zmqport(int action) {

  std::ostringstream os;
  // print help
  if (action == slsDetectorDefs::HELP_ACTION) {
    os << "Command: zmqport" << std::endl;
    os << R"V0G0N( )V0G0N" << std::endl;
    return os.str();
  }

  // infer action based on number of arguments
  if (action == -1) {
    throw RuntimeError("infer_action is disabled");
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

    throw RuntimeError("Invalid action: supported actions are ['GET', 'PUT']");
  }

  // generate code for each action
  auto detector_type = det->getDetectorType().squash();
  if (action == slsDetectorDefs::GET_ACTION) {
    if (args.size() == 0) {
      auto t = det->getClientZmqPort(std::vector<int>{ det_id });
      os << OutString(t) << '\n';
    }
  }

  if (action == slsDetectorDefs::PUT_ACTION) {
    if (args.size() == 1) {
      auto arg0 = StringTo<int>(args[0]);
      det->setClientZmqPort(arg0, det_id);
      os << args.front() << '\n';
    }
  }

  return os.str();
}
}