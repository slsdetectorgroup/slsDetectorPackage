#include "Caller.h"
#include "sls/logger.h"
#include "sls/string_utils.h"
#include <iostream>

namespace sls {

std::string Caller::activate(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: activate" << std::endl;
        os << R"V0G0N([0, 1] 
	[Eiger] 1 is default. 0 deactivates readout and does not send data. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getActive(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setActive(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([n_clk in MHz]
	[Ctb] ADC clock frequency in MHz. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getADCClock(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setADCClock(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([bitmask]
	[Ctb] ADC Enable Mask for 1Gb Enable for each 32 ADC channel. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<uint32_t>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to uint32_t");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getADCEnableMask(std::vector<int>{det_id});
            os << OutStringHex(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<uint32_t>(args[0]);
            det->setADCEnableMask(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([bitmask]
	[Ctb] ADC Enable Mask for 10Gb mode for each 32 ADC channel. However, if any of a consecutive 4 bits are enabled, the complete 4 bits are enabled. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<uint32_t>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to uint32_t");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getTenGigaADCEnableMask(std::vector<int>{det_id});
            os << OutStringHex(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<uint32_t>(args[0]);
            det->setTenGigaADCEnableMask(arg0, std::vector<int>{det_id});
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
		[Ctb][Xilinx_Ctb] Get the adc index for the given name. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            if (det->getDetectorType().squash(defs::GENERIC) !=
                    defs::CHIPTESTBOARD &&
                det->getDetectorType().squash(defs::GENERIC) !=
                    defs::XILINX_CHIPTESTBOARD) {
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
        os << R"V0G0N([bitmask]
	[Ctb][Jungfrau][Moench] ADC Inversion Mask.
	[Jungfrau][Moench] Inversions on top of the default mask. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<uint32_t>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to uint32_t");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getADCInvert(std::vector<int>{det_id});
            os << OutStringHex(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<uint32_t>(args[0]);
            det->setADCInvert(arg0, std::vector<int>{det_id});
            os << args.front() << '\n';
        }
    }

    return os.str();
}

std::string Caller::adclist(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: adclist" << std::endl;
        os << R"V0G0N([adcname1 adcname2 .. adcname32] 
		[Ctb][Xilinx_Ctb] Set the list of adc names for this board. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
            if (cmd != "daclist" && det_type != defs::CHIPTESTBOARD &&
                det_type != defs::XILINX_CHIPTESTBOARD) {
                throw RuntimeError(cmd + " only allowed for CTB.");
            }
            if (det_id != -1) {
                throw RuntimeError("Cannot execute adclist at module level");
            }
            auto t = det->getAdcNames();
            os << ToString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        auto det_type = det->getDetectorType().squash(defs::GENERIC);
        ;
        if (cmd != "daclist" && det_type != defs::CHIPTESTBOARD &&
            det_type != defs::XILINX_CHIPTESTBOARD) {
            throw RuntimeError(cmd + " only allowed for CTB.");
        }
        if (cmd == "daclist" && det_type != defs::CHIPTESTBOARD &&
            det_type != defs::XILINX_CHIPTESTBOARD) {
            throw RuntimeError("This detector already has fixed dac names. "
                               "Cannot change them.");
        }
        if (det_id != -1) {
            throw RuntimeError("Cannot execute adclist at module level");
        }
        det->setAdcNames(args);
        os << ToString(args) << '\n';
    }

    return os.str();
}

std::string Caller::adcname(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: adcname" << std::endl;
        os << R"V0G0N([0-31][name] 
		[Ctb][Xilinx_Ctb] Set the adc at the given position to the given name. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            if (det->getDetectorType().squash(defs::GENERIC) !=
                    defs::CHIPTESTBOARD &&
                det->getDetectorType().squash(defs::GENERIC) !=
                    defs::XILINX_CHIPTESTBOARD) {
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
            if (det->getDetectorType().squash(defs::GENERIC) !=
                    defs::CHIPTESTBOARD &&
                det->getDetectorType().squash(defs::GENERIC) !=
                    defs::XILINX_CHIPTESTBOARD) {
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

std::string Caller::adcphase(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: adcphase" << std::endl;
        os << R"V0G0N([n_value] [(optional)deg]
	[Jungfrau][Moench][Ctb][Gotthard] Phase shift of ADC clock. 
	[Jungfrau][Moench] Absolute phase shift. If deg used, then shift in degrees. Changing Speed also resets adcphase to recommended defaults.
	[Ctb] Absolute phase shift. If deg used, then shift in degrees. Changing adcclk also resets adcphase and sets it to previous values.
	[Gotthard] Relative phase shift. Cannot get )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
        }

        if (args.size() == 1) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

        if (args.size() == 2) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
            if (det_type == defs::EIGER || det_type == defs::MYTHEN3 ||
                det_type == defs::GOTTHARD2) {
                throw RuntimeError(
                    "adcphase not implemented for this detector");
            }
            auto t = det->getADCPhase(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }

        if (args.size() == 1) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
            if (det_type == defs::EIGER || det_type == defs::MYTHEN3 ||
                det_type == defs::GOTTHARD2) {
                throw RuntimeError(
                    "adcphase not implemented for this detector");
            }
            if (args[0] != "deg") {
                throw RuntimeError("Unknown adcphase   argument " + args[0] +
                                   ". Did you mean deg?    ");
            }
            auto t = det->getADCPhaseInDegrees(std::vector<int>{det_id});
            os << OutString(t) << " deg" << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
            if (det_type == defs::EIGER || det_type == defs::MYTHEN3 ||
                det_type == defs::GOTTHARD2) {
                throw RuntimeError(
                    "adcphase not implemented for this detector");
            }
            auto arg0 = StringTo<int>(args[0]);
            det->setADCPhase(arg0, std::vector<int>{det_id});
            os << args.front() << '\n';
        }

        if (args.size() == 2) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
            if (det_type == defs::EIGER || det_type == defs::MYTHEN3 ||
                det_type == defs::GOTTHARD2) {
                throw RuntimeError(
                    "adcphase not implemented for this detector");
            }
            if (args[1] != "deg") {
                throw RuntimeError("Unknown adcphase 2nd argument " + args[1] +
                                   ". Did you mean deg?");
            }
            auto arg0 = StringTo<int>(args[0]);
            det->setADCPhaseInDegrees(arg0, std::vector<int>{det_id});
            os << args[0] << ' ' << args[1] << '\n';
        }
    }

    return os.str();
}

std::string Caller::adcpipeline(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: adcpipeline" << std::endl;
        os << R"V0G0N([n_value]
	[Ctb][Moench] Pipeline for ADC clock. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getADCPipeline(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setADCPipeline(arg0, std::vector<int>{det_id});
            os << args.front() << '\n';
        }
    }

    return os.str();
}

std::string Caller::adcreg(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: adcreg" << std::endl;
        os << R"V0G0N([address] [value]
	[Jungfrau][Moench][Ctb][Gotthard] Writes to an adc register in hex. Advanced user Function! )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            try {
                StringTo<uint32_t>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to uint32_t");
            }
            try {
                StringTo<uint32_t>(args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to uint32_t");
            }
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            auto arg0 = StringTo<uint32_t>(args[0]);
            auto arg1 = StringTo<uint32_t>(args[1]);
            det->writeAdcRegister(arg0, arg1, std::vector<int>{det_id});
            os << ToString(args) << '\n';
        }
    }

    return os.str();
}

std::string Caller::adcvpp(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: adcvpp" << std::endl;
        os << R"V0G0N([dac or mV value][(optional unit) mV] 
	[Ctb] Vpp of ADC.
	 0 -> 1V ; 1 -> 1.14V ; 2 -> 1.33V ; 3 -> 1.6V ; 4 -> 2V. 
	Advanced User function! )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
            try {
                StringTo<bool>("0");
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>("1");
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
            try {
                StringTo<bool>("0");
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to bool");
            }
        }

        if (args.size() == 2) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
            try {
                StringTo<bool>("1");
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto arg0 = StringTo<bool>("0");
            auto t = det->getADCVpp(arg0, std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }

        if (args.size() == 1) {
            if ((args[0] != "mv") && (args[0] != "mV")) {
                throw RuntimeError("Unknown argument " + args[0] +
                                   ". Did you mean mV?");
            }
            auto arg0 = StringTo<bool>("1");
            auto t = det->getADCVpp(arg0, std::vector<int>{det_id});
            os << OutString(t) << " mV" << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            auto arg1 = StringTo<bool>("0");
            det->setADCVpp(arg0, arg1, std::vector<int>{det_id});
            os << args[0] << '\n';
        }

        if (args.size() == 2) {
            if ((args[1] != "mv") && (args[1] != "mV")) {
                throw RuntimeError("Unknown argument " + args[1] +
                                   ". Did you mean mV?");
            }
            auto arg0 = StringTo<int>(args[0]);
            auto arg1 = StringTo<bool>("1");
            det->setADCVpp(arg0, arg1, std::vector<int>{det_id});
            os << args[0] << " mV" << '\n';
        }
    }

    return os.str();
}

std::string Caller::apulse(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: apulse" << std::endl;
        os << R"V0G0N([0, 1]
	[Mythen3] Enables or disables analog pulsing. Default is disabled )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getAnalogPulsing(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setAnalogPulsing(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([n_samples]
	[Ctb] Number of analog samples expected. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getNumberOfAnalogSamples(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setNumberOfAnalogSamples(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([0, 1]
	[Jungfrau] Auto comparator disable mode. By default, the on-chip gain switching is active during the entire exposure.This mode disables the on - chip gain switching comparator automatically after 93.75% (only for chipv1.0) of exposure time (only for longer than 100us). It is possible to set the duration for chipv1.1 using compdisabletime command.
	Default is 0 or this mode disabled(comparator enabled throughout). 1 enables mode. 0 disables mode.  )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getAutoComparatorDisable(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setAutoComparatorDisable(arg0, std::vector<int>{det_id});
            os << args.front() << '\n';
        }
    }

    return os.str();
}

std::string Caller::blockingtrigger(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: blockingtrigger" << std::endl;
        os << R"V0G0N(
	[Eiger][Jungfrau][Moench] Sends software trigger signal to detector and blocks till the frames are sent out for that trigger. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 0) {
            bool block = true;
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 0) {
            bool block = true;
            if (det_id != -1) {
                throw RuntimeError(
                    "Cannot execute blockingtrigger at module level");
            }
            det->sendSoftwareTrigger(block);
            os << "successful" << '\n';
        }
    }

    return os.str();
}

std::string Caller::burstperiod(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: burstperiod" << std::endl;
        os << R"V0G0N([duration] [(optional unit) ns|us|ms|s]
	[Gotthard2] Period between 2 bursts. Only in burst mode and auto timing mode. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

        if (args.size() == 1) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                std::string tmp_time(args[0]);
                std::string unit = RemoveUnit(tmp_time);
                auto converted_time = StringTo<time::ns>(tmp_time, unit);
            } catch (...) {
                throw RuntimeError("Could not convert argument to time::ns");
            }
        }

        if (args.size() == 2) {
            try {
                StringTo<time::ns>(args[0], args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert arguments to time::ns");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getBurstPeriod(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }

        if (args.size() == 1) {
            auto t = det->getBurstPeriod(std::vector<int>{det_id});
            os << OutString(t, args[0]) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            std::string tmp_time(args[0]);
            std::string unit = RemoveUnit(tmp_time);
            auto converted_time = StringTo<time::ns>(tmp_time, unit);
            det->setBurstPeriod(converted_time, std::vector<int>{det_id});
            os << args[0] << '\n';
        }

        if (args.size() == 2) {
            auto converted_time = StringTo<time::ns>(args[0], args[1]);
            det->setBurstPeriod(converted_time, std::vector<int>{det_id});
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int64_t>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int64_t");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getNumberOfBursts(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            if (det_id != -1) {
                throw RuntimeError("Cannot execute bursts at module level");
            }
            auto arg0 = StringTo<int64_t>(args[0]);
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getNumberOfBurstsLeft(std::vector<int>{det_id});
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
	Advanced User function! )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 0) {
            det->executeBusTest(std::vector<int>{det_id});
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
        os << R"V0G0N([0, 1]
	[Gotthard2] Enable or disable CDS gain. Default is disabled. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getCDSGain(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setCDSGain(arg0, std::vector<int>{det_id});
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
	[Jungfrau] Returns chip version. Can be 1.0 or 1.1 )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getChipVersion(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    return os.str();
}

std::string Caller::clearbit(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: clearbit" << std::endl;
        os << R"V0G0N([reg address in hex] [bit index]
	Clears bit in address. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            try {
                StringTo<uint32_t>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to uint32_t");
            }
            try {
                StringTo<int>(args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            if (StringTo<int>(args[1]) < 0 || StringTo<int>(args[1]) > 31) {
                throw RuntimeError("Bit number out of range: " + args[1]);
            }
            auto arg0 = StringTo<uint32_t>(args[0]);
            auto arg1 = StringTo<int>(args[1]);
            det->clearBit(arg0, arg1, std::vector<int>{det_id});
            os << ToString(args) << '\n';
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
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
        os << R"V0G0N([Gotthard] Resets Region of interest in detector. All channels enabled. Default is all channels enabled. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 0) {
            det->clearROI(std::vector<int>{det_id});
            os << "successful" << '\n';
        }
    }

    return os.str();
}

std::string Caller::clientversion(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: clientversion" << std::endl;
        os << R"V0G0N(
	Client software version )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getClientVersion();
            os << t << '\n';
        }
    }

    return os.str();
}

std::string Caller::clkdiv(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: clkdiv" << std::endl;
        os << R"V0G0N([n_clock (0-5)] [n_divider]
	[Gotthard2][Mythen3] Clock Divider of clock n_clock. Must be greater than 1. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
            defs::detectorType type =
                det->getDetectorType().squash(defs::GENERIC);
            ;
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            defs::detectorType type =
                det->getDetectorType().squash(defs::GENERIC);
            ;
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
            try {
                StringTo<int>(args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            defs::detectorType type =
                det->getDetectorType().squash(defs::GENERIC);
            ;
            if (type != defs::GOTTHARD2 && type != defs::MYTHEN3) {
                throw RuntimeError("clkdiv not implemented for this detector.");
            }
            auto arg0 = StringTo<int>(args[0]);
            auto t = det->getClockDivider(arg0, std::vector<int>{det_id});
            os << args[0] << ' ' << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            defs::detectorType type =
                det->getDetectorType().squash(defs::GENERIC);
            ;
            if (type != defs::GOTTHARD2 && type != defs::MYTHEN3) {
                throw RuntimeError("clkdiv not implemented for this detector.");
            }
            auto arg0 = StringTo<int>(args[0]);
            auto arg1 = StringTo<int>(args[1]);
            det->setClockDivider(arg0, arg1, std::vector<int>{det_id});
            os << args[0] << ' ' << args[1] << '\n';
        }
    }

    return os.str();
}

std::string Caller::clkfreq(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: clkfreq" << std::endl;
        os << R"V0G0N([n_clock (0-5)] [freq_in_Hz]
	[Gotthard2][Mythen3] Frequency of clock n_clock in Hz. Use clkdiv to set frequency. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
            defs::detectorType type =
                det->getDetectorType().squash(defs::GENERIC);
            ;
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            defs::detectorType type =
                det->getDetectorType().squash(defs::GENERIC);
            ;
            if (type != defs::GOTTHARD2 && type != defs::MYTHEN3) {
                throw RuntimeError(
                    "clkfreq not implemented for this detector.");
            }
            auto arg0 = StringTo<int>(args[0]);
            auto t = det->getClockFrequency(arg0, std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    return os.str();
}

std::string Caller::clkphase(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: clkphase" << std::endl;
        os << R"V0G0N([n_clock (0-5)] [phase] [deg (optional)]
	[Gotthard2][Mythen3] Phase of clock n_clock. If deg, then phase shift in degrees, else absolute phase shift values. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
            defs::detectorType type =
                det->getDetectorType().squash(defs::GENERIC);
            ;
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

        if (args.size() == 2) {
            defs::detectorType type =
                det->getDetectorType().squash(defs::GENERIC);
            ;
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2 && args.size() != 3) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            defs::detectorType type =
                det->getDetectorType().squash(defs::GENERIC);
            ;
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
            try {
                StringTo<int>(args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
        }

        if (args.size() == 3) {
            defs::detectorType type =
                det->getDetectorType().squash(defs::GENERIC);
            ;
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
            try {
                StringTo<int>(args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            defs::detectorType type =
                det->getDetectorType().squash(defs::GENERIC);
            ;
            if (type != defs::GOTTHARD2 && type != defs::MYTHEN3) {
                throw RuntimeError(
                    "clkphase not implemented for this detector.");
            }
            auto arg0 = StringTo<int>(args[0]);
            auto t = det->getClockPhase(arg0, std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }

        if (args.size() == 2) {
            defs::detectorType type =
                det->getDetectorType().squash(defs::GENERIC);
            ;
            if (type != defs::GOTTHARD2 && type != defs::MYTHEN3) {
                throw RuntimeError(
                    "clkphase not implemented for this detector.");
            }
            if (args[1] != "deg") {
                throw RuntimeError("Cannot scan argument" + args[1] +
                                   ". Did you mean deg?");
            }
            auto arg0 = StringTo<int>(args[0]);
            auto t =
                det->getClockPhaseinDegrees(arg0, std::vector<int>{det_id});
            os << OutString(t) << " deg" << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            defs::detectorType type =
                det->getDetectorType().squash(defs::GENERIC);
            ;
            if (type != defs::GOTTHARD2 && type != defs::MYTHEN3) {
                throw RuntimeError(
                    "clkphase not implemented for this detector.");
            }
            auto arg0 = StringTo<int>(args[0]);
            auto arg1 = StringTo<int>(args[1]);
            det->setClockPhase(arg0, arg1, std::vector<int>{det_id});
            os << args[1] << '\n';
        }

        if (args.size() == 3) {
            defs::detectorType type =
                det->getDetectorType().squash(defs::GENERIC);
            ;
            if (type != defs::GOTTHARD2 && type != defs::MYTHEN3) {
                throw RuntimeError(
                    "clkphase not implemented for this detector.");
            }
            if (args[2] != "deg") {
                throw RuntimeError("Cannot scan argument" + args[2] +
                                   ". Did you mean deg?");
            }
            auto arg0 = StringTo<int>(args[0]);
            auto arg1 = StringTo<int>(args[1]);
            det->setClockPhaseinDegrees(arg0, arg1, std::vector<int>{det_id});
            os << args[1] << " " << args[2] << '\n';
        }
    }

    return os.str();
}

std::string Caller::column(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: column" << std::endl;
        os << R"V0G0N([value]
	Set Detector column (udp header) to value. 
	Gui uses it to rearrange for complete image )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getColumn(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setColumn(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([duration] [(optional unit) ns|us|ms|s]
	[Jungfrau] Time before end of exposure when comparator is disabled. It is only possible for chipv1.1. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

        if (args.size() == 1) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                std::string tmp_time(args[0]);
                std::string unit = RemoveUnit(tmp_time);
                auto converted_time = StringTo<time::ns>(tmp_time, unit);
            } catch (...) {
                throw RuntimeError("Could not convert argument to time::ns");
            }
        }

        if (args.size() == 2) {
            try {
                StringTo<time::ns>(args[0], args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert arguments to time::ns");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getComparatorDisableTime(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }

        if (args.size() == 1) {
            auto t = det->getComparatorDisableTime(std::vector<int>{det_id});
            os << OutString(t, args[0]) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            std::string tmp_time(args[0]);
            std::string unit = RemoveUnit(tmp_time);
            auto converted_time = StringTo<time::ns>(tmp_time, unit);
            det->setComparatorDisableTime(converted_time,
                                          std::vector<int>{det_id});
            os << args[0] << '\n';
        }

        if (args.size() == 2) {
            auto converted_time = StringTo<time::ns>(args[0], args[1]);
            det->setComparatorDisableTime(converted_time,
                                          std::vector<int>{det_id});
            os << args[0] << args[1] << '\n';
        }
    }

    return os.str();
}

std::string Caller::confadc(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: confadc" << std::endl;
        os << R"V0G0N([chip index 0-9, -1 for all] [adc index 0-31, -1 for all] [7 bit configuration value in hex]
	[Gotthard2] Sets  configuration for specific chip and adc, but configures 1 chip (all adcs for that chip) at a time. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 2) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
            try {
                StringTo<int>(args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 3) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 3) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
            try {
                StringTo<int>(args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
            try {
                StringTo<int>(args[2]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 2 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 2) {
            auto arg0 = StringTo<int>(args[0]);
            auto arg1 = StringTo<int>(args[1]);
            auto t =
                det->getADCConfiguration(arg0, arg1, std::vector<int>{det_id});
            os << OutStringHex(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 3) {
            auto arg0 = StringTo<int>(args[0]);
            auto arg1 = StringTo<int>(args[1]);
            auto arg2 = StringTo<int>(args[2]);
            det->setADCConfiguration(arg0, arg1, arg2,
                                     std::vector<int>{det_id});
            os << '[' << args[0] << ", " << args[1] << ", "
               << ToStringHex(StringTo<int>(args[2])) << "]" << '\n';
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
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

std::string Caller::configtransceiver(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: configtransceiver" << std::endl;
        os << R"V0G0N(
	[Xilinx Ctb] Waits for transceiver to be aligned. Chip had to be configured (powered on) before this. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 0) {
            det->configureTransceiver(std::vector<int>{det_id});
            os << "successful" << '\n';
        }
    }

    return os.str();
}

std::string Caller::dac(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        return GetHelpDacWrapper(cmd, args);
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
            defs::dacIndex dacIndex =
                ((det->getDetectorType().squash() == defs::CHIPTESTBOARD ||
                  det->getDetectorType().squash() ==
                      defs::XILINX_CHIPTESTBOARD) &&
                 !is_int(args[0]))
                    ? det->getDacIndex(args[0])
                    : StringTo<defs::dacIndex>(args[0]);
            try {
                StringTo<bool>("0");
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to bool");
            }
        }

        if (args.size() == 2) {
            defs::dacIndex dacIndex =
                ((det->getDetectorType().squash() == defs::CHIPTESTBOARD ||
                  det->getDetectorType().squash() ==
                      defs::XILINX_CHIPTESTBOARD) &&
                 !is_int(args[0]))
                    ? det->getDacIndex(args[0])
                    : StringTo<defs::dacIndex>(args[0]);
            try {
                StringTo<bool>("1");
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to bool");
            }
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2 && args.size() != 3) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            defs::dacIndex dacIndex =
                ((det->getDetectorType().squash() == defs::CHIPTESTBOARD ||
                  det->getDetectorType().squash() ==
                      defs::XILINX_CHIPTESTBOARD) &&
                 !is_int(args[0]))
                    ? det->getDacIndex(args[0])
                    : StringTo<defs::dacIndex>(args[0]);
            try {
                StringTo<int>(args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
            try {
                StringTo<bool>("0");
            } catch (...) {
                throw RuntimeError("Could not convert argument 2 to bool");
            }
        }

        if (args.size() == 3) {
            defs::dacIndex dacIndex =
                ((det->getDetectorType().squash() == defs::CHIPTESTBOARD ||
                  det->getDetectorType().squash() ==
                      defs::XILINX_CHIPTESTBOARD) &&
                 !is_int(args[0]))
                    ? det->getDacIndex(args[0])
                    : StringTo<defs::dacIndex>(args[0]);
            try {
                StringTo<int>(args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
            try {
                StringTo<bool>("1");
            } catch (...) {
                throw RuntimeError("Could not convert argument 2 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            defs::dacIndex dacIndex =
                ((det->getDetectorType().squash() == defs::CHIPTESTBOARD ||
                  det->getDetectorType().squash() ==
                      defs::XILINX_CHIPTESTBOARD) &&
                 !is_int(args[0]))
                    ? det->getDacIndex(args[0])
                    : StringTo<defs::dacIndex>(args[0]);
            if (is_int(args[0]) &&
                det->getDetectorType().squash() != defs::CHIPTESTBOARD &&
                det->getDetectorType().squash() != defs::XILINX_CHIPTESTBOARD) {
                throw RuntimeError(
                    "Dac indices can only be used for chip test board. Use "
                    "daclist to get list of dac names for current detector.");
            }
            auto arg1 = StringTo<bool>("0");
            auto t = det->getDAC(dacIndex, arg1, std::vector<int>{det_id});
            os << args[0] << ' ' << OutString(t) << '\n';
        }

        if (args.size() == 2) {
            defs::dacIndex dacIndex =
                ((det->getDetectorType().squash() == defs::CHIPTESTBOARD ||
                  det->getDetectorType().squash() ==
                      defs::XILINX_CHIPTESTBOARD) &&
                 !is_int(args[0]))
                    ? det->getDacIndex(args[0])
                    : StringTo<defs::dacIndex>(args[0]);
            if (is_int(args[0]) &&
                det->getDetectorType().squash() != defs::CHIPTESTBOARD &&
                det->getDetectorType().squash() != defs::XILINX_CHIPTESTBOARD) {
                throw RuntimeError(
                    "Dac indices can only be used for chip test board. Use "
                    "daclist to get list of dac names for current detector.");
            }
            if (args[1] != "mv" && args[1] != "mV") {
                throw RuntimeError("Unknown argument " + args[1] +
                                   ". Did you mean mV?");
            }
            auto arg1 = StringTo<bool>("1");
            auto t = det->getDAC(dacIndex, arg1, std::vector<int>{det_id});
            os << args[0] << ' ' << OutString(t) << " mV" << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            defs::dacIndex dacIndex =
                ((det->getDetectorType().squash() == defs::CHIPTESTBOARD ||
                  det->getDetectorType().squash() ==
                      defs::XILINX_CHIPTESTBOARD) &&
                 !is_int(args[0]))
                    ? det->getDacIndex(args[0])
                    : StringTo<defs::dacIndex>(args[0]);
            if (is_int(args[0]) &&
                det->getDetectorType().squash() != defs::CHIPTESTBOARD &&
                det->getDetectorType().squash() != defs::XILINX_CHIPTESTBOARD) {
                throw RuntimeError(
                    "Dac indices can only be used for chip test board. Use "
                    "daclist to get list of dac names for current detector.");
            }
            auto arg1 = StringTo<int>(args[1]);
            auto arg2 = StringTo<bool>("0");
            det->setDAC(dacIndex, arg1, arg2, std::vector<int>{det_id});
            os << args[0] << ' ' << args[1] << '\n';
        }

        if (args.size() == 3) {
            defs::dacIndex dacIndex =
                ((det->getDetectorType().squash() == defs::CHIPTESTBOARD ||
                  det->getDetectorType().squash() ==
                      defs::XILINX_CHIPTESTBOARD) &&
                 !is_int(args[0]))
                    ? det->getDacIndex(args[0])
                    : StringTo<defs::dacIndex>(args[0]);
            if (is_int(args[0]) &&
                det->getDetectorType().squash() != defs::CHIPTESTBOARD &&
                det->getDetectorType().squash() != defs::XILINX_CHIPTESTBOARD) {
                throw RuntimeError(
                    "Dac indices can only be used for chip test board. Use "
                    "daclist to get list of dac names for current detector.");
            }
            auto arg1 = StringTo<int>(args[1]);
            auto arg2 = StringTo<bool>("1");
            det->setDAC(dacIndex, arg1, arg2, std::vector<int>{det_id});
            os << args[0] << ' ' << args[1] << " mV" << '\n';
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
		[Ctb][Xilinx_Ctb] Get the dac index for the given name. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
            defs::dacIndex index = defs::DAC_0;
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            defs::dacIndex index = defs::DAC_0;
            if (det->getDetectorType().squash(defs::GENERIC) !=
                    defs::CHIPTESTBOARD &&
                det->getDetectorType().squash(defs::GENERIC) !=
                    defs::XILINX_CHIPTESTBOARD) {
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

std::string Caller::daclist(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: daclist" << std::endl;
        os << R"V0G0N([dacname1 dacname2 .. dacname18] 
		[Ctb][Xilinx_Ctb] Set the list of dac names for this detector.
		[All] Gets the list of dac names for every dac for this detector. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
            if (cmd != "daclist" && det_type != defs::CHIPTESTBOARD &&
                det_type != defs::XILINX_CHIPTESTBOARD) {
                throw RuntimeError(cmd + " only allowed for CTB.");
            }
            if (det_id != -1) {
                throw RuntimeError("Cannot execute daclist at module level");
            }
            auto t = det->getDacNames();
            os << ToString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        auto det_type = det->getDetectorType().squash(defs::GENERIC);
        ;
        if (cmd != "daclist" && det_type != defs::CHIPTESTBOARD &&
            det_type != defs::XILINX_CHIPTESTBOARD) {
            throw RuntimeError(cmd + " only allowed for CTB.");
        }
        if (cmd == "daclist" && det_type != defs::CHIPTESTBOARD &&
            det_type != defs::XILINX_CHIPTESTBOARD) {
            throw RuntimeError("This detector already has fixed dac names. "
                               "Cannot change them.");
        }
        if (det_id != -1) {
            throw RuntimeError("Cannot execute daclist at module level");
        }
        det->setDacNames(args);
        os << ToString(args) << '\n';
    }

    return os.str();
}

std::string Caller::dacname(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: dacname" << std::endl;
        os << R"V0G0N([0-17][name] 
		[Ctb][Xilinx_Ctb] Set the dac at the given position to the given name. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
            defs::dacIndex index = defs::DAC_0;
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            defs::dacIndex index = defs::DAC_0;
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            defs::dacIndex index = defs::DAC_0;
            if (det->getDetectorType().squash(defs::GENERIC) !=
                    defs::CHIPTESTBOARD &&
                det->getDetectorType().squash(defs::GENERIC) !=
                    defs::XILINX_CHIPTESTBOARD) {
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
            if (det->getDetectorType().squash(defs::GENERIC) !=
                    defs::CHIPTESTBOARD &&
                det->getDetectorType().squash(defs::GENERIC) !=
                    defs::XILINX_CHIPTESTBOARD) {
                throw RuntimeError(cmd + " only allowed for CTB.");
            }
            if (det_id != -1) {
                throw RuntimeError("Cannot execute dacname at module level");
            }
            det->setDacName(
                static_cast<defs::dacIndex>(StringTo<int>(args[0]) + index),
                args[1]);
            os << ToString(args) << '\n';
        }
    }

    return os.str();
}

std::string Caller::datastream(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: datastream" << std::endl;
        os << R"V0G0N([left|right] [0, 1]
	[Eiger] Enables or disables data streaming from left or/and right side of detector for 10 GbE mode. 1 (enabled) by default. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
            try {
                StringTo<defs::portPosition>(args[0]);
            } catch (...) {
                throw RuntimeError(
                    "Could not convert argument 0 to defs::portPosition");
            }
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            try {
                StringTo<defs::portPosition>(args[0]);
            } catch (...) {
                throw RuntimeError(
                    "Could not convert argument 0 to defs::portPosition");
            }
            try {
                StringTo<bool>(args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<defs::portPosition>(args[0]);
            auto t = det->getDataStream(arg0, std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            auto arg0 = StringTo<defs::portPosition>(args[0]);
            auto arg1 = StringTo<bool>(args[1]);
            det->setDataStream(arg0, arg1, std::vector<int>{det_id});
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
        os << R"V0G0N([n_clk in MHz]
	[Ctb] Clock for latching the digital bits in MHz. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getDBITClock(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setDBITClock(arg0, std::vector<int>{det_id});
            os << args.front() << '\n';
        }
    }

    return os.str();
}

std::string Caller::dbitphase(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: dbitphase" << std::endl;
        os << R"V0G0N([n_value] [(optional)deg]
	[Ctb][Jungfrau] Phase shift of clock to latch digital bits. Absolute phase shift. If deg used, then shift in degrees. 
	[Ctb]Changing dbitclk also resets dbitphase and sets to previous values. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
        }

        if (args.size() == 1) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

        if (args.size() == 2) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
            if (det_type == defs::EIGER || det_type == defs::MYTHEN3 ||
                det_type == defs::GOTTHARD2 || det_type == defs::MOENCH) {
                throw RuntimeError(
                    "dbitphase not implemented for this detector");
            }
            auto t = det->getDBITPhase(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }

        if (args.size() == 1) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
            if (det_type == defs::EIGER || det_type == defs::MYTHEN3 ||
                det_type == defs::GOTTHARD2 || det_type == defs::MOENCH) {
                throw RuntimeError(
                    "dbitphase not implemented for this detector");
            }
            if (args[0] != "deg") {
                throw RuntimeError("Unknown dbitphase argument " + args[0] +
                                   ". Did you mean deg?  ");
            }
            auto t = det->getDBITPhaseInDegrees(std::vector<int>{det_id});
            os << OutString(t) << " deg" << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
            if (det_type == defs::EIGER || det_type == defs::MYTHEN3 ||
                det_type == defs::GOTTHARD2 || det_type == defs::MOENCH) {
                throw RuntimeError(
                    "dbitphase not implemented for this detector");
            }
            auto arg0 = StringTo<int>(args[0]);
            det->setDBITPhase(arg0, std::vector<int>{det_id});
            os << args.front() << '\n';
        }

        if (args.size() == 2) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
            if (det_type == defs::EIGER || det_type == defs::MYTHEN3 ||
                det_type == defs::GOTTHARD2 || det_type == defs::MOENCH) {
                throw RuntimeError(
                    "dbitphase not implemented for this detector");
            }
            if (args[1] != "deg") {
                throw RuntimeError("Unknown dbitphase 2nd argument " + args[1] +
                                   ". Did you mean deg?  ");
            }
            auto arg0 = StringTo<int>(args[0]);
            det->setDBITPhaseInDegrees(arg0, std::vector<int>{det_id});
            os << args[0] << ' ' << args[1] << '\n';
        }
    }

    return os.str();
}

std::string Caller::dbitpipeline(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: dbitpipeline" << std::endl;
        os << R"V0G0N([n_value]
	[Ctb][Gotthard2] Pipeline of the clock for latching digital bits.
	[Gotthard2] Options: 0-7
	[Ctb] Options: 0-255 )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getDBITPipeline(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setDBITPipeline(arg0, std::vector<int>{det_id});
            os << args.front() << '\n';
        }
    }

    return os.str();
}

std::string Caller::defaultdac(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: defaultdac" << std::endl;
        os << R"V0G0N([dac name][value][(optional)setting]
	Sets the default for that dac to this value.
	[Jungfrau][Moench][Mythen3] When settings is provided, it sets the default value only for that setting )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
            try {
                StringTo<defs::dacIndex>(args[0]);
            } catch (...) {
                throw RuntimeError(
                    "Could not convert argument 0 to defs::dacIndex");
            }
        }

        if (args.size() == 2) {
            try {
                StringTo<defs::dacIndex>(args[0]);
            } catch (...) {
                throw RuntimeError(
                    "Could not convert argument 0 to defs::dacIndex");
            }
            try {
                StringTo<defs::detectorSettings>(args[1]);
            } catch (...) {
                throw RuntimeError(
                    "Could not convert argument 1 to defs::detectorSettings");
            }
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2 && args.size() != 3) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            try {
                StringTo<defs::dacIndex>(args[0]);
            } catch (...) {
                throw RuntimeError(
                    "Could not convert argument 0 to defs::dacIndex");
            }
            try {
                StringTo<int>(args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
        }

        if (args.size() == 3) {
            try {
                StringTo<defs::dacIndex>(args[0]);
            } catch (...) {
                throw RuntimeError(
                    "Could not convert argument 0 to defs::dacIndex");
            }
            try {
                StringTo<int>(args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
            try {
                StringTo<defs::detectorSettings>(args[2]);
            } catch (...) {
                throw RuntimeError(
                    "Could not convert argument 2 to defs::detectorSettings");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<defs::dacIndex>(args[0]);
            auto t = det->getDefaultDac(arg0, std::vector<int>{det_id});
            os << args[0] << ' ' << OutString(t) << '\n';
        }

        if (args.size() == 2) {
            auto arg0 = StringTo<defs::dacIndex>(args[0]);
            auto arg1 = StringTo<defs::detectorSettings>(args[1]);
            auto t = det->getDefaultDac(arg0, arg1, std::vector<int>{det_id});
            os << args[0] << ' ' << args[1] << ' ' << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            auto arg0 = StringTo<defs::dacIndex>(args[0]);
            auto arg1 = StringTo<int>(args[1]);
            det->setDefaultDac(arg0, arg1, std::vector<int>{det_id});
            os << args[0] << ' ' << args[1] << '\n';
        }

        if (args.size() == 3) {
            auto arg0 = StringTo<defs::dacIndex>(args[0]);
            auto arg1 = StringTo<int>(args[1]);
            auto arg2 = StringTo<defs::detectorSettings>(args[2]);
            det->setDefaultDac(arg0, arg1, arg2, std::vector<int>{det_id});
            os << args[0] << ' ' << args[2] << ' ' << args[1] << '\n';
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 0) {
            det->loadDefaultPattern(std::vector<int>{det_id});
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
        os << R"V0G0N([duration] [(optional unit) ns|us|ms|s]
	[Jungfrau][Moench][Gotthard][Mythen3][Gotthard2][Ctb][Moench][Xilinx Ctb] Delay after trigger )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

        if (args.size() == 1) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                std::string tmp_time(args[0]);
                std::string unit = RemoveUnit(tmp_time);
                auto converted_time = StringTo<time::ns>(tmp_time, unit);
            } catch (...) {
                throw RuntimeError("Could not convert argument to time::ns");
            }
        }

        if (args.size() == 2) {
            try {
                StringTo<time::ns>(args[0], args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert arguments to time::ns");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getDelayAfterTrigger(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }

        if (args.size() == 1) {
            auto t = det->getDelayAfterTrigger(std::vector<int>{det_id});
            os << OutString(t, args[0]) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            std::string tmp_time(args[0]);
            std::string unit = RemoveUnit(tmp_time);
            auto converted_time = StringTo<time::ns>(tmp_time, unit);
            det->setDelayAfterTrigger(converted_time, std::vector<int>{det_id});
            os << args[0] << '\n';
        }

        if (args.size() == 2) {
            auto converted_time = StringTo<time::ns>(args[0], args[1]);
            det->setDelayAfterTrigger(converted_time, std::vector<int>{det_id});
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
        os << R"V0G0N(
	[Gotthard][Jungfrau][Moench][Mythen3][Gotthard2][Ctb][Xilinx Ctb] Delay Left in Acquisition. 
	[Gotthard2] only in continuous mode. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

        if (args.size() == 1) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getDelayAfterTriggerLeft(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }

        if (args.size() == 1) {
            auto t = det->getDelayAfterTriggerLeft(std::vector<int>{det_id});
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
	On-board detector server software version )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getDetectorServerVersion(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    return os.str();
}

std::string Caller::detsize(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: detsize" << std::endl;
        os << R"V0G0N([nx] [ny]
	Detector size, ie. Number of channels in x and y dim. This is used to calculate module coordinates included in UDP data. 
	By default, it adds module in y dimension for 2d detectors and in x dimension for 1d detectors packet header. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getDetectorSize();
            os << t << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            det->setDetectorSize(
                defs::xy(StringTo<int>(args[0]), StringTo<int>(args[1])));
            os << ToString(args) << '\n';
        }
    }

    return os.str();
}

std::string Caller::diodelay(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: diodelay" << std::endl;
        os << R"V0G0N([0-775]
	[Ctb] Delay for diode. Delay is in ps and max of 775 ps. Resolution is 25 ps. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            try {
                StringTo<uint64_t>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to uint64_t");
            }
            try {
                StringTo<int>(args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            auto arg0 = StringTo<uint64_t>(args[0]);
            auto arg1 = StringTo<int>(args[1]);
            det->setDigitalIODelay(arg0, arg1, std::vector<int>{det_id});
            os << ToString(args) << '\n';
        }
    }

    return os.str();
}

std::string Caller::dpulse(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: dpulse" << std::endl;
        os << R"V0G0N([0, 1]
	[Mythen3] Enables or disables digital pulsing. Default is disabled )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getDigitalPulsing(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setDigitalPulsing(arg0, std::vector<int>{det_id});
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
	[Jungfrau][Moench][Gotthard][Ctb][Mythen3][Gotthard2][Xilinx Ctb] 16 )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getDynamicRange(std::vector<int>{det_id});
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
	Gets the list of dynamic ranges for this detector. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
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
        os << R"V0G0N([n_value]
	[Ctb] Number of digital samples expected. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getNumberOfDigitalSamples(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setNumberOfDigitalSamples(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([duration] [(optional unit) ns|us|ms|s]
	[Eiger][Jungfrau][Moench][Gotthard][Gotthard2][Ctb][Xilinx Ctb] Exposure time
	[Mythen3] Exposure time of all gate signals in auto and trigger mode (internal gating). To specify gate index, use exptime1, exptime2, exptime3. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

        if (args.size() == 1) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                std::string tmp_time(args[0]);
                std::string unit = RemoveUnit(tmp_time);
                auto converted_time = StringTo<time::ns>(tmp_time, unit);
            } catch (...) {
                throw RuntimeError("Could not convert argument to time::ns");
            }
        }

        if (args.size() == 2) {
            try {
                StringTo<time::ns>(args[0], args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert arguments to time::ns");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    auto detector_type = det->getDetectorType().squash();
    if (action == slsDetectorDefs::GET_ACTION) {
        if (detector_type == defs::MYTHEN3) {
            if (args.size() == 0) {
                auto t = det->getExptimeForAllGates(std::vector<int>{det_id});
                os << OutString(t) << '\n';
            }

            if (args.size() == 1) {
                auto t = det->getExptimeForAllGates(std::vector<int>{det_id});
                os << OutString(t, args[0]) << '\n';
            }

        }

        else {

            if (args.size() == 0) {
                auto t = det->getExptime(std::vector<int>{det_id});
                os << OutString(t) << '\n';
            }

            if (args.size() == 1) {
                auto t = det->getExptime(std::vector<int>{det_id});
                os << OutString(t, args[0]) << '\n';
            }
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            std::string tmp_time(args[0]);
            std::string unit = RemoveUnit(tmp_time);
            auto converted_time = StringTo<time::ns>(tmp_time, unit);
            det->setExptime(converted_time, std::vector<int>{det_id});
            os << args[0] << '\n';
        }

        if (args.size() == 2) {
            auto converted_time = StringTo<time::ns>(args[0], args[1]);
            det->setExptime(converted_time, std::vector<int>{det_id});
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
        os << R"V0G0N([n_value]
	[Mythen3] Exposure time of gate signal 1 in auto and trigger mode (internal gating). )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
            int gateIndex = 0;
        }

        if (args.size() == 1) {
            int gateIndex = 0;
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            int gateIndex = 0;
            try {
                std::string tmp_time(args[0]);
                std::string unit = RemoveUnit(tmp_time);
                auto converted_time = StringTo<time::ns>(tmp_time, unit);
            } catch (...) {
                throw RuntimeError("Could not convert argument to time::ns");
            }
        }

        if (args.size() == 2) {
            int gateIndex = 0;
            try {
                StringTo<time::ns>(args[0], args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert arguments to time::ns");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            int gateIndex = 0;
            auto t = det->getExptime(gateIndex, std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }

        if (args.size() == 1) {
            int gateIndex = 0;
            auto t = det->getExptime(gateIndex, std::vector<int>{det_id});
            os << OutString(t, args[0]) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            int gateIndex = 0;
            std::string tmp_time(args[0]);
            std::string unit = RemoveUnit(tmp_time);
            auto converted_time = StringTo<time::ns>(tmp_time, unit);
            det->setExptime(gateIndex, converted_time,
                            std::vector<int>{det_id});
            os << args[0] << '\n';
        }

        if (args.size() == 2) {
            int gateIndex = 0;
            auto converted_time = StringTo<time::ns>(args[0], args[1]);
            det->setExptime(gateIndex, converted_time,
                            std::vector<int>{det_id});
            os << args[0] << args[1] << '\n';
        }
    }

    return os.str();
}

std::string Caller::exptime2(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: exptime2" << std::endl;
        os << R"V0G0N([n_value]
	[Mythen3] Exposure time of gate signal 2 in auto and trigger mode (internal gating). )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
            int gateIndex = 1;
        }

        if (args.size() == 1) {
            int gateIndex = 1;
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            int gateIndex = 1;
            try {
                std::string tmp_time(args[0]);
                std::string unit = RemoveUnit(tmp_time);
                auto converted_time = StringTo<time::ns>(tmp_time, unit);
            } catch (...) {
                throw RuntimeError("Could not convert argument to time::ns");
            }
        }

        if (args.size() == 2) {
            int gateIndex = 1;
            try {
                StringTo<time::ns>(args[0], args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert arguments to time::ns");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            int gateIndex = 1;
            auto t = det->getExptime(gateIndex, std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }

        if (args.size() == 1) {
            int gateIndex = 1;
            auto t = det->getExptime(gateIndex, std::vector<int>{det_id});
            os << OutString(t, args[0]) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            int gateIndex = 1;
            std::string tmp_time(args[0]);
            std::string unit = RemoveUnit(tmp_time);
            auto converted_time = StringTo<time::ns>(tmp_time, unit);
            det->setExptime(gateIndex, converted_time,
                            std::vector<int>{det_id});
            os << args[0] << '\n';
        }

        if (args.size() == 2) {
            int gateIndex = 1;
            auto converted_time = StringTo<time::ns>(args[0], args[1]);
            det->setExptime(gateIndex, converted_time,
                            std::vector<int>{det_id});
            os << args[0] << args[1] << '\n';
        }
    }

    return os.str();
}

std::string Caller::exptime3(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: exptime3" << std::endl;
        os << R"V0G0N([n_value]
	[Mythen3] Exposure time of gate signal 3 in auto and trigger mode (internal gating). )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
            int gateIndex = 2;
        }

        if (args.size() == 1) {
            int gateIndex = 2;
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            int gateIndex = 2;
            try {
                std::string tmp_time(args[0]);
                std::string unit = RemoveUnit(tmp_time);
                auto converted_time = StringTo<time::ns>(tmp_time, unit);
            } catch (...) {
                throw RuntimeError("Could not convert argument to time::ns");
            }
        }

        if (args.size() == 2) {
            int gateIndex = 2;
            try {
                StringTo<time::ns>(args[0], args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert arguments to time::ns");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            int gateIndex = 2;
            auto t = det->getExptime(gateIndex, std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }

        if (args.size() == 1) {
            int gateIndex = 2;
            auto t = det->getExptime(gateIndex, std::vector<int>{det_id});
            os << OutString(t, args[0]) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            int gateIndex = 2;
            std::string tmp_time(args[0]);
            std::string unit = RemoveUnit(tmp_time);
            auto converted_time = StringTo<time::ns>(tmp_time, unit);
            det->setExptime(gateIndex, converted_time,
                            std::vector<int>{det_id});
            os << args[0] << '\n';
        }

        if (args.size() == 2) {
            int gateIndex = 2;
            auto converted_time = StringTo<time::ns>(args[0], args[1]);
            det->setExptime(gateIndex, converted_time,
                            std::vector<int>{det_id});
            os << args[0] << args[1] << '\n';
        }
    }

    return os.str();
}

std::string Caller::exptimel(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: exptimel" << std::endl;
        os << R"V0G0N([(optional unit) ns|us|ms|s]
	[Gotthard] Exposure time left for current frame.  )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

        if (args.size() == 1) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getExptimeLeft(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }

        if (args.size() == 1) {
            auto t = det->getExptimeLeft(std::vector<int>{det_id});
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getNumberOfAdditionalStorageCells(
                std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            if (det_id != -1) {
                throw RuntimeError(
                    "Cannot execute extrastoragecells at module level");
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
        os << R"V0G0N([0, 1]
	[Ctb] Enable for external sampling signal for digital data to signal by extsampling src command. For advanced users only. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getExternalSampling(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setExternalSampling(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([0-63]
	[Ctb] Sampling source signal for digital data. For advanced users only. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getExternalSamplingSource(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setExternalSamplingSource(arg0, std::vector<int>{det_id});
            os << args.front() << '\n';
        }
    }

    return os.str();
}

std::string Caller::extsig(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: extsig" << std::endl;
        os << R"V0G0N([n_signal] [signal_type]
	[Gotthard][Mythen3] External signal mode for trigger timing mode.
	[Gotthard] [0] [trigger_in_rising_edge|trigger_in_falling_edge]
	[Mythen3] [0-7] [trigger_in_rising_edge|trigger_in_falling_edge|inversion_on|inversion_off]
	 where 0 is master input trigger signal, 1-3 is master input gate signals, 4 is busy out signal and 5-7 is master output gate signals. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
            try {
                StringTo<defs::externalSignalFlag>(args[1]);
            } catch (...) {
                throw RuntimeError(
                    "Could not convert argument 1 to defs::externalSignalFlag");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            auto t =
                det->getExternalSignalFlags(arg0, std::vector<int>{det_id});
            os << args[0] << ' ' << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            auto arg0 = StringTo<int>(args[0]);
            auto arg1 = StringTo<defs::externalSignalFlag>(args[1]);
            det->setExternalSignalFlags(arg0, arg1, std::vector<int>{det_id});
            os << args[0] << ' ' << args[1] << '\n';
        }
    }

    return os.str();
}

std::string Caller::fformat(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: fformat" << std::endl;
        os << R"V0G0N([binary|hdf5]
	File format of data file. For HDF5, package must be compiled with HDF5 flags. Default is binary. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<defs::fileFormat>(args[0]);
            } catch (...) {
                throw RuntimeError(
                    "Could not convert argument 0 to defs::fileFormat");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getFileFormat(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<defs::fileFormat>(args[0]);
            det->setFileFormat(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([0-12]
	[Jungfrau] Set Filter Cell. Only for chipv1.1. Advanced user Command )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getNumberOfFilterCells(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setNumberOfFilterCells(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([value] [Gotthard2][Jungfrau] Set filter resistor. Increasing values for increasing resistance.
	[Gotthard2] Options: [0|1|2|3]. Default is 0.
	[Jungfrau] Options: [0|1]. Default is 1. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getFilterResistor(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setFilterResistor(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([n_value]
	File or Acquisition index. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<uint64_t>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to uint64_t");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getAcquisitionIndex(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<uint64_t>(args[0]);
            det->setAcquisitionIndex(arg0, std::vector<int>{det_id});
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
	[Jungfrau][Moench][Gotthard][Mythen3][Gotthard2][Ctb][Xilinx Ctb] Firmware test, ie. reads a read fixed pattern from a register. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 0) {
            det->executeFirmwareTest(std::vector<int>{det_id});
            os << "successful" << '\n';
        }
    }

    return os.str();
}

std::string Caller::firmwareversion(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: firmwareversion" << std::endl;
        os << R"V0G0N(
	Firmware version of detector in format [0xYYMMDD] or an increasing 2 digit number for Eiger. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    auto detector_type = det->getDetectorType().squash();
    if (action == slsDetectorDefs::GET_ACTION) {
        if (detector_type == defs::EIGER) {
            if (args.size() == 0) {
                auto t = det->getFirmwareVersion(std::vector<int>{det_id});
                os << OutString(t) << '\n';
            }

        }

        else {

            if (args.size() == 0) {
                auto t = det->getFirmwareVersion(std::vector<int>{det_id});
                os << OutStringHex(t) << '\n';
            }
        }
    }

    return os.str();
}

std::string Caller::fliprows(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: fliprows" << std::endl;
        os << R"V0G0N([0, 1]
	[Eiger] flips rows paramater sent to slsreceiver to stream as json parameter to flip rows in gui 
	[Jungfrau][Moench] flips rows in the detector itself. For bottom module and number of interfaces must be set to 2. slsReceiver and slsDetectorGui does not handle. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getFlipRows(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setFlipRows(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([0, 1]
	[Eiger][Jungfrau][Moench] 10GbE Flow Control. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getTenGigaFlowControl(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setTenGigaFlowControl(arg0, std::vector<int>{det_id});
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
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
            auto arg0 = StringTo<bool>(args[0]);
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
        os << R"V0G0N([name]
	File name prefix for output data file. Default is run. File name: [file name prefix]_d[detector index]_f[sub file index]_[acquisition/file index].raw. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getFileNamePrefix(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            det->setFileNamePrefix(args[0], std::vector<int>{det_id});
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
        os << R"V0G0N([0, 1]
	Enable or disable file overwriting. Default is 1. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getFileOverWrite(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setFileOverWrite(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([path]
	Directory where output data files are written in receiver. Default is '/'. 
	If path does not exist, it will try to create it. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getFilePath(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            det->setFilePath(args[0], std::vector<int>{det_id});
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
	[Jungfrau][Moench][Mythen3][Gotthard2][Ctb][Xilinx Ctb] Number of frames from start run control.
	[Gotthard2] only in continuous mode. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getNumberOfFramesFromStart(std::vector<int>{det_id});
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int64_t>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int64_t");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getNumberOfFrames(std::vector<int>{det_id});
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
	[Gotthard][Jungfrau][Moench][Mythen3][Gotthard2][Ctb][Xilinx Ctb] Number of frames left in acquisition. 
	[Gotthard2] only in continuous auto mode. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getNumberOfFramesLeft(std::vector<int>{det_id});
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
        os << R"V0G0N([(optional unit) ns|us|ms|s]
	[Jungfrau][Moench][Mythen3][Gotthard2][Ctb][Xilinx Ctb] Timestamp at a frame start.
	[Gotthard2] not in burst and auto mode. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

        if (args.size() == 1) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getMeasurementTime(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }

        if (args.size() == 1) {
            auto t = det->getMeasurementTime(std::vector<int>{det_id});
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
        os << R"V0G0N([0, 1]
	Enable or disable receiver file write. Default is 1. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getFileWrite(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setFileWrite(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([dynamicgain|forceswitchg1|forceswitchg2|fixg1|fixg2|fixg0]
	[Jungfrau] Gain mode.
	CAUTION: Do not use fixg0 without caution, you can damage the detector!!! )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<defs::gainMode>(args[0]);
            } catch (...) {
                throw RuntimeError(
                    "Could not convert argument 0 to defs::gainMode");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getGainMode(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<defs::gainMode>(args[0]);
            det->setGainMode(arg0, std::vector<int>{det_id});
            os << args.front() << '\n';
        }
    }

    return os.str();
}

std::string Caller::gappixels(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: gappixels" << std::endl;
        os << R"V0G0N([0, 1]
	[Eiger][Jungfrau][Moench] Include Gap pixels in client data call back in Detecor api. Will not be in detector streaming, receiver file or streaming. Default is 0.  )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            if (det_id != -1) {
                throw RuntimeError("Cannot execute gappixels at module level");
            }
            auto t = det->getGapPixelsinCallback();
            os << t << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            if (det_id != -1) {
                throw RuntimeError("Cannot execute gappixels at module level");
            }
            auto arg0 = StringTo<bool>(args[0]);
            det->setGapPixelsinCallback(arg0);
            os << args.front() << '\n';
        }
    }

    return os.str();
}

std::string Caller::gatedelay(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: gatedelay" << std::endl;
        os << R"V0G0N([duration] [(optional unit) ns|us|ms|s]
	[Mythen3] Gate Delay of all gate signals in auto and trigger mode (internal gating). )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

        if (args.size() == 1) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            int gateIndex = -1;
            try {
                std::string tmp_time(args[0]);
                std::string unit = RemoveUnit(tmp_time);
                auto converted_time = StringTo<time::ns>(tmp_time, unit);
            } catch (...) {
                throw RuntimeError("Could not convert argument to time::ns");
            }
        }

        if (args.size() == 2) {
            int gateIndex = -1;
            try {
                StringTo<time::ns>(args[0], args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert arguments to time::ns");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getGateDelayForAllGates(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }

        if (args.size() == 1) {
            auto t = det->getGateDelayForAllGates(std::vector<int>{det_id});
            os << OutString(t, args[0]) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            int gateIndex = -1;
            std::string tmp_time(args[0]);
            std::string unit = RemoveUnit(tmp_time);
            auto converted_time = StringTo<time::ns>(tmp_time, unit);
            det->setGateDelay(gateIndex, converted_time,
                              std::vector<int>{det_id});
            os << args[0] << '\n';
        }

        if (args.size() == 2) {
            int gateIndex = -1;
            auto converted_time = StringTo<time::ns>(args[0], args[1]);
            det->setGateDelay(gateIndex, converted_time,
                              std::vector<int>{det_id});
            os << args[0] << args[1] << '\n';
        }
    }

    return os.str();
}

std::string Caller::gatedelay1(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: gatedelay1" << std::endl;
        os << R"V0G0N([duration] [(optional unit) ns|us|ms|s]
	[Mythen3] Gate Delay of gate signal 1 in auto and trigger mode (internal gating). )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
            int gateIndex = 0;
        }

        if (args.size() == 1) {
            int gateIndex = 0;
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            int gateIndex = 0;
            try {
                std::string tmp_time(args[0]);
                std::string unit = RemoveUnit(tmp_time);
                auto converted_time = StringTo<time::ns>(tmp_time, unit);
            } catch (...) {
                throw RuntimeError("Could not convert argument to time::ns");
            }
        }

        if (args.size() == 2) {
            int gateIndex = 0;
            try {
                StringTo<time::ns>(args[0], args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert arguments to time::ns");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            int gateIndex = 0;
            auto t = det->getGateDelay(gateIndex, std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }

        if (args.size() == 1) {
            int gateIndex = 0;
            auto t = det->getGateDelay(gateIndex, std::vector<int>{det_id});
            os << OutString(t, args[0]) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            int gateIndex = 0;
            std::string tmp_time(args[0]);
            std::string unit = RemoveUnit(tmp_time);
            auto converted_time = StringTo<time::ns>(tmp_time, unit);
            det->setGateDelay(gateIndex, converted_time,
                              std::vector<int>{det_id});
            os << args[0] << '\n';
        }

        if (args.size() == 2) {
            int gateIndex = 0;
            auto converted_time = StringTo<time::ns>(args[0], args[1]);
            det->setGateDelay(gateIndex, converted_time,
                              std::vector<int>{det_id});
            os << args[0] << args[1] << '\n';
        }
    }

    return os.str();
}

std::string Caller::gatedelay2(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: gatedelay2" << std::endl;
        os << R"V0G0N([duration] [(optional unit) ns|us|ms|s]
	[Mythen3] Gate Delay of gate signal 2 in auto and trigger mode (internal gating). )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
            int gateIndex = 1;
        }

        if (args.size() == 1) {
            int gateIndex = 1;
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            int gateIndex = 1;
            try {
                std::string tmp_time(args[0]);
                std::string unit = RemoveUnit(tmp_time);
                auto converted_time = StringTo<time::ns>(tmp_time, unit);
            } catch (...) {
                throw RuntimeError("Could not convert argument to time::ns");
            }
        }

        if (args.size() == 2) {
            int gateIndex = 1;
            try {
                StringTo<time::ns>(args[0], args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert arguments to time::ns");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            int gateIndex = 1;
            auto t = det->getGateDelay(gateIndex, std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }

        if (args.size() == 1) {
            int gateIndex = 1;
            auto t = det->getGateDelay(gateIndex, std::vector<int>{det_id});
            os << OutString(t, args[0]) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            int gateIndex = 1;
            std::string tmp_time(args[0]);
            std::string unit = RemoveUnit(tmp_time);
            auto converted_time = StringTo<time::ns>(tmp_time, unit);
            det->setGateDelay(gateIndex, converted_time,
                              std::vector<int>{det_id});
            os << args[0] << '\n';
        }

        if (args.size() == 2) {
            int gateIndex = 1;
            auto converted_time = StringTo<time::ns>(args[0], args[1]);
            det->setGateDelay(gateIndex, converted_time,
                              std::vector<int>{det_id});
            os << args[0] << args[1] << '\n';
        }
    }

    return os.str();
}

std::string Caller::gatedelay3(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: gatedelay3" << std::endl;
        os << R"V0G0N([duration] [(optional unit) ns|us|ms|s]
	[Mythen3] Gate Delay of gate signal 3 in auto and trigger mode (internal gating). )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
            int gateIndex = 2;
        }

        if (args.size() == 1) {
            int gateIndex = 2;
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            int gateIndex = 2;
            try {
                std::string tmp_time(args[0]);
                std::string unit = RemoveUnit(tmp_time);
                auto converted_time = StringTo<time::ns>(tmp_time, unit);
            } catch (...) {
                throw RuntimeError("Could not convert argument to time::ns");
            }
        }

        if (args.size() == 2) {
            int gateIndex = 2;
            try {
                StringTo<time::ns>(args[0], args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert arguments to time::ns");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            int gateIndex = 2;
            auto t = det->getGateDelay(gateIndex, std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }

        if (args.size() == 1) {
            int gateIndex = 2;
            auto t = det->getGateDelay(gateIndex, std::vector<int>{det_id});
            os << OutString(t, args[0]) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            int gateIndex = 2;
            std::string tmp_time(args[0]);
            std::string unit = RemoveUnit(tmp_time);
            auto converted_time = StringTo<time::ns>(tmp_time, unit);
            det->setGateDelay(gateIndex, converted_time,
                              std::vector<int>{det_id});
            os << args[0] << '\n';
        }

        if (args.size() == 2) {
            int gateIndex = 2;
            auto converted_time = StringTo<time::ns>(args[0], args[1]);
            det->setGateDelay(gateIndex, converted_time,
                              std::vector<int>{det_id});
            os << args[0] << args[1] << '\n';
        }
    }

    return os.str();
}

std::string Caller::gates(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: gates" << std::endl;
        os << R"V0G0N([n_gates]
	[Mythen3] Number of external gates in gating or trigger_gating mode (external gating). )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getNumberOfGates(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setNumberOfGates(arg0, std::vector<int>{det_id});
            os << args.front() << '\n';
        }
    }

    return os.str();
}

std::string Caller::getbit(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: getbit" << std::endl;
        os << R"V0G0N([reg address in hex] [bit index]
	Gets bit in address. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 2) {
            try {
                StringTo<uint32_t>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to uint32_t");
            }
            try {
                StringTo<int>(args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 2) {
            if (StringTo<int>(args[1]) < 0 || StringTo<int>(args[1]) > 31) {
                throw RuntimeError("Bit number out of range: " + args[1]);
            }
            auto arg0 = StringTo<uint32_t>(args[0]);
            auto arg1 = StringTo<int>(args[1]);
            auto t = det->getBit(arg0, arg1, std::vector<int>{det_id});
            os << OutString(t) << '\n';
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
	Hardware version of detector. 
	[Eiger] Hardware version of front FPGA on detector. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getHardwareVersion(std::vector<int>{det_id});
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
        os << R"V0G0N([n_value]
	High voltage to the sensor in Voltage. 
	[Gotthard] [0|90|110|120|150|180|200] 
	[Eiger][Mythen3][Gotthard2] 0-200 
	[Jungfrau][Moench][Ctb] [0|60-200] )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getHighVoltage(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setHighVoltage(arg0, std::vector<int>{det_id});
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
	[Ctb] Measured current of power supply a in mA. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getMeasuredCurrent(defs::I_POWER_A,
                                             std::vector<int>{det_id});
            os << OutString(t) << '\n';
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
	[Ctb] Measured current of power supply b in mA. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getMeasuredCurrent(defs::I_POWER_B,
                                             std::vector<int>{det_id});
            os << OutString(t) << '\n';
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
	[Ctb] Measured current of power supply c in mA. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getMeasuredCurrent(defs::I_POWER_C,
                                             std::vector<int>{det_id});
            os << OutString(t) << '\n';
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
	[Ctb] Measured current of power supply d in mA. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getMeasuredCurrent(defs::I_POWER_D,
                                             std::vector<int>{det_id});
            os << OutString(t) << '\n';
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
	[Ctb] Measured current of power supply io in mA. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getMeasuredCurrent(defs::I_POWER_IO,
                                             std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    return os.str();
}

std::string Caller::imagetest(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: imagetest" << std::endl;
        os << R"V0G0N([0, 1]
	[Gotthard] 1 adds channel intensity with precalculated values when taking an acquisition. Default is 0.
	[Eiger][Jungfrau][Moench] Only for Virtual servers. If 0, each pixel intensity incremented by 1. If 1, all pixels almost saturated. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getImageTestMode(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setImageTestMode(arg0, std::vector<int>{det_id});
            os << args.front() << '\n';
        }
    }

    return os.str();
}

std::string Caller::initialchecks(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: initialchecks" << std::endl;
        os << R"V0G0N([0, 1]
	[Mythen3][Gotthard2] Enable or disable intial compatibility and other checks at detector start up. It is enabled by default. Must come before 'hostname' command to take effect. Can be used to reprogram fpga when current firmware is incompatible.
	Advanced User function! )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            if (det_id != -1) {
                throw RuntimeError(
                    "Cannot execute initialchecks at module level");
            }
            auto t = det->getInitialChecks();
            os << t << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            if (det_id != -1) {
                throw RuntimeError(
                    "Cannot execute initialchecks at module level");
            }
            auto arg0 = StringTo<bool>(args[0]);
            det->setInitialChecks(arg0);
            os << args.front() << '\n';
        }
    }

    return os.str();
}

std::string Caller::inj_ch(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: inj_ch" << std::endl;
        os << R"V0G0N([offset] [increment]
	[Gotthard2] Inject channels with current source for calibration. Offset is starting channel that is injected, increment determines succeeding channels to be injected. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
            try {
                StringTo<int>(args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getInjectChannel(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            auto arg0 = StringTo<int>(args[0]);
            auto arg1 = StringTo<int>(args[1]);
            det->setInjectChannel(arg0, arg1, std::vector<int>{det_id});
            os << ToString(args) << '\n';
        }
    }

    return os.str();
}

std::string Caller::interpolation(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: interpolation" << std::endl;
        os << R"V0G0N([0, 1]
	[Mythen3] Enables or disables interpolation. Default is disabled.  Interpolation mode enables all counters and disables vth3. Disabling sets back counter mask and vth3. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getInterpolation(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setInterpolation(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([0, 1]
	[Eiger] 1 interrupts last subframe at required exposure time. 0 will wait for last sub frame to finish exposing. 0 is default. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getInterruptSubframe(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setInterruptSubframe(arg0, std::vector<int>{det_id});
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getKernelVersion(std::vector<int>{det_id});
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getLastClientIP(std::vector<int>{det_id});
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
        os << R"V0G0N([0, 1]
	[Ctb] Switches on/off all LEDs. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getLEDEnable(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setLEDEnable(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([0, 1]
	Lock detector to one IP, 1: locks. Default is unlocked )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getDetectorLock(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setDetectorLock(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([0, 1]
	[Eiger][Gotthard2][Jungfrau][Moench] Sets (half) module to master and other(s) to slaves.
	[Gotthard][Gotthard2][Mythen3][Eiger][Jungfrau][Moench] Gets if the current (half) module is master. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getMaster(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
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
	[Jungfrau][Moench][Ctb] Absolute maximum Phase shift of ADC clock. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getMaxADCPhaseShift(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    return os.str();
}

std::string Caller::maxclkphaseshift(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: maxclkphaseshift" << std::endl;
        os << R"V0G0N([n_clock (0-5)]
	[Gotthard2][Mythen3] Absolute Maximum Phase shift of clock n_clock. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
            defs::detectorType type =
                det->getDetectorType().squash(defs::GENERIC);
            ;
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            defs::detectorType type =
                det->getDetectorType().squash(defs::GENERIC);
            ;
            if (type != defs::GOTTHARD2 && type != defs::MYTHEN3) {
                throw RuntimeError(
                    "maxclkphaseshift not implemented for this detector.");
            }
            auto arg0 = StringTo<int>(args[0]);
            auto t = det->getMaxClockPhaseShift(arg0, std::vector<int>{det_id});
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
	[Ctb][Jungfrau] Absolute maximum Phase shift of of the clock to latch digital bits. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getMaxDBITPhaseShift(std::vector<int>{det_id});
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
        os << R"V0G0N([(optional unit) ns|us|ms|s]
	[Eiger] Measured frame period between last frame and previous one. Can be measured with minimum 2 frames in an acquisition. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

        if (args.size() == 1) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getMeasuredPeriod(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }

        if (args.size() == 1) {
            auto t = det->getMeasuredPeriod(std::vector<int>{det_id});
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
        os << R"V0G0N([(optional unit) ns|us|ms|s]
	[Eiger] Measured sub frame period between last sub frame and previous one. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

        if (args.size() == 1) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getMeasuredSubFramePeriod(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }

        if (args.size() == 1) {
            auto t = det->getMeasuredSubFramePeriod(std::vector<int>{det_id});
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getModuleId(std::vector<int>{det_id});
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
        os << R"V0G0N([n_value]
	[Eiger][Jungfrau][Moench][Ctb][Xilinx Ctb] Next frame number. Stopping acquisition might result in different frame numbers for different modules. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<uint64_t>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to uint64_t");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getNextFrameNumber(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<uint64_t>(args[0]);
            det->setNextFrameNumber(arg0, std::vector<int>{det_id});
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
	Number of modules in shared memory. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
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
        os << R"V0G0N([1, 2]
	[Jungfrau][Moench] Number of udp interfaces to stream data from detector. Default: 1.
	Also enables second interface in receiver for listening (Writes a file per interface if writing enabled).
	Also restarts client and receiver zmq sockets if zmq streaming enabled.
	[Eiger] Only gets with result 2. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getNumberofUDPInterfaces(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setNumberofUDPInterfaces(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([0, 1]
	[Eiger] Enable or disable show overflow flag in 32 bit mode. Default is disabled. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getOverFlowMode(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setOverFlowMode(arg0, std::vector<int>{det_id});
            os << args.front() << '\n';
        }
    }

    return os.str();
}

std::string Caller::packageversion(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: packageversion" << std::endl;
        os << R"V0G0N(
	Package version. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getPackageVersion();
            os << t << '\n';
        }
    }

    return os.str();
}

std::string Caller::parallel(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: parallel" << std::endl;
        os << R"V0G0N([0, 1]
	[Eiger][Mythen3][Gotthard2][Moench] Enable or disable parallel mode.
	[Mythen3] If exptime is too short, the acquisition will return ERROR status and take fewer frames than expected.
	[Mythen3][Eiger][Moench] Default: Non parallel.
	[Gotthard2] Default: Parallel. Non parallel mode works only in continuous mode. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getParallelMode(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setParallelMode(arg0, std::vector<int>{det_id});
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
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
        os << R"V0G0N([0, 1]
	[Eiger] Sets up detector to do partial or complete reset at start of acquisition. 0 complete reset, 1 partial reset. Default is complete reset. Advanced function! )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getPartialReset(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setPartialReset(arg0, std::vector<int>{det_id});
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
	[Ctb][Mythen3][Xilinx Ctb] Gets the pattern file name including path of the last pattern uploaded. Returns an empty if nothing was uploaded or via a server default file )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getPatterFileName(std::vector<int>{det_id});
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
        os << R"V0G0N([64 bit mask]
	[Ctb] 64 bit mask defining input (0) and output (1) signals. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<uint64_t>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to uint64_t");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getPatternIOControl(std::vector<int>{det_id});
            os << OutStringHex(t, 16) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<uint64_t>(args[0]);
            det->setPatternIOControl(arg0, std::vector<int>{det_id});
            os << ToStringHex(arg0, 16) << '\n';
        }
    }

    return os.str();
}

std::string Caller::patlimits(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: patlimits" << std::endl;
        os << R"V0G0N([start addr] [stop addr] 
	[Ctb][Mythen3][Xilinx Ctb] Limits of complete pattern )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        auto arg0 = StringTo<int>("-1");
        auto t = det->getPatternLoopAddresses(arg0, std::vector<int>{det_id});
        os << OutStringHex(t, 4) << '\n';
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        auto arg0 = StringTo<int>("-1");
        auto arg1 = StringTo<int>(args[0]);
        auto arg2 = StringTo<int>(args[1]);
        det->setPatternLoopAddresses(arg0, arg1, arg2,
                                     std::vector<int>{det_id});
        os << '[' << ToStringHex(arg1, 4) << ", " << ToStringHex(arg2, 4) << ']'
           << '\n';
    }

    return os.str();
}

std::string Caller::patloop(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: patloop" << std::endl;
        os << R"V0G0N([0-6] [start addr] [stop addr] 
	[Ctb][Mythen3][Xilinx Ctb] Limits of the loop level provided.
	[Mythen3] Level options: 0-3 only. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 2;
        GetLevelAndUpdateArgIndex(action, "patloop", level, iArg, nGetArgs,
                                  nPutArgs);
        auto t = det->getPatternLoopAddresses(level, std::vector<int>{det_id});
        os << level << ' ' << OutStringHex(t, 4) << '\n';
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 2;
        GetLevelAndUpdateArgIndex(action, "patloop", level, iArg, nGetArgs,
                                  nPutArgs);
        int start = StringTo<int>(args[iArg++]);
        int stop = StringTo<int>(args[iArg++]);
        det->setPatternLoopAddresses(level, start, stop,
                                     std::vector<int>{det_id});
        os << level << ' ' << '[' << ToStringHex(start, 4) << ", "
           << ToStringHex(stop, 4) << ']' << '\n';
    }

    return os.str();
}

std::string Caller::patloop0(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: patloop0" << std::endl;
        os << R"V0G0N(Deprecated command. Use patloop. )V0G0N" << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 2;
        GetLevelAndUpdateArgIndex(action, "patloop", level, iArg, nGetArgs,
                                  nPutArgs);
        auto t = det->getPatternLoopAddresses(level, std::vector<int>{det_id});
        os << OutStringHex(t, 4) << '\n';
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 2;
        GetLevelAndUpdateArgIndex(action, "patloop", level, iArg, nGetArgs,
                                  nPutArgs);
        int start = StringTo<int>(args[iArg++]);
        int stop = StringTo<int>(args[iArg++]);
        det->setPatternLoopAddresses(level, start, stop,
                                     std::vector<int>{det_id});
        os << '[' << ToStringHex(start, 4) << ", " << ToStringHex(stop, 4)
           << ']' << '\n';
    }

    return os.str();
}

std::string Caller::patloop1(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: patloop1" << std::endl;
        os << R"V0G0N(Deprecated command. Use patloop. )V0G0N" << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 2;
        GetLevelAndUpdateArgIndex(action, "patloop", level, iArg, nGetArgs,
                                  nPutArgs);
        auto t = det->getPatternLoopAddresses(level, std::vector<int>{det_id});
        os << OutStringHex(t, 4) << '\n';
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 2;
        GetLevelAndUpdateArgIndex(action, "patloop", level, iArg, nGetArgs,
                                  nPutArgs);
        int start = StringTo<int>(args[iArg++]);
        int stop = StringTo<int>(args[iArg++]);
        det->setPatternLoopAddresses(level, start, stop,
                                     std::vector<int>{det_id});
        os << '[' << ToStringHex(start, 4) << ", " << ToStringHex(stop, 4)
           << ']' << '\n';
    }

    return os.str();
}

std::string Caller::patloop2(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: patloop2" << std::endl;
        os << R"V0G0N(Deprecated command. Use patloop. )V0G0N" << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 2;
        GetLevelAndUpdateArgIndex(action, "patloop", level, iArg, nGetArgs,
                                  nPutArgs);
        auto t = det->getPatternLoopAddresses(level, std::vector<int>{det_id});
        os << OutStringHex(t, 4) << '\n';
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 2;
        GetLevelAndUpdateArgIndex(action, "patloop", level, iArg, nGetArgs,
                                  nPutArgs);
        int start = StringTo<int>(args[iArg++]);
        int stop = StringTo<int>(args[iArg++]);
        det->setPatternLoopAddresses(level, start, stop,
                                     std::vector<int>{det_id});
        os << '[' << ToStringHex(start, 4) << ", " << ToStringHex(stop, 4)
           << ']' << '\n';
    }

    return os.str();
}

std::string Caller::patmask(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: patmask" << std::endl;
        os << R"V0G0N([64 bit mask]
	[Ctb][Mythen3][Xilinx Ctb] Selects the bits that will have a pattern mask applied to the selected patmask for every pattern. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<uint64_t>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to uint64_t");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getPatternMask(std::vector<int>{det_id});
            os << OutStringHex(t, 16) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<uint64_t>(args[0]);
            det->setPatternMask(arg0, std::vector<int>{det_id});
            os << ToStringHex(arg0, 16) << '\n';
        }
    }

    return os.str();
}

std::string Caller::patnloop(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: patnloop" << std::endl;
        os << R"V0G0N([0-6] [n_cycles] 
	[Ctb][Mythen3][Xilinx Ctb] Number of cycles of the loop level provided.
	[Mythen3] Level options: 0-3 only. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 1;
        GetLevelAndUpdateArgIndex(action, "patnloop", level, iArg, nGetArgs,
                                  nPutArgs);
        auto t = det->getPatternLoopCycles(level, std::vector<int>{det_id});
        os << level << ' ' << OutString(t) << '\n';
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 1;
        GetLevelAndUpdateArgIndex(action, "patnloop", level, iArg, nGetArgs,
                                  nPutArgs);
        std::string nloops = args[iArg++];
        auto arg1 = StringTo<int>(nloops);
        det->setPatternLoopCycles(level, arg1, std::vector<int>{det_id});
        os << level << ' ' << nloops << '\n';
    }

    return os.str();
}

std::string Caller::patnloop0(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: patnloop0" << std::endl;
        os << R"V0G0N(Deprecated command. Use patnloop. )V0G0N" << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 1;
        GetLevelAndUpdateArgIndex(action, "patnloop", level, iArg, nGetArgs,
                                  nPutArgs);
        auto t = det->getPatternLoopCycles(level, std::vector<int>{det_id});
        os << OutString(t) << '\n';
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 1;
        GetLevelAndUpdateArgIndex(action, "patnloop", level, iArg, nGetArgs,
                                  nPutArgs);
        std::string nloops = args[iArg++];
        auto arg1 = StringTo<int>(nloops);
        det->setPatternLoopCycles(level, arg1, std::vector<int>{det_id});
        os << nloops << '\n';
    }

    return os.str();
}

std::string Caller::patnloop1(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: patnloop1" << std::endl;
        os << R"V0G0N(Deprecated command. Use patnloop. )V0G0N" << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 1;
        GetLevelAndUpdateArgIndex(action, "patnloop", level, iArg, nGetArgs,
                                  nPutArgs);
        auto t = det->getPatternLoopCycles(level, std::vector<int>{det_id});
        os << OutString(t) << '\n';
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 1;
        GetLevelAndUpdateArgIndex(action, "patnloop", level, iArg, nGetArgs,
                                  nPutArgs);
        std::string nloops = args[iArg++];
        auto arg1 = StringTo<int>(nloops);
        det->setPatternLoopCycles(level, arg1, std::vector<int>{det_id});
        os << nloops << '\n';
    }

    return os.str();
}

std::string Caller::patnloop2(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: patnloop2" << std::endl;
        os << R"V0G0N(Deprecated command. Use patnloop. )V0G0N" << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 1;
        GetLevelAndUpdateArgIndex(action, "patnloop", level, iArg, nGetArgs,
                                  nPutArgs);
        auto t = det->getPatternLoopCycles(level, std::vector<int>{det_id});
        os << OutString(t) << '\n';
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 1;
        GetLevelAndUpdateArgIndex(action, "patnloop", level, iArg, nGetArgs,
                                  nPutArgs);
        std::string nloops = args[iArg++];
        auto arg1 = StringTo<int>(nloops);
        det->setPatternLoopCycles(level, arg1, std::vector<int>{det_id});
        os << nloops << '\n';
    }

    return os.str();
}

std::string Caller::patsetbit(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: patsetbit" << std::endl;
        os << R"V0G0N([64 bit mask]
	[Ctb][Mythen3][Xilinx Ctb] Sets the mask applied to every pattern to the selected bits. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<uint64_t>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to uint64_t");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getPatternBitMask(std::vector<int>{det_id});
            os << OutStringHex(t, 16) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<uint64_t>(args[0]);
            det->setPatternBitMask(arg0, std::vector<int>{det_id});
            os << ToStringHex(arg0, 16) << '\n';
        }
    }

    return os.str();
}

std::string Caller::pattern(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: patternX" << std::endl;
        os << R"V0G0N([fname]
	[Mythen3][Ctb][Xilinx Ctb] Loads ASCII pattern file directly to server (instead of executing line by line) )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            det->setPattern(args[0], std::vector<int>{det_id});
            os << args.front() << '\n';
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
	[Mythen3] Starts Pattern )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 0) {
            det->startPattern(std::vector<int>{det_id});
            os << "successful" << '\n';
        }
    }

    return os.str();
}

std::string Caller::patwait(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: patwait" << std::endl;
        os << R"V0G0N([0-6] [addr] 
	[Ctb][Mythen3][Xilinx Ctb] Wait address for loop level provided. 
	[Mythen3] Level options: 0-3 only. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 1;
        GetLevelAndUpdateArgIndex(action, "patwait", level, iArg, nGetArgs,
                                  nPutArgs);
        auto t = det->getPatternWaitAddr(level, std::vector<int>{det_id});
        os << level << ' ' << OutStringHex(t, 4) << '\n';
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 1;
        GetLevelAndUpdateArgIndex(action, "patwait", level, iArg, nGetArgs,
                                  nPutArgs);
        int addr = StringTo<int>(args[iArg++]);
        det->setPatternWaitAddr(level, addr, std::vector<int>{det_id});
        os << level << ' ' << ToStringHex(addr, 4) << '\n';
    }

    return os.str();
}

std::string Caller::patwait0(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: patwait0" << std::endl;
        os << R"V0G0N(Deprecated command. Use patwait. )V0G0N" << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 1;
        GetLevelAndUpdateArgIndex(action, "patwait", level, iArg, nGetArgs,
                                  nPutArgs);
        auto t = det->getPatternWaitAddr(level, std::vector<int>{det_id});
        os << OutStringHex(t, 4) << '\n';
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 1;
        GetLevelAndUpdateArgIndex(action, "patwait", level, iArg, nGetArgs,
                                  nPutArgs);
        int addr = StringTo<int>(args[iArg++]);
        det->setPatternWaitAddr(level, addr, std::vector<int>{det_id});
        os << ToStringHex(addr, 4) << '\n';
    }

    return os.str();
}

std::string Caller::patwait1(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: patwait1" << std::endl;
        os << R"V0G0N(Deprecated command. Use patwait. )V0G0N" << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 1;
        GetLevelAndUpdateArgIndex(action, "patwait", level, iArg, nGetArgs,
                                  nPutArgs);
        auto t = det->getPatternWaitAddr(level, std::vector<int>{det_id});
        os << OutStringHex(t, 4) << '\n';
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 1;
        GetLevelAndUpdateArgIndex(action, "patwait", level, iArg, nGetArgs,
                                  nPutArgs);
        int addr = StringTo<int>(args[iArg++]);
        det->setPatternWaitAddr(level, addr, std::vector<int>{det_id});
        os << ToStringHex(addr, 4) << '\n';
    }

    return os.str();
}

std::string Caller::patwait2(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: patwait2" << std::endl;
        os << R"V0G0N(Deprecated command. Use patwait. )V0G0N" << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 1;
        GetLevelAndUpdateArgIndex(action, "patwait", level, iArg, nGetArgs,
                                  nPutArgs);
        auto t = det->getPatternWaitAddr(level, std::vector<int>{det_id});
        os << OutStringHex(t, 4) << '\n';
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 1;
        GetLevelAndUpdateArgIndex(action, "patwait", level, iArg, nGetArgs,
                                  nPutArgs);
        int addr = StringTo<int>(args[iArg++]);
        det->setPatternWaitAddr(level, addr, std::vector<int>{det_id});
        os << ToStringHex(addr, 4) << '\n';
    }

    return os.str();
}

std::string Caller::patwaittime(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: patwaittime" << std::endl;
        os << R"V0G0N([0-6] [n_clk] 
	[Ctb][Mythen3][Xilinx Ctb] Wait time in clock cycles for the loop provided.
	[Mythen3] Level options: 0-3 only. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 1;
        GetLevelAndUpdateArgIndex(action, "patwaittime", level, iArg, nGetArgs,
                                  nPutArgs);
        auto t = det->getPatternWaitTime(level, std::vector<int>{det_id});
        os << level << ' ' << OutString(t) << '\n';
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 1;
        GetLevelAndUpdateArgIndex(action, "patwaittime", level, iArg, nGetArgs,
                                  nPutArgs);
        uint64_t waittime = StringTo<uint64_t>(args[iArg++]);
        det->setPatternWaitTime(level, waittime, std::vector<int>{det_id});
        os << level << ' ' << waittime << '\n';
    }

    return os.str();
}

std::string Caller::patwaittime0(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: patwaittime0" << std::endl;
        os << R"V0G0N(Deprecated command. Use patwaittime. )V0G0N" << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 1;
        GetLevelAndUpdateArgIndex(action, "patwaittime", level, iArg, nGetArgs,
                                  nPutArgs);
        auto t = det->getPatternWaitTime(level, std::vector<int>{det_id});
        os << OutString(t) << '\n';
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 1;
        GetLevelAndUpdateArgIndex(action, "patwaittime", level, iArg, nGetArgs,
                                  nPutArgs);
        uint64_t waittime = StringTo<uint64_t>(args[iArg++]);
        det->setPatternWaitTime(level, waittime, std::vector<int>{det_id});
        os << waittime << '\n';
    }

    return os.str();
}

std::string Caller::patwaittime1(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: patwaittime1" << std::endl;
        os << R"V0G0N(Deprecated command. Use patwaittime. )V0G0N" << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 1;
        GetLevelAndUpdateArgIndex(action, "patwaittime", level, iArg, nGetArgs,
                                  nPutArgs);
        auto t = det->getPatternWaitTime(level, std::vector<int>{det_id});
        os << OutString(t) << '\n';
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 1;
        GetLevelAndUpdateArgIndex(action, "patwaittime", level, iArg, nGetArgs,
                                  nPutArgs);
        uint64_t waittime = StringTo<uint64_t>(args[iArg++]);
        det->setPatternWaitTime(level, waittime, std::vector<int>{det_id});
        os << waittime << '\n';
    }

    return os.str();
}

std::string Caller::patwaittime2(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: patwaittime2" << std::endl;
        os << R"V0G0N(Deprecated command. Use patwaittime. )V0G0N" << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 1;
        GetLevelAndUpdateArgIndex(action, "patwaittime", level, iArg, nGetArgs,
                                  nPutArgs);
        auto t = det->getPatternWaitTime(level, std::vector<int>{det_id});
        os << OutString(t) << '\n';
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        int level = -1, iArg = 0, nGetArgs = 0, nPutArgs = 1;
        GetLevelAndUpdateArgIndex(action, "patwaittime", level, iArg, nGetArgs,
                                  nPutArgs);
        uint64_t waittime = StringTo<uint64_t>(args[iArg++]);
        det->setPatternWaitTime(level, waittime, std::vector<int>{det_id});
        os << waittime << '\n';
    }

    return os.str();
}

std::string Caller::patword(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: patword" << std::endl;
        os << R"V0G0N([step or address] [64 bit mask]
	[Ctb][Mythen3][Xilinx Ctb] 64 bit pattern at address of pattern memory.
	[Ctb] read is same as executing pattern )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
            try {
                StringTo<uint64_t>(args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to uint64_t");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            auto t = det->getPatternWord(arg0, std::vector<int>{det_id});
            os << '[' << ToStringHex(arg0, 4) << ", " << OutStringHex(t, 16)
               << "]" << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            auto arg0 = StringTo<int>(args[0]);
            auto arg1 = StringTo<uint64_t>(args[1]);
            det->setPatternWord(arg0, arg1, std::vector<int>{det_id});
            os << '[' << ToStringHex(arg0, 4) << ", " << ToStringHex(arg1, 16)
               << "]" << '\n';
        }
    }

    return os.str();
}

std::string Caller::pedestalmode(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: pedestalmode" << std::endl;
        os << R"V0G0N( [frames<uint8_t>] [loops<uint16_t>]
		[Jungfrau] Enable pedestal mode. 
		The number of frames or triggers is overwritten by: 
		(#pedestal_frames x #pedestal_loops x 2). 
		In auto timing mode or in trigger mode with #frames > 1, 
		#frames is overwritten and #triggers = 1, 
		else #triggers is overwritten and #frames = 1. 
		One cannot set #frames, #triggers or timing mode in pedestal mode (exception thrown).

pedestalmode [0]
		[Jungfrau] Disable pedestal mode.
		Disabling pedestal mode will set back the normal mode values of #frames and #triggers. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
        }

        if (args.size() == 2) {
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getPedestalMode(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            if (args[0] != "0") {
                throw RuntimeError(
                    "Unknown argument " + args[0] +
                    ". Did you mean 0 to disable pedestal mode?");
            }
            det->setPedestalMode(defs::pedestalParameters());
            os << ToString(args) << '\n';
        }

        if (args.size() == 2) {
            det->setPedestalMode(defs::pedestalParameters(
                StringTo<uint8_t>(args[0]), StringTo<uint16_t>(args[1])));
            os << ToString(args) << '\n';
        }
    }

    return os.str();
}

std::string Caller::period(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: period" << std::endl;
        os << R"V0G0N([duration] [(optional unit) ns|us|ms|s]
	Period between frames )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

        if (args.size() == 1) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                std::string tmp_time(args[0]);
                std::string unit = RemoveUnit(tmp_time);
                auto converted_time = StringTo<time::ns>(tmp_time, unit);
            } catch (...) {
                throw RuntimeError("Could not convert argument to time::ns");
            }
        }

        if (args.size() == 2) {
            try {
                StringTo<time::ns>(args[0], args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert arguments to time::ns");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getPeriod(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }

        if (args.size() == 1) {
            auto t = det->getPeriod(std::vector<int>{det_id});
            os << OutString(t, args[0]) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            std::string tmp_time(args[0]);
            std::string unit = RemoveUnit(tmp_time);
            auto converted_time = StringTo<time::ns>(tmp_time, unit);
            det->setPeriod(converted_time, std::vector<int>{det_id});
            os << args[0] << '\n';
        }

        if (args.size() == 2) {
            auto converted_time = StringTo<time::ns>(args[0], args[1]);
            det->setPeriod(converted_time, std::vector<int>{det_id});
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
        os << R"V0G0N(
	[Gotthard][Jungfrau][Moench][Ctb][Mythen3][Gotthard2][Xilinx Ctb] Period left for current frame. 
	[Gotthard2] only in continuous mode. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

        if (args.size() == 1) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getPeriodLeft(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }

        if (args.size() == 1) {
            auto t = det->getPeriodLeft(std::vector<int>{det_id});
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
        os << R"V0G0N([pos|neg]
	[Mythen3] Sets negative or positive polarity. Default is positive )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<defs::polarity>(args[0]);
            } catch (...) {
                throw RuntimeError(
                    "Could not convert argument 0 to defs::polarity");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getPolarity(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<defs::polarity>(args[0]);
            det->setPolarity(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([n]
	Port number of the control server on detector for detector-client tcp interface. Default is 1952. Normally unchanged. Set different ports for virtual servers on same pc. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<uint16_t>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to uint16_t");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getControlPort(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<uint16_t>(args[0]);
            det->setControlPort(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([0, 1]
	[Jungfrau][Moench][Mythen3][Gotthard2][Xilinx Ctb] Power the chip. 
	[Jungfrau][Moench] Default is 0. Get will return power status. Can be off if temperature event occured (temperature over temp_threshold with temp_control enabled. Will configure chip (only chip v1.1)
	[Mythen3][Gotthard2] Default is 1. If module not connected or wrong module, powerchip will fail.
	[Xilinx Ctb] Default is 0. Also configures the chip if powered on. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getPowerChip(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setPowerChip(arg0, std::vector<int>{det_id});
            os << args.front() << '\n';
        }
    }

    return os.str();
}

std::string Caller::powerindex(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: powerindex" << std::endl;
        os << R"V0G0N([name] 
		[Ctb][Xilinx_Ctb] Get the power index for the given name. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
            defs::dacIndex index = defs::V_POWER_A;
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            defs::dacIndex index = defs::V_POWER_A;
            if (det->getDetectorType().squash(defs::GENERIC) !=
                    defs::CHIPTESTBOARD &&
                det->getDetectorType().squash(defs::GENERIC) !=
                    defs::XILINX_CHIPTESTBOARD) {
                throw RuntimeError(cmd + " only allowed for CTB.");
            }
            if (det_id != -1) {
                throw RuntimeError("Cannot execute powerindex at module level");
            }
            auto t = det->getPowerIndex(args[0]);
            os << ToString(static_cast<int>(t) - index) << '\n';
        }
    }

    return os.str();
}

std::string Caller::powerlist(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: powerlist" << std::endl;
        os << R"V0G0N([powername1 powername2 .. powername4] 
		[Ctb][Xilinx_Ctb] Set the list of power names for this board. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
            if (cmd != "daclist" && det_type != defs::CHIPTESTBOARD &&
                det_type != defs::XILINX_CHIPTESTBOARD) {
                throw RuntimeError(cmd + " only allowed for CTB.");
            }
            if (det_id != -1) {
                throw RuntimeError("Cannot execute powerlist at module level");
            }
            auto t = det->getPowerNames();
            os << ToString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        auto det_type = det->getDetectorType().squash(defs::GENERIC);
        ;
        if (cmd != "daclist" && det_type != defs::CHIPTESTBOARD &&
            det_type != defs::XILINX_CHIPTESTBOARD) {
            throw RuntimeError(cmd + " only allowed for CTB.");
        }
        if (cmd == "daclist" && det_type != defs::CHIPTESTBOARD &&
            det_type != defs::XILINX_CHIPTESTBOARD) {
            throw RuntimeError("This detector already has fixed dac names. "
                               "Cannot change them.");
        }
        if (det_id != -1) {
            throw RuntimeError("Cannot execute powerlist at module level");
        }
        det->setPowerNames(args);
        os << ToString(args) << '\n';
    }

    return os.str();
}

std::string Caller::powername(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: powername" << std::endl;
        os << R"V0G0N([0-4][name] 
		[Ctb][Xilinx_Ctb] Set the power at the given position to the given name. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
            defs::dacIndex index = defs::V_POWER_A;
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            defs::dacIndex index = defs::V_POWER_A;
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            defs::dacIndex index = defs::V_POWER_A;
            if (det->getDetectorType().squash(defs::GENERIC) !=
                    defs::CHIPTESTBOARD &&
                det->getDetectorType().squash(defs::GENERIC) !=
                    defs::XILINX_CHIPTESTBOARD) {
                throw RuntimeError(cmd + " only allowed for CTB.");
            }
            if (det_id != -1) {
                throw RuntimeError("Cannot execute powername at module level");
            }
            auto t = det->getPowerName(
                static_cast<defs::dacIndex>(StringTo<int>(args[0]) + index));
            os << args[0] << ' ' << t << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            defs::dacIndex index = defs::V_POWER_A;
            if (det->getDetectorType().squash(defs::GENERIC) !=
                    defs::CHIPTESTBOARD &&
                det->getDetectorType().squash(defs::GENERIC) !=
                    defs::XILINX_CHIPTESTBOARD) {
                throw RuntimeError(cmd + " only allowed for CTB.");
            }
            if (det_id != -1) {
                throw RuntimeError("Cannot execute powername at module level");
            }
            det->setPowerName(
                static_cast<defs::dacIndex>(StringTo<int>(args[0]) + index),
                args[1]);
            os << ToString(args) << '\n';
        }
    }

    return os.str();
}

std::string Caller::powervalues(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: powervalues" << std::endl;
        os << R"V0G0N([name] 
		[Ctb][Xilinx_Ctb] Get values of all powers. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {

            std::string suffix = " mV";
            auto t = det->getPowerList();

            auto names = det->getPowerNames();
            auto name_it = names.begin();
            os << '[';
            if (t.size() > 0) {

                auto it = t.cbegin();
                os << ToString(*name_it++) << ' ';
                os << OutString(det->getPower(*it++, std::vector<int>{det_id}))
                   << suffix;
                while (it != t.cend()) {
                    os << ", " << ToString(*name_it++) << ' ';
                    os << OutString(
                              det->getPower(*it++, std::vector<int>{det_id}))
                       << suffix;
                }
            }

            os << "]\n";
        }
    }

    return os.str();
}

std::string Caller::programfpga(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: programfpga" << std::endl;
        os << R"V0G0N([fname.pof | fname.rbf (full path)][(opitonal)--force-delete-normal-file]
	[Jungfrau][Moench][Ctb] Programs FPGA from pof file (full path). Then, detector controller is rebooted. 
		Use --force-delete-normal-file argument, if normal file found in device tree, it must be deleted, a new device drive created and programming continued.
	[Mythen3][Gotthard2] Programs FPGA from rbf file (full path). Then, detector controller is rebooted. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>("0");
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to bool");
            }
        }

        if (args.size() == 2) {
            try {
                StringTo<bool>("1");
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to bool");
            }
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg1 = StringTo<bool>("0");
            det->programFPGA(args[0], arg1, std::vector<int>{det_id});
            os << "successful" << '\n';
        }

        if (args.size() == 2) {
            if (args[1] != "--force-delete-normal-file") {
                throw RuntimeError("Could not scan second argument. Did you "
                                   "mean --force-delete-normal-file?");
            }
            auto arg1 = StringTo<bool>("1");
            det->programFPGA(args[0], arg1, std::vector<int>{det_id});
            os << "successful" << '\n';
        }
    }

    return os.str();
}

std::string Caller::pulse(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: pulse" << std::endl;
        os << R"V0G0N([n_times] [x] [y]
	[Eiger] Pulse pixel n number of times at coordinates (x, y). Advanced User! )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 3) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 3) {
            defs::xy c =
                defs::xy(StringTo<int>(args[1]), StringTo<int>(args[2]));
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 3) {
            defs::xy c =
                defs::xy(StringTo<int>(args[1]), StringTo<int>(args[2]));
            auto arg0 = StringTo<int>(args[0]);
            det->pulsePixel(arg0, c, std::vector<int>{det_id});
            os << ToString(args) << '\n';
        }
    }

    return os.str();
}

std::string Caller::pulsechip(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: pulsechip" << std::endl;
        os << R"V0G0N([n_times] 
	[Eiger] Pulse chip n times. If n is -1, resets to normal mode (reset chip completely at start of acquisition, where partialreset = 0). Advanced User! )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->pulseChip(arg0, std::vector<int>{det_id});
            os << args.front() << '\n';
        }
    }

    return os.str();
}

std::string Caller::pulsenmove(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: pulsenmove" << std::endl;
        os << R"V0G0N([n_times] [x] [y]
	[Eiger] Pulse pixel n number of times and moves relatively by (x, y). Advanced User! )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 3) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 3) {
            defs::xy c =
                defs::xy(StringTo<int>(args[1]), StringTo<int>(args[2]));
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 3) {
            defs::xy c =
                defs::xy(StringTo<int>(args[1]), StringTo<int>(args[2]));
            auto arg0 = StringTo<int>(args[0]);
            det->pulsePixelNMove(arg0, c, std::vector<int>{det_id});
            os << ToString(args) << '\n';
        }
    }

    return os.str();
}

std::string Caller::pumpprobe(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: pumpprobe" << std::endl;
        os << R"V0G0N([0, 1]
	[Mythen3] Enables or disables pump probe mode. Default is disabled. Pump probe mode only enables vth2. Disabling sets back to previous value. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getPumpProbe(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setPumpProbe(arg0, std::vector<int>{det_id});
            os << args.front() << '\n';
        }
    }

    return os.str();
}

std::string Caller::quad(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: quad" << std::endl;
        os << R"V0G0N([0, 1]
	[Eiger] Sets detector size to a quad. 0 (disabled) is default. (Specific hardware required). )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getQuad(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            if (det_id != -1) {
                throw RuntimeError("Cannot execute quad at module level");
            }
            auto arg0 = StringTo<bool>(args[0]);
            det->setQuad(arg0);
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
        os << R"V0G0N(
	[1-256]
		[Eiger] Number of rows to readout per half module starting from the centre. Options: 0 - 256. 256 is default. The permissible values depend on dynamic range and 10Gbe enabled.
	[8-512 (multiple of 8)]
		[Jungfrau] Number of rows per module starting from the centre. Options: 8 - 512, must be multiples of 8. Default is 512.
		[Moench] Number of rows per module starting from the centre. Options:16 - 400, must be multiples of 16. Default is 400. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getReadNRows(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setReadNRows(arg0, std::vector<int>{det_id});
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
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

std::string Caller::readoutspeed(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: readoutspeed" << std::endl;
        os << R"V0G0N(
	[0 or full_speed|1 or half_speed|2 or quarter_speed]
	[Eiger][Jungfrau][Moench] Readout speed of chip.
	[Eiger][Moench] Default speed is full_speed.
	[Jungfrau] Default speed is half_speed. full_speed option only available from v2.0 boards and is recommended to set number of interfaces to 2. Also overwrites adcphase to recommended default.
	 [144|108]
		[Gotthard2] Readout speed of chip in MHz. Default is 108. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
            try {
                StringTo<defs::speedLevel>(args[0]);
            } catch (...) {
                throw RuntimeError(
                    "Could not convert argument 0 to defs::speedLevel");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
            if (det_type == defs::CHIPTESTBOARD ||
                det_type == defs::XILINX_CHIPTESTBOARD) {
                throw RuntimeError(
                    "ReadoutSpeed not implemented. Did you mean runclk?");
            }
            auto t = det->getReadoutSpeed(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
            if (det_type == defs::CHIPTESTBOARD ||
                det_type == defs::XILINX_CHIPTESTBOARD) {
                throw RuntimeError(
                    "ReadoutSpeed not implemented. Did you mean runclk?");
            }
            auto arg0 = StringTo<defs::speedLevel>(args[0]);
            det->setReadoutSpeed(arg0, std::vector<int>{det_id});
            os << ToString(StringTo<defs::speedLevel>(args[0])) << '\n';
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
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
	[Jungfrau][Moench][Ctb][Gotthard][Mythen3][Gotthard2][Xilinx Ctb] Reboot controller of detector. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 0) {
            det->rebootController(std::vector<int>{det_id});
            os << "successful" << '\n';
        }
    }

    return os.str();
}

std::string Caller::reg(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: reg" << std::endl;
        os << R"V0G0N([address] [32 bit value]
	[Mythen3][Gotthard2] Reads/writes to a 32 bit register in hex. Advanced Function!
	Goes to stop server. Hence, can be called while calling blocking acquire().
	[Eiger] +0x100 for only left, +0x200 for only right. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
            try {
                StringTo<uint32_t>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to uint32_t");
            }
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            try {
                StringTo<uint32_t>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to uint32_t");
            }
            try {
                StringTo<uint32_t>(args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to uint32_t");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<uint32_t>(args[0]);
            auto t = det->readRegister(arg0, std::vector<int>{det_id});
            os << OutStringHex(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            auto arg0 = StringTo<uint32_t>(args[0]);
            auto arg1 = StringTo<uint32_t>(args[1]);
            det->writeRegister(arg0, arg1, std::vector<int>{det_id});
            os << ToString(args) << '\n';
        }
    }

    return os.str();
}

std::string Caller::resetdacs(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: resetdacs" << std::endl;
        os << R"V0G0N([(optional) hard] 
	[Eiger][Jungfrau][Moench][Gotthard][Gotthard2][Mythen3]Reset dac values to the defaults. A 'hard' optional reset will reset the dacs to the hardcoded defaults in on-board detector server. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            if (args[0] != "hard") {
                throw RuntimeError("Unknown argument " + args[0] +
                                   ". Did you mean hard?");
            }
            det->resetToDefaultDacs("1", std::vector<int>{det_id});
            os << "successful" << '\n';
        }

        if (args.size() == 0) {
            det->resetToDefaultDacs("0", std::vector<int>{det_id});
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
	[Jungfrau][Moench][Ctb][Xilinx Ctb] Reset FPGA. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 0) {
            det->resetFPGA(std::vector<int>{det_id});
            os << "successful" << '\n';
        }
    }

    return os.str();
}

std::string Caller::roi(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: roi" << std::endl;
        os << R"V0G0N([xmin] [xmax] 
	[Gotthard] Region of interest in detector.
	Options: Only a single ROI per module. 
	Either all channels or a single adc or 2 chips (256 channels). Default is all channels enabled (-1 -1).  )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            defs::ROI t =
                defs::ROI(StringTo<int>(args[0]), StringTo<int>(args[1]));
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getROI(std::vector<int>{det_id});
            os << t << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            defs::ROI t =
                defs::ROI(StringTo<int>(args[0]), StringTo<int>(args[1]));
            if (det_id == -1 && det->size() > 1) {
                throw RuntimeError("Cannot execute ROI at multi module level");
            }
            det->setROI(t, det_id);
            os << t << '\n';
        }
    }

    return os.str();
}

std::string Caller::romode(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: romode" << std::endl;
        os << R"V0G0N([analog|digital|analog_digital|transceiver|digital_transceiver]
	[Ctb][xilinx Ctb] Readout mode. [Ctb] Default is analog. [Xilinx Ctb] Default is Transceiver (only one implemented so far) )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<defs::readoutMode>(args[0]);
            } catch (...) {
                throw RuntimeError(
                    "Could not convert argument 0 to defs::readoutMode");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getReadoutMode(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<defs::readoutMode>(args[0]);
            det->setReadoutMode(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([value]
	Set Detector row (udp header) to value. 
	Gui uses it to rearrange for complete image )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getRow(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setRow(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([n_clk in MHz]
	[Ctb] Run clock in MHz. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getRUNClock(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setRUNClock(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([(optional unit) ns|us|ms|s]
	[Jungfrau][Moench][Mythen3][Gotthard2][Ctb][Xilinx Ctb] Time from detector start up.
	[Gotthard2] not in burst and auto mode. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

        if (args.size() == 1) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getActualTime(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }

        if (args.size() == 1) {
            auto t = det->getActualTime(std::vector<int>{det_id});
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
        os << R"V0G0N([0, 1]
	Starts a thread in slsReceiver to arping the interface it is listening to every minute. Useful in 10G mode. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getRxArping(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setRxArping(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N(Resets Region of interest in receiver. Default is all channels/pixels enabled. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 0) {
            if (det_id != -1) {
                throw RuntimeError(
                    "Cannot execute rx_clearroi at module level");
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
        os << R"V0G0N([n_bytes]
	[Ctb] Offset in bytes in digital data to skip in receiver. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getRxDbitOffset(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setRxDbitOffset(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([nodiscard (default)|discardempty|discardpartial(fastest)]
	Frame discard policy of receiver. nodiscard does not discard frames, discardempty discards empty frames, discardpartial discards partial frames. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<defs::frameDiscardPolicy>(args[0]);
            } catch (...) {
                throw RuntimeError(
                    "Could not convert argument 0 to defs::frameDiscardPolicy");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getRxFrameDiscardPolicy(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<defs::frameDiscardPolicy>(args[0]);
            det->setRxFrameDiscardPolicy(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([n_frames]
	Set the number of frames in the receiver fifo depth (buffer between listener and writer threads). )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getRxFifoDepth(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setRxFifoDepth(arg0, std::vector<int>{det_id});
            os << args.front() << '\n';
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
	Current frame index received for each port in receiver during acquisition. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getRxCurrentFrameIndex(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    return os.str();
}

std::string Caller::rx_framescaught(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: rx_framescaught" << std::endl;
        os << R"V0G0N(
	Number of frames caught by each port in receiver. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getFramesCaught(std::vector<int>{det_id});
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
        os << R"V0G0N([n_frames]
	Number of frames per file in receiver in an acquisition. Default depends on detector type. 0 is infinite or all frames in single file. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getFramesPerFile(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setFramesPerFile(arg0, std::vector<int>{det_id});
            os << args.front() << '\n';
        }
    }

    return os.str();
}

std::string Caller::rx_jsonpara(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: rx_jsonpara" << std::endl;
        os << R"V0G0N([key1] [value1]
	[Receiver] Additional json header parameter streamed out from receiver. If not found in header, the pair is appended. An empty values deletes parameter. Max 20 characters for each key/value. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
        }

        if (args.size() == 2) {
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            auto t = det->getAdditionalJsonParameter(args[0],
                                                     std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            det->setAdditionalJsonParameter(args[0], "",
                                            std::vector<int>{det_id});
            os << args[0] << " deleted" << '\n';
        }

        if (args.size() == 2) {
            det->setAdditionalJsonParameter(args[0], args[1],
                                            std::vector<int>{det_id});
            os << "{" << args[0] << ": " << args[1] << "}" << '\n';
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getRxLastClientIP(std::vector<int>{det_id});
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
        os << R"V0G0N([0, 1]
	Lock receiver to one client IP, 1 locks, 0 unlocks. Default is unlocked. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getRxLock(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setRxLock(arg0, std::vector<int>{det_id});
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getNumMissingPackets(std::vector<int>{det_id});
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
        os << R"V0G0N([0, 1]
	Partial frames padding enable in the receiver. Default: enabled. Disabling is fastest. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getPartialFramesPadding(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setPartialFramesPadding(arg0, std::vector<int>{det_id});
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
	Prints the receiver configuration. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->printRxConfiguration(std::vector<int>{det_id});
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t =
                det->getRxRealUDPSocketBufferSize(std::vector<int>{det_id});
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
        os << R"V0G0N([0, 1]
	Switch on or off receiver text output during acquisition. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getRxSilentMode(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setRxSilentMode(arg0, std::vector<int>{det_id});
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
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

std::string Caller::rx_status(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: rx_status" << std::endl;
        os << R"V0G0N([running, idle, transmitting]
	Receiver listener status. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getReceiverStatus(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (true) {
            throw RuntimeError("Cannot put. Did you mean to use command: "
                               "\"rx_start\" or \"rx_stop\"?");
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
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
        os << R"V0G0N([port]
	TCP port for client-receiver communication. Default is 1954. Must be different if multiple receivers on same pc. Must be first command to set a receiver parameter. Multi command will automatically increment for individual modules. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<uint16_t>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to uint16_t");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getRxPort(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<uint16_t>(args[0]);
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getRxThreadIds(std::vector<int>{det_id});
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
        os << R"V0G0N([n_size]
	UDP socket buffer size in receiver. Tune rmem_default and rmem_max accordingly. Max value is INT_MAX/2. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getRxUDPSocketBufferSize(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setRxUDPSocketBufferSize(arg0, std::vector<int>{det_id});
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
	Receiver version )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getReceiverVersion(std::vector<int>{det_id});
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
        os << R"V0G0N([nth frame]
	Frequency of frames streamed out from receiver via zmq
	Default: 1, Means every frame is streamed out. 
	If 2, every second frame is streamed out. 
	If 0, streaming timer is the timeout, after which current frame is sent out. (default timeout is 500 ms). Usually used for gui purposes. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getRxZmqFrequency(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setRxZmqFrequency(arg0, std::vector<int>{det_id});
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getRxZmqHwm(std::vector<int>{det_id});
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

std::string Caller::rx_zmqport(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: rx_zmqport" << std::endl;
        os << R"V0G0N([port]
	Zmq port for data to be streamed out of the receiver. Also restarts receiver zmq streaming if enabled. Default is 30001. Modified only when using an intermediate process between receiver and client(gui). Must be different for every detector (and udp port). Multi command will automatically increment for individual modules. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<uint16_t>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to uint16_t");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getRxZmqPort(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<uint16_t>(args[0]);
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
        os << R"V0G0N([fnum]
	The starting frame index to stream out. 0 by default, which streams the first frame in an acquisition, and then depending on the rx zmq frequency/ timer )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getRxZmqStartingFrame(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setRxZmqStartingFrame(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([0, 1]
	Enable/ disable data streaming from receiver via zmq (eg. to GUI or to another process for further processing). This creates/ destroys zmq streamer threads in receiver. 
	Switching to Gui automatically enables data streaming in receiver. 
	Switching back to command line acquire will require disabling data streaming in receiver for fast applications.  )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getRxZmqDataStream(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setRxZmqDataStream(arg0, std::vector<int>{det_id});
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
	[Ctb][Mythen3][Xilinx Ctb] Saves pattern to file (ascii). 
	[Ctb] Also executes pattern. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            if (det_id != -1) {
                throw RuntimeError(
                    "Cannot execute savepattern at module level");
            }
            det->savePattern(args[0]);
            os << args.front() << '\n';
        }
    }

    return os.str();
}

std::string Caller::scan(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: scan" << std::endl;
        os << R"V0G0N([dac_name|0|trimbits] [start_val] [stop_val] [step_size] [dac settling time ns|us|ms|s]
	Enables/ disables scans for dac and trimbits 
	Enabling scan sets number of frames to number of steps in receiver. 
	To cancel scan configuration, set dac to '0', which also sets number of frames to 1. 
	[Eiger][Mythen3] Use trimbits as dac name for a trimbit scan. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1 && args.size() != 4 && args.size() != 5) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
        }

        if (args.size() == 4) {
        }

        if (args.size() == 5) {
            try {
                std::string tmp_time(args[4]);
                std::string unit = RemoveUnit(tmp_time);
                auto t = StringTo<time::ns>(tmp_time, unit);
            } catch (...) {
                throw RuntimeError("Could not convert argument to time::ns");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getScan(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            if (StringTo<int>(args[0]) != 0) {
                throw RuntimeError("Unknown argument " + args[0] +
                                   ". Did you mean 0 to disable scan?");
            }
            if (det_id != -1) {
                throw RuntimeError("Cannot execute scan at module level");
            }
            det->setScan(defs::scanParameters());
            os << ToString(args) << '\n';
        }

        if (args.size() == 4) {
            if (det_id != -1) {
                throw RuntimeError("Cannot execute scan at module level");
            }
            det->setScan(defs::scanParameters(
                StringTo<defs::dacIndex>(args[0]), StringTo<int>(args[1]),
                StringTo<int>(args[2]), StringTo<int>(args[3])));
            os << ToString(args) << '\n';
        }

        if (args.size() == 5) {
            std::string tmp_time(args[4]);
            std::string unit = RemoveUnit(tmp_time);
            auto t = StringTo<time::ns>(tmp_time, unit);
            if (det_id != -1) {
                throw RuntimeError("Cannot execute scan at module level");
            }
            det->setScan(defs::scanParameters(
                StringTo<defs::dacIndex>(args[0]), StringTo<int>(args[1]),
                StringTo<int>(args[2]), StringTo<int>(args[3]), t));
            os << ToString(args) << '\n';
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getScanErrorMessage(std::vector<int>{det_id});
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
        os << R"V0G0N([0, 1]
	[Jungfrau][Moench] The udp interface to stream data from detector. Effective only when number of interfaces is 1. Default: 0 (outer) )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getSelectedUDPInterface(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->selectUDPInterface(arg0, std::vector<int>{det_id});
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
	[Jungfrau][Moench][Gotthard][Mythen3][Gotthard2][Ctb]
Serial number of detector. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getSerialNumber(std::vector<int>{det_id});
            os << OutStringHex(t) << '\n';
        }
    }

    return os.str();
}

std::string Caller::setbit(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: setbit" << std::endl;
        os << R"V0G0N([reg address in hex] [bit index]
	Sets bit in address. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            try {
                StringTo<uint32_t>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to uint32_t");
            }
            try {
                StringTo<int>(args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            if (StringTo<int>(args[1]) < 0 || StringTo<int>(args[1]) > 31) {
                throw RuntimeError("Bit number out of range: " + args[1]);
            }
            auto arg0 = StringTo<uint32_t>(args[0]);
            auto arg1 = StringTo<int>(args[1]);
            det->setBit(arg0, arg1, std::vector<int>{det_id});
            os << ToString(args) << '\n';
        }
    }

    return os.str();
}

std::string Caller::settings(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: settings" << std::endl;
        os << R"V0G0N([standard, fast, highgain, dynamicgain, lowgain, mediumgain, veryhighgain, highgain0, fixgain1, fixgain2, forceswitchg1, forceswitchg2, verylowgain, g1_hg, g1_lg, g2_hc_hg, g2_hc_lg, g2_lc_hg, g2_lc_lg, g4_hg, g4_lg, gain0]
	 Detector Settings
	[Jungfrau] - [ gain0 | highgain0]
	[Gotthard] - [dynamicgain | highgain | lowgain | mediumgain | veryhighgain]
	[Gotthard] Also loads default dacs on to the detector.
	[Gotthard2] - [dynamicgain | fixgain1 | fixgain2]
	[Mythen3] - [standard | fast | highgain] Also changes vrshaper and vrpreamp. 
	[Eiger] Use threshold or thresholdnotb. 
	[Eiger] threshold and settings loaded from file found in settingspath. 
	[Moench] - [g1_hg | g1_lg | g2_hc_hg | g2_hc_lg | g2_lc_hg | g2_lc_lg | g4_hg | g4_lg] )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<defs::detectorSettings>(args[0]);
            } catch (...) {
                throw RuntimeError(
                    "Could not convert argument 0 to defs::detectorSettings");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getSettings(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<defs::detectorSettings>(args[0]);
            det->setSettings(arg0, std::vector<int>{det_id});
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
	List of settings implemented for this detector. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
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
        os << R"V0G0N([path]
	[Eiger][Mythen3] Directory where settings files are loaded from/to. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getSettingsPath(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            det->setSettingsPath(args[0], std::vector<int>{det_id});
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
		[Ctb][Xilinx_Ctb] Get the signal index for the given name. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            if (det->getDetectorType().squash(defs::GENERIC) !=
                    defs::CHIPTESTBOARD &&
                det->getDetectorType().squash(defs::GENERIC) !=
                    defs::XILINX_CHIPTESTBOARD) {
                throw RuntimeError(cmd + " only allowed for CTB.");
            }
            if (det_id != -1) {
                throw RuntimeError(
                    "Cannot execute signalindex at module level");
            }
            auto t = det->getSignalIndex(args[0]);
            os << static_cast<int>(t) << '\n';
        }
    }

    return os.str();
}

std::string Caller::signallist(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: signallist" << std::endl;
        os << R"V0G0N([signalname1 signalname2 .. signalname63] 
		[Ctb][Xilinx_Ctb] Set the list of signal names for this board. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
            if (cmd != "daclist" && det_type != defs::CHIPTESTBOARD &&
                det_type != defs::XILINX_CHIPTESTBOARD) {
                throw RuntimeError(cmd + " only allowed for CTB.");
            }
            if (det_id != -1) {
                throw RuntimeError("Cannot execute signallist at module level");
            }
            auto t = det->getSignalNames();
            os << ToString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        auto det_type = det->getDetectorType().squash(defs::GENERIC);
        ;
        if (cmd != "daclist" && det_type != defs::CHIPTESTBOARD &&
            det_type != defs::XILINX_CHIPTESTBOARD) {
            throw RuntimeError(cmd + " only allowed for CTB.");
        }
        if (cmd == "daclist" && det_type != defs::CHIPTESTBOARD &&
            det_type != defs::XILINX_CHIPTESTBOARD) {
            throw RuntimeError("This detector already has fixed dac names. "
                               "Cannot change them.");
        }
        if (det_id != -1) {
            throw RuntimeError("Cannot execute signallist at module level");
        }
        det->setSignalNames(args);
        os << ToString(args) << '\n';
    }

    return os.str();
}

std::string Caller::signalname(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: signalname" << std::endl;
        os << R"V0G0N([0-63][name] 
		[Ctb][Xilinx_Ctb] Set the signal at the given position to the given name. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            if (det->getDetectorType().squash(defs::GENERIC) !=
                    defs::CHIPTESTBOARD &&
                det->getDetectorType().squash(defs::GENERIC) !=
                    defs::XILINX_CHIPTESTBOARD) {
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
            if (det->getDetectorType().squash(defs::GENERIC) !=
                    defs::CHIPTESTBOARD &&
                det->getDetectorType().squash(defs::GENERIC) !=
                    defs::XILINX_CHIPTESTBOARD) {
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
		[Ctb][Xilinx_Ctb] Get the slowadc index for the given name. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
            defs::dacIndex index = defs::SLOW_ADC0;
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            defs::dacIndex index = defs::SLOW_ADC0;
            if (det->getDetectorType().squash(defs::GENERIC) !=
                    defs::CHIPTESTBOARD &&
                det->getDetectorType().squash(defs::GENERIC) !=
                    defs::XILINX_CHIPTESTBOARD) {
                throw RuntimeError(cmd + " only allowed for CTB.");
            }
            if (det_id != -1) {
                throw RuntimeError(
                    "Cannot execute slowadcindex at module level");
            }
            auto t = det->getSlowADCIndex(args[0]);
            os << ToString(static_cast<int>(t) - index) << '\n';
        }
    }

    return os.str();
}

std::string Caller::slowadclist(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: slowadclist" << std::endl;
        os << R"V0G0N([slowadcname1 slowadcname2 .. slowadcname7] 
		[Ctb][Xilinx_Ctb] Set the list of slowadc names for this board. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto det_type = det->getDetectorType().squash(defs::GENERIC);
            ;
            if (cmd != "daclist" && det_type != defs::CHIPTESTBOARD &&
                det_type != defs::XILINX_CHIPTESTBOARD) {
                throw RuntimeError(cmd + " only allowed for CTB.");
            }
            if (det_id != -1) {
                throw RuntimeError(
                    "Cannot execute slowadclist at module level");
            }
            auto t = det->getSlowADCNames();
            os << ToString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        auto det_type = det->getDetectorType().squash(defs::GENERIC);
        ;
        if (cmd != "daclist" && det_type != defs::CHIPTESTBOARD &&
            det_type != defs::XILINX_CHIPTESTBOARD) {
            throw RuntimeError(cmd + " only allowed for CTB.");
        }
        if (cmd == "daclist" && det_type != defs::CHIPTESTBOARD &&
            det_type != defs::XILINX_CHIPTESTBOARD) {
            throw RuntimeError("This detector already has fixed dac names. "
                               "Cannot change them.");
        }
        if (det_id != -1) {
            throw RuntimeError("Cannot execute slowadclist at module level");
        }
        det->setSlowADCNames(args);
        os << ToString(args) << '\n';
    }

    return os.str();
}

std::string Caller::slowadcname(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: slowadcname" << std::endl;
        os << R"V0G0N([0-7][name] 
		[Ctb][Xilinx_Ctb] Set the slowadc at the given position to the given name. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
            defs::dacIndex index = defs::SLOW_ADC0;
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            defs::dacIndex index = defs::SLOW_ADC0;
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            defs::dacIndex index = defs::SLOW_ADC0;
            if (det->getDetectorType().squash(defs::GENERIC) !=
                    defs::CHIPTESTBOARD &&
                det->getDetectorType().squash(defs::GENERIC) !=
                    defs::XILINX_CHIPTESTBOARD) {
                throw RuntimeError(cmd + " only allowed for CTB.");
            }
            if (det_id != -1) {
                throw RuntimeError(
                    "Cannot execute slowadcname at module level");
            }
            auto t = det->getSlowADCName(
                static_cast<defs::dacIndex>(StringTo<int>(args[0]) + index));
            os << args[0] << ' ' << t << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            defs::dacIndex index = defs::SLOW_ADC0;
            if (det->getDetectorType().squash(defs::GENERIC) !=
                    defs::CHIPTESTBOARD &&
                det->getDetectorType().squash(defs::GENERIC) !=
                    defs::XILINX_CHIPTESTBOARD) {
                throw RuntimeError(cmd + " only allowed for CTB.");
            }
            if (det_id != -1) {
                throw RuntimeError(
                    "Cannot execute slowadcname at module level");
            }
            det->setSlowADCName(
                static_cast<defs::dacIndex>(StringTo<int>(args[0]) + index),
                args[1]);
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
		[Ctb][Xilinx_Ctb] Get values of all slow adcs. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {

            std::string suffix = " uV";
            auto t = det->getSlowADCList();

            auto names = det->getSlowADCNames();
            auto name_it = names.begin();
            os << '[';
            if (t.size() > 0) {

                auto it = t.cbegin();
                os << ToString(*name_it++) << ' ';
                os << OutString(
                          det->getSlowADC(*it++, std::vector<int>{det_id}))
                   << suffix;
                while (it != t.cend()) {
                    os << ", " << ToString(*name_it++) << ' ';
                    os << OutString(
                              det->getSlowADC(*it++, std::vector<int>{det_id}))
                       << suffix;
                }
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 0) {
            det->startDetector(std::vector<int>{det_id});
            os << "successful" << '\n';
        }
    }

    return os.str();
}

std::string Caller::status(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: status" << std::endl;
        os << R"V0G0N([running, error, transmitting, finished, waiting, idle]
	Detector status. Goes to stop server. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getDetectorStatus(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (true) {
            throw RuntimeError("Cannot put. Did you mean to use command: "
                               "\"start\" or \"stop\"?");
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 0) {
            det->stopDetector(std::vector<int>{det_id});
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
        os << R"V0G0N([n]
	Port number of the stop server on detector for detector-client tcp interface. Default is 1953. Normally unchanged. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<uint16_t>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to uint16_t");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getStopPort(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<uint16_t>(args[0]);
            det->setStopPort(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([duration (0-1638375 ns)] [(optional unit) ns|us|ms|s]
	[Jungfrau] Additional time delay between 2 consecutive exposures in burst mode (resolution of 25ns). Only applicable for chipv1.0. For advanced users only. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

        if (args.size() == 1) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                std::string tmp_time(args[0]);
                std::string unit = RemoveUnit(tmp_time);
                auto converted_time = StringTo<time::ns>(tmp_time, unit);
            } catch (...) {
                throw RuntimeError("Could not convert argument to time::ns");
            }
        }

        if (args.size() == 2) {
            try {
                StringTo<time::ns>(args[0], args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert arguments to time::ns");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getStorageCellDelay(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }

        if (args.size() == 1) {
            auto t = det->getStorageCellDelay(std::vector<int>{det_id});
            os << OutString(t, args[0]) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            std::string tmp_time(args[0]);
            std::string unit = RemoveUnit(tmp_time);
            auto converted_time = StringTo<time::ns>(tmp_time, unit);
            det->setStorageCellDelay(converted_time, std::vector<int>{det_id});
            os << args[0] << '\n';
        }

        if (args.size() == 2) {
            auto converted_time = StringTo<time::ns>(args[0], args[1]);
            det->setStorageCellDelay(converted_time, std::vector<int>{det_id});
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
        os << R"V0G0N([0-max]
	[Jungfrau] Storage cell that stores the first acquisition of the series. max is 15 (default) for chipv1.0 and 3 (default) for chipv1.1. For advanced users only. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getStorageCellStart(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setStorageCellStart(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([duration] [(optional unit) ns|us|ms|s]
	[Eiger] Dead time of EIGER subframes in 32 bit mode. Subperiod = subexptime + subdeadtime. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

        if (args.size() == 1) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                std::string tmp_time(args[0]);
                std::string unit = RemoveUnit(tmp_time);
                auto converted_time = StringTo<time::ns>(tmp_time, unit);
            } catch (...) {
                throw RuntimeError("Could not convert argument to time::ns");
            }
        }

        if (args.size() == 2) {
            try {
                StringTo<time::ns>(args[0], args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert arguments to time::ns");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getSubDeadTime(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }

        if (args.size() == 1) {
            auto t = det->getSubDeadTime(std::vector<int>{det_id});
            os << OutString(t, args[0]) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            std::string tmp_time(args[0]);
            std::string unit = RemoveUnit(tmp_time);
            auto converted_time = StringTo<time::ns>(tmp_time, unit);
            det->setSubDeadTime(converted_time, std::vector<int>{det_id});
            os << args[0] << '\n';
        }

        if (args.size() == 2) {
            auto converted_time = StringTo<time::ns>(args[0], args[1]);
            det->setSubDeadTime(converted_time, std::vector<int>{det_id});
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
        os << R"V0G0N([duration] [(optional unit) ns|us|ms|s]
	[Eiger] Exposure time of EIGER subframes in 32 bit mode. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

        if (args.size() == 1) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                std::string tmp_time(args[0]);
                std::string unit = RemoveUnit(tmp_time);
                auto converted_time = StringTo<time::ns>(tmp_time, unit);
            } catch (...) {
                throw RuntimeError("Could not convert argument to time::ns");
            }
        }

        if (args.size() == 2) {
            try {
                StringTo<time::ns>(args[0], args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert arguments to time::ns");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getSubExptime(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }

        if (args.size() == 1) {
            auto t = det->getSubExptime(std::vector<int>{det_id});
            os << OutString(t, args[0]) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            std::string tmp_time(args[0]);
            std::string unit = RemoveUnit(tmp_time);
            auto converted_time = StringTo<time::ns>(tmp_time, unit);
            det->setSubExptime(converted_time, std::vector<int>{det_id});
            os << args[0] << '\n';
        }

        if (args.size() == 2) {
            auto converted_time = StringTo<time::ns>(args[0], args[1]);
            det->setSubExptime(converted_time, std::vector<int>{det_id});
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
	[Jungfrau][Moench] Enables or disables synchronization between modules. Sync mode requires at least one master configured. Also requires flatband cabling between master and slave with termination board. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getSynchronization(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            if (det_id != -1) {
                throw RuntimeError("Cannot execute sync at module level");
            }
            auto arg0 = StringTo<bool>(args[0]);
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
	[Ctb] Sync clock in MHz. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getSYNCClock(std::vector<int>{det_id});
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
	[Eiger]Temperature close to the 10GbE )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getTemperature(defs::TEMPERATURE_10GE,
                                         std::vector<int>{det_id});
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
	[Jungfrau][Moench][Gotthard] ADC Temperature )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getTemperature(defs::TEMPERATURE_ADC,
                                         std::vector<int>{det_id});
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
        os << R"V0G0N([0, 1]
	[Jungfrau][Moench] Temperature control enable. Default is 0 (disabled). If temperature crosses threshold temperature and temperature control is enabled, power to chip will be switched off and temperature event occurs. To power on chip again, temperature has to be less than threshold temperature and temperature event has to be cleared. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getTemperatureControl(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setTemperatureControl(arg0, std::vector<int>{det_id});
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
	[Eiger]Temperature close to the dc dc converter )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getTemperature(defs::TEMPERATURE_DCDC,
                                         std::vector<int>{det_id});
            os << OutString(t) << " °C" << '\n';
        }
    }

    return os.str();
}

std::string Caller::temp_event(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: temp_event" << std::endl;
        os << R"V0G0N([0]
	[Jungfrau][Moench] 1, if a temperature event occured. To clear this event, set it to 0.
	If temperature crosses threshold temperature and temperature control is enabled, power to chip will be switched off and temperature event occurs. To power on chip again, temperature has to be less than threshold temperature and temperature event has to be cleared. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getTemperatureEvent(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            if (StringTo<int>(args[0]) != 0) {
                throw RuntimeError("Unknown argument for temp event: ( " +
                                   args[0] +
                                   " ). Did you mean 0 to reset event?");
            }
            det->resetTemperatureEvent(std::vector<int>{det_id});
            os << "cleared" << '\n';
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
	[Eiger][Jungfrau][Moench][Gotthard][Mythen3][Gotthard2][Xilinx CTB] FPGA Temperature )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getTemperature(defs::TEMPERATURE_FPGA,
                                         std::vector<int>{det_id});
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
	[Eiger]Temperature close to the FPGA )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getTemperature(defs::TEMPERATURE_FPGAEXT,
                                         std::vector<int>{det_id});
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getTemperature(defs::TEMPERATURE_FPGA2,
                                         std::vector<int>{det_id});
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
	[Eiger]Temperature of the right front end board fpga. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getTemperature(defs::TEMPERATURE_FPGA3,
                                         std::vector<int>{det_id});
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
	[Ctb]Temperature of the slow adc )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getTemperature(defs::SLOW_ADC_TEMP,
                                         std::vector<int>{det_id});
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getTemperature(defs::TEMPERATURE_SODL,
                                         std::vector<int>{det_id});
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getTemperature(defs::TEMPERATURE_SODR,
                                         std::vector<int>{det_id});
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
        os << R"V0G0N([n_temp (in degrees)]
	[Jungfrau][Moench] Threshold temperature in degrees. If temperature crosses threshold temperature and temperature control is enabled, power to chip will be switched off and temperature event occurs. To power on chip again, temperature has to be less than threshold temperature and temperature event has to be cleared. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getThresholdTemperature(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setThresholdTemperature(arg0, std::vector<int>{det_id});
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getTemperatureList();
            os << ToString(t) << '\n';
        }
    }

    return os.str();
}

std::string Caller::tempvalues(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: tempvalues" << std::endl;
        os << R"V0G0N(
	Gets the values for every temperature for this detector. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {

            std::string suffix = " °C";
            auto t = det->getTemperatureList();
            os << '[';
            if (t.size() > 0) {

                auto it = t.cbegin();
                os << ToString(*it) << ' ';
                os << OutString(
                          det->getTemperature(*it++, std::vector<int>{det_id}))
                   << suffix;
                while (it != t.cend()) {
                    os << ", " << ToString(*it) << ' ';
                    os << OutString(det->getTemperature(
                              *it++, std::vector<int>{det_id}))
                       << suffix;
                }
            }

            os << "]\n";
        }
    }

    return os.str();
}

std::string Caller::tengiga(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: tengiga" << std::endl;
        os << R"V0G0N([0, 1]
	[Eiger][Ctb][Mythen3] 10GbE Enable. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getTenGiga(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setTenGiga(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([auto|trigger|gating|burst_trigger]
	Timing Mode of detector.
	[Jungfrau][Moench][Gotthard][Ctb][Gotthard2][Xilinx Ctb] [auto|trigger]
	[Mythen3] [auto|trigger|gating|trigger_gating]
	[Eiger] [auto|trigger|gating|burst_trigger] )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<defs::timingMode>(args[0]);
            } catch (...) {
                throw RuntimeError(
                    "Could not convert argument 0 to defs::timingMode");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getTimingMode(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<defs::timingMode>(args[0]);
            det->setTimingMode(arg0, std::vector<int>{det_id});
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
	Gets the list of timing modes for this detector. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
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
        os << R"V0G0N([internal|external]
	[Gotthard2] Timing source. Internal is crystal and external is system timing. Default is internal. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<defs::timingSourceType>(args[0]);
            } catch (...) {
                throw RuntimeError(
                    "Could not convert argument 0 to defs::timingSourceType");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getTimingSource(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<defs::timingSourceType>(args[0]);
            det->setTimingSource(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([0, 1]
	[Eiger] Sets half module to top (1), else bottom. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getTop(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setTop(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([bitmask]
	[Ctb][Xilinx Ctb] Transceiver Enable Mask. Enable for each 4 Transceiver channel. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<uint32_t>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to uint32_t");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getTransceiverEnableMask(std::vector<int>{det_id});
            os << OutStringHex(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<uint32_t>(args[0]);
            det->setTransceiverEnableMask(arg0, std::vector<int>{det_id});
            os << args.front() << '\n';
        }
    }

    return os.str();
}

std::string Caller::trigger(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: trigger" << std::endl;
        os << R"V0G0N(
	[Eiger][Mythen3][Jungfrau][Moench] Sends software trigger signal to detector )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 0) {
            bool block = false;
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 0) {
            bool block = false;
            if (det_id != -1) {
                throw RuntimeError("Cannot execute trigger at module level");
            }
            det->sendSoftwareTrigger(block);
            os << "successful" << '\n';
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int64_t>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int64_t");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getNumberOfTriggers(std::vector<int>{det_id});
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
	[Gotthard][Jungfrau][Moench][Mythen3][Gotthard2][Ctb][Xilinx Ctb] Number of triggers left in acquisition. Only when external trigger used. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getNumberOfTriggersLeft(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    return os.str();
}

std::string Caller::trimbits(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: trimbits" << std::endl;
        os << R"V0G0N([fname]
	[Eiger][Mythen3] Put will load the trimbit file to detector. If no extension specified, serial number of each module is attached. Get will save the trimbits from the detector to file with serial number added to file name. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            det->saveTrimbits(args[0], std::vector<int>{det_id});
            os << args[0] << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            det->loadTrimbits(args[0], std::vector<int>{det_id});
            os << args[0] << '\n';
        }
    }

    return os.str();
}

std::string Caller::trimval(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: trimval" << std::endl;
        os << R"V0G0N([n_trimval]
	[Eiger][Mythen3] All trimbits set to this value. Returns -1 if all trimbits are different values. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getAllTrimbits(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setAllTrimbits(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([n_value]
	[Ctb][Xilinx Ctb] Number of transceiver samples expected. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t =
                det->getNumberOfTransceiverSamples(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setNumberOfTransceiverSamples(arg0, std::vector<int>{det_id});
            os << args.front() << '\n';
        }
    }

    return os.str();
}

std::string Caller::txdelay(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: txdelay" << std::endl;
        os << R"V0G0N([n_delay]
	[Eiger][Jungfrau][Moench][Mythen3] Set transmission delay for all modules in the detector using the step size provided.Sets up 
		[Eiger] txdelay_left to (2 * mod_index * n_delay), 
		[Eiger] txdelay_right to ((2 * mod_index + 1) * n_delay) and 
		[Eiger] txdelay_frame to (2 *num_modules * n_delay)  
		[Jungfrau][Moench][Mythen3] txdelay_frame to (num_modules * n_delay)  
for every module. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            if (det_id != -1) {
                throw RuntimeError("Cannot execute txdelay at module level");
            }
            auto t = det->getTransmissionDelay();
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            if (det_id != -1) {
                throw RuntimeError("Cannot execute txdelay at module level");
            }
            auto arg0 = StringTo<int>(args[0]);
            det->setTransmissionDelay(arg0);
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
        os << R"V0G0N([n_delay]
	[Eiger][Jungfrau][Moench][Mythen3] Transmission delay of first udp packet being streamed out of the module.
	[Jungfrau][Moench] [0-31] Each value represents 1 ms
	[Eiger] Additional delay to txdelay_left and txdelay_right. Each value represents 10ns. Typical value is 50000.
	[Mythen3] [0-16777215] Each value represents 8 ns (125 MHz clock), max is 134 ms. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getTransmissionDelayFrame(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setTransmissionDelayFrame(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([n_delay]
	[Eiger] Transmission delay of first packet in an image being streamed out of the module's left UDP port. Each value represents 10ns. Typical value is 50000. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getTransmissionDelayLeft(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setTransmissionDelayLeft(arg0, std::vector<int>{det_id});
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
        os << R"V0G0N([n_delay]
	[Eiger] Transmission delay of first packet in an image being streamed out of the module's right UDP port. Each value represents 10ns. Typical value is 50000. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getTransmissionDelayRight(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setTransmissionDelayRight(arg0, std::vector<int>{det_id});
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
	Returns detector type. Can be Eiger, Jungfrau, Gotthard, Moench, Mythen3, Gotthard2, ChipTestBoard, Xilinx_ChipTestBoard )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getDetectorType(std::vector<int>{det_id});
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
	Clears udp destination details on the detector. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 0) {
            det->clearUDPDestinations(std::vector<int>{det_id});
            os << "successful" << '\n';
        }
    }

    return os.str();
}

std::string Caller::udp_dstlist(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: udp_dstlist" << std::endl;
        os << R"V0G0N([ip=x.x.x.x] [(optional)ip2=x.x.x.x] 
		[mac=xx:xx:xx:xx:xx:xx] [(optional)mac2=xx:xx:xx:xx:xx:xx]
		[port=value] [(optional)port2=value]
		The order of ip, mac and port does not matter. entry_value can be >0 only for [Eiger][Jungfrau][Moench][Mythen3][Gotthard2] where round robin is implemented. If 'auto' used, then ip is set to ip of rx_hostname. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            if (det_id == -1) {
                throw RuntimeError(
                    "Can execute udp_dstlist only at module level.");
            }
            if (rx_id < 0 || rx_id >= MAX_UDP_DESTINATION) {
                throw RuntimeError("Invalid receiver index " +
                                   std::to_string(rx_id) +
                                   " to set round robin entry.");
            }
            auto t =
                det->getDestinationUDPList(rx_id, std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (det_id == -1) {
            throw RuntimeError("Can execute udp_dstlist only at module level.");
        }
        if (rx_id < 0 || rx_id >= MAX_UDP_DESTINATION) {
            throw RuntimeError("Invalid receiver index " +
                               std::to_string(rx_id) +
                               " to set round robin entry.");
        }
        if (args.empty()) {
            throw RuntimeError("udp_dstlist require at least one argument.");
        }
        det->setDestinationUDPList(getUdpEntry(), det_id);
        os << ToString(args) << '\n';
    }

    return os.str();
}

std::string Caller::udp_dstmac(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: udp_dstmac" << std::endl;
        os << R"V0G0N([x:x:x:x:x:x]
	Mac address of the receiver (destination) udp interface. Not mandatory to set as udp_dstip retrieves it from slsReceiver process, but must be set if you use a custom receiver (not slsReceiver). Use router mac if router between detector and receiver. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getDestinationUDPMAC(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            det->setDestinationUDPMAC(MacAddr(args[0]),
                                      std::vector<int>{det_id});
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
        os << R"V0G0N([x:x:x:x:x:x]
	[Jungfrau][Moench] Mac address of the receiver (destination) udp interface 2. Not mandatory to set as udp_dstip2 retrieves it from slsReceiver process but must be set if you use a custom receiver (not slsReceiver). 
	 [Jungfrau][Moench] top half or inner interface 
	 [Gotthard2] veto debugging. Use router mac if router between detector and receiver. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getDestinationUDPMAC2(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            det->setDestinationUDPMAC2(MacAddr(args[0]),
                                       std::vector<int>{det_id});
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
        os << R"V0G0N([n]
	Port number of the receiver (destination) udp interface. Default is 50001. 
	If multi command, ports for each module is calculated (incremented by 1 if no 2nd interface) )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<uint16_t>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to uint16_t");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getDestinationUDPPort(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<uint16_t>(args[0]);
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
        os << R"V0G0N([n]
	[Jungfrau][Moench][Eiger][Gotthard2] Port number of the receiver (destination) udp interface 2. Default is 50002. 
	If multi command, ports for each module is calculated (incremented by 2) 
	[Jungfrau][Moench] top half or inner interface 
	[Eiger] right half 
	[Gotthard2] veto debugging )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<uint16_t>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to uint16_t");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getDestinationUDPPort2(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<uint16_t>(args[0]);
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
        os << R"V0G0N(
[0 - 31 (or number of udp destinations)]
	[Jungfrau][Moench][Gotthard2]
[0-63]
	[Mythen3]

	 One can set which is the first destination that the detector will stream images out from in a round robin fashion. The entry must not have been empty. Default: 0 )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getFirstUDPDestination(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setFirstUDPDestination(arg0, std::vector<int>{det_id});
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getNumberofUDPDestinations(std::vector<int>{det_id});
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 0) {
            det->reconfigureUDPDestination(std::vector<int>{det_id});
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
        os << R"V0G0N([x:x:x:x:x:x]
	Mac address of the detector (source) udp interface. 
	[Eiger] Do not set as detector will replace with its own DHCP Mac (1G) or DHCP Mac + 1 (10G). )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getSourceUDPMAC(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            det->setSourceUDPMAC(MacAddr(args[0]), std::vector<int>{det_id});
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
        os << R"V0G0N([x:x:x:x:x:x]
	[Jungfrau][Moench] Mac address of the top half or inner (source) udp interface.  )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getSourceUDPMAC2(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            det->setSourceUDPMAC2(MacAddr(args[0]), std::vector<int>{det_id});
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

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 0) {
            det->validateUDPConfiguration(std::vector<int>{det_id});
            os << "successful" << '\n';
        }
    }

    return os.str();
}

std::string Caller::update(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: update" << std::endl;
        os << R"V0G0N(
	Without tftp: [server_name (incl fullpath)] [fname.pof (incl full path)] This does not use tftp.
		[Jungfrau][Moench][Gotthard][Ctb] Updates the firmware, detector server, deletes old server, creates the symbolic link and then reboots detector controller. 
		[Mythen3][Gotthard2] will require a script to start up the shorter named server link at start up. 
		server_name is full path name of detector server binary
		fname is full path of programming file )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            if (args[args.size() - 1].find(".pof") == std::string::npos &&
                args[args.size() - 1].find(".rbf") == std::string::npos) {
                throw RuntimeError("Programming file must be a pof/rbf file.");
            }
            det->updateFirmwareAndServer(args[0], args[1],
                                         std::vector<int>{det_id});
            os << "successful" << '\n';
        }
    }

    return os.str();
}

std::string Caller::updatedetectorserver(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: updatedetectorserver" << std::endl;
        os << R"V0G0N([server_name  with full path]
	[Jungfrau][Moench][Eiger][Ctb][Mythen3][Gotthard2] Copies detector server via TCP (without tftp). Makes a symbolic link with a shorter name (without vx.x.x). Then, detector controller reboots (except Eiger).
	[Jungfrau][Moench][Ctb]Also changes respawn server to the link, which is effective after a reboot. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            det->updateDetectorServer(args[0], std::vector<int>{det_id});
            os << "successful" << '\n';
        }
    }

    return os.str();
}

std::string Caller::updatekernel(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: updatekernel" << std::endl;
        os << R"V0G0N([kernel_name with full path]
	[Jungfrau][Moench][Ctb][Mythen3][Gotthard2] Advanced Command!! You could damage the detector. Please use with caution.
	Updates the kernel image. Then, detector controller reboots with new kernel. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            det->updateKernel(args[0], std::vector<int>{det_id});
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
        os << R"V0G0N([0|1]
	Restart the detector server in update mode or not. This is useful when server-firmware compatibility is at its worst and server cannot start up normally )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getUpdateMode(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setUpdateMode(arg0, std::vector<int>{det_id});
            os << args.front() << '\n';
        }
    }

    return os.str();
}

std::string Caller::user(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: user" << std::endl;
        os << R"V0G0N(
	User details from shared memory (hostname, type, PID, User, Date). )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            if (det_id != -1) {
                throw RuntimeError("Cannot execute user at module level");
            }
            auto t = det->getUserDetails();
            os << t << '\n';
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
	[Ctb][Xilinx Ctb] Power supply a in mV. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getPower(defs::V_POWER_A, std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg1 = StringTo<int>(args[0]);
            det->setPower(defs::V_POWER_A, arg1, std::vector<int>{det_id});
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
	[Ctb][Xilinx Ctb] Power supply b in mV. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getPower(defs::V_POWER_B, std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg1 = StringTo<int>(args[0]);
            det->setPower(defs::V_POWER_B, arg1, std::vector<int>{det_id});
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
	[Ctb][Xilinx Ctb] Power supply c in mV. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getPower(defs::V_POWER_C, std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg1 = StringTo<int>(args[0]);
            det->setPower(defs::V_POWER_C, arg1, std::vector<int>{det_id});
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
	[Ctb] Power supply chip in mV. Do not use it unless you are completely sure you will not fry the board. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t =
                det->getPower(defs::V_POWER_CHIP, std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg1 = StringTo<int>(args[0]);
            det->setPower(defs::V_POWER_CHIP, arg1, std::vector<int>{det_id});
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
	[Ctb][Xilinx Ctb] Power supply d in mV. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getPower(defs::V_POWER_D, std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg1 = StringTo<int>(args[0]);
            det->setPower(defs::V_POWER_D, arg1, std::vector<int>{det_id});
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
	[Ctb][Xilinx Ctb] Power supply io in mV. Minimum 1200 mV. Must be the first power regulator to be set after fpga reset (on-board detector server start up). )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getPower(defs::V_POWER_IO, std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg1 = StringTo<int>(args[0]);
            det->setPower(defs::V_POWER_IO, arg1, std::vector<int>{det_id});
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
	[Ctb][Xilinx Ctb] Soft limit for power supplies (ctb only) and DACS in mV. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getPower(defs::V_LIMIT, std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg1 = StringTo<int>(args[0]);
            det->setPower(defs::V_LIMIT, arg1, std::vector<int>{det_id});
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
        os << R"V0G0N([chip index 0-9, -1 for all][10 bit hex value] 
	[Gotthard2] On chip Dac for comparator current of ADC. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
            try {
                StringTo<int>(args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 2 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            auto arg1 = StringTo<int>(args[0]);
            auto t = det->getOnChipDAC(defs::VB_COMP_ADC, arg1,
                                       std::vector<int>{det_id});
            os << args[0] << ' ' << OutStringHex(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            auto arg1 = StringTo<int>(args[0]);
            auto arg2 = StringTo<int>(args[1]);
            det->setOnChipDAC(defs::VB_COMP_ADC, arg1, arg2,
                              std::vector<int>{det_id});
            os << args[0] << ' ' << args[1] << '\n';
        }
    }

    return os.str();
}

std::string Caller::vchip_comp_fe(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: vchip_comp_fe" << std::endl;
        os << R"V0G0N([chip index 0-9, -1 for all][10 bit hex value] 
	[Gotthard2] On chip Dac for comparator current of analogue front end. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
            try {
                StringTo<int>(args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 2 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            auto arg1 = StringTo<int>(args[0]);
            auto t = det->getOnChipDAC(defs::VB_COMP_FE, arg1,
                                       std::vector<int>{det_id});
            os << args[0] << ' ' << OutStringHex(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            auto arg1 = StringTo<int>(args[0]);
            auto arg2 = StringTo<int>(args[1]);
            det->setOnChipDAC(defs::VB_COMP_FE, arg1, arg2,
                              std::vector<int>{det_id});
            os << args[0] << ' ' << args[1] << '\n';
        }
    }

    return os.str();
}

std::string Caller::vchip_cs(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: vchip_cs" << std::endl;
        os << R"V0G0N([chip index 0-9, -1 for all][10 bit hex value] 
	[Gotthard2] On chip Dac for current injection into preamplifier. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
            try {
                StringTo<int>(args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 2 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            auto arg1 = StringTo<int>(args[0]);
            auto t =
                det->getOnChipDAC(defs::VB_CS, arg1, std::vector<int>{det_id});
            os << args[0] << ' ' << OutStringHex(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            auto arg1 = StringTo<int>(args[0]);
            auto arg2 = StringTo<int>(args[1]);
            det->setOnChipDAC(defs::VB_CS, arg1, arg2,
                              std::vector<int>{det_id});
            os << args[0] << ' ' << args[1] << '\n';
        }
    }

    return os.str();
}

std::string Caller::vchip_opa_1st(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: vchip_opa_1st" << std::endl;
        os << R"V0G0N([chip index 0-9, -1 for all][10 bit hex value] 
	[Gotthard2] On chip Dac for opa current for driving the other DACs in chip. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
            try {
                StringTo<int>(args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 2 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            auto arg1 = StringTo<int>(args[0]);
            auto t = det->getOnChipDAC(defs::VB_OPA_1ST, arg1,
                                       std::vector<int>{det_id});
            os << args[0] << ' ' << OutStringHex(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            auto arg1 = StringTo<int>(args[0]);
            auto arg2 = StringTo<int>(args[1]);
            det->setOnChipDAC(defs::VB_OPA_1ST, arg1, arg2,
                              std::vector<int>{det_id});
            os << args[0] << ' ' << args[1] << '\n';
        }
    }

    return os.str();
}

std::string Caller::vchip_opa_fd(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: vchip_opa_fd" << std::endl;
        os << R"V0G0N([chip index 0-9, -1 for all][10 bit hex value] 
	[Gotthard2] On chip Dac current for CDS opa stage. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
            try {
                StringTo<int>(args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 2 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            auto arg1 = StringTo<int>(args[0]);
            auto t = det->getOnChipDAC(defs::VB_OPA_FD, arg1,
                                       std::vector<int>{det_id});
            os << args[0] << ' ' << OutStringHex(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            auto arg1 = StringTo<int>(args[0]);
            auto arg2 = StringTo<int>(args[1]);
            det->setOnChipDAC(defs::VB_OPA_FD, arg1, arg2,
                              std::vector<int>{det_id});
            os << args[0] << ' ' << args[1] << '\n';
        }
    }

    return os.str();
}

std::string Caller::vchip_ref_comp_fe(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: vchip_ref_comp_fe" << std::endl;
        os << R"V0G0N([chip index 0-9, -1 for all][10 bit hex value] 
	[Gotthard2] On chip Dac for reference voltage of the comparator of analogue front end. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
            try {
                StringTo<int>(args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 2 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            auto arg1 = StringTo<int>(args[0]);
            auto t = det->getOnChipDAC(defs::VREF_COMP_FE, arg1,
                                       std::vector<int>{det_id});
            os << args[0] << ' ' << OutStringHex(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            auto arg1 = StringTo<int>(args[0]);
            auto arg2 = StringTo<int>(args[1]);
            det->setOnChipDAC(defs::VREF_COMP_FE, arg1, arg2,
                              std::vector<int>{det_id});
            os << args[0] << ' ' << args[1] << '\n';
        }
    }

    return os.str();
}

std::string Caller::veto(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: veto" << std::endl;
        os << R"V0G0N([0, 1]
	[Gotthard2] Enable or disable veto data data from chip. Default is 0. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<bool>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to bool");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getVeto(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<bool>(args[0]);
            det->setVeto(arg0, std::vector<int>{det_id});
            os << args.front() << '\n';
        }
    }

    return os.str();
}

std::string Caller::vetoalg(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: vetoalg" << std::endl;
        os << R"V0G0N([hits|raw] [lll|10gbe]
	[Gotthard2] Set the veto algorithm. Default is hits. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 1) {
            defs::streamingInterface interface =
                StringTo<defs::streamingInterface>(args[0]);
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            defs::vetoAlgorithm alg = StringTo<defs::vetoAlgorithm>(args[0]);
            defs::streamingInterface interface =
                StringTo<defs::streamingInterface>(args[1]);
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 1) {
            defs::streamingInterface interface =
                StringTo<defs::streamingInterface>(args[0]);
            if (interface == defs::streamingInterface::NONE) {
                throw RuntimeError(
                    "Must specify an interface to set algorithm");
            }
            auto t = det->getVetoAlgorithm(interface, std::vector<int>{det_id});
            os << OutString(t) << ' ' << ToString(interface) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            defs::vetoAlgorithm alg = StringTo<defs::vetoAlgorithm>(args[0]);
            defs::streamingInterface interface =
                StringTo<defs::streamingInterface>(args[1]);
            if (interface == defs::streamingInterface::NONE) {
                throw RuntimeError(
                    "Must specify an interface to set algorithm");
            }
            det->setVetoAlgorithm(alg, interface, std::vector<int>{det_id});
            os << ToString(alg) << ' ' << ToString(interface) << '\n';
        }
    }

    return os.str();
}

std::string Caller::vetofile(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: vetofile" << std::endl;
        os << R"V0G0N([chip index 0-9, -1 for all] [file name] 
	[Gotthard2] Set veto reference for each 128 channels for specific chip. The file should have 128 rows of gain index and 12 bit value in dec )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (true) {
            throw RuntimeError("Cannot get vetofile. Did you mean vetophoton?");
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            auto arg0 = StringTo<int>(args[0]);
            det->setVetoFile(arg0, args[1], std::vector<int>{det_id});
            os << ToString(args) << '\n';
        }
    }

    return os.str();
}

std::string Caller::vetophoton(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: vetophoton" << std::endl;
        os << R"V0G0N([ichip] [#photons] [energy in keV] [reference file]
	[Gotthard2] Set veto reference for 128 channels for chip ichip according to reference file and #photons and energy in keV.
[ichip] [output file]
	 Get gain indices and veto reference for 128 channels for chip ichip, saved to file. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 2) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 4) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 4) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
            try {
                StringTo<int>(args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
            try {
                StringTo<int>(args[2]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 2 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 2) {
            auto arg0 = StringTo<int>(args[0]);
            det->getVetoPhoton(arg0, args[1], std::vector<int>{det_id});
            os << "saved to file " << args[1] << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 4) {
            auto arg0 = StringTo<int>(args[0]);
            auto arg1 = StringTo<int>(args[1]);
            auto arg2 = StringTo<int>(args[2]);
            det->setVetoPhoton(arg0, arg1, arg2, args[3],
                               std::vector<int>{det_id});
            os << ToString(args) << '\n';
        }
    }

    return os.str();
}

std::string Caller::vetoref(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: vetoref" << std::endl;
        os << R"V0G0N([gain index] [12 bit value]
	[Gotthard2] Set veto reference for all 128 channels for all chips. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
            try {
                StringTo<int>(args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (true) {
            throw RuntimeError("Cannot get vetoref. Did you mean vetophoton?");
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            auto arg0 = StringTo<int>(args[0]);
            auto arg1 = StringTo<int>(args[1]);
            det->setVetoReference(arg0, arg1);
            os << ToString(args) << '\n';
        }
    }

    return os.str();
}

std::string Caller::virtualFunction(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: virtual" << std::endl;
        os << R"V0G0N([n_servers] [starting_port_number]
	Connecs to n virtual server at local host starting at specific control port. Every virtual server will have a stop port (control port + 1) )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 2) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 2) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
            try {
                StringTo<uint16_t>(args[1]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 1 to uint16_t");
            }
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 2) {
            if (det_id != -1) {
                throw RuntimeError("Cannot execute virtual at module level");
            }
            auto arg0 = StringTo<int>(args[0]);
            auto arg1 = StringTo<uint16_t>(args[1]);
            det->setVirtualDetectorServers(arg0, arg1);
            os << ToString(args) << '\n';
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
	[Ctb] Measured voltage of power supply a in mV. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getMeasuredPower(defs::V_POWER_A,
                                           std::vector<int>{det_id});
            os << OutString(t) << '\n';
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
	[Ctb] Measured voltage of power supply b in mV. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getMeasuredPower(defs::V_POWER_B,
                                           std::vector<int>{det_id});
            os << OutString(t) << '\n';
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
	[Ctb] Measured voltage of power supply c in mV. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getMeasuredPower(defs::V_POWER_C,
                                           std::vector<int>{det_id});
            os << OutString(t) << '\n';
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
	[Ctb] Measured voltage of power supply d in mV. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getMeasuredPower(defs::V_POWER_D,
                                           std::vector<int>{det_id});
            os << OutString(t) << '\n';
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
	[Ctb] Measured voltage of power supply io in mV. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else {

        throw RuntimeError(
            "INTERNAL ERROR: Invalid action: supported actions are ['GET']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getMeasuredPower(defs::V_POWER_IO,
                                           std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    return os.str();
}

std::string Caller::zmqhwm(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: zmqhwm" << std::endl;
        os << R"V0G0N([n_limit] 
	Client's zmq receive high water mark. Default is the zmq library's default (1000), can also be set here using -1. 
	This is a high number and can be set to 2 for gui purposes. 
	One must also set the receiver's send high water mark to similar value. Final effect is sum of them.
	 Setting it via command line is useful only before zmq enabled (before opening gui). )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<int>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to int");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getClientZmqHwm();
            os << t << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<int>(args[0]);
            det->setClientZmqHwm(arg0);
            os << det->getClientZmqHwm() << '\n';
        }
    }

    return os.str();
}

std::string Caller::zmqip(int action) {

    std::ostringstream os;
    // print help
    if (action == slsDetectorDefs::HELP_ACTION) {
        os << "Command: zmqip" << std::endl;
        os << R"V0G0N([x.x.x.x]
	Ip Address to listen to zmq data streamed out from receiver or intermediate process. Default connects to receiver zmq Ip Address (from rx_hostname). Modified only when using an intermediate process between receiver and client(gui). Also restarts client zmq streaming if enabled. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getClientZmqIp(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            det->setClientZmqIp(IpAddr(args[0]), std::vector<int>{det_id});
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
        os << R"V0G0N([port]
	Zmq port in client(gui) or intermediate process for data to be streamed to from receiver. Default connects to receiver zmq streaming out port (30001). Modified only when using an intermediate process between receiver and client(gui). Also restarts client zmq streaming if enabled. Must be different for every detector (and udp port). Multi command will automatically increment for individual modules. )V0G0N"
           << std::endl;
        return os.str();
    }

    // check if action and arguments are valid
    if (action == slsDetectorDefs::GET_ACTION) {
        if (1 && args.size() != 0) {
            throw RuntimeError("Wrong number of arguments for action GET");
        }

        if (args.size() == 0) {
        }

    }

    else if (action == slsDetectorDefs::PUT_ACTION) {
        if (1 && args.size() != 1) {
            throw RuntimeError("Wrong number of arguments for action PUT");
        }

        if (args.size() == 1) {
            try {
                StringTo<uint16_t>(args[0]);
            } catch (...) {
                throw RuntimeError("Could not convert argument 0 to uint16_t");
            }
        }

    }

    else {

        throw RuntimeError("INTERNAL ERROR: Invalid action: supported actions "
                           "are ['GET', 'PUT']");
    }

    // generate code for each action
    if (action == slsDetectorDefs::GET_ACTION) {
        if (args.size() == 0) {
            auto t = det->getClientZmqPort(std::vector<int>{det_id});
            os << OutString(t) << '\n';
        }
    }

    if (action == slsDetectorDefs::PUT_ACTION) {
        if (args.size() == 1) {
            auto arg0 = StringTo<uint16_t>(args[0]);
            det->setClientZmqPort(arg0, det_id);
            os << args.front() << '\n';
        }
    }

    return os.str();
}

} // namespace sls