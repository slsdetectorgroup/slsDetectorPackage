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

std::string Caller::adcenable(int action) {

std::ostringstream os;
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
{

}
}

}

else if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() != 1) {
throw RuntimeError("Wrong number of arguments for action PUT");
}

if (args.size() == 1) {
{

try {
StringTo<uint32_t>(args[0]);
} catch (...) {
  throw RuntimeError("Could not convert argument 0 to uint32_t");
}
}
}

}

else {

throw RuntimeError("Invalid action PUT");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

auto t = det->getADCEnableMask(std::vector<int>{ det_id });
os << OutStringHex(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<uint32_t>(args[0]);
det->setADCEnableMask(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::adcenable10g(int action) {

std::ostringstream os;
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
{

}
}

}

else if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() != 1) {
throw RuntimeError("Wrong number of arguments for action PUT");
}

if (args.size() == 1) {
{

try {
StringTo<uint32_t>(args[0]);
} catch (...) {
  throw RuntimeError("Could not convert argument 0 to uint32_t");
}
}
}

}

else {

throw RuntimeError("Invalid action PUT");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

auto t = det->getTenGigaADCEnableMask(std::vector<int>{ det_id });
os << OutStringHex(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<uint32_t>(args[0]);
det->setTenGigaADCEnableMask(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::adcinvert(int action) {

std::ostringstream os;
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
{

}
}

}

else if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() != 1) {
throw RuntimeError("Wrong number of arguments for action PUT");
}

if (args.size() == 1) {
{

try {
StringTo<uint32_t>(args[0]);
} catch (...) {
  throw RuntimeError("Could not convert argument 0 to uint32_t");
}
}
}

}

else {

throw RuntimeError("Invalid action PUT");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

auto t = det->getADCInvert(std::vector<int>{ det_id });
os << OutStringHex(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<uint32_t>(args[0]);
det->setADCInvert(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::burstperiod(int action) {

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
{

}
}

if (args.size() == 1) {
{

}
}

}

else if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() != 1 && args.size() != 2) {
throw RuntimeError("Wrong number of arguments for action PUT");
}

if (args.size() == 1) {
{

try {
std::string tmp_time(args[0]);
std::string unit = RemoveUnit(tmp_time);
auto converted_time = StringTo < time::ns > (tmp_time, unit);
} catch (...) {  throw RuntimeError("Could not convert argument to time::ns");}
}
}

if (args.size() == 2) {
{

try {
StringTo < time::ns > (args[0], args[1]);
} catch (...) {  throw RuntimeError("Could not convert arguments to time::ns");}
}
}

}

else {

throw RuntimeError("Invalid action PUT");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

auto t = det->getBurstPeriod(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

if (args.size() == 1) {
{

auto t = det->getBurstPeriod(std::vector<int>{ det_id });
os << OutString(t , args[0]) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

std::string tmp_time(args[0]);
std::string unit = RemoveUnit(tmp_time);
auto converted_time = StringTo < time::ns > (tmp_time, unit);
det->setBurstPeriod(converted_time, std::vector<int>{ det_id });
os << args[0] << '\n';
}
}

if (args.size() == 2) {
{

auto converted_time = StringTo < time::ns > (args[0], args[1]);
det->setBurstPeriod(converted_time, std::vector<int>{ det_id });
os << args[0]<< args[1] << '\n';
}
}

}

return os.str();
}

std::string Caller::compdisabletime(int action) {

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
{

}
}

if (args.size() == 1) {
{

}
}

}

else if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() != 1 && args.size() != 2) {
throw RuntimeError("Wrong number of arguments for action PUT");
}

if (args.size() == 1) {
{

try {
std::string tmp_time(args[0]);
std::string unit = RemoveUnit(tmp_time);
auto converted_time = StringTo < time::ns > (tmp_time, unit);
} catch (...) {  throw RuntimeError("Could not convert argument to time::ns");}
}
}

if (args.size() == 2) {
{

try {
StringTo < time::ns > (args[0], args[1]);
} catch (...) {  throw RuntimeError("Could not convert arguments to time::ns");}
}
}

}

