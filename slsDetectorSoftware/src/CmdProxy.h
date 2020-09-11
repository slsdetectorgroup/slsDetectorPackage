#pragma once

#include "Detector.h"
#include "Result.h"
#include "network_utils.h"
#include "sls_detector_exceptions.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>

/** Macro to make an integer command.
 * CMDNAME name of the function that does the command
 * GETFCN Detector function to get
 * SETFCN Detector function to set
 * CONV Function to convert from string to the correct integer type
 * HLPSTR Help string for --help and docs
 */

#define TIME_COMMAND(CMDNAME, GETFCN, SETFCN, HLPSTR)                          \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            auto t = det->GETFCN({det_id});                                    \
            if (args.empty()) {                                                \
                os << OutString(t) << '\n';                                    \
            } else if (args.size() == 1) {                                     \
                os << OutString(t, args[0]) << '\n';                           \
            } else {                                                           \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (args.size() == 1) {                                            \
                std::string time_str(args[0]);                                 \
                std::string unit = RemoveUnit(time_str);                       \
                auto t = StringTo<time::ns>(time_str, unit);                   \
                det->SETFCN(t, {det_id});                                      \
            } else if (args.size() == 2) {                                     \
                auto t = StringTo<time::ns>(args[0], args[1]);                 \
                det->SETFCN(t, {det_id});                                      \
            } else {                                                           \
                WrongNumberOfParameters(2);                                    \
            }                                                                  \
            /* TODO: os << args << '\n'; (doesnt work for vectors in .h)*/     \
            if (args.size() > 1) {                                             \
                os << args[0] << args[1] << '\n';                              \
            } else {                                                           \
                os << args[0] << '\n';                                         \
            }                                                                  \
        } else {                                                               \
            throw sls::RuntimeError("Unknown action");                         \
        }                                                                      \
        return os.str();                                                       \
    }

/** time get only */
#define TIME_GET_COMMAND(CMDNAME, GETFCN, HLPSTR)                              \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            auto t = det->GETFCN({det_id});                                    \
            if (args.empty()) {                                                \
                os << OutString(t) << '\n';                                    \
            } else if (args.size() == 1) {                                     \
                os << OutString(t, args[0]) << '\n';                           \
            } else {                                                           \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            throw sls::RuntimeError("cannot put");                             \
        } else {                                                               \
            throw sls::RuntimeError("Unknown action");                         \
        }                                                                      \
        return os.str();                                                       \
    }

/** string  */
#define STRING_COMMAND(CMDNAME, GETFCN, SETFCN, HLPSTR)                        \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            if (!args.empty()) {                                               \
                WrongNumberOfParameters(0);                                    \
            }                                                                  \
            auto t = det->GETFCN({det_id});                                    \
            os << OutString(t) << '\n';                                        \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (args.size() != 1) {                                            \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
            det->SETFCN(args[0], {det_id});                                    \
            os << args.front() << '\n';                                        \
        } else {                                                               \
            throw sls::RuntimeError("Unknown action");                         \
        }                                                                      \
        return os.str();                                                       \
    }

/** int or enum hex with 16 bit width (64 bit)*/
#define INTEGER_COMMAND_HEX_WIDTH16(CMDNAME, GETFCN, SETFCN, CONV, HLPSTR)     \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            if (!args.empty()) {                                               \
                WrongNumberOfParameters(0);                                    \
            }                                                                  \
            auto t = det->GETFCN({det_id});                                    \
            os << OutStringHex(t, 16) << '\n';                                 \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (args.size() != 1) {                                            \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
            auto val = CONV(args[0]);                                          \
            det->SETFCN(val, {det_id});                                        \
            os << ToStringHex(val, 16) << '\n';                                \
        } else {                                                               \
            throw sls::RuntimeError("Unknown action");                         \
        }                                                                      \
        return os.str();                                                       \
    }

/** int or enum hex */
#define INTEGER_COMMAND_HEX(CMDNAME, GETFCN, SETFCN, CONV, HLPSTR)             \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            if (!args.empty()) {                                               \
                WrongNumberOfParameters(0);                                    \
            }                                                                  \
            auto t = det->GETFCN({det_id});                                    \
            os << OutStringHex(t) << '\n';                                     \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (args.size() != 1) {                                            \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
            auto val = CONV(args[0]);                                          \
            det->SETFCN(val, {det_id});                                        \
            os << args.front() << '\n';                                        \
        } else {                                                               \
            throw sls::RuntimeError("Unknown action");                         \
        }                                                                      \
        return os.str();                                                       \
    }

/** int or enum */
#define INTEGER_COMMAND(CMDNAME, GETFCN, SETFCN, CONV, HLPSTR)                 \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            if (!args.empty()) {                                               \
                WrongNumberOfParameters(0);                                    \
            }                                                                  \
            auto t = det->GETFCN({det_id});                                    \
            os << OutString(t) << '\n';                                        \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (args.size() != 1) {                                            \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
            auto val = CONV(args[0]);                                          \
            det->SETFCN(val, {det_id});                                        \
            os << args.front() << '\n';                                        \
        } else {                                                               \
            throw sls::RuntimeError("Unknown action");                         \
        }                                                                      \
        return os.str();                                                       \
    }

/** int, no id */
#define INTEGER_COMMAND_NOID(CMDNAME, GETFCN, SETFCN, CONV, HLPSTR)            \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (det_id != -1) {                                                    \
            throw sls::RuntimeError("Cannot execute this at module level");    \
        }                                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            if (!args.empty()) {                                               \
                WrongNumberOfParameters(0);                                    \
            }                                                                  \
            auto t = det->GETFCN();                                            \
            os << OutString(t) << '\n';                                        \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (args.size() != 1) {                                            \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
            auto val = CONV(args[0]);                                          \
            det->SETFCN(val);                                                  \
            os << args.front() << '\n';                                        \
        } else {                                                               \
            throw sls::RuntimeError("Unknown action");                         \
        }                                                                      \
        return os.str();                                                       \
    }

/** int with index */
#define INTEGER_IND_COMMAND(CMDNAME, GETFCN, SETFCN, CONV, INDEX, HLPSTR)      \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            if (!args.empty()) {                                               \
                WrongNumberOfParameters(0);                                    \
            }                                                                  \
            auto t = det->GETFCN(INDEX, {det_id});                             \
            os << OutString(t) << '\n';                                        \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (args.size() != 1) {                                            \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
            auto val = CONV(args[0]);                                          \
            det->SETFCN(INDEX, val, {det_id});                                 \
            os << args.front() << '\n';                                        \
        } else {                                                               \
            throw sls::RuntimeError("Unknown action");                         \
        }                                                                      \
        return os.str();                                                       \
    }

/** int with user index */
#define INTEGER_USER_IND_COMMAND(CMDNAME, GETFCN, SETFCN, CONV, INDEX, HLPSTR) \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            if (args.size() != 1) {                                            \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
            auto t = det->GETFCN(INDEX, StringTo<int>(args[0]), {det_id});     \
            os << args[0] << ' ' << OutStringHex(t) << '\n';                   \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (args.size() != 2) {                                            \
                WrongNumberOfParameters(2);                                    \
            }                                                                  \
            auto val = CONV(args[1]);                                          \
            det->SETFCN(INDEX, StringTo<int>(args[0]), val, {det_id});         \
            os << args[0] << ' ' << args[1] << '\n';                           \
        } else {                                                               \
            throw sls::RuntimeError("Unknown action");                         \
        }                                                                      \
        return os.str();                                                       \
    }

/** dac */
#define DAC_COMMAND(CMDNAME, GETFCN, SETFCN, DAC_INDEX, HLPSTR)                \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            bool mv = false;                                                   \
            if (args.size() == 1) {                                            \
                if ((args[0] != "mv") && (args[0] != "mV")) {                  \
                    throw sls::RuntimeError("Unknown argument " + args[0] +    \
                                            ". Did you mean mV?");             \
                }                                                              \
                mv = true;                                                     \
            } else if (args.size() > 1) {                                      \
                WrongNumberOfParameters(0);                                    \
            }                                                                  \
            auto t = det->GETFCN(DAC_INDEX, mv, {det_id});                     \
            os << OutString(t) << (!args.empty() ? " mV\n" : "\n");            \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            bool mv = false;                                                   \
            if (args.size() == 2) {                                            \
                if ((args[1] != "mv") && (args[1] != "mV")) {                  \
                    throw sls::RuntimeError("Unknown argument " + args[1] +    \
                                            ". Did you mean mV?");             \
                }                                                              \
                mv = true;                                                     \
            } else if (args.size() > 2 || args.empty()) {                      \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
            det->SETFCN(DAC_INDEX, StringTo<int>(args[0]), mv, {det_id});      \
            os << args.front() << (args.size() > 1 ? " mV\n" : "\n");          \
        } else {                                                               \
            throw sls::RuntimeError("Unknown action");                         \
        }                                                                      \
        return os.str();                                                       \
    }

/** set only, no arguments, no id */
#define EXECUTE_SET_COMMAND_NOID(CMDNAME, SETFCN, HLPSTR)                      \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (det_id != -1) {                                                    \
            throw sls::RuntimeError("Cannot execute this at module level");    \
        }                                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            throw sls::RuntimeError("Cannot get");                             \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (!args.empty()) {                                               \
                WrongNumberOfParameters(0);                                    \
            }                                                                  \
            det->SETFCN();                                                     \
            os << "successful\n";                                              \
        } else {                                                               \
            throw sls::RuntimeError("Unknown action");                         \
        }                                                                      \
        return os.str();                                                       \
    }

/** set only, no arguments */
#define EXECUTE_SET_COMMAND(CMDNAME, SETFCN, HLPSTR)                           \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            throw sls::RuntimeError("Cannot get");                             \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (!args.empty()) {                                               \
                WrongNumberOfParameters(0);                                    \
            }                                                                  \
            det->SETFCN({det_id});                                             \
            os << "successful\n";                                              \
        } else {                                                               \
            throw sls::RuntimeError("Unknown action");                         \
        }                                                                      \
        return os.str();                                                       \
    }

/** set only, 1 argument, no id */
#define EXECUTE_SET_COMMAND_NOID_1ARG(CMDNAME, SETFCN, HLPSTR)                 \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (det_id != -1) {                                                    \
            throw sls::RuntimeError("Cannot execute this at module level");    \
        }                                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            throw sls::RuntimeError("Cannot get");                             \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (args.size() != 1) {                                            \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
            det->SETFCN(args[0]);                                              \
            os << args.front() << '\n';                                        \
        } else {                                                               \
            throw sls::RuntimeError("Unknown action");                         \
        }                                                                      \
        return os.str();                                                       \
    }

/** set only, 1 argument */
#define EXECUTE_SET_COMMAND_1ARG(CMDNAME, SETFCN, HLPSTR)                      \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            throw sls::RuntimeError("Cannot get");                             \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (args.size() != 1) {                                            \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
            det->SETFCN(args[0], {det_id});                                    \
            os << args.front() << '\n';                                        \
        } else {                                                               \
            throw sls::RuntimeError("Unknown action");                         \
        }                                                                      \
        return os.str();                                                       \
    }

/** get only */
#define GET_COMMAND(CMDNAME, GETFCN, HLPSTR)                                   \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            if (!args.empty()) {                                               \
                WrongNumberOfParameters(0);                                    \
            }                                                                  \
            auto t = det->GETFCN({det_id});                                    \
            os << OutString(t) << '\n';                                        \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            throw sls::RuntimeError("Cannot put");                             \
        } else {                                                               \
            throw sls::RuntimeError("Unknown action");                         \
        }                                                                      \
        return os.str();                                                       \
    }

/** get only no id (vector, not result) */
#define GET_COMMAND_NOID(CMDNAME, GETFCN, HLPSTR)                              \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            if (!args.empty()) {                                               \
                WrongNumberOfParameters(0);                                    \
            }                                                                  \
            auto t = det->GETFCN();                                            \
            os << sls::ToString(t) << '\n';                                    \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            throw sls::RuntimeError("Cannot put");                             \
        } else {                                                               \
            throw sls::RuntimeError("Unknown action");                         \
        }                                                                      \
        return os.str();                                                       \
    }

/** get only hex*/
#define GET_COMMAND_HEX(CMDNAME, GETFCN, HLPSTR)                               \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            if (!args.empty()) {                                               \
                WrongNumberOfParameters(0);                                    \
            }                                                                  \
            auto t = det->GETFCN({det_id});                                    \
            os << OutStringHex(t) << '\n';                                     \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            throw sls::RuntimeError("Cannot put");                             \
        } else {                                                               \
            throw sls::RuntimeError("Unknown action");                         \
        }                                                                      \
        return os.str();                                                       \
    }

