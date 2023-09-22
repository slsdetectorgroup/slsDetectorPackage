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


std::string Caller::activate(int action) {

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

auto t = det->getActive(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setActive(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::adcclk(int action) {

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

auto t = det->getADCClock(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setADCClock(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
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

std::string Caller::adcpipeline(int action) {

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

auto t = det->getADCPipeline(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setADCPipeline(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::apulse(int action) {

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

auto t = det->getAnalogPulsing(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setAnalogPulsing(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::asamples(int action) {

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

auto t = det->getNumberOfAnalogSamples(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setNumberOfAnalogSamples(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::autocompdisable(int action) {

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

auto t = det->getAutoComparatorDisable(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setAutoComparatorDisable(arg0, std::vector<int>{ det_id });
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

std::string Caller::cdsgain(int action) {

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
StringTo<bool>(args[0]);
} catch (...) {
  throw RuntimeError("Could not convert argument 0 to bool");
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

auto t = det->getCDSGain(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<bool>(args[0]);
det->setCDSGain(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::column(int action) {

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

auto t = det->getColumn(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setColumn(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
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

std::string Caller::dbitclk(int action) {

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

auto t = det->getDBITClock(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setDBITClock(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::dbitpipeline(int action) {

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

auto t = det->getDBITPipeline(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setDBITPipeline(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
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

std::string Caller::dpulse(int action) {

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

auto t = det->getDigitalPulsing(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setDigitalPulsing(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::dsamples(int action) {

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

auto t = det->getNumberOfDigitalSamples(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setNumberOfDigitalSamples(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
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

std::string Caller::extsampling(int action) {

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

auto t = det->getExternalSampling(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setExternalSampling(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::extsamplingsrc(int action) {

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

auto t = det->getExternalSamplingSource(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setExternalSamplingSource(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::fformat(int action) {

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
StringTo<slsDetectorDefs::fileFormat>(args[0]);
} catch (...) {
  throw RuntimeError("Could not convert argument 0 to slsDetectorDefs::fileFormat");
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

auto t = det->getFileFormat(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<slsDetectorDefs::fileFormat>(args[0]);
det->setFileFormat(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::filtercells(int action) {

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

auto t = det->getNumberOfFilterCells(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setNumberOfFilterCells(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::filterresistor(int action) {

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

auto t = det->getFilterResistor(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setFilterResistor(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::findex(int action) {

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

auto t = det->getAcquisitionIndex(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<uint64_t>(args[0]);
det->setAcquisitionIndex(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::fliprows(int action) {

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

auto t = det->getFlipRows(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setFlipRows(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::flowcontrol10g(int action) {

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

auto t = det->getTenGigaFlowControl(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setTenGigaFlowControl(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
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

std::string Caller::foverwrite(int action) {

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

auto t = det->getFileOverWrite(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setFileOverWrite(arg0, std::vector<int>{ det_id });
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

std::string Caller::fwrite(int action) {

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

auto t = det->getFileWrite(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setFileWrite(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::gainmode(int action) {

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
StringTo<slsDetectorDefs::gainMode>(args[0]);
} catch (...) {
  throw RuntimeError("Could not convert argument 0 to slsDetectorDefs::gainMode");
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

auto t = det->getGainMode(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<slsDetectorDefs::gainMode>(args[0]);
det->setGainMode(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::gates(int action) {

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

auto t = det->getNumberOfGates(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setNumberOfGates(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::highvoltage(int action) {

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

auto t = det->getHighVoltage(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setHighVoltage(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::imagetest(int action) {

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

auto t = det->getImageTestMode(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setImageTestMode(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::interpolation(int action) {

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

auto t = det->getInterpolation(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setInterpolation(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::interruptsubframe(int action) {

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

auto t = det->getInterruptSubframe(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setInterruptSubframe(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::led(int action) {

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

auto t = det->getLEDEnable(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setLEDEnable(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::lock(int action) {

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

auto t = det->getDetectorLock(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setDetectorLock(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::master(int action) {

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

auto t = det->getMaster(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setMaster(arg0, det_id);
os << args.front() << '\n';
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

std::string Caller::nextframenumber(int action) {

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

auto t = det->getNextFrameNumber(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<uint64_t>(args[0]);
det->setNextFrameNumber(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::numinterfaces(int action) {

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

auto t = det->getNumberofUDPInterfaces(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setNumberofUDPInterfaces(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::overflow(int action) {

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

auto t = det->getOverFlowMode(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setOverFlowMode(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::parallel(int action) {

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

auto t = det->getParallelMode(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setParallelMode(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::partialreset(int action) {

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

auto t = det->getPartialReset(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setPartialReset(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
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

std::string Caller::polarity(int action) {

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
StringTo<defs::polarity>(args[0]);
} catch (...) {
  throw RuntimeError("Could not convert argument 0 to defs::polarity");
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

auto t = det->getPolarity(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<defs::polarity>(args[0]);
det->setPolarity(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::port(int action) {

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

auto t = det->getControlPort(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setControlPort(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::powerchip(int action) {

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

auto t = det->getPowerChip(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setPowerChip(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::pumpprobe(int action) {

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

auto t = det->getPumpProbe(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setPumpProbe(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::readnrows(int action) {

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

auto t = det->getReadNRows(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setReadNRows(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::romode(int action) {

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
StringTo<slsDetectorDefs::readoutMode>(args[0]);
} catch (...) {
  throw RuntimeError("Could not convert argument 0 to slsDetectorDefs::readoutMode");
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

auto t = det->getReadoutMode(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<slsDetectorDefs::readoutMode>(args[0]);
det->setReadoutMode(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::row(int action) {

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

auto t = det->getRow(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setRow(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::runclk(int action) {

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

auto t = det->getRUNClock(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setRUNClock(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
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

std::string Caller::rx_arping(int action) {

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

auto t = det->getRxArping(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setRxArping(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::rx_dbitoffset(int action) {

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

auto t = det->getRxDbitOffset(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setRxDbitOffset(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::rx_discardpolicy(int action) {

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
StringTo<slsDetectorDefs::frameDiscardPolicy>(args[0]);
} catch (...) {
  throw RuntimeError("Could not convert argument 0 to slsDetectorDefs::frameDiscardPolicy");
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

auto t = det->getRxFrameDiscardPolicy(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<slsDetectorDefs::frameDiscardPolicy>(args[0]);
det->setRxFrameDiscardPolicy(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::rx_fifodepth(int action) {

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

auto t = det->getRxFifoDepth(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setRxFifoDepth(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::rx_framesperfile(int action) {

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

auto t = det->getFramesPerFile(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setFramesPerFile(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::rx_lock(int action) {

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

auto t = det->getRxLock(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setRxLock(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::rx_padding(int action) {

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

auto t = det->getPartialFramesPadding(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setPartialFramesPadding(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::rx_silent(int action) {

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

auto t = det->getRxSilentMode(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setRxSilentMode(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::rx_tcpport(int action) {

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

auto t = det->getRxPort(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setRxPort(arg0, det_id);
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::rx_udpsocksize(int action) {

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

auto t = det->getRxUDPSocketBufferSize(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setRxUDPSocketBufferSize(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::rx_zmqfreq(int action) {

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

auto t = det->getRxZmqFrequency(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setRxZmqFrequency(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::rx_zmqip(int action) {

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

auto t = det->getRxZmqIP(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

det->setRxZmqIP(IpAddr(args[0]), std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::rx_zmqport(int action) {

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

auto t = det->getRxZmqPort(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setRxZmqPort(arg0, det_id);
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::rx_zmqstartfnum(int action) {

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

auto t = det->getRxZmqStartingFrame(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setRxZmqStartingFrame(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::rx_zmqstream(int action) {

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

auto t = det->getRxZmqDataStream(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setRxZmqDataStream(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::selinterface(int action) {

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

auto t = det->getSelectedUDPInterface(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->selectUDPInterface(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::settings(int action) {

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
StringTo<slsDetectorDefs::detectorSettings>(args[0]);
} catch (...) {
  throw RuntimeError("Could not convert argument 0 to slsDetectorDefs::detectorSettings");
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

auto t = det->getSettings(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<slsDetectorDefs::detectorSettings>(args[0]);
det->setSettings(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
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

std::string Caller::stopport(int action) {

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

auto t = det->getStopPort(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setStopPort(arg0, std::vector<int>{ det_id });
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

std::string Caller::storagecell_start(int action) {

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

auto t = det->getStorageCellStart(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setStorageCellStart(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
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

std::string Caller::temp_control(int action) {

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

auto t = det->getTemperatureControl(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setTemperatureControl(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::temp_threshold(int action) {

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

auto t = det->getThresholdTemperature(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setThresholdTemperature(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::tengiga(int action) {

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

auto t = det->getTenGiga(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setTenGiga(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::timing(int action) {

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
StringTo<slsDetectorDefs::timingMode>(args[0]);
} catch (...) {
  throw RuntimeError("Could not convert argument 0 to slsDetectorDefs::timingMode");
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

auto t = det->getTimingMode(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<slsDetectorDefs::timingMode>(args[0]);
det->setTimingMode(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::timingsource(int action) {

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
StringTo<slsDetectorDefs::timingSourceType>(args[0]);
} catch (...) {
  throw RuntimeError("Could not convert argument 0 to slsDetectorDefs::timingSourceType");
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

auto t = det->getTimingSource(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<slsDetectorDefs::timingSourceType>(args[0]);
det->setTimingSource(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::top(int action) {

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

auto t = det->getTop(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setTop(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
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

std::string Caller::trimval(int action) {

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

auto t = det->getAllTrimbits(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setAllTrimbits(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::tsamples(int action) {

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

auto t = det->getNumberOfTransceiverSamples(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setNumberOfTransceiverSamples(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::txdelay_frame(int action) {

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

auto t = det->getTransmissionDelayFrame(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setTransmissionDelayFrame(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::txdelay_left(int action) {

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

auto t = det->getTransmissionDelayLeft(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setTransmissionDelayLeft(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::txdelay_right(int action) {

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

auto t = det->getTransmissionDelayRight(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setTransmissionDelayRight(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::udp_dstmac(int action) {

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

auto t = det->getDestinationUDPMAC(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

det->setDestinationUDPMAC(MacAddr(args[0]), std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::udp_dstmac2(int action) {

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

auto t = det->getDestinationUDPMAC2(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

det->setDestinationUDPMAC2(MacAddr(args[0]), std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::udp_dstport(int action) {

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

auto t = det->getDestinationUDPPort(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setDestinationUDPPort(arg0, det_id);
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::udp_dstport2(int action) {

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

auto t = det->getDestinationUDPPort2(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setDestinationUDPPort2(arg0, det_id);
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::udp_firstdst(int action) {

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

auto t = det->getFirstUDPDestination(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setFirstUDPDestination(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::udp_srcmac(int action) {

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

auto t = det->getSourceUDPMAC(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

det->setSourceUDPMAC(MacAddr(args[0]), std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::udp_srcmac2(int action) {

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

auto t = det->getSourceUDPMAC2(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

det->setSourceUDPMAC2(MacAddr(args[0]), std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::updatemode(int action) {

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

auto t = det->getUpdateMode(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setUpdateMode(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::veto(int action) {

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

auto t = det->getVeto(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setVeto(arg0, std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::zmqip(int action) {

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

auto t = det->getClientZmqIp(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

det->setClientZmqIp(IpAddr(args[0]), std::vector<int>{ det_id });
os << args.front() << '\n';
}
}

}

return os.str();
}

std::string Caller::zmqport(int action) {

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

auto t = det->getClientZmqPort(std::vector<int>{ det_id });
os << OutString(t) << '\n';
}
}

}

if (action == slsDetectorDefs::PUT_ACTION) {
if (args.size() == 1) {
{

auto arg0 = StringTo<int>(args[0]);
det->setClientZmqPort(arg0, det_id);
os << args.front() << '\n';
}
}

}

return os.str();
}



}