else {

throw RuntimeError("Invalid action PUT");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

auto t = det->getComparatorDisableTime(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

if (args.size() == 1) {
{

auto t = det->getComparatorDisableTime(std::vector<int>{ det_id });
os << OutString(t , args[0]) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

std::string tmp_time(args[0]);
std::string unit = RemoveUnit(tmp_time);
auto converted_time = StringTo < time::ns > (tmp_time, unit);
det->setComparatorDisableTime(converted_time, std::vector<int>{ det_id });
os << args[0] << '\n';
}
}

if (args.size() == 2) {
{

auto converted_time = StringTo < time::ns > (args[0], args[1]);
det->setComparatorDisableTime(converted_time, std::vector<int>{ det_id });
os << args[0]<< args[1] << '\n';
}
}

}

return os.str();
}

std::string Caller::delay(int action) {

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
{

}
}

if (args.size() == 1) {
{

}
}

}

else if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() != 1 && args.size() != 2) {
throw RuntimeError("Wrong number of arguments for action PUT");
}

if (args.size() == 1) {
{

try {
std::string tmp_time(args[0]);
std::string unit = RemoveUnit(tmp_time);
auto converted_time = StringTo < time::ns > (tmp_time, unit);
} catch (...) {  throw RuntimeError("Could not convert argument to time::ns");}
}
}

if (args.size() == 2) {
{

try {
StringTo < time::ns > (args[0], args[1]);
} catch (...) {  throw RuntimeError("Could not convert arguments to time::ns");}
}
}

}

else {

throw RuntimeError("Invalid action PUT");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

auto t = det->getDelayAfterTrigger(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

if (args.size() == 1) {
{

auto t = det->getDelayAfterTrigger(std::vector<int>{ det_id });
os << OutString(t , args[0]) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

std::string tmp_time(args[0]);
std::string unit = RemoveUnit(tmp_time);
auto converted_time = StringTo < time::ns > (tmp_time, unit);
det->setDelayAfterTrigger(converted_time, std::vector<int>{ det_id });
os << args[0] << '\n';
}
}

if (args.size() == 2) {
{

auto converted_time = StringTo < time::ns > (args[0], args[1]);
det->setDelayAfterTrigger(converted_time, std::vector<int>{ det_id });
os << args[0]<< args[1] << '\n';
}
}

}

return os.str();
}

std::string Caller::delayl(int action) {

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
{

}
}

if (args.size() == 1) {
{

}
}

}

else {

throw RuntimeError("Invalid action GET");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

auto t = det->getDelayAfterTriggerLeft(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

if (args.size() == 1) {
{

auto t = det->getDelayAfterTriggerLeft(std::vector<int>{ det_id });
os << OutString(t , args[0]) << '\n';
}
}

}

return os.str();
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
{

}
}

if (args.size() == 1) {
{

}
}

}

else if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() != 1 && args.size() != 2) {
throw RuntimeError("Wrong number of arguments for action PUT");
}

if (args.size() == 1) {
{

try {
std::string tmp_time(args[0]);
std::string unit = RemoveUnit(tmp_time);
auto converted_time = StringTo < time::ns > (tmp_time, unit);
} catch (...) {  throw RuntimeError("Could not convert argument to time::ns");}
}
}

if (args.size() == 2) {
{

try {
StringTo < time::ns > (args[0], args[1]);
} catch (...) {  throw RuntimeError("Could not convert arguments to time::ns");}
}
}

}