#define GET_IND_COMMAND(CMDNAME, GETFCN, VAL, APPEND, HLPSTR)                  \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            if (!args.empty()) {                                               \
                WrongNumberOfParameters(0);                                    \
            }                                                                  \
            auto t = det->GETFCN(VAL, {det_id});                               \
            os << OutString(t) << APPEND << '\n';                              \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            throw sls::RuntimeError("Cannot put");                             \
        } else {                                                               \
            throw sls::RuntimeError("Unknown action");                         \
        }                                                                      \
        return os.str();                                                       \
    }

namespace sls {

class CmdProxy {
  public:
    explicit CmdProxy(Detector *ptr) : det(ptr) {}

    void Call(const std::string &command,
              const std::vector<std::string> &arguments, int detector_id = -1,
              int action = -1, std::ostream &os = std::cout);

    bool ReplaceIfDepreciated(std::string &command);
    size_t GetFunctionMapSize() const noexcept { return functions.size(); };
    std::vector<std::string> GetProxyCommands();

  private:
    Detector *det;
    std::string cmd;
    std::vector<std::string> args;
    int det_id{-1};

    template <typename V> std::string OutStringHex(const V &value) {
        if (value.equal())
            return ToStringHex(value.front());
        return ToStringHex(value);
    }

    template <typename V> std::string OutStringHex(const V &value, int width) {
        if (value.equal())
            return ToStringHex(value.front(), width);
        return ToStringHex(value, width);
    }

    template <typename V> std::string OutString(const V &value) {
        if (value.equal())
            return ToString(value.front());
        return ToString(value);
    }
    template <typename V>
    std::string OutString(const V &value, const std::string &unit) {
        if (value.equal())
            return ToString(value.front(), unit);
        return ToString(value, unit);
    }

    // inline unsigned int stoiHex(const std::string& s) {
    //     unsigned long lresult = stoul(s, nullptr, 16);
    //     unsigned int result = lresult;
    //     if (result != lresult) {
    //         throw std::out_of_range("cannot convert to unsigned int");
    //     }
    //     return result;
    // }

    // inline unsigned long int stoulHex(const std::string& s) {
    //     unsigned long result = stoul(s, nullptr, 16);
    //     return result;
    // }

    using FunctionMap = std::map<std::string, std::string (CmdProxy::*)(int)>;
    using StringMap = std::map<std::string, std::string>;

    StringMap depreciated_functions{
        /* configuration */
        {"detectorversion", "firmwareversion"},
        {"softwareversion", "detectorserverversion"},
        {"receiverversion", "rx_version"},
        {"thisversion", "clientversion"},
        {"detsizechan", "detsize"},

        /* acquisition parameters */
        {"cycles", "triggers"},
        {"cyclesl", "triggersl"},
        {"clkdivider", "speed"},
        {"vhighvoltage", "highvoltage"},
        {"digitest", "imagetest"},

        /** temperature */

        /** dacs */
        {"vtr", "vtrim"},
        {"vrf", "vrpreamp"},
        {"vrs", "vrshaper"},
        {"vcall", "vcal"},
        {"vis", "vishaper"},
        {"vshaper", "vrshaper"},
        {"vpreamp", "vrpreamp"},
        {"vshaperneg", "vrshaper_n"},
        {"viinsh", "vishaper"},
        {"vpl", "vcal_n"},
        {"vph", "vcal_p"},

        /* acquisition */
        {"busy", "clearbusy"},
        {"receiver", "rx_status"},
        {"framescaught", "rx_framescaught"},

        /* Network Configuration (Detector<->Receiver) */
        {"detectorip", "udp_srcip"},
        {"detectorip2", "udp_srcip2"},
        {"detectormac", "udp_srcmac"},
        {"detectormac2", "udp_srcmac2"},
        {"rx_udpip", "udp_dstip"},
        {"rx_udpip2", "udp_dstip2"},
        {"rx_udpmac", "udp_dstmac"},
        {"rx_udpmac2", "udp_dstmac2"},
        {"rx_udpport", "udp_dstport"},
        {"rx_udpport2", "udp_dstport2"},
        {"flowcontrol_10g", "flowcontrol10g"},

        /* Receiver Config */
        {"r_silent", "rx_silent"},
        {"r_discardpolicy", "rx_discardpolicy"},
        {"r_padding", "rx_padding"},
        {"r_lock", "rx_lock"},
        {"r_lastclient", "rx_lastclient"},

        /* File */
        {"fileformat", "fformat"},
        {"outdir", "fpath"},
        {"index", "findex"},
        {"enablefwrite", "fwrite"},
        {"masterfile", "fmaster"},
        {"overwrite", "foverwrite"},
        {"r_framesperfile", "rx_framesperfile"},

        /* ZMQ Streaming Parameters (Receiver<->Client) */
        {"r_readfreq", "rx_zmqfreq"},
        {"rx_readfreq", "rx_zmqfreq"},
        {"rx_datastream", "rx_zmqstream"},

        /* Eiger Specific */
        {"trimdir", "settingspath"},
        {"settingsdir", "settingspath"},
        {"resmat", "partialreset"},

        /* Jungfrau Specific */
        /* Gotthard Specific */
        /* Gotthard2 Specific */
        /* Mythen3 Specific */
        /* CTB Specific */
        {"adc", "slowadc"},
        {"flags", "romode"},
        {"i_a", "im_a"},
        {"i_b", "im_b"},
        {"i_c", "im_c"},
        {"i_d", "im_d"},
        {"i_io", "im_io"},

        /* Pattern */
        /* Moench */
        /* Advanced */
        /* Insignificant */
        {"frameindex", "rx_frameindex"}

    };

    // Initialize maps for translating name and function
    FunctionMap functions{
        {"list", &CmdProxy::ListCommands},

        /* configuration */
        {"config", &CmdProxy::config},
        {"free", &CmdProxy::Free},
        {"parameters", &CmdProxy::parameters},
        {"hostname", &CmdProxy::Hostname},
        {"virtual", &CmdProxy::VirtualServer},
        {"versions", &CmdProxy::Versions},
        {"packageversion", &CmdProxy::PackageVersion},
        {"clientversion", &CmdProxy::ClientVersion},
        {"firmwareversion", &CmdProxy::FirmwareVersion},
        {"detectorserverversion", &CmdProxy::detectorserverversion},
        {"rx_version", &CmdProxy::rx_version},
        {"detectornumber", &CmdProxy::detectornumber},
        {"type", &CmdProxy::type},
        {"nmod", &CmdProxy::nmod},
        {"detsize", &CmdProxy::DetectorSize},
        {"settingslist", &CmdProxy::settingslist},
        {"settings", &CmdProxy::settings},
        {"trimbits", &CmdProxy::trimbits},
        {"trimval", &CmdProxy::trimval},
        {"gappixels", &CmdProxy::GapPixels},

        /* acquisition parameters */
        {"acquire", &CmdProxy::Acquire},
        {"frames", &CmdProxy::frames},
        {"triggers", &CmdProxy::triggers},
        {"exptime", &CmdProxy::Exptime},
        {"period", &CmdProxy::period},
        {"delay", &CmdProxy::delay},
        {"framesl", &CmdProxy::framesl},
        {"triggersl", &CmdProxy::triggersl},
        {"delayl", &CmdProxy::delayl},
        {"periodl", &CmdProxy::periodl},
        {"dr", &CmdProxy::DynamicRange},
        {"drlist", &CmdProxy::drlist},
        {"timing", &CmdProxy::timing},
        {"timinglist", &CmdProxy::timinglist},
        {"speed", &CmdProxy::Speed},
        {"adcphase", &CmdProxy::Adcphase},
        {"maxadcphaseshift", &CmdProxy::maxadcphaseshift},
        {"dbitphase", &CmdProxy::Dbitphase},
        {"maxdbitphaseshift", &CmdProxy::maxdbitphaseshift},
        {"clkfreq", &CmdProxy::ClockFrequency},
        {"clkphase", &CmdProxy::ClockPhase},
        {"maxclkphaseshift", &CmdProxy::MaxClockPhaseShift},
        {"clkdiv", &CmdProxy::ClockDivider},
        {"highvoltage", &CmdProxy::highvoltage},
        {"powerchip", &CmdProxy::powerchip},
        {"imagetest", &CmdProxy::imagetest},
        {"extsig", &CmdProxy::ExternalSignal},
        {"parallel", &CmdProxy::parallel},

        /** temperature */
        {"templist", &CmdProxy::templist},
        {"tempvalues", &CmdProxy::TemperatureValues},
        {"temp_adc", &CmdProxy::temp_adc},
        {"temp_fpga", &CmdProxy::temp_fpga},
        {"temp_fpgaext", &CmdProxy::temp_fpgaext},
        {"temp_10ge", &CmdProxy::temp_10ge},
        {"temp_dcdc", &CmdProxy::temp_dcdc},
        {"temp_sodl", &CmdProxy::temp_sodl},
        {"temp_sodr", &CmdProxy::temp_sodr},
        {"temp_fpgafl", &CmdProxy::temp_fpgafl},
        {"temp_fpgafr", &CmdProxy::temp_fpgafr},
        {"temp_slowadc", &CmdProxy::temp_slowadc},

        /* dacs */
        {"vthreshold", &CmdProxy::vthreshold},
        {"vsvp", &CmdProxy::vsvp},
        {"vsvn", &CmdProxy::vsvn},
        {"vtrim", &CmdProxy::vtrim},
        {"vrpreamp", &CmdProxy::vrpreamp},
        {"vrshaper", &CmdProxy::vrshaper},
        {"vtgstv", &CmdProxy::vtgstv},
        {"vcmp_ll", &CmdProxy::vcmp_ll},
        {"vcmp_lr", &CmdProxy::vcmp_lr},
        {"vcal", &CmdProxy::vcal},
        {"vcmp_rl", &CmdProxy::vcmp_rl},
        {"vcmp_rr", &CmdProxy::vcmp_rr},
        {"rxb_rb", &CmdProxy::rxb_rb},
        {"rxb_lb", &CmdProxy::rxb_lb},
        {"vcp", &CmdProxy::vcp},
        {"vcn", &CmdProxy::vcn},
        {"vishaper", &CmdProxy::vishaper},
        {"iodelay", &CmdProxy::iodelay},
        {"vref_ds", &CmdProxy::vref_ds},
        {"vcascn_pb", &CmdProxy::vcascn_pb},
        {"vcascp_pb", &CmdProxy::vcascp_pb},
        {"vout_cm", &CmdProxy::vout_cm},
        {"vcasc_out", &CmdProxy::vcasc_out},
        {"vin_cm", &CmdProxy::vin_cm},
        {"vref_comp", &CmdProxy::vref_comp},
        {"ib_test_c", &CmdProxy::ib_test_c},
        {"vrshaper_n", &CmdProxy::vrshaper_n},
        {"vipre", &CmdProxy::vipre},
        {"vdcsh", &CmdProxy::vdcsh},
        {"vth1", &CmdProxy::vth1},
        {"vth2", &CmdProxy::vth2},
        {"vth3", &CmdProxy::vth3},
        {"vcal_n", &CmdProxy::vcal_n},
        {"vcal_p", &CmdProxy::vcal_p},
        {"vcassh", &CmdProxy::vcassh},
        {"vcas", &CmdProxy::vcas},
        {"vicin", &CmdProxy::vicin},
        {"vipre_out", &CmdProxy::vipre_out},
        {"vref_h_adc", &CmdProxy::vref_h_adc},
        {"vb_comp_fe", &CmdProxy::vb_comp_fe},
        {"vb_comp_adc", &CmdProxy::vb_comp_adc},
        {"vcom_cds", &CmdProxy::vcom_cds},
        {"vref_rstore", &CmdProxy::vref_rstore},
        {"vb_opa_1st", &CmdProxy::vb_opa_1st},
        {"vref_comp_fe", &CmdProxy::vref_comp_fe},
        {"vcom_adc1", &CmdProxy::vcom_adc1},
        {"vref_prech", &CmdProxy::vref_prech},
        {"vref_l_adc", &CmdProxy::vref_l_adc},
        {"vref_cds", &CmdProxy::vref_cds},
        {"vb_cs", &CmdProxy::vb_cs},
        {"vb_opa_fd", &CmdProxy::vb_opa_fd},
        {"vcom_adc2", &CmdProxy::vcom_adc2},
        {"adcvpp", &CmdProxy::adcvpp},
        {"vb_ds", &CmdProxy::vb_ds},
        {"vb_comp", &CmdProxy::vb_comp},
        {"vb_pixbuf", &CmdProxy::vb_pixbuf},
        {"vin_com", &CmdProxy::vin_com},
        {"vdd_prot", &CmdProxy::vdd_prot},
        {"vbp_colbuf", &CmdProxy::vbp_colbuf},
        {"vb_sda", &CmdProxy::vb_sda},
        {"vcasc_sfp", &CmdProxy::vcasc_sfp},
        {"vipre_cds", &CmdProxy::vipre_cds},
        {"ibias_sfp", &CmdProxy::ibias_sfp},

        {"dac", &CmdProxy::Dac},
        {"daclist", &CmdProxy::daclist},
        {"dacvalues", &CmdProxy::DacValues},

        /* on chip dacs */
        {"vchip_comp_fe", &CmdProxy::vchip_comp_fe},
        {"vchip_opa_1st", &CmdProxy::vchip_opa_1st},
        {"vchip_opa_fd", &CmdProxy::vchip_opa_fd},
        {"vchip_comp_adc", &CmdProxy::vchip_comp_adc},
        {"vchip_ref_comp_fe", &CmdProxy::vchip_ref_comp_fe},
        {"vchip_cs", &CmdProxy::vchip_cs},

        /* acquisition */
        {"clearbusy", &CmdProxy::clearbusy},
        {"rx_start", &CmdProxy::rx_start},
        {"rx_stop", &CmdProxy::rx_stop},
        {"start", &CmdProxy::start},
        {"stop", &CmdProxy::stop},
        {"rx_status", &CmdProxy::ReceiverStatus},
        {"status", &CmdProxy::DetectorStatus},
        {"rx_framescaught", &CmdProxy::rx_framescaught},
        {"rx_missingpackets", &CmdProxy::rx_missingpackets},
        {"startingfnum", &CmdProxy::startingfnum},
        {"trigger", &CmdProxy::trigger},
        {"scan", &CmdProxy::Scan},
        {"scanerrmsg", &CmdProxy::scanerrmsg},

        /* Network Configuration (Detector<->Receiver) */
        {"numinterfaces", &CmdProxy::numinterfaces},
        {"selinterface", &CmdProxy::selinterface},
        {"udp_srcip", &CmdProxy::udp_srcip},
        {"udp_srcip2", &CmdProxy::udp_srcip2},
        {"udp_dstip", &CmdProxy::UDPDestinationIP},
        {"udp_dstip2", &CmdProxy::UDPDestinationIP2},
        {"udp_srcmac", &CmdProxy::udp_srcmac},
        {"udp_srcmac2", &CmdProxy::udp_srcmac2},
        {"udp_dstmac", &CmdProxy::udp_dstmac},
        {"udp_dstmac2", &CmdProxy::udp_dstmac2},
        {"udp_dstport", &CmdProxy::udp_dstport},
        {"udp_dstport2", &CmdProxy::udp_dstport2},
        {"udp_reconfigure", &CmdProxy::udp_reconfigure},
        {"udp_validate", &CmdProxy::udp_validate},
        {"rx_printconfig", &CmdProxy::rx_printconfig},
        {"tengiga", &CmdProxy::tengiga},
        {"flowcontrol10g", &CmdProxy::flowcontrol10g},
        {"txndelay_frame", &CmdProxy::txndelay_frame},
        {"txndelay_left", &CmdProxy::txndelay_left},
        {"txndelay_right", &CmdProxy::txndelay_right},

        /* Receiver Config */
        {"rx_hostname", &CmdProxy::ReceiverHostname},
        {"rx_tcpport", &CmdProxy::rx_tcpport},
        {"rx_fifodepth", &CmdProxy::rx_fifodepth},
        {"rx_silent", &CmdProxy::rx_silent},
        {"rx_discardpolicy", &CmdProxy::rx_discardpolicy},
        {"rx_padding", &CmdProxy::rx_padding},
        {"rx_udpsocksize", &CmdProxy::rx_udpsocksize},
        {"rx_realudpsocksize", &CmdProxy::rx_realudpsocksize},
        {"rx_lock", &CmdProxy::rx_lock},
        {"rx_lastclient", &CmdProxy::rx_lastclient},
        {"rx_threads", &CmdProxy::rx_threads},

        /* File */
        {"fformat", &CmdProxy::fformat},
        {"fpath", &CmdProxy::fpath},
        {"fname", &CmdProxy::fname},
        {"findex", &CmdProxy::findex},
        {"fwrite", &CmdProxy::fwrite},
        {"fmaster", &CmdProxy::fmaster},
        {"foverwrite", &CmdProxy::foverwrite},
        {"rx_framesperfile", &CmdProxy::rx_framesperfile},

        /* ZMQ Streaming Parameters (Receiver<->Client) */
        {"rx_zmqstream", &CmdProxy::rx_zmqstream},
        {"rx_zmqfreq", &CmdProxy::rx_zmqfreq},
        {"rx_zmqstartfnum", &CmdProxy::rx_zmqstartfnum},
        {"rx_zmqport", &CmdProxy::rx_zmqport},
        {"zmqport", &CmdProxy::zmqport},
        {"rx_zmqip", &CmdProxy::rx_zmqip},
        {"zmqip", &CmdProxy::zmqip},

        /* Eiger Specific */
        {"subexptime", &CmdProxy::subexptime},
        {"subdeadtime", &CmdProxy::subdeadtime},
        {"threshold", &CmdProxy::Threshold},
        {"thresholdnotb", &CmdProxy::ThresholdNoTb},
        {"settingspath", &CmdProxy::settingspath},
        {"overflow", &CmdProxy::overflow},
        {"flippeddatax", &CmdProxy::flippeddatax},
        {"trimen", &CmdProxy::TrimEnergies},
        {"ratecorr", &CmdProxy::RateCorrection},
        {"readnlines", &CmdProxy::readnlines},
        {"interruptsubframe", &CmdProxy::interruptsubframe},
        {"measuredperiod", &CmdProxy::measuredperiod},
        {"measuredsubperiod", &CmdProxy::measuredsubperiod},
        {"activate", &CmdProxy::Activate},
        {"partialreset", &CmdProxy::partialreset},
        {"pulse", &CmdProxy::PulsePixel},
        {"pulsenmove", &CmdProxy::PulsePixelAndMove},
        {"pulsechip", &CmdProxy::PulseChip},
        {"quad", &CmdProxy::Quad},

        /* Jungfrau Specific */
        {"temp_threshold", &CmdProxy::temp_threshold},
        {"temp_control", &CmdProxy::temp_control},
        {"temp_event", &CmdProxy::TemperatureEvent},
        {"auto_comp_disable", &CmdProxy::auto_comp_disable},
        {"storagecells", &CmdProxy::storagecells},
        {"storagecell_start", &CmdProxy::storagecell_start},
        {"storagecell_delay", &CmdProxy::storagecell_delay},

        /* Gotthard Specific */
        {"roi", &CmdProxy::ROI},
        {"clearroi", &CmdProxy::ClearROI},
        {"exptimel", &CmdProxy::exptimel},

        /* Gotthard2 Specific */
        {"bursts", &CmdProxy::bursts},
        {"burstperiod", &CmdProxy::burstperiod},
        {"inj_ch", &CmdProxy::InjectChannel},
        {"vetophoton", &CmdProxy::VetoPhoton},
        {"vetoref", &CmdProxy::VetoReference},
        {"vetofile", &CmdProxy::VetoFile},
        {"burstmode", &CmdProxy::BurstMode},
        {"cdsgain", &CmdProxy::cdsgain},
        {"filter", &CmdProxy::filter},
        {"currentsource", &CmdProxy::currentsource},
        {"timingsource", &CmdProxy::timingsource},
        {"veto", &CmdProxy::veto},
        {"confadc", &CmdProxy::ConfigureADC},
        {"badchannels", &CmdProxy::BadChannels},

        /* Mythen3 Specific */
        {"counters", &CmdProxy::Counters},
        {"gates", &CmdProxy::gates},
        {"exptime1", &CmdProxy::Exptime},
        {"exptime2", &CmdProxy::Exptime},
        {"exptime3", &CmdProxy::Exptime},
        {"gatedelay", &CmdProxy::GateDelay},
        {"gatedelay1", &CmdProxy::GateDelay},
        {"gatedelay2", &CmdProxy::GateDelay},
        {"gatedelay3", &CmdProxy::GateDelay},

        /* CTB/ Moench Specific */
        {"samples", &CmdProxy::Samples},
        {"asamples", &CmdProxy::asamples},
        {"adcclk", &CmdProxy::adcclk},
        {"runclk", &CmdProxy::runclk},
        {"syncclk", &CmdProxy::syncclk},
        {"adcpipeline", &CmdProxy::adcpipeline},
        {"v_limit", &CmdProxy::v_limit},
        {"adcenable", &CmdProxy::adcenable},
        {"adcenable10g", &CmdProxy::adcenable10g},

        /* CTB Specific */
        {"dsamples", &CmdProxy::dsamples},
        {"romode", &CmdProxy::romode},
        {"dbitclk", &CmdProxy::dbitclk},
        {"dbitpipeline", &CmdProxy::dbitpipeline},
        {"v_a", &CmdProxy::v_a},
        {"v_b", &CmdProxy::v_b},
        {"v_c", &CmdProxy::v_c},
        {"v_d", &CmdProxy::v_d},
        {"v_io", &CmdProxy::v_io},
        {"v_chip", &CmdProxy::v_chip},
        {"vm_a", &CmdProxy::vm_a},
        {"vm_b", &CmdProxy::vm_b},
        {"vm_c", &CmdProxy::vm_c},
        {"vm_d", &CmdProxy::vm_d},
        {"vm_io", &CmdProxy::vm_io},
        {"im_a", &CmdProxy::im_a},
        {"im_b", &CmdProxy::im_b},
        {"im_c", &CmdProxy::im_c},
        {"im_d", &CmdProxy::im_d},
        {"im_io", &CmdProxy::im_io},
        {"slowadc", &CmdProxy::SlowAdc},
        {"extsampling", &CmdProxy::extsampling},
        {"extsamplingsrc", &CmdProxy::extsamplingsrc},
        {"rx_dbitlist", &CmdProxy::ReceiverDbitList},
        {"rx_dbitoffset", &CmdProxy::rx_dbitoffset},
        {"diodelay", &CmdProxy::DigitalIODelay},
        {"led", &CmdProxy::led},

        /* Pattern */
        {"pattern", &CmdProxy::Pattern},
        {"savepattern", &CmdProxy::savepattern},
        {"patioctrl", &CmdProxy::patioctrl},
        {"patword", &CmdProxy::PatternWord},
        {"patlimits", &CmdProxy::PatternLoopAddresses},
        {"patloop0", &CmdProxy::PatternLoopAddresses},
        {"patloop1", &CmdProxy::PatternLoopAddresses},
        {"patloop2", &CmdProxy::PatternLoopAddresses},
        {"patnloop0", &CmdProxy::PatternLoopCycles},
        {"patnloop1", &CmdProxy::PatternLoopCycles},
        {"patnloop2", &CmdProxy::PatternLoopCycles},
        {"patwait0", &CmdProxy::PatternWaitAddress},
        {"patwait1", &CmdProxy::PatternWaitAddress},
        {"patwait2", &CmdProxy::PatternWaitAddress},
        {"patwaittime0", &CmdProxy::PatternWaitTime},
        {"patwaittime1", &CmdProxy::PatternWaitTime},
        {"patwaittime2", &CmdProxy::PatternWaitTime},
        {"patmask", &CmdProxy::patmask},
        {"patsetbit", &CmdProxy::patsetbit},
        {"patternstart", &CmdProxy::patternstart},

        /* Moench */
        {"rx_jsonaddheader", &CmdProxy::AdditionalJsonHeader},
        {"rx_jsonpara", &CmdProxy::JsonParameter},

        /* Advanced */
        {"programfpga", &CmdProxy::ProgramFpga},
        {"resetfpga", &CmdProxy::resetfpga},
        {"copydetectorserver", &CmdProxy::CopyDetectorServer},
        {"rebootcontroller", &CmdProxy::rebootcontroller},
        {"update", &CmdProxy::UpdateFirmwareAndDetectorServer},
        {"reg", &CmdProxy::Register},
        {"adcreg", &CmdProxy::AdcRegister},
        {"setbit", &CmdProxy::BitOperations},
        {"clearbit", &CmdProxy::BitOperations},
        {"getbit", &CmdProxy::BitOperations},
        {"firmwaretest", &CmdProxy::firmwaretest},
        {"bustest", &CmdProxy::bustest},
        {"initialchecks", &CmdProxy::InitialChecks},
        {"adcinvert", &CmdProxy::adcinvert},

        /* Insignificant */
        {"port", &CmdProxy::port},
        {"stopport", &CmdProxy::stopport},
        {"lock", &CmdProxy::lock},
        {"lastclient", &CmdProxy::lastclient},
        {"execcommand", &CmdProxy::ExecuteCommand},
        {"nframes", &CmdProxy::nframes},
        {"now", &CmdProxy::now},
        {"timestamp", &CmdProxy::timestamp},
        {"user", &CmdProxy::UserDetails},
        {"rx_frameindex", &CmdProxy::rx_frameindex}

    };