else {

throw RuntimeError("Invalid action PUT");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (detector_type == defs::MYTHEN3) {
if (args.size() == 0) {
{

auto t = det->getExptimeForAllGates(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

if (args.size() == 1) {
{

auto t = det->getExptimeForAllGates(std::vector<int>{ det_id });
os << OutString(t , args[0]) << '\n';
}
}

}

else {

if (args.size() == 0) {
{

auto t = det->getExptime(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

if (args.size() == 1) {
{

auto t = det->getExptime(std::vector<int>{ det_id });
os << OutString(t , args[0]) << '\n';
}
}

}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

std::string tmp_time(args[0]);
std::string unit = RemoveUnit(tmp_time);
auto converted_time = StringTo < time::ns > (tmp_time, unit);
det->setExptime(converted_time, std::vector<int>{ det_id });
os << args[0] << '\n';
}
}

if (args.size() == 2) {
{

auto converted_time = StringTo < time::ns > (args[0], args[1]);
det->setExptime(converted_time, std::vector<int>{ det_id });
os << args[0]<< args[1] << '\n';
}
}

}

return os.str();
}

std::string Caller::exptime1(int action) {

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
{

int gateIndex = 0;
}
}

if (args.size() == 1) {
{

int gateIndex = 0;
}
}

}

else if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() != 1 && args.size() != 2) {
throw RuntimeError("Wrong number of arguments for action PUT");
}

if (args.size() == 1) {
{

int gateIndex = 0;
try {
std::string tmp_time(args[0]);
std::string unit = RemoveUnit(tmp_time);
auto converted_time = StringTo < time::ns > (tmp_time, unit);
} catch (...) {  throw RuntimeError("Could not convert argument to time::ns");}
}
}

if (args.size() == 2) {
{

int gateIndex = 0;
try {
StringTo < time::ns > (args[0], args[1]);
} catch (...) {  throw RuntimeError("Could not convert arguments to time::ns");}
}
}

}

else {

throw RuntimeError("Invalid action PUT");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

int gateIndex = 0;
auto t = det->getExptime(gateIndex, std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

if (args.size() == 1) {
{

int gateIndex = 0;
auto t = det->getExptime(gateIndex, std::vector<int>{ det_id });
os << OutString(t , args[0]) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (detector_type == defs::MYTHEN3) {
if (args.size() == 1) {
{

int gateIndex = 0;
std::string tmp_time(args[0]);
std::string unit = RemoveUnit(tmp_time);
auto converted_time = StringTo < time::ns > (tmp_time, unit);
det->setExptime(gateIndex, converted_time, std::vector<int>{ det_id });
os << args[0] << '\n';
}
}

if (args.size() == 2) {
{

int gateIndex = 0;
auto converted_time = StringTo < time::ns > (args[0], args[1]);
det->setExptime(gateIndex, converted_time, std::vector<int>{ det_id });
os << args[0]<< args[1] << '\n';
}
}

}

else {

if (args.size() == 1) {
{

int gateIndex = 0;
std::string tmp_time(args[0]);
std::string unit = RemoveUnit(tmp_time);
auto converted_time = StringTo < time::ns > (tmp_time, unit);
det->setExptime(converted_time, std::vector<int>{ det_id });
os << args[0] << '\n';
}
}

if (args.size() == 2) {
{

int gateIndex = 0;
auto converted_time = StringTo < time::ns > (args[0], args[1]);
det->setExptime(converted_time, std::vector<int>{ det_id });
os << args[0]<< args[1] << '\n';
}
}

}

}

return os.str();
}

std::string Caller::exptime2(int action) {

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
{

int gateIndex = 1;
}
}

if (args.size() == 1) {
{

int gateIndex = 1;
}
}

}

else if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() != 1 && args.size() != 2) {
throw RuntimeError("Wrong number of arguments for action PUT");
}

if (args.size() == 1) {
{

int gateIndex = 1;
try {
std::string tmp_time(args[0]);
std::string unit = RemoveUnit(tmp_time);
auto converted_time = StringTo < time::ns > (tmp_time, unit);
} catch (...) {  throw RuntimeError("Could not convert argument to time::ns");}
}
}

if (args.size() == 2) {
{

int gateIndex = 1;
try {
StringTo < time::ns > (args[0], args[1]);
} catch (...) {  throw RuntimeError("Could not convert arguments to time::ns");}
}
}

}