    void WrongNumberOfParameters(size_t expected);

    /* Commands */
    std::string ListCommands(int action);
    /* configuration */
    std::string Free(int action);
    // std::string config2(int action);
    std::string Hostname(int action);
    std::string VirtualServer(int action);
    std::string FirmwareVersion(int action);
    std::string Versions(int action);
    std::string PackageVersion(int action);
    std::string ClientVersion(int action);
    std::string DetectorSize(int action);
    std::string GapPixels(int action);
    /* acquisition parameters */
    std::string Acquire(int action);
    std::string Exptime(int action);
    std::string DynamicRange(int action);
    std::string Speed(int action);
    std::string Adcphase(int action);
    std::string Dbitphase(int action);
    std::string ClockFrequency(int action);
    std::string ClockPhase(int action);
    std::string MaxClockPhaseShift(int action);
    std::string ClockDivider(int action);
    std::string ExternalSignal(int action);
    /** temperature */
    std::string TemperatureValues(int action);
    /* dacs */
    std::string Dac(int action);
    std::string DacValues(int action);
    /* acquisition */
    std::string ReceiverStatus(int action);
    std::string DetectorStatus(int action);
    std::string Scan(int action);
    /* Network Configuration (Detector<->Receiver) */
    std::string UDPDestinationIP(int action);
    std::string UDPDestinationIP2(int action);
    /* Receiver Config */
    std::string ReceiverHostname(int action);
    /* File */
    /* ZMQ Streaming Parameters (Receiver<->Client) */
    /* Eiger Specific */
    std::string Threshold(int action);
    std::string ThresholdNoTb(int action);
    std::string TrimEnergies(int action);
    std::string RateCorrection(int action);
    std::string Activate(int action);
    std::string PulsePixel(int action);
    std::string PulsePixelAndMove(int action);
    std::string PulseChip(int action);
    std::string Quad(int action);
    /* Jungfrau Specific */
    std::string TemperatureEvent(int action);
    /* Gotthard Specific */
    std::string ROI(int action);
    std::string ClearROI(int action);
    /* Gotthard2 Specific */
    std::string InjectChannel(int action);
    std::string VetoPhoton(int action);
    std::string VetoReference(int action);
    std::string VetoFile(int action);
    std::string BurstMode(int action);
    std::string ConfigureADC(int action);
    std::string BadChannels(int action);
    /* Mythen3 Specific */
    std::string Counters(int action);
    std::string GateDelay(int action);
    /* CTB/ Moench Specific */
    std::string Samples(int action);
    /* CTB Specific */
    std::string SlowAdc(int action);
    std::string ReceiverDbitList(int action);
    std::string DigitalIODelay(int action);
    /* Pattern */
    std::string Pattern(int action);
    std::string PatternWord(int action);
    std::string PatternLoopAddresses(int action);
    std::string PatternLoopCycles(int action);
    std::string PatternWaitAddress(int action);
    std::string PatternWaitTime(int action);
    /* Moench */
    std::string AdditionalJsonHeader(int action);
    std::string JsonParameter(int action);
    /* Advanced */
    std::string ProgramFpga(int action);
    std::string CopyDetectorServer(int action);
    std::string UpdateFirmwareAndDetectorServer(int action);
    std::string Register(int action);
    std::string AdcRegister(int action);
    std::string BitOperations(int action);
    std::string InitialChecks(int action);
    /* Insignificant */
    std::string ExecuteCommand(int action);
    std::string UserDetails(int action);

    /* configuration */
    EXECUTE_SET_COMMAND_NOID_1ARG(
        config, loadConfig,
        "[fname]\n\tFrees shared memory before loading configuration file. "
        "Set up once.");

    EXECUTE_SET_COMMAND_NOID_1ARG(
        parameters, loadParameters,
        "[fname]\n\tSets detector measurement parameters to those contained in "
        "fname. Set up per measurement.");

    GET_COMMAND_HEX(
        detectorserverversion, getDetectorServerVersion,
        "\n\tOn-board detector server software version in format [0xYYMMDD].");

    GET_COMMAND_HEX(rx_version, getReceiverVersion,
                    "\n\tReceiver version in format [0xYYMMDD].");

    GET_COMMAND_HEX(detectornumber, getSerialNumber,
                    "\n\tReceiver version in format [0xYYMMDD].");

    GET_COMMAND(type, getDetectorType,
                "\n\tSerial number or MAC of detector (hex).");

    GET_COMMAND_NOID(nmod, size, "\n\tNumber of modules in shared memory.");

    GET_COMMAND_NOID(settingslist, getSettingsList,
                     "\n\tList of settings implemented for this detector.");

    INTEGER_COMMAND(settings, getSettings, setSettings,
                    sls::StringTo<slsDetectorDefs::detectorSettings>,
                    "[standard, fast, highgain, dynamicgain, lowgain, "
                    "mediumgain, veryhighgain, dynamichg0, "
                    "fixgain1, fixgain2, forceswitchg1, forceswitchg2, "
                    "verylowgain, g1_hg, g1_lg, g2_hc_hg, g2_hc_lg, "
                    "g2_lc_hg, g2_lc_lg, g4_hg, g4_lg]"
                    "\n\t Detector Settings"
                    "\n\t[Jungfrau] - [dynamicgain | dynamichg0 | fixgain1 | "
                    "fixgain2 | forceswitchg1 | forceswitchg2]"
                    "\n\t[Gotthard] - [dynamicgain | highgain | lowgain | "
                    "mediumgain | veryhighgain]"
                    "\n\t[Gotthard2] - [dynamicgain | fixgain1 | fixgain2]"
                    "\n\t[Moench] - [g1_hg | g1_lg | g2_hc_hg | g2_hc_lg | "
                    "g2_lc_hg | g2_lc_lg | g4_hg | g4_lg]"
                    "\n\t[Eiger] Use threshold or thresholdnotb. \n\t[Eiger] "
                    "settings loaded from file found in settingspath.");

    EXECUTE_SET_COMMAND_1ARG(
        trimbits, loadTrimbits,
        "[fname]\n\t[Eiger][Mythen3] Loads the trimbit file to detector. If no "
        "extension specified, serial number of each module is attached.");

    INTEGER_COMMAND(trimval, getAllTrimbits, setAllTrimbits, StringTo<int>,
                    "[n_trimval]\n\t[Eiger][Mythen3] All trimbits set to this "
                    "value. Returns -1 if all trimbits are different values.");

    /* acquisition parameters */

    INTEGER_COMMAND_NOID(
        frames, getNumberOfFrames, setNumberOfFrames, StringTo<int64_t>,
        "[n_frames]\n\tNumber of frames per acquisition. In "
        "trigger mode, number of frames per trigger. Cannot be set in modular "
        "level. In scan mode, number of frames is set to number of "
        "steps.\n\t[Gotthard2] Burst mode has a maximum of 2720 frames.");

    INTEGER_COMMAND_NOID(triggers, getNumberOfTriggers, setNumberOfTriggers,
                         StringTo<int64_t>,
                         "[n_triggers]\n\tNumber of triggers per aquire. Use "
                         "timing command to set timing mode.");

    TIME_COMMAND(
        period, getPeriod, setPeriod,
        "[duration] [(optional unit) ns|us|ms|s]\n\tPeriod between frames");

    TIME_COMMAND(delay, getDelayAfterTrigger, setDelayAfterTrigger,
                 "[duration] [(optional unit) "
                 "ns|us|ms|s]\n\t[Jungfrau][Gotthard][Mythen3][Gotthard2][Ctb]["
                 "Moench] Delay after trigger");

    GET_COMMAND(framesl, getNumberOfFramesLeft,
                "\n\t[Gotthard][Jungfrau][Mythen3][Gotthard2][CTB][Moench] "
                "Number of frames left in acquisition."
                "\n\t[Gotthard2] only in continuous mode.");

    GET_COMMAND(triggersl, getNumberOfTriggersLeft,
                "\n\t[Gotthard][Jungfrau][Mythen3][Gotthard2][CTB][Moench] "
                "Number of triggers left in acquisition."
                "\n\t[Gotthard2] only in continuous mode.");

    TIME_GET_COMMAND(delayl, getDelayAfterTriggerLeft,
                     "\n\t[Gotthard][Jungfrau][Mythen3][Gotthard2][CTB][Moench]"
                     " DelayLeft Delay Left in Acquisition."
                     "\n\t[Gotthard2] only in continuous mode.");

    TIME_GET_COMMAND(periodl, getPeriodLeft,
                     "\n\t[Gotthard][Jungfrau][Mythen3][Gotthard2][CTB][Moench]"
                     " Period left for current frame."
                     "\n\t[Gotthard2] only in continuous mode.");

    GET_COMMAND_NOID(drlist, getDynamicRangeList,
                     "\n\tGets the list of dynamic ranges for this detector.");

    INTEGER_COMMAND(timing, getTimingMode, setTimingMode,
                    sls::StringTo<slsDetectorDefs::timingMode>,
                    "[auto|trigger|gating|burst_trigger]\n\tTiming Mode of "
                    "detector.\n\t[Jungfrau][Gotthard][Ctb][Moench][Gotthard2] "
                    "[auto|trigger]\n\t[Mythen3] "
                    "[auto|trigger|gating|trigger_gating]\n\t[Eiger] "
                    "[auto|trigger|gating|burst_trigger]");

    GET_COMMAND_NOID(timinglist, getTimingModeList,
                     "\n\tGets the list of timing modes for this detector.");

    GET_COMMAND(maxadcphaseshift, getMaxADCPhaseShift,
                "\n\t[Jungfrau][CTB][Moench] Absolute maximum Phase shift of "
                "ADC clock.");

    GET_COMMAND(maxdbitphaseshift, getMaxDBITPhaseShift,
                "\n\t[CTB][Jungfrau] Absolute maximum Phase shift of of the "
                "clock to latch digital bits.");

    INTEGER_COMMAND(highvoltage, getHighVoltage, setHighVoltage, StringTo<int>,
                    "[n_value]\n\tHigh voltage to the sensor in Voltage."
                    "\n\t[Gotthard] [0|90|110|120|150|180|200]"
                    "\n\t[Eiger][Mythen3][Gotthard2] 0-200"
                    "\n\t[Jungfrau][Ctb][Moench] [0|60-200]");

    INTEGER_COMMAND(powerchip, getPowerChip, setPowerChip, StringTo<int>,
                    "[0, 1]\n\t[Jungfrau][Mythen3][Gotthard2][Moench] Power "
                    "the chip. Default 0."
                    "\n\t[Jungfrau] Get will return power status."
                    "Can be off if temperature event occured (temperature over "
                    "temp_threshold with temp_control enabled."
                    "\n\t[Mythen3] If module not connected or wrong module, 1 "
                    "will fail. By default, not powered on"
                    "\n\t[Gotthard2] If module not connected or wrong module, "
                    "1 will fail. By default, powered on at server start up.");

    INTEGER_COMMAND(
        imagetest, getImageTestMode, setImageTestMode, StringTo<int>,
        "[0, 1]\n\t[Gotthard] 1 adds channel intensity with precalculated "
        "values when taking an acquisition. Default is 0."
        "\n\t[Eiger][Jungfrau] Only for Virtual servers. If 0, each pixel "
        "intensity incremented by 1. If 1, all pixels almost saturated.");

    INTEGER_COMMAND(parallel, getParallelMode, setParallelMode, StringTo<int>,
                    "[0, 1]\n\t[Eiger][Mythen3] Enable or disable parallel "
                    "mode.\n\t[Mythen3] If exptime is too short, the "
                    "acquisition will return ERROR status and take fewer "
                    "frames than expected.");

    /** temperature */
    GET_COMMAND_NOID(
        templist, getTemperatureList,
        "\n\tList of temperature commands implemented for this detector.");

    GET_IND_COMMAND(temp_adc, getTemperature, slsDetectorDefs::TEMPERATURE_ADC,
                    " °C", "[n_value]\n\t[Jungfrau][Gotthard] ADC Temperature");

    GET_IND_COMMAND(
        temp_fpga, getTemperature, slsDetectorDefs::TEMPERATURE_FPGA, " °C",
        "[n_value]\n\t[Eiger][Jungfrau][Gotthard] FPGA Temperature");

    GET_IND_COMMAND(temp_fpgaext, getTemperature,
                    slsDetectorDefs::TEMPERATURE_FPGAEXT, " °C",
                    "[n_value]\n\t[Eiger]Temperature close to the FPGA");