else {

throw RuntimeError("Invalid action PUT");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

int gateIndex = 1;
auto t = det->getExptime(gateIndex, std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

if (args.size() == 1) {
{

int gateIndex = 1;
auto t = det->getExptime(gateIndex, std::vector<int>{ det_id });
os << OutString(t , args[0]) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (detector_type == defs::MYTHEN3) {
if (args.size() == 1) {
{

int gateIndex = 0;
std::string tmp_time(args[0]);
std::string unit = RemoveUnit(tmp_time);
auto converted_time = StringTo < time::ns > (tmp_time, unit);
det->setExptime(gateIndex, converted_time, std::vector<int>{ det_id });
os << args[0] << '\n';
}
}

if (args.size() == 2) {
{

int gateIndex = 0;
auto converted_time = StringTo < time::ns > (args[0], args[1]);
det->setExptime(gateIndex, converted_time, std::vector<int>{ det_id });
os << args[0]<< args[1] << '\n';
}
}

}

else {

if (args.size() == 1) {
{

int gateIndex = 1;
std::string tmp_time(args[0]);
std::string unit = RemoveUnit(tmp_time);
auto converted_time = StringTo < time::ns > (tmp_time, unit);
det->setExptime(converted_time, std::vector<int>{ det_id });
os << args[0] << '\n';
}
}

if (args.size() == 2) {
{

int gateIndex = 1;
auto converted_time = StringTo < time::ns > (args[0], args[1]);
det->setExptime(converted_time, std::vector<int>{ det_id });
os << args[0]<< args[1] << '\n';
}
}

}

}

return os.str();
}

std::string Caller::exptime3(int action) {

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
{

int gateIndex = 2;
}
}

if (args.size() == 1) {
{

int gateIndex = 2;
}
}

}

else if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() != 1 && args.size() != 2) {
throw RuntimeError("Wrong number of arguments for action PUT");
}

if (args.size() == 1) {
{

int gateIndex = 2;
try {
std::string tmp_time(args[0]);
std::string unit = RemoveUnit(tmp_time);
auto converted_time = StringTo < time::ns > (tmp_time, unit);
} catch (...) {  throw RuntimeError("Could not convert argument to time::ns");}
}
}

if (args.size() == 2) {
{

int gateIndex = 2;
try {
StringTo < time::ns > (args[0], args[1]);
} catch (...) {  throw RuntimeError("Could not convert arguments to time::ns");}
}
}

}

else {

throw RuntimeError("Invalid action PUT");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

int gateIndex = 2;
auto t = det->getExptime(gateIndex, std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

if (args.size() == 1) {
{

int gateIndex = 2;
auto t = det->getExptime(gateIndex, std::vector<int>{ det_id });
os << OutString(t , args[0]) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (detector_type == defs::MYTHEN3) {
if (args.size() == 1) {
{

int gateIndex = 0;
std::string tmp_time(args[0]);
std::string unit = RemoveUnit(tmp_time);
auto converted_time = StringTo < time::ns > (tmp_time, unit);
det->setExptime(gateIndex, converted_time, std::vector<int>{ det_id });
os << args[0] << '\n';
}
}

if (args.size() == 2) {
{

int gateIndex = 0;
auto converted_time = StringTo < time::ns > (args[0], args[1]);
det->setExptime(gateIndex, converted_time, std::vector<int>{ det_id });
os << args[0]<< args[1] << '\n';
}
}

}

else {

if (args.size() == 1) {
{

int gateIndex = 2;
std::string tmp_time(args[0]);
std::string unit = RemoveUnit(tmp_time);
auto converted_time = StringTo < time::ns > (tmp_time, unit);
det->setExptime(converted_time, std::vector<int>{ det_id });
os << args[0] << '\n';
}
}

if (args.size() == 2) {
{

int gateIndex = 2;
auto converted_time = StringTo < time::ns > (args[0], args[1]);
det->setExptime(converted_time, std::vector<int>{ det_id });
os << args[0]<< args[1] << '\n';
}
}

}

}

return os.str();
}

std::string Caller::exptimel(int action) {

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
{

}
}

if (args.size() == 1) {
{

}
}

}

else {

throw RuntimeError("Invalid action GET");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

auto t = det->getExptimeLeft(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

if (args.size() == 1) {
{

auto t = det->getExptimeLeft(std::vector<int>{ det_id });
os << OutString(t , args[0]) << '\n';
}
}

}

return os.str();
}

std::string Caller::fname(int action) {

std::ostringstream os;
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
{

}
}

}

else if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() != 1) {
throw RuntimeError("Wrong number of arguments for action PUT");
}

if (args.size() == 1) {
{

}
}

}

else {

throw RuntimeError("Invalid action PUT");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

auto t = det->getFileNamePrefix(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

det->setFileNamePrefix(args[0], std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::fpath(int action) {

std::ostringstream os;
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
{

}
}

}

else if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() != 1) {
throw RuntimeError("Wrong number of arguments for action PUT");
}

if (args.size() == 1) {
{

}
}

}

else {

throw RuntimeError("Invalid action PUT");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

auto t = det->getFilePath(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

det->setFilePath(args[0], std::vector<int>{ det_id });
os << args.front() << '\n';
}
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

if (args.size() == 0) {
{

}
}

}

else if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() != 1) {
throw RuntimeError("Wrong number of arguments for action PUT");
}

if (args.size() == 1) {
{

try {
StringTo<int>(args[0]);
} catch (...) {
  throw RuntimeError("Could not convert argument 0 to int");
}
}
}

}

else {

throw RuntimeError("Invalid action PUT");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

auto t = det->getNumberOfFrames(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setNumberOfFrames(arg0);
os << args[0] << '\n';
}
}

}

return os.str();
}

std::string Caller::frametime(int action) {

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
{

}
}

if (args.size() == 1) {
{

}
}

}

else {

throw RuntimeError("Invalid action GET");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

auto t = det->getMeasurementTime(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

if (args.size() == 1) {
{

auto t = det->getMeasurementTime(std::vector<int>{ det_id });
os << OutString(t , args[0]) << '\n';
}
}

}

return os.str();
}

std::string Caller::measuredperiod(int action) {

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
{

}
}

if (args.size() == 1) {
{

}
}

}

else {

throw RuntimeError("Invalid action GET");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

auto t = det->getMeasuredPeriod(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

if (args.size() == 1) {
{

auto t = det->getMeasuredPeriod(std::vector<int>{ det_id });
os << OutString(t , args[0]) << '\n';
}
}

}

return os.str();
}

std::string Caller::measuredsubperiod(int action) {

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
{

}
}

if (args.size() == 1) {
{

}
}

}

else {

throw RuntimeError("Invalid action GET");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

auto t = det->getMeasuredSubFramePeriod(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

if (args.size() == 1) {
{

auto t = det->getMeasuredSubFramePeriod(std::vector<int>{ det_id });
os << OutString(t , args[0]) << '\n';
}
}

}

return os.str();
}

std::string Caller::patioctrl(int action) {

std::ostringstream os;
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
{

}
}

}

else if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() != 1) {
throw RuntimeError("Wrong number of arguments for action PUT");
}

if (args.size() == 1) {
{

try {
StringTo<uint64_t>(args[0]);
} catch (...) {
  throw RuntimeError("Could not convert argument 0 to uint64_t");
}
}
}

}

else {

throw RuntimeError("Invalid action PUT");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

auto t = det->getPatternIOControl(std::vector<int>{ det_id });
os << OutStringHex(t, 16) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<uint64_t>(args[0]);
det->setPatternIOControl(arg0, std::vector<int>{ det_id });
os << ToStringHex(args[0], 16) << '\n';
}
}

}

return os.str();
}

std::string Caller::patmask(int action) {

std::ostringstream os;
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
{

}
}

}

else if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() != 1) {
throw RuntimeError("Wrong number of arguments for action PUT");
}

if (args.size() == 1) {
{

try {
StringTo<uint64_t>(args[0]);
} catch (...) {
  throw RuntimeError("Could not convert argument 0 to uint64_t");
}
}
}

}

else {

throw RuntimeError("Invalid action PUT");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

auto t = det->getPatternMask(std::vector<int>{ det_id });
os << OutStringHex(t, 16) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<uint64_t>(args[0]);
det->setPatternMask(arg0, std::vector<int>{ det_id });
os << ToStringHex(args[0], 16) << '\n';
}
}

}

return os.str();
}

std::string Caller::patsetbit(int action) {

std::ostringstream os;
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
{

}
}

}

else if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() != 1) {
throw RuntimeError("Wrong number of arguments for action PUT");
}

if (args.size() == 1) {
{

try {
StringTo<uint64_t>(args[0]);
} catch (...) {
  throw RuntimeError("Could not convert argument 0 to uint64_t");
}
}
}

}