    GET_IND_COMMAND(temp_10ge, getTemperature,
                    slsDetectorDefs::TEMPERATURE_10GE, " °C",
                    "[n_value]\n\t[Eiger]Temperature close to the 10GbE");

    GET_IND_COMMAND(
        temp_dcdc, getTemperature, slsDetectorDefs::TEMPERATURE_DCDC, " °C",
        "[n_value]\n\t[Eiger]Temperature close to the dc dc converter");

    GET_IND_COMMAND(
        temp_sodl, getTemperature, slsDetectorDefs::TEMPERATURE_SODL, " °C",
        "[n_value]\n\t[Eiger]Temperature close to the left so-dimm memory");

    GET_IND_COMMAND(
        temp_sodr, getTemperature, slsDetectorDefs::TEMPERATURE_SODR, " °C",
        "[n_value]\n\t[Eiger]Temperature close to the right so-dimm memory");

    GET_IND_COMMAND(
        temp_fpgafl, getTemperature, slsDetectorDefs::TEMPERATURE_FPGA2, " °C",
        "[n_value]\n\t[Eiger]Temperature of the left front end board fpga. "
        "Cannot call this while blocking acquire is going on.");

    GET_IND_COMMAND(
        temp_fpgafr, getTemperature, slsDetectorDefs::TEMPERATURE_FPGA3, " °C",
        "[n_value]\n\t[Eiger]Temperature of the left front end board fpga. "
        "Cannot call this while blocking acquire is going on.");

    GET_IND_COMMAND(temp_slowadc, getTemperature,
                    slsDetectorDefs::SLOW_ADC_TEMP, " °C",
                    "[n_value]\n\t[Ctb]Temperature of the slow adc");

    /* dacs */

    DAC_COMMAND(vthreshold, getDAC, setDAC, defs::VTHRESHOLD,
                "[dac or mV value][(optional unit) mV] \n\t[Eiger][Mythen3] "
                "Detector threshold voltage for single photon counters.");

    DAC_COMMAND(vsvp, getDAC, setDAC, defs::VSVP,
                "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for "
                "?? "); // TODO

    DAC_COMMAND(vsvn, getDAC, setDAC, defs::VSVN,
                "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for "
                "?? \n\t[Mythen3] voltage "
                "to define feedback resistance of the first shaper"); // TODO

    DAC_COMMAND(vtrim, getDAC, setDAC, defs::VTRIM,
                "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for "
                "?? \n\t[Mythen3] Dac for "
                "the voltage defining the trim bit size."); // TODO

    DAC_COMMAND(vrpreamp, getDAC, setDAC, defs::VRPREAMP,
                "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for "
                "?? \n\t[Mythen3] voltage "
                "to define the preamplifier feedback resistance."); // TODO

    DAC_COMMAND(vrshaper, getDAC, setDAC, defs::VRSHAPER,
                "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for "
                "?? \n\t[Mythen3] voltage to define feedback resistance of "
                "the first shaper"); // TODO

    DAC_COMMAND(vtgstv, getDAC, setDAC, defs::VTGSTV,
                "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for "
                "?? "); // TODO

    DAC_COMMAND(vcmp_ll, getDAC, setDAC, defs::VCMP_LL,
                "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for "
                "?? "); // TODO

    DAC_COMMAND(vcmp_lr, getDAC, setDAC, defs::VCMP_LR,
                "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for "
                "?? "); // TODO

    DAC_COMMAND(vcal, getDAC, setDAC, defs::VCAL,
                "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for "
                "?? "); // TODO

    DAC_COMMAND(vcmp_rl, getDAC, setDAC, defs::VCMP_RL,
                "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for "
                "?? "); // TODO

    DAC_COMMAND(vcmp_rr, getDAC, setDAC, defs::VCMP_RR,
                "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for "
                "?? "); // TODO

    DAC_COMMAND(rxb_rb, getDAC, setDAC, defs::RXB_RB,
                "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for "
                "?? "); // TODO

    DAC_COMMAND(rxb_lb, getDAC, setDAC, defs::RXB_LB,
                "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for "
                "?? "); // TODO

    DAC_COMMAND(vcp, getDAC, setDAC, defs::VCP,
                "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for "
                "?? "); // TODO

    DAC_COMMAND(vcn, getDAC, setDAC, defs::VCN,
                "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for "
                "?? "); // TODO

    DAC_COMMAND(vishaper, getDAC, setDAC, defs::VISHAPER,
                "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for "
                "?? \n\t[Mythen3] Dac for "
                "the bias current for the shaper."); // TODO

    DAC_COMMAND(iodelay, getDAC, setDAC, defs::IO_DELAY,
                "[dac or mV value][(optional unit) mV] \n\t[Eiger] Dac for "
                "?? "); // TODO

    DAC_COMMAND(vref_ds, getDAC, setDAC, defs::VREF_DS,
                "[dac or mV value][(optional unit) mV] "
                "\n\t[Gotthard][Jungfrau] Dac for ?? "); // TODO

    DAC_COMMAND(vcascn_pb, getDAC, setDAC, defs::VCASCN_PB,
                "[dac or mV value][(optional unit) mV] \n\t[Gotthard] Dac for "
                "?? "); // TODO

    DAC_COMMAND(vcascp_pb, getDAC, setDAC, defs::VCASCP_PB,
                "[dac or mV value][(optional unit) mV] \n\t[Gotthard] Dac for "
                "?? "); // TODO

    DAC_COMMAND(vout_cm, getDAC, setDAC, defs::VOUT_CM,
                "[dac or mV value][(optional unit) mV] \n\t[Gotthard] Dac for "
                "?? \n\t[Moench] Dac for 5"); // TODO

    DAC_COMMAND(vcasc_out, getDAC, setDAC, defs::VCASC_OUT,
                "[dac or mV value][(optional unit) mV] \n\t[Gotthard] Dac for "
                "?? "); // TODO

    DAC_COMMAND(vin_cm, getDAC, setDAC, defs::VIN_CM,
                "[dac or mV value][(optional unit) mV] \n\t[Gotthard] Dac for "
                "?? \n\t[Moench] Dac for 2"); // TODO

    DAC_COMMAND(vref_comp, getDAC, setDAC, defs::VREF_COMP,
                "[dac or mV value][(optional unit) mV] "
                "\n\t[Gotthard][Jungfrau] Dac for ?? "); // TODO

    DAC_COMMAND(ib_test_c, getDAC, setDAC, defs::IB_TESTC,
                "[dac or mV value][(optional unit) mV] \n\t[Gotthard] Dac for "
                "?? "); // TODO

    DAC_COMMAND(vrshaper_n, getDAC, setDAC, defs::VRSHAPER_N,
                "[dac or mV value][(optional unit) mV] \n\t[Mythen3] voltage "
                "to define feedback resistance of the second shaper.");

    DAC_COMMAND(
        vipre, getDAC, setDAC, defs::VIPRE,
        "[dac or mV value][(optional unit) mV] \n\t[Mythen3] Dac for the "
        "preamplifier's input transistor current.\n\t[Moench] Dac for 1");

    DAC_COMMAND(vdcsh, getDAC, setDAC, defs::VDCSH,
                "[dac or mV value][(optional unit) mV] \n\t[Mythen3] Dac for "
                "the reference (DC) voltage for the shaper.");

    DAC_COMMAND(vth1, getDAC, setDAC, defs::VTH1,
                "[dac or mV value][(optional unit) mV] \n\t[Mythen3] Dac for "
                "first detector threshold voltage.");

    DAC_COMMAND(vth2, getDAC, setDAC, defs::VTH2,
                "[dac or mV value][(optional unit) mV] \n\t[Mythen3] Dac for "
                "second detector threshold voltage.");

    DAC_COMMAND(vth3, getDAC, setDAC, defs::VTH3,
                "[dac or mV value][(optional unit) mV] \n\t[Mythen3] Dac for "
                "third detector threshold voltage.");

    DAC_COMMAND(vcal_n, getDAC, setDAC, defs::VCAL_N,
                "[dac or mV value][(optional unit) mV] \n\t[Mythen3] Dac for "
                "the low voltage for analog pulsing.");

    DAC_COMMAND(vcal_p, getDAC, setDAC, defs::VCAL_P,
                "[dac or mV value][(optional unit) mV] \n\t[Mythen3] Dac for "
                "the high voltage for analog pulsing.");

    DAC_COMMAND(vcassh, getDAC, setDAC, defs::VCASSH,
                "[dac or mV value][(optional unit) mV] \n\t[Mythen3] Dac for "
                "the shaper's cascode voltage.");

    DAC_COMMAND(vcas, getDAC, setDAC, defs::VCAS,
                "[dac or mV value][(optional unit) mV] \n\t[Mythen3] Dac for "
                "the preamplifier's cascode voltage.");

    DAC_COMMAND(vicin, getDAC, setDAC, defs::VICIN,
                "[dac or mV value][(optional unit) mV] \n\t[Mythen3] Dac for "
                "the bias current for the comparator.");

    DAC_COMMAND(vipre_out, getDAC, setDAC, defs::VIPRE_OUT,
                "[dac or mV value][(optional unit) mV] \n\t[Mythen3] Dac for "
                "preamplifier's output transistor current."); // TODO

    DAC_COMMAND(vref_h_adc, getDAC, setDAC, defs::VREF_H_ADC,
                "[dac or mV value][(optional unit) mV] \n\t[Gotthard2] Dac for "
                "reference voltage high of ADC.");

    DAC_COMMAND(vb_comp_fe, getDAC, setDAC, defs::VB_COMP_FE,
                "[dac or mV value][(optional unit) mV] \n\t[Gotthard2] Dac for "
                "comparator current of analogue front end.");

    DAC_COMMAND(vb_comp_adc, getDAC, setDAC, defs::VB_COMP_ADC,
                "[dac or mV value][(optional unit) mV] \n\t[Gotthard2] Dac for "
                "comparator current of ADC.");

    DAC_COMMAND(vcom_cds, getDAC, setDAC, defs::VCOM_CDS,
                "[dac or mV value][(optional unit) mV] \n\t[Gotthard2] Dac for "
                "common mode voltage of CDS stage.");

    DAC_COMMAND(
        vref_rstore, getDAC, setDAC, defs::VREF_RSTORE,
        "[dac or mV value][(optional unit) mV] \n\t[Gotthard2] Dac for "
        "reference charging voltage of temparory storage cell in high gain.");

    DAC_COMMAND(vb_opa_1st, getDAC, setDAC, defs::VB_OPA_1ST,
                "[dac or mV value][(optional unit) mV] \n\t[Gotthard2] dac dac "
                "for opa current for driving the other DACs in chip.");

    DAC_COMMAND(vref_comp_fe, getDAC, setDAC, defs::VREF_COMP_FE,
                "[dac or mV value][(optional unit) mV] \n\t[Gotthard2] Dac for "
                "reference voltage of the comparator of analogue front end.");

    DAC_COMMAND(vcom_adc1, getDAC, setDAC, defs::VCOM_ADC1,
                "[dac or mV value][(optional unit) mV] \n\t[Gotthard2] Dac for "
                "common mode voltage of ADC DAC bank 1.");

    DAC_COMMAND(
        vref_prech, getDAC, setDAC, defs::VREF_PRECH,
        "[dac or mV value][(optional unit) mV] \n\t[Gotthard2][Jungfrau] Dac "
        "for reference votlage for precharing the preamplifier."); // TODO also
                                                                   // for
                                                                   // jungfrau?

    DAC_COMMAND(vref_l_adc, getDAC, setDAC, defs::VREF_L_ADC,
                "[dac or mV value][(optional unit) mV] \n\t[Gotthard2] Dac for "
                "reference voltage low for ADC.");

    DAC_COMMAND(vref_cds, getDAC, setDAC, defs::VREF_CDS,
                "[dac or mV value][(optional unit) mV] \n\t[Gotthard2] Dac for "
                "reference voltage of CDS applied to the temporary storage "
                "cell in medium and low gain.");

    DAC_COMMAND(vb_cs, getDAC, setDAC, defs::VB_CS,
                "[dac or mV value][(optional unit) mV] \n\t[Gotthard2] Dac for "
                "current injection into preamplifier.");

    DAC_COMMAND(vb_opa_fd, getDAC, setDAC, defs::VB_OPA_FD,
                "[dac or mV value][(optional unit) mV] \n\t[Gotthard2] Dac for "
                "current for CDS opa stage.");

    DAC_COMMAND(vcom_adc2, getDAC, setDAC, defs::VCOM_ADC2,
                "[dac or mV value][(optional unit) mV] \n\t[Gotthard2] Dac for "
                "common mode voltage of ADC DAC bank 2.");

    DAC_COMMAND(
        adcvpp, getDAC, setDAC, defs::ADC_VPP,
        "[dac or mV value][(optional unit) mV] \n\t[Ctb][Moench] Vpp of "
        "ADC.\n\t 0 -> 1V ; 1 -> 1.14V ; 2 -> 1.33V ; 3 -> 1.6V ; 4 -> 2V.");

    DAC_COMMAND(vb_ds, getDAC, setDAC, defs::VB_DS,
                "[dac or mV value][(optional unit) mV] \n\t[Jungfrau] Dac for "
                "??"); // TODO

    DAC_COMMAND(vb_comp, getDAC, setDAC, defs::VB_COMP,
                "[dac or mV value][(optional unit) mV] \n\t[Jungfrau] Dac for "
                "??"); // TODO

    DAC_COMMAND(vb_pixbuf, getDAC, setDAC, defs::VB_PIXBUF,
                "[dac or mV value][(optional unit) mV] \n\t[Jungfrau] Dac for "
                "??"); // TODO

    DAC_COMMAND(vin_com, getDAC, setDAC, defs::VIN_COM,
                "[dac or mV value][(optional unit) mV] \n\t[Jungfrau] Dac for "
                "??"); // TODO

    DAC_COMMAND(vdd_prot, getDAC, setDAC, defs::VDD_PROT,
                "[dac or mV value][(optional unit) mV] \n\t[Jungfrau] Dac for "
                "??"); // TODO

    DAC_COMMAND(vbp_colbuf, getDAC, setDAC, defs::VBP_COLBUF,
                "[dac or mV value][(optional unit) mV] \n\t[Moench] Dac for 0");

    DAC_COMMAND(vb_sda, getDAC, setDAC, defs::VB_SDA,
                "[dac or mV value][(optional unit) mV] \n\t[Moench] Dac for 3");

    DAC_COMMAND(vcasc_sfp, getDAC, setDAC, defs::VCASC_SFP,
                "[dac or mV value][(optional unit) mV] \n\t[Moench] Dac for 4");

    DAC_COMMAND(vipre_cds, getDAC, setDAC, defs::VIPRE_CDS,
                "[dac or mV value][(optional unit) mV] \n\t[Moench] Dac for 6");

    DAC_COMMAND(ibias_sfp, getDAC, setDAC, defs::IBIAS_SFP,
                "[dac or mV value][(optional unit) mV] \n\t[Moench] Dac for 7");

    GET_COMMAND_NOID(
        daclist, getDacList,
        "\n\tGets the list of commands for every dac for this detector.");

    /* on chip dacs */
    INTEGER_USER_IND_COMMAND(
        vchip_comp_fe, getOnChipDAC, setOnChipDAC, StringTo<int>,
        defs::VB_COMP_FE,
        "[chip index 0-10, -1 for all][10 bit hex value] \n\t[Gotthard2] On "
        "chip Dac for comparator current of analogue front end.");

    INTEGER_USER_IND_COMMAND(
        vchip_opa_1st, getOnChipDAC, setOnChipDAC, StringTo<int>,
        defs::VB_OPA_1ST,
        "[chip index 0-10, -1 for all][10 bit hex value] \n\t[Gotthard2] On "
        "chip Dac for opa current for driving the other DACs in chip.");

    INTEGER_USER_IND_COMMAND(
        vchip_opa_fd, getOnChipDAC, setOnChipDAC, StringTo<int>,
        defs::VB_OPA_FD,
        "[chip index 0-10, -1 for all][10 bit hex value] \n\t[Gotthard2] On "
        "chip Dac current for CDS opa stage.");

    INTEGER_USER_IND_COMMAND(
        vchip_comp_adc, getOnChipDAC, setOnChipDAC, StringTo<int>,
        defs::VB_COMP_ADC,
        "[chip index 0-10, -1 for all][10 bit hex value] \n\t[Gotthard2] On "
        "chip Dac for comparator current of ADC.");

    INTEGER_USER_IND_COMMAND(
        vchip_ref_comp_fe, getOnChipDAC, setOnChipDAC, StringTo<int>,
        defs::VREF_COMP_FE,
        "[chip index 0-10, -1 for all][10 bit hex value] \n\t[Gotthard2] On "
        "chip Dac for reference voltage of the comparator of analogue front "
        "end.");

    INTEGER_USER_IND_COMMAND(
        vchip_cs, getOnChipDAC, setOnChipDAC, StringTo<int>, defs::VB_CS,
        "[chip index 0-10, -1 for all][10 bit hex value] \n\t[Gotthard2] On "
        "chip Dac for current injection into preamplifier.");

    /* acquisition */

    EXECUTE_SET_COMMAND_NOID(clearbusy, clearAcquiringFlag,
                             "\n\tClears Acquiring Flag for unexpected acquire "
                             "command terminations.");

    EXECUTE_SET_COMMAND_NOID(
        rx_start, startReceiver,
        "\n\tStarts receiver listener for detector data packets and create a "
        "data file (if file write enabled).");

    EXECUTE_SET_COMMAND_NOID(
        rx_stop, stopReceiver,
        "\n\tStops receiver listener for detector data packets and closes "
        "current data file (if file write enabled).");

    EXECUTE_SET_COMMAND_NOID(
        start, startDetector,
        "\n\tStarts detector acquisition. Status changes to RUNNING or WAITING "
        "and automatically returns to idle at the end of acquisition.");

    EXECUTE_SET_COMMAND_NOID(
        stop, stopDetector,
        "\n\tAbort detector acquisition. Status changes to IDLE or STOPPED.");

    GET_COMMAND(rx_framescaught, getFramesCaught,
                "\n\tNumber of frames caught by receiver.");

    GET_COMMAND(rx_missingpackets, getNumMissingPackets,
                "\n\tNumber of missing packets for each port in receiver.");

    INTEGER_COMMAND(startingfnum, getStartingFrameNumber,
                    setStartingFrameNumber, StringTo<uint64_t>,
                    "[n_value]\n\t[Eiger][Jungfrau] Starting frame number for "
                    "next acquisition. Stopping acquiistion might result in "
                    "different frame numbers for different modules.");

    EXECUTE_SET_COMMAND(
        trigger, sendSoftwareTrigger,
        "\n\t[Eiger][Mythen3] Sends software trigger signal to detector.");

    GET_COMMAND(scanerrmsg, getScanErrorMessage,
                "\n\tGets Scan error message if scan ended in error for non "
                "blocking acquisitions.");

    /* Network Configuration (Detector<->Receiver) */

    INTEGER_COMMAND(
        numinterfaces, getNumberofUDPInterfaces, setNumberofUDPInterfaces,
        StringTo<int>,
        "[1, 2]\n\t[Jungfrau][Gotthard2] Number of udp interfaces to stream "
        "data from detector. Default: 1.\n\tAlso enables second interface in "
        "receiver for listening (Writes a file per interface if writing "
        "enabled).\n\tAlso restarts client and receiver zmq sockets if zmq "
        "streaming enabled.\n\t[Gotthard2] second interface enabled to send "
        "veto information via 10Gbps for debugging. By default, if veto "
        "enabled, it is sent via 2.5 gbps interface.");

    INTEGER_COMMAND(
        selinterface, getSelectedUDPInterface, selectUDPInterface,
        StringTo<int>,
        "[0, 1]\n\t[Jungfrau] The udp interface to stream data from detector. "
        "Effective only when number of interfaces is 1. Default: 0 (outer)");

    INTEGER_COMMAND(udp_srcip, getSourceUDPIP, setSourceUDPIP, IpAddr,
                    "[x.x.x.x]\n\tIp address of the detector (source) udp "
                    "interface. Must be same subnet as destination udp ip.");

    INTEGER_COMMAND(
        udp_srcip2, getSourceUDPIP2, setSourceUDPIP2, IpAddr,
        "[x.x.x.x]\n\t[Jungfrau] Ip address of the bottom half of detector "
        "(source) udp interface. Must be same subnet as destination udp ip2.");

    INTEGER_COMMAND(udp_srcmac, getSourceUDPMAC, setSourceUDPMAC, MacAddr,
                    "[x:x:x:x:x:x]\n\tMac address of the detector (source) udp "
                    "interface. ");

    INTEGER_COMMAND(udp_srcmac2, getSourceUDPMAC2, setSourceUDPMAC2, MacAddr,
                    "[x:x:x:x:x:x]\n\t[Jungfrau] Mac address of the bottom "
                    "half of detector (source) udp interface. ");

    INTEGER_COMMAND(
        udp_dstmac, getDestinationUDPMAC, setDestinationUDPMAC, MacAddr,
        "[x:x:x:x:x:x]\n\tMac address of the receiver (destination) udp "
        "interface. Can be unused as udp_dstip retrieves it.");

    INTEGER_COMMAND(
        udp_dstmac2, getDestinationUDPMAC2, setDestinationUDPMAC2, MacAddr,
        "[x:x:x:x:x:x]\n\t[Jungfrau] Mac address of the receiver (destination) "
        "udp interface where the second half of detector data is sent to. Can "
        "be unused as udp_dstip2 retrieves it.");

    INTEGER_COMMAND(udp_dstport, getDestinationUDPPort, setDestinationUDPPort,
                    StringTo<int>,
                    "[n]\n\tPort number of the receiver (destination) udp "
                    "interface. Default is 50001.");

    INTEGER_COMMAND(
        udp_dstport2, getDestinationUDPPort2, setDestinationUDPPort2,
        StringTo<int>,
        "[n]\n\tDefault is 50002.\n\t[Jungfrau] Port number of the receiver "
        "(destination) udp interface where the second half of detector data is "
        "sent to. \n\t[Eiger] Port number of the reciever (desintation) udp "
        "interface where the right half of the detector data is sent to.");

    EXECUTE_SET_COMMAND(
        udp_reconfigure, reconfigureUDPDestination,
        "\n\tReconfigures Detector with UDP destination. More for debugging as "
        "the configuration is done automatically when the detector has "
        "sufficient UDP details.");

    EXECUTE_SET_COMMAND(
        udp_validate, validateUDPConfiguration,
        "\n\tValidates that UDP configuration in the detector is "
        "valid. If not configured, it will throw with error message "
        "requesting missing udp information.");

    GET_COMMAND(rx_printconfig, printRxConfiguration,
                "\n\tPrints the receiver configuration.");

    INTEGER_COMMAND(tengiga, getTenGiga, setTenGiga, StringTo<int>,
                    "[0, 1]\n\t[Eiger][Ctb][Moench][Mythen3] 10GbE Enable.");

    INTEGER_COMMAND(flowcontrol10g, getTenGigaFlowControl,
                    setTenGigaFlowControl, StringTo<int>,
                    "[0, 1]\n\t[Eiger][Jungfrau] 10GbE Flow Control.");

    INTEGER_COMMAND(
        txndelay_frame, getTransmissionDelayFrame, setTransmissionDelayFrame,
        StringTo<int>,
        "[n_delay]\n\t[Eiger][Jungfrau][Mythen3] Transmission delay of each "
        "image being "
        "streamed out of the module.\n\t[Jungfrau] [0-31] Each value "
        "represents 1 ms\n\t[Eiger] Additional delay to txndelay_left and "
        "txndelay_right. Each value represents 10ns. Typical value is "
        "50000.\n\t[Mythen3] [0-16777215] Each value represents 8 ns (125 MHz "
        "clock), max is 134 ms.");

    INTEGER_COMMAND(
        txndelay_left, getTransmissionDelayLeft, setTransmissionDelayLeft,
        StringTo<int>,
        "[n_delay]\n\t[Eiger] Transmission delay of first packet in an image "
        "being streamed out of the module's left UDP port. Each value "
        "represents 10ns. Typical value is 50000.");

    INTEGER_COMMAND(
        txndelay_right, getTransmissionDelayRight, setTransmissionDelayRight,
        StringTo<int>,
        "[n_delay]\n\t[Eiger] Transmission delay of first packet in an image "
        "being streamed out of the module's right UDP port. Each value "
        "represents 10ns. Typical value is 50000.");

    /* Receiver Config */

    INTEGER_COMMAND(
        rx_tcpport, getRxPort, setRxPort, StringTo<int>,
        "[port]\n\tTCP port for client-receiver communication. Default is "
        "1954. Must be different if multiple receivers on same pc. Must be "
        "first command to set a receiver parameter. Multi command will "
        "automatically increment for individual modules.");

    INTEGER_COMMAND(rx_fifodepth, getRxFifoDepth, setRxFifoDepth, StringTo<int>,
                    "[n_frames]\n\tSet the number of frames in the receiver "
                    "fifo depth (buffer between listener and writer threads).");

    INTEGER_COMMAND(
        rx_silent, getRxSilentMode, setRxSilentMode, StringTo<int>,
        "[0, 1]\n\tSwitch on or off receiver text output during acquisition.");