else {

throw RuntimeError("Invalid action PUT");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

auto t = det->getPatternBitMask(std::vector<int>{ det_id });
os << OutStringHex(t, 16) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<uint64_t>(args[0]);
det->setPatternBitMask(arg0, std::vector<int>{ det_id });
os << ToStringHex(args[0], 16) << '\n';
}
}

}

return os.str();
}

std::string Caller::period(int action) {

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
{

}
}

if (args.size() == 1) {
{

}
}

}

else if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() != 1 && args.size() != 2) {
throw RuntimeError("Wrong number of arguments for action PUT");
}

if (args.size() == 1) {
{

try {
std::string tmp_time(args[0]);
std::string unit = RemoveUnit(tmp_time);
auto converted_time = StringTo < time::ns > (tmp_time, unit);
} catch (...) {  throw RuntimeError("Could not convert argument to time::ns");}
}
}

if (args.size() == 2) {
{

try {
StringTo < time::ns > (args[0], args[1]);
} catch (...) {  throw RuntimeError("Could not convert arguments to time::ns");}
}
}

}

else {

throw RuntimeError("Invalid action PUT");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

auto t = det->getPeriod(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

if (args.size() == 1) {
{

auto t = det->getPeriod(std::vector<int>{ det_id });
os << OutString(t , args[0]) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

std::string tmp_time(args[0]);
std::string unit = RemoveUnit(tmp_time);
auto converted_time = StringTo < time::ns > (tmp_time, unit);
det->setPeriod(converted_time, std::vector<int>{ det_id });
os << args[0] << '\n';
}
}

if (args.size() == 2) {
{

auto converted_time = StringTo < time::ns > (args[0], args[1]);
det->setPeriod(converted_time, std::vector<int>{ det_id });
os << args[0]<< args[1] << '\n';
}
}

}

return os.str();
}

std::string Caller::periodl(int action) {

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
{

}
}

if (args.size() == 1) {
{

}
}

}

else {

throw RuntimeError("Invalid action GET");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

auto t = det->getPeriodLeft(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

if (args.size() == 1) {
{

auto t = det->getPeriodLeft(std::vector<int>{ det_id });
os << OutString(t , args[0]) << '\n';
}
}

}

return os.str();
}

std::string Caller::runtime(int action) {

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
{

}
}

if (args.size() == 1) {
{

}
}

}

else {

throw RuntimeError("Invalid action GET");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

auto t = det->getActualTime(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

if (args.size() == 1) {
{

auto t = det->getActualTime(std::vector<int>{ det_id });
os << OutString(t , args[0]) << '\n';
}
}

}

return os.str();
}

std::string Caller::settingspath(int action) {

std::ostringstream os;
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
{

}
}

}

else if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() != 1) {
throw RuntimeError("Wrong number of arguments for action PUT");
}

if (args.size() == 1) {
{

}
}

}

else {

throw RuntimeError("Invalid action PUT");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

auto t = det->getSettingsPath(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

det->setSettingsPath(args[0], std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::storagecell_delay(int action) {

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
{

}
}

if (args.size() == 1) {
{

}
}

}

else if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() != 1 && args.size() != 2) {
throw RuntimeError("Wrong number of arguments for action PUT");
}

if (args.size() == 1) {
{

try {
std::string tmp_time(args[0]);
std::string unit = RemoveUnit(tmp_time);
auto converted_time = StringTo < time::ns > (tmp_time, unit);
} catch (...) {  throw RuntimeError("Could not convert argument to time::ns");}
}
}

if (args.size() == 2) {
{

try {
StringTo < time::ns > (args[0], args[1]);
} catch (...) {  throw RuntimeError("Could not convert arguments to time::ns");}
}
}

}

else {

throw RuntimeError("Invalid action PUT");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

auto t = det->getStorageCellDelay(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

if (args.size() == 1) {
{

auto t = det->getStorageCellDelay(std::vector<int>{ det_id });
os << OutString(t , args[0]) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

std::string tmp_time(args[0]);
std::string unit = RemoveUnit(tmp_time);
auto converted_time = StringTo < time::ns > (tmp_time, unit);
det->setStorageCellDelay(converted_time, std::vector<int>{ det_id });
os << args[0] << '\n';
}
}

if (args.size() == 2) {
{

auto converted_time = StringTo < time::ns > (args[0], args[1]);
det->setStorageCellDelay(converted_time, std::vector<int>{ det_id });
os << args[0]<< args[1] << '\n';
}
}

}

return os.str();
}

std::string Caller::subdeadtime(int action) {

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
{

}
}

if (args.size() == 1) {
{

}
}

}

else if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() != 1 && args.size() != 2) {
throw RuntimeError("Wrong number of arguments for action PUT");
}

if (args.size() == 1) {
{

try {
std::string tmp_time(args[0]);
std::string unit = RemoveUnit(tmp_time);
auto converted_time = StringTo < time::ns > (tmp_time, unit);
} catch (...) {  throw RuntimeError("Could not convert argument to time::ns");}
}
}

if (args.size() == 2) {
{

try {
StringTo < time::ns > (args[0], args[1]);
} catch (...) {  throw RuntimeError("Could not convert arguments to time::ns");}
}
}

}

else {

throw RuntimeError("Invalid action PUT");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

auto t = det->getSubDeadTime(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

if (args.size() == 1) {
{

auto t = det->getSubDeadTime(std::vector<int>{ det_id });
os << OutString(t , args[0]) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

std::string tmp_time(args[0]);
std::string unit = RemoveUnit(tmp_time);
auto converted_time = StringTo < time::ns > (tmp_time, unit);
det->setSubDeadTime(converted_time, std::vector<int>{ det_id });
os << args[0] << '\n';
}
}

if (args.size() == 2) {
{

auto converted_time = StringTo < time::ns > (args[0], args[1]);
det->setSubDeadTime(converted_time, std::vector<int>{ det_id });
os << args[0]<< args[1] << '\n';
}
}

}

return os.str();
}

std::string Caller::subexptime(int action) {

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
{

}
}

if (args.size() == 1) {
{

}
}

}

else if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() != 1 && args.size() != 2) {
throw RuntimeError("Wrong number of arguments for action PUT");
}

if (args.size() == 1) {
{

try {
std::string tmp_time(args[0]);
std::string unit = RemoveUnit(tmp_time);
auto converted_time = StringTo < time::ns > (tmp_time, unit);
} catch (...) {  throw RuntimeError("Could not convert argument to time::ns");}
}
}

if (args.size() == 2) {
{

try {
StringTo < time::ns > (args[0], args[1]);
} catch (...) {  throw RuntimeError("Could not convert arguments to time::ns");}
}
}

}

else {

throw RuntimeError("Invalid action PUT");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

auto t = det->getSubExptime(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

if (args.size() == 1) {
{

auto t = det->getSubExptime(std::vector<int>{ det_id });
os << OutString(t , args[0]) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

std::string tmp_time(args[0]);
std::string unit = RemoveUnit(tmp_time);
auto converted_time = StringTo < time::ns > (tmp_time, unit);
det->setSubExptime(converted_time, std::vector<int>{ det_id });
os << args[0] << '\n';
}
}

if (args.size() == 2) {
{

auto converted_time = StringTo < time::ns > (args[0], args[1]);
det->setSubExptime(converted_time, std::vector<int>{ det_id });
os << args[0]<< args[1] << '\n';
}
}

}

return os.str();
}

std::string Caller::transceiverenable(int action) {

std::ostringstream os;
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
{

}
}

}

else if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() != 1) {
throw RuntimeError("Wrong number of arguments for action PUT");
}

if (args.size() == 1) {
{

try {
StringTo<uint32_t>(args[0]);
} catch (...) {
  throw RuntimeError("Could not convert argument 0 to uint32_t");
}
}
}

}

else {

throw RuntimeError("Invalid action PUT");
}

// generate code for each action
auto detector_type = det->getDetectorType().squash();
if (action == slsDetectorDefs::GET_ACTION) {
if (args.size() == 0) {
{

auto t = det->getTransceiverEnableMask(std::vector<int>{ det_id });
os << OutStringHex(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<uint32_t>(args[0]);
det->setTransceiverEnableMask(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}


}