    INTEGER_COMMAND(
        rx_discardpolicy, getRxFrameDiscardPolicy, setRxFrameDiscardPolicy,
        sls::StringTo<slsDetectorDefs::frameDiscardPolicy>,
        "[nodiscard (default)|discardempty|discardpartial(fastest)]\n\tFrame "
        "discard policy of receiver. nodiscard does not discard frames, "
        "discardempty discards empty frames, discardpartial discards partial "
        "frames.");

    INTEGER_COMMAND(rx_padding, getPartialFramesPadding,
                    setPartialFramesPadding, StringTo<int>,
                    "[0, 1]\n\tPartial frames padding enable in the "
                    "receiver. Default: enabled. Disabling is fastest.");

    INTEGER_COMMAND(
        rx_udpsocksize, getRxUDPSocketBufferSize, setRxUDPSocketBufferSize,
        StringTo<int64_t>,
        "[n_size]\n\tUDP socket buffer size in receiver. Tune rmem_default and "
        "rmem_max accordingly.");

    GET_COMMAND(rx_realudpsocksize, getRxRealUDPSocketBufferSize,
                "\n\tActual udp socket buffer size. Double the size of "
                "rx_udpsocksize due to kernel bookkeeping.");

    INTEGER_COMMAND(rx_lock, getRxLock, setRxLock, StringTo<int>,
                    "[0, 1]\n\tLock receiver to one client IP, 1 locks, 0 "
                    "unlocks. Default is unlocked. 1: locks");

    GET_COMMAND(
        rx_lastclient, getRxLastClientIP,
        "\n\tClient IP Address that last communicated with the receiver.");

    GET_COMMAND(
        rx_threads, getRxThreadIds,
        "\n\tGet thread ids from the receiver in order of [parent, tcp, "
        "listener 0, "
        "processor 0, streamer 0, listener 1, processor 1, streamer 1]. If no "
        "streamer yet or there is no second interface, it gives 0 in its "
        "place.");

    /* File */

    INTEGER_COMMAND(
        fformat, getFileFormat, setFileFormat,
        sls::StringTo<slsDetectorDefs::fileFormat>,
        "[binary|hdf5]\n\tFile format of data file. For HDF5, package must be "
        "compiled with HDF5 flags. Default is binary.");

    STRING_COMMAND(
        fpath, getFilePath, setFilePath,
        "[path]\n\tDirectory where output data files are written in receiver. "
        "If path does not exist, it will try to create it.");

    STRING_COMMAND(fname, getFileNamePrefix, setFileNamePrefix,
                   "[name]\n\tFile name prefix for output data file. Default "
                   "is run. File name: [file name prefix]_d[detector "
                   "index]_f[sub file index]_[acquisition/file index].raw.");

    INTEGER_COMMAND(findex, getAcquisitionIndex, setAcquisitionIndex,
                    StringTo<int64_t>,
                    "[n_value]\n\tFile or Acquisition index.");

    INTEGER_COMMAND(
        fwrite, getFileWrite, setFileWrite, StringTo<int>,
        "[0, 1]\n\tEnable or disable receiver file write. Default is 1.");

    INTEGER_COMMAND_NOID(
        fmaster, getMasterFileWrite, setMasterFileWrite, StringTo<int>,
        "[0, 1]\n\tEnable or disable receiver master file. Default is 1.");

    INTEGER_COMMAND(
        foverwrite, getFileOverWrite, setFileOverWrite, StringTo<int>,
        "[0, 1]\n\tEnable or disable file overwriting. Default is 1.");

    INTEGER_COMMAND(rx_framesperfile, getFramesPerFile, setFramesPerFile,
                    StringTo<int>,
                    "[n_frames]\n\tNumber of frames per file in receiver. 0 is "
                    "infinite or all frames in single file.");

    /* ZMQ Streaming Parameters (Receiver<->Client) */

    INTEGER_COMMAND(
        rx_zmqstream, getRxZmqDataStream, setRxZmqDataStream, StringTo<int>,
        "[0, 1]\n\tEnable/ disable data streaming from receiver via zmq (eg. "
        "to GUI or to another process for further processing). This creates/ "
        "destroys zmq streamer threads in receiver. \n\tSwitching to Gui "
        "automatically enables data streaming in receiver. \n\tSwitching back "
        "to command line acquire will require disabling data streaming in "
        "receiver for fast applications. ");

    INTEGER_COMMAND(
        rx_zmqfreq, getRxZmqFrequency, setRxZmqFrequency, StringTo<int>,
        "[nth frame]\n\tFrequency of frames streamed out from receiver via "
        "zmq\n\tDefault: 1, Means every frame is streamed out. \n\tIf 2, every "
        "second frame is streamed out. \n\tIf 0, streaming timer is the "
        "timeout, after which current frame is sent out. (default timeout is "
        "200 ms). Usually used for gui purposes.");

    INTEGER_COMMAND(rx_zmqstartfnum, getRxZmqStartingFrame,
                    setRxZmqStartingFrame, StringTo<int>,
                    "[fnum]\n\tThe starting frame index to stream out. 0 by "
                    "default, which streams the first frame in an acquisition, "
                    "and then depending on the rx zmq frequency/ timer");

    INTEGER_COMMAND(
        rx_zmqport, getRxZmqPort, setRxZmqPort, StringTo<int>,
        "[port]\n\tZmq port for data to be streamed out of the receiver. Also "
        "restarts receiver zmq streaming if enabled. Default is 30001. "
        "Modified only when using an intermediate process between receiver and "
        "client(gui). Must be different for every detector (and udp port). "
        "Multi command will automatically increment for individual modules.");

    INTEGER_COMMAND(
        zmqport, getClientZmqPort, setClientZmqPort, StringTo<int>,
        "[port]\n\tZmq port in client(gui) or intermediate process for data to "
        "be streamed to from receiver. efault connects to receiver zmq "
        "streaming out port (30001). Modified only when using an intermediate "
        "process between receiver and client(gui). Also restarts client zmq "
        "streaming if enabled. Must be different for every detector (and udp "
        "port). Multi command will automatically increment for individual "
        "modules.");

    INTEGER_COMMAND(
        rx_zmqip, getRxZmqIP, setRxZmqIP, IpAddr,
        "[x.x.x.x]\n\tZmq Ip Address from which data is to be streamed out of "
        "the receiver. Also restarts receiver zmq streaming if enabled. "
        "Default is from rx_hostname. Modified only when using an intermediate "
        "process between receiver.");

    INTEGER_COMMAND(
        zmqip, getClientZmqIp, setClientZmqIp, IpAddr,
        "[x.x.x.x]\n\tZmq IP Address in client(gui) or intermediate process "
        "for data to be streamed to from receiver.  Default connects to "
        "receiver zmq Ip Address (from rx_hostname). Modified only when using "
        "an intermediate process between receiver and client(gui). Also "
        "restarts client zmq streaming if enabled.");

    /* Eiger Specific */

    TIME_COMMAND(subexptime, getSubExptime, setSubExptime,
                 "[duration] [(optional unit) ns|us|ms|s]\n\t[Eiger] Exposure "
                 "time of EIGER subframes in 32 bit mode.");

    TIME_COMMAND(subdeadtime, getSubDeadTime, setSubDeadTime,
                 "[duration] [(optional unit) ns|us|ms|s]\n\t[Eiger] Dead time "
                 "of EIGER subframes in 32 bit mode. Subperiod = subexptime + "
                 "subdeadtime.");

    STRING_COMMAND(
        settingspath, getSettingsPath, setSettingsPath,
        "[path]\n\t[Eiger] Directory where settings files are loaded from/to.");

    INTEGER_COMMAND(overflow, getOverFlowMode, setOverFlowMode, StringTo<int>,
                    "[0, 1]\n\t[Eiger] Enable or disable show overflow flag in "
                    "32 bit mode. Default is disabled.");

    INTEGER_COMMAND(
        flippeddatax, getBottom, setBottom, StringTo<int>,
        "[0, 1]\n\t[Eiger] Top or Bottom Half of Eiger module. 1 is bottom, 0 "
        "is top. Used to let Receivers and Gui know to flip the bottom image "
        "over the x axis. Files are not written without the flip however.");

    INTEGER_COMMAND(
        readnlines, getPartialReadout, setPartialReadout, StringTo<int>,
        "[1 - 256]\n\t[Eiger] Number of rows to readout per half module "
        "starting from the centre. 256 is default. The permissible values "
        "depend on dynamic range and 10Gbe enabled.");

    INTEGER_COMMAND(interruptsubframe, getInterruptSubframe,
                    setInterruptSubframe, StringTo<int>,
                    "[0, 1]\n\t[Eiger] 1 interrupts last subframe at required "
                    "exposure time. 0 will wait for last sub frame to finish "
                    "exposing. 0 is default.");

    TIME_GET_COMMAND(measuredperiod, getMeasuredPeriod,
                     "[(optional unit) ns|us|ms|s]\n\t[Eiger] Measured frame "
                     "period between last frame and previous one. Can be "
                     "measured with minimum 2 frames in an acquisition.");

    TIME_GET_COMMAND(measuredsubperiod, getMeasuredSubFramePeriod,
                     "[(optional unit) ns|us|ms|s]\n\t[Eiger] Measured sub "
                     "frame period between last sub frame and previous one.");

    INTEGER_COMMAND(
        partialreset, getPartialReset, setPartialReset, StringTo<int>,
        "[0, 1]\n\t[Eiger] Sets up detector to do partial or complete reset at "
        "start of acquisition. 0 complete reset, 1 partial reset. Default is "
        "complete reset. Advanced function!");

    /* Jungfrau Specific */

    INTEGER_COMMAND(
        temp_threshold, getThresholdTemperature, setThresholdTemperature,
        StringTo<int>,
        "[n_temp (in degrees)]\n\t[Jungfrau] Threshold temperature in degrees. "
        "If temperature crosses threshold temperature and temperature control "
        "is enabled, power to chip will be switched off and temperature event "
        "occurs. To power on chip again, temperature has to be less than "
        "threshold temperature and temperature event has to be cleared.");

    INTEGER_COMMAND(
        temp_control, getTemperatureControl, setTemperatureControl,
        StringTo<int>,
        "[0, 1]\n\t[Jungfrau] Temperature control enable. Default is 0 "
        "(disabled). If temperature crosses threshold temperature and "
        "temperature control is enabled, power to chip will be switched off "
        "and temperature event occurs. To power on chip again, temperature has "
        "to be less than threshold temperature and temperature event has to be "
        "cleared.");

    INTEGER_COMMAND(
        auto_comp_disable, getAutoCompDisable, setAutoCompDisable,
        StringTo<int>,
        "[0, 1]\n\t[Jungfrau] Auto comparator disable mode. By default, the "
        "on-chip gain switching is active during the entire exposure.This mode "
        "disables the on - chip gain switching comparator automatically after "
        "93.75% of exposure time (only for longer than 100us). \n\tDefault is "
        "0 or this mode disabled(comparator enabled throughout). 1 enables "
        "mode. 0 disables mode. ");

    INTEGER_COMMAND_NOID(
        storagecells, getNumberOfAdditionalStorageCells,
        setNumberOfAdditionalStorageCells, StringTo<int>,
        "[0-15]\n\t[Jungfrau] Number of additional storage cells. Default is "
        "0. For advanced users only. \n\tThe #images = #frames x #triggers x "
        "(#storagecells + 1).");

    INTEGER_COMMAND(
        storagecell_start, getStorageCellStart, setStorageCellStart,
        StringTo<int>,
        "[0-15]\n\t[Jungfrau] Storage cell that stores the first acquisition "
        "of the series. Default is 15. For advanced users only.");

    TIME_COMMAND(
        storagecell_delay, getStorageCellDelay, setStorageCellDelay,
        "[duration (0-1638375 ns)] [(optional unit) ns|us|ms|s]\n\t[Jungfrau] "
        "Additional time delay between 2 consecutive exposures in burst mode "
        "(resolution of 25ns). For advanced users only.");

    /* Gotthard Specific */
    TIME_GET_COMMAND(exptimel, getExptimeLeft,
                     "[(optional unit) ns|us|ms|s]\n\t[Gotthard] Exposure time "
                     "left for current frame. ");

    /* Gotthard2 Specific */
    INTEGER_COMMAND_NOID(
        bursts, getNumberOfBursts, setNumberOfBursts, StringTo<int64_t>,
        "[n_bursts]\n\t[Gotthard2] Number of bursts per aquire. Only in auto "
        "timing mode and burst mode. Use timing command to set timing mode and "
        "burstmode command to set burst mode.");

    TIME_COMMAND(burstperiod, getBurstPeriod, setBurstPeriod,
                 "[duration] [(optional unit) ns|us|ms|s]\n\t[Gotthard2] Burst "
                 "period. Only in burst mode and auto timing mode.");

    INTEGER_COMMAND(cdsgain, getCDSGain, setCDSGain, StringTo<bool>,
                    "[0, 1]\n\t[Gotthard2] Enable or disable CDS gain. Default "
                    "is disabled.");

    INTEGER_COMMAND(
        filter, getFilter, setFilter, StringTo<int>,
        "[0|1|2|3]\n\t[Gotthard2] Set filter resistor. Default is 0.");

    INTEGER_COMMAND(currentsource, getCurrentSource, setCurrentSource,
                    StringTo<int>,
                    "[0, 1]\n\t[Gotthard2] Enable or disable current source. "
                    "Default is disabled.");

    INTEGER_COMMAND(
        timingsource, getTimingSource, setTimingSource,
        sls::StringTo<slsDetectorDefs::timingSourceType>,
        "[internal|external]\n\t[Gotthard2] Timing source. Internal is crystal "
        "and external is system timing. Default is internal.");

    INTEGER_COMMAND(veto, getVeto, setVeto, StringTo<int>,
                    "[0, 1]\n\t[Gotthard2] Enable or disable veto data "
                    "streaming from detector. Default is 0.");

    /* Mythen3 Specific */

    INTEGER_COMMAND(gates, getNumberOfGates, setNumberOfGates, StringTo<int>,
                    "[n_gates]\n\t[Mythen3] Number of external gates in gating "
                    "or trigger_gating mode (external gating).");

    /* CTB/ Moench Specific */

    INTEGER_COMMAND(
        asamples, getNumberOfAnalogSamples, setNumberOfAnalogSamples,
        StringTo<int>,
        "[n_samples]\n\t[CTB][Moench] Number of analog samples expected.");

    INTEGER_COMMAND(
        adcclk, getADCClock, setADCClock, StringTo<int>,
        "[n_clk in MHz]\n\t[Ctb][Moench] ADC clock frequency in MHz.");

    INTEGER_COMMAND(runclk, getRUNClock, setRUNClock, StringTo<int>,
                    "[n_clk in MHz]\n\t[Ctb][Moench] Run clock in MHz.");

    GET_COMMAND(syncclk, getSYNCClock,
                "[n_clk in MHz]\n\t[Ctb][Moench] Sync clock in MHz.");

    INTEGER_COMMAND(adcpipeline, getADCPipeline, setADCPipeline, StringTo<int>,
                    "[n_value]\n\t[Ctb][Moench] Pipeline for ADC clock.");

    INTEGER_IND_COMMAND(v_limit, getVoltage, setVoltage, StringTo<int>,
                        defs::V_LIMIT,
                        "[n_value]\n\t[Ctb][Moench] Soft limit for power "
                        "supplies(ctb only) and DACS in mV.");

    INTEGER_COMMAND_HEX(adcenable, getADCEnableMask, setADCEnableMask,
                        StringTo<uint32_t>,
                        "[bitmask]\n\t[Ctb][Moench] ADC Enable Mask for 1Gb "
                        "Mode for each 32 ADC channel.");

    INTEGER_COMMAND_HEX(
        adcenable10g, getTenGigaADCEnableMask, setTenGigaADCEnableMask,
        StringTo<uint32_t>,
        "[bitmask]\n\t[Ctb][Moench] ADC Enable Mask for 10Gb mode for each 32 "
        "ADC channel. However, if any of consecutive 4 bits are enabled, the "
        "complete 4 bits are enabled.");

    /* CTB Specific */

    INTEGER_COMMAND(dsamples, getNumberOfDigitalSamples,
                    setNumberOfDigitalSamples, StringTo<int>,
                    "[n_value]\n\t[CTB] Number of digital samples expected.");

    INTEGER_COMMAND(romode, getReadoutMode, setReadoutMode,
                    sls::StringTo<slsDetectorDefs::readoutMode>,
                    "[analog|digital|analog_digital]\n\t[CTB] Readout mode. "
                    "Default is analog.");

    INTEGER_COMMAND(
        dbitclk, getDBITClock, setDBITClock, StringTo<int>,
        "[n_clk in MHz]\n\t[Ctb] Clock for latching the digital bits in MHz.");

    INTEGER_COMMAND(
        dbitpipeline, getDBITPipeline, setDBITPipeline, StringTo<int>,
        "[n_value]\n\t[Ctb] Pipeline of the clock for latching digital bits.");

    INTEGER_IND_COMMAND(v_a, getVoltage, setVoltage, StringTo<int>,
                        defs::V_POWER_A,
                        "[n_value]\n\t[Ctb] Voltage supply a in mV.");

    INTEGER_IND_COMMAND(v_b, getVoltage, setVoltage, StringTo<int>,
                        defs::V_POWER_B,
                        "[n_value]\n\t[Ctb] Voltage supply b in mV.");

    INTEGER_IND_COMMAND(v_c, getVoltage, setVoltage, StringTo<int>,
                        defs::V_POWER_C,
                        "[n_value]\n\t[Ctb] Voltage supply c in mV.");

    INTEGER_IND_COMMAND(v_d, getVoltage, setVoltage, StringTo<int>,
                        defs::V_POWER_D,
                        "[n_value]\n\t[Ctb] Voltage supply d in mV.");

    INTEGER_IND_COMMAND(
        v_io, getVoltage, setVoltage, StringTo<int>, defs::V_POWER_IO,
        "[n_value]\n\t[Ctb] Voltage supply io in mV. Minimum 1200 mV. Must be "
        "the first power regulator to be set after fpga reset (on-board "
        "detector server start up).");

    INTEGER_IND_COMMAND(
        v_chip, getVoltage, setVoltage, StringTo<int>, defs::V_POWER_CHIP,
        "[n_value]\n\t[Ctb] Voltage supply chip in mV. Do not use it unless "
        "you are completely sure you will not fry the board.");

    GET_IND_COMMAND(vm_a, getMeasuredVoltage, defs::V_POWER_A, "",
                    "\n\t[Ctb] Measured voltage of power supply a in mV.");

    GET_IND_COMMAND(vm_b, getMeasuredVoltage, defs::V_POWER_B, "",
                    "\n\t[Ctb] Measured voltage of power supply b in mV.");

    GET_IND_COMMAND(vm_c, getMeasuredVoltage, defs::V_POWER_C, "",
                    "\n\t[Ctb] Measured voltage of power supply c in mV.");

    GET_IND_COMMAND(vm_d, getMeasuredVoltage, defs::V_POWER_D, "",
                    "\n\t[Ctb] Measured voltage of power supply d in mV.");

    GET_IND_COMMAND(vm_io, getMeasuredVoltage, defs::V_POWER_IO, "",
                    "\n\t[Ctb] Measured voltage of power supply io in mV.");

    GET_IND_COMMAND(im_a, getMeasuredCurrent, defs::I_POWER_A, "",
                    "\n\t[Ctb] Measured current of power supply a in mA.");

    GET_IND_COMMAND(im_b, getMeasuredCurrent, defs::I_POWER_B, "",
                    "\n\t[Ctb] Measured current of power supply b in mA.");

    GET_IND_COMMAND(im_c, getMeasuredCurrent, defs::I_POWER_C, "",
                    "\n\t[Ctb] Measured current of power supply c in mA.");

    GET_IND_COMMAND(im_d, getMeasuredCurrent, defs::I_POWER_D, "",
                    "\n\t[Ctb] Measured current of power supply d in mA.");

    GET_IND_COMMAND(im_io, getMeasuredCurrent, defs::I_POWER_IO, "",
                    "\n\t[Ctb] Measured current of power supply io in mA.");

    INTEGER_COMMAND(
        extsampling, getExternalSampling, setExternalSampling, StringTo<int>,
        "[0, 1]\n\t[Ctb] Enable for external sampling signal to extsamplingsrc "
        "signal for digital data. For advanced users only.");

    INTEGER_COMMAND(extsamplingsrc, getExternalSamplingSource,
                    setExternalSamplingSource, StringTo<int>,
                    "[0-63]\n\t[Ctb] Sampling source signal for digital data. "
                    "For advanced users only.");

    INTEGER_COMMAND(rx_dbitoffset, getRxDbitOffset, setRxDbitOffset,
                    StringTo<int>,
                    "[n_bytes]\n\t[Ctb] Offset in bytes in digital data to "
                    "skip in receiver.");

    INTEGER_COMMAND(led, getLEDEnable, setLEDEnable, StringTo<int>,
                    "[0, 1]\n\t[Ctb] Switches on/off all LEDs.");

    /* Pattern */

    EXECUTE_SET_COMMAND_NOID_1ARG(
        savepattern, savePattern,
        "[fname]\n\t[Ctb][Moench][Mythen3] Saves pattern to file (ascii). Also "
        "executes pattern.");

    INTEGER_COMMAND_HEX_WIDTH16(patioctrl, getPatternIOControl,
                                setPatternIOControl, StringTo<uint64_t>,
                                "[64 bit mask]\n\t[Ctb][Moench] 64 bit mask "
                                "defining input (0) and output (1) signals.");

    INTEGER_COMMAND_HEX_WIDTH16(
        patmask, getPatternMask, setPatternMask, StringTo<uint64_t>,
        "[64 bit mask]\n\t[Ctb][Moench][Mythen3] Sets the mask applied to "
        "every pattern to the selected bits.");

    INTEGER_COMMAND_HEX_WIDTH16(
        patsetbit, getPatternBitMask, setPatternBitMask, StringTo<uint64_t>,
        "[64 bit mask]\n\t[Ctb][Moench][Mythen3] Selects the bits that will "
        "have a pattern mask applied to the selected patmask for every "
        "pattern.");

    EXECUTE_SET_COMMAND(patternstart, startPattern,
                        "\n\t[Mythen3] Starts Pattern");

    /* Moench */
    /* Advanced */

    EXECUTE_SET_COMMAND(resetfpga, resetFPGA,
                        "\n\t[Jungfrau][Ctb][Moench] Reset FPGA.");

    EXECUTE_SET_COMMAND(rebootcontroller, rebootController,
                        "\n\t[Jungfrau][Ctb][Moench][Gotthard][Mythen3]["
                        "Gotthard2] Reboot controler (blackfin) of detector.");

    EXECUTE_SET_COMMAND(
        firmwaretest, executeFirmwareTest,
        "\n\t[Jungfrau][Gotthard][Mythen3][Gotthard2][Ctb][Moench] Firmware "
        "test, ie. reads a read fixed pattern from a register.");

    EXECUTE_SET_COMMAND(
        bustest, executeBusTest,
        "\n\t[Jungfrau][Gotthard][Mythen3][Gotthard2][Ctb][Moench] Bus test, "
        "ie. keeps writing and reading back different values in R/W register.");

    INTEGER_COMMAND_HEX(
        adcinvert, getADCInvert, setADCInvert, StringTo<uint32_t>,
        "[bitmask]\n\t[Ctb][Moench][Jungfrau] ADC Inversion "
        "Mask.\n\t[Jungfrau][Moench] Inversions on top of the default mask.");

    /* Insignificant */

    INTEGER_COMMAND(
        port, getControlPort, setControlPort, StringTo<int>,
        "[n]\n\tPort number of the control server on detector for "
        "detector-client tcp interface. Default is 1952. Normally unchanged.");

    INTEGER_COMMAND(
        stopport, getStopPort, setStopPort, StringTo<int>,
        "[n]\n\tPort number of the stop server on detector for detector-client "
        "tcp interface. Default is 1953. Normally unchanged.");

    INTEGER_COMMAND(lock, getDetectorLock, setDetectorLock, StringTo<int>,
                    "[0, 1]\n\tLock detector to one IP, 1: locks");

    GET_COMMAND(
        lastclient, getLastClientIP,
        "\n\tClient IP Address that last communicated with the detector.");

    GET_COMMAND(nframes, getNumberOfFramesFromStart,
                "\n\t[Jungfrau][Mythen3][Gotthard2][Moench][CTB] "
                "Number of frames from start run control."
                "\n\t[Gotthard2] only in continuous mode.");

    TIME_GET_COMMAND(now, getActualTime,
                     "[(optional unit) "
                     "ns|us|ms|s]\n\t[Jungfrau][Mythen3][Gotthard2][Moench]["
                     "CTB] Time from detector start up."
                     "\n\t[Gotthard2] only in continuous mode.");

    TIME_GET_COMMAND(timestamp, getMeasurementTime,
                     "[(optional unit) "
                     "ns|us|ms|s]\n\t[Jungfrau][Mythen3][Gotthard2][Moench]["
                     "CTB] Timestamp at a frame start."
                     "\n\t[Gotthard2] only in continuous mode.");

    GET_COMMAND(
        rx_frameindex, getRxCurrentFrameIndex,
        "\n\tCurrent frame index received in receiver during acquisition.");
};

} // namespace sls
