#pragma once

#include "Detector.h"
#include "Result.h"
#include "sls_detector_exceptions.h"
#include "network_utils.h"
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
            if (args.size() == 0) {                                            \
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
            if (args.size() == 0) {                                            \
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
            auto t = det->GETFCN({det_id});                                    \
            if (args.size() != 0) {                                            \
                WrongNumberOfParameters(0);                                    \
            }                                                                  \
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


/** int or enum */
#define INTEGER_COMMAND_HEX(CMDNAME, GETFCN, SETFCN, CONV, HLPSTR)             \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            auto t = det->GETFCN({det_id});                                    \
            if (args.size() != 0) {                                            \
                WrongNumberOfParameters(0);                                    \
            }                                                                  \
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

/** int or enum  hex val */
#define INTEGER_COMMAND(CMDNAME, GETFCN, SETFCN, CONV, HLPSTR)                 \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            auto t = det->GETFCN({det_id});                                    \
            if (args.size() != 0) {                                            \
                WrongNumberOfParameters(0);                                    \
            }                                                                  \
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
            auto t = det->GETFCN();                                            \
            if (args.size() != 0) {                                            \
                WrongNumberOfParameters(0);                                    \
            }                                                                  \
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

/** int with index, */
#define INTEGER_IND_COMMAND(CMDNAME, GETFCN, SETFCN, CONV, INDEX, HLPSTR)      \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            auto t = det->GETFCN(INDEX, {det_id});                             \
            if (args.size() != 0) {                                            \
                WrongNumberOfParameters(0);                                    \
            }                                                                  \
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
                if (args[0] != "mv") {                                         \
                    throw sls::RuntimeError("Unknown argument " + args[0] + ". Did you mean mv?"); \
                }                                                              \
                mv = true;                                                     \
            } else if (args.size() > 1) {                                      \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
            auto t = det->GETFCN(DAC_INDEX, mv, {det_id});                     \
            os << OutString(t) << (args.size() > 1 ? " mv\n" : "\n");          \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            bool mv = false;                                                   \
            if (args.size() == 2) {                                            \
                if (args[1] != "mv") {                                         \
                    throw sls::RuntimeError("Unknown argument " + args[1] + ". Did you mean mv?"); \
                }                                                              \
                mv = true;                                                     \
            } else if (args.size() > 2 || args.size() < 1) {                   \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
            det->SETFCN(DAC_INDEX, std::stoi(args[0]), mv, {det_id});          \
            os << args.front() << (args.size() > 1 ? " mv\n" : "\n");          \
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
            if (args.size() != 0) {                                            \
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
            if (args.size() != 0) {                                            \
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

/** get only */
#define GET_COMMAND(CMDNAME, GETFCN, HLPSTR)                                   \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            if (args.size() != 0) {                                            \
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

/** get only hex*/
#define GET_COMMAND_HEX(CMDNAME, GETFCN, HLPSTR)                               \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            if (args.size() != 0) {                                            \
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
            if (args.size() != 0) {                                            \
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

    std::string Call(const std::string &command,
                     const std::vector<std::string> &arguments, int detector_id,
                     int action = -1, std::ostream &os = std::cout);

    bool ReplaceIfDepreciated(std::string &command);
    size_t GetFunctionMapSize() const noexcept { return functions.size(); };
    std::vector<std::string> GetAllCommands();
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

    inline unsigned int stoui(const std::string& s) {
        unsigned long lresult = stoul(s, 0, 10);
        unsigned int result = lresult;
        if (result != lresult) throw std::out_of_range("cannot convert to unsigned int");
        return result;
    }

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
                                    {"r_readfreq", "rx_readfreq"},

                                    /* Eiger Specific */
                                    {"trimdir", "settingspath"},
                                    {"settingsdir", "settingspath"},
                                    {"resmat", "partialreset"},

                                    /* Jungfrau Specific */
                                    /* Gotthard Specific */
                                    {"digitest", "imagetest"},

                                    /* Gotthard2 Specific */
                                    /* CTB Specific */
                                    {"flags", "romode"},
                                    {"i_a", "im_a"},
                                    {"i_b", "im_b"},
                                    {"i_c", "im_c"},
                                    {"i_d", "im_d"},
                                    {"i_io", "im_io"}

                                    /* Pattern */
                                   

                                    };

    // Initialize maps for translating name and function
    FunctionMap functions{{"list", &CmdProxy::ListCommands},
                         
                          /* configuration */
                          //{"config", &CmdProxy::config},
                          {"parameters", &CmdProxy::parameters},
                          {"hostname", &CmdProxy::Hostname},
                          {"versions", &CmdProxy::Versions},
                          {"packageversion", &CmdProxy::PackageVersion},
                          {"clientversion", &CmdProxy::ClientVersion},
                          {"firmwareversion", &CmdProxy::FirmwareVersion},
                          {"detectorserverversion", &CmdProxy::detectorserverversion},
                          {"rx_version", &CmdProxy::rx_version},
                          {"detectornumber", &CmdProxy::detectornumber},
                          {"type", &CmdProxy::type},
                          {"detsize", &CmdProxy::DetectorSize},
                          {"settings", &CmdProxy::settings},

                          /* acquisition parameters */
                          {"frames", &CmdProxy::frames},                          
                          {"triggers", &CmdProxy::triggers},
                          {"exptime", &CmdProxy::exptime},
                          {"period", &CmdProxy::period},
                          {"delay", &CmdProxy::delay},
                          {"delay", &CmdProxy::delay},
                          {"framesl", &CmdProxy::framesl},
                          {"triggersl", &CmdProxy::triggersl},
                          {"delayl", &CmdProxy::delayl},
                          {"speed", &CmdProxy::Speed},
                          {"adcphase", &CmdProxy::Adcphase},
                          {"maxadcphaseshift", &CmdProxy::maxadcphaseshift},
                          {"clkfreq", &CmdProxy::ClockFrequency},
                          {"clkphase", &CmdProxy::ClockPhase},
                          {"maxclkphaseshift", &CmdProxy::MaxClockPhaseShift},
                          {"clkdiv", &CmdProxy::ClockDivider},                           
                          {"vhighvoltage", &CmdProxy::vhighvoltage},
                          {"temp_adc", &CmdProxy::temp_adc},
                          {"temp_fpga", &CmdProxy::temp_fpga},
                          {"temp_fpgaext", &CmdProxy::temp_fpgaext},
                          {"temp_10ge", &CmdProxy::temp_10ge},
                          {"temp_dcdc", &CmdProxy::temp_dcdc},
                          {"temp_sodl", &CmdProxy::temp_sodl},
                          {"temp_sodr", &CmdProxy::temp_sodr},
                          {"temp_fpgafl", &CmdProxy::temp_fpgafl},
                          {"temp_fpgafr", &CmdProxy::temp_fpgafr},
                          // dacs
                          {"timing", &CmdProxy::timing},

                          /* acquisition */
                          {"clearbusy", &CmdProxy::clearbusy}, 
                          {"rx_start", &CmdProxy::rx_start},
                          {"rx_stop", &CmdProxy::rx_stop},
                          {"start", &CmdProxy::start},
                          {"stop", &CmdProxy::stop},
                          {"rx_status", &CmdProxy::rx_status}, 
                          {"rx_framescaught", &CmdProxy::rx_framescaught},
                          {"startingfnum", &CmdProxy::startingfnum},
                          {"trigger", &CmdProxy::trigger},

                          /* Network Configuration (Detector<->Receiver) */
                          {"numinterfaces", &CmdProxy::numinterfaces},
                          {"selinterface", &CmdProxy::selinterface},
                          {"udp_srcip", &CmdProxy::udp_srcip},
                          {"udp_srcip2", &CmdProxy::udp_srcip2},
                          {"udp_dstip", &CmdProxy::udp_dstip},
                          {"udp_dstip2", &CmdProxy::udp_dstip2},
                          {"udp_srcmac", &CmdProxy::udp_srcmac},
                          {"udp_srcmac2", &CmdProxy::udp_srcmac2},
                          {"udp_dstmac", &CmdProxy::udp_dstmac},
                          {"udp_dstmac2", &CmdProxy::udp_dstmac2},
                          {"udp_dstport", &CmdProxy::udp_dstport},
                          {"udp_dstport2", &CmdProxy::udp_dstport2},
                          {"rx_printconfig", &CmdProxy::rx_printconfig}, 
                          {"tengiga", &CmdProxy::tengiga},                          
                          {"flowcontrol_10g", &CmdProxy::flowcontrol_10g}, 
                          {"txndelay_frame", &CmdProxy::txndelay_frame}, 
                          {"txndelay_left", &CmdProxy::txndelay_left},
                          {"txndelay_right", &CmdProxy::txndelay_right},

                          /* Receiver Config */ 
                          {"rx_hostname", &CmdProxy::rx_hostname}, 
                          {"rx_tcpport", &CmdProxy::rx_tcpport},  
                          {"rx_fifodepth", &CmdProxy::rx_fifodepth},
                          {"rx_silent", &CmdProxy::rx_silent},
                          {"rx_discardpolicy", &CmdProxy::rx_discardpolicy},
                          {"rx_padding", &CmdProxy::rx_padding},
                          {"rx_udpsocksize", &CmdProxy::rx_udpsocksize},
                          {"rx_realudpsocksize", &CmdProxy::rx_realudpsocksize},
                          {"rx_lock", &CmdProxy::rx_lock},
                          {"rx_lastclient", &CmdProxy::rx_lastclient},

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
                          {"rx_datastream", &CmdProxy::rx_datastream},
                          {"rx_readfreq", &CmdProxy::rx_readfreq},
                          {"rx_zmqport", &CmdProxy::rx_zmqport},
                          {"zmqport", &CmdProxy::zmqport},
                          {"rx_zmqip", &CmdProxy::rx_zmqip},
                          {"zmqip", &CmdProxy::zmqip},

                          /* Eiger Specific */
                          {"dr", &CmdProxy::DynamicRange},                          
                          {"subexptime", &CmdProxy::subexptime}, 
                          {"subdeadtime", &CmdProxy::subdeadtime},
                          {"threshold", &CmdProxy::Threshold},
                          {"thresholdnotb", &CmdProxy::ThresholdNoTb}, 
                          {"settingspath", &CmdProxy::settingspath},
                          {"trimbits", &CmdProxy::trimbits},
                          {"gappixels", &CmdProxy::GapPixels},
                          {"parallel", &CmdProxy::parallel},
                          {"overflow", &CmdProxy::overflow},
                          {"storeinram", &CmdProxy::storeinram},
                          {"flippeddatax", &CmdProxy::flippeddatax},
                          {"trimval", &CmdProxy::trimval},
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
                          {"powerchip", &CmdProxy::powerchip}, 
                          {"auto_comp_disable", &CmdProxy::auto_comp_disable}, 
                          {"storagecells", &CmdProxy::storagecells}, 
                          {"storagecell_start", &CmdProxy::storagecell_start},
                          {"storagecell_delay", &CmdProxy::storagecell_delay},

                          /* Gotthard Specific */
                          {"roi", &CmdProxy::ROI},
                          {"clearroi", &CmdProxy::ClearROI},
                          {"exptimel", &CmdProxy::exptimel},
                          {"periodl", &CmdProxy::periodl},
                          {"extsig", &CmdProxy::extsig},
                          {"imagetest", &CmdProxy::imagetest},

                          /* Gotthard2 Specific */      
                          /* CTB Specific */
                          {"samples", &CmdProxy::Samples},
                          {"asamples", &CmdProxy::asamples},
                          {"dsamples", &CmdProxy::dsamples},
                          {"romode", &CmdProxy::romode},
                          {"dbitphase", &CmdProxy::Dbitphase},
                          {"maxdbitphaseshift", &CmdProxy::maxdbitphaseshift},
                          {"adcclk", &CmdProxy::adcclk},  
                          {"dbitclk", &CmdProxy::dbitclk},  
                          {"runclk", &CmdProxy::runclk},  
                          {"syncclk", &CmdProxy::syncclk},  
                          {"adcpipeline", &CmdProxy::adcpipeline},
                          {"dbitpipeline", &CmdProxy::dbitpipeline},
                          {"v_limit", &CmdProxy::v_limit},
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
                          {"adc", &CmdProxy::SlowAdc},  
                          {"adcenable", &CmdProxy::adcenable}, 
                          {"adcinvert", &CmdProxy::adcinvert}, 
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
                          {"patclkctrl", &CmdProxy::patclkctrl}, 
                          {"patword", &CmdProxy::PatternWord}, 



                          {"adcvpp", &CmdProxy::adcvpp},
                          {"lastclient", &CmdProxy::lastclient},    
                          {"lock", &CmdProxy::lock}                
                          };


    void WrongNumberOfParameters(size_t expected);

    /* Commands */
    std::string ListCommands(int action);
    /* configuration */
    std::string Hostname(int action); 
    std::string FirmwareVersion(int action);     
    std::string Versions(int action); 
    std::string PackageVersion(int action);     
    std::string ClientVersion(int action);
    std::string DetectorSize(int action);
    /* acquisition parameters */
    std::string Speed(int action);
    std::string Adcphase(int action);
    std::string ClockFrequency(int action);
    std::string ClockPhase(int action);
    std::string MaxClockPhaseShift(int action);
    std::string ClockDivider(int action);
    /* acquisition */
    /* Network Configuration (Detector<->Receiver) */
    /* Receiver Config */
    /* File */
    /* ZMQ Streaming Parameters (Receiver<->Client) */
    /* Eiger Specific */
    std::string DynamicRange(int action);
    std::string Threshold(int action);
    std::string ThresholdNoTb(int action);  
    std::string GapPixels(int action);
    std::string TrimEnergies(int action);
    std::string RateCorrection(int action);
    std::string Activate(int action);
    std::string PulsePixel(int action);
    std::string PulsePixelAndMove(int action);
    std::string PulseChip(int action);
    std::string Quad(int action);
    /* Jungfrau Specific */
    std::string TemperatureEvent(int action);
    std::string PowerChip(int action);
    /* Gotthard Specific */
    std::string ROI(int action);
    std::string ClearROI(int action);
    /* Gotthard2 Specific */
    /* CTB Specific */
    std::string Samples(int action);
    std::string Dbitphase(int action);
    std::string SlowAdc(int action);
    std::string ReceiverDbitList(int action);
    std::string DigitalIODelay(int action);
    /* Pattern */
    std::string Pattern(int action);
    std::string PatternWord(int action);

    /* configuration */
    EXECUTE_SET_COMMAND_NOID_1ARG(config, loadConfig, 
                "[fname]\n\tConfigures detector to configuration contained in fname. Set up once.");  

    EXECUTE_SET_COMMAND_NOID_1ARG(parameters, loadParameters, 
                "[fname]\n\tSets detector measurement parameters to those contained in fname. Set up per measurement.");  
    
    GET_COMMAND_HEX(detectorserverversion, getDetectorServerVersion, 
                "\n\tOn-board detector server software version in format [0xYYMMDD].");   

    GET_COMMAND_HEX(rx_version, getReceiverVersion, 
                "\n\tReceiver version in format [0xYYMMDD].");   

    GET_COMMAND_HEX(detectornumber, getSerialNumber, 
                "\n\tReceiver version in format [0xYYMMDD].");   

    GET_COMMAND(type, getDetectorType, 
                "\n\tSerial number or MAC of detector (hex).");   

    INTEGER_COMMAND(settings, getSettings, setSettings, sls::StringTo<slsDetectorDefs::detectorSettings>,
                    "[standard, fast, highgain, dynamicgain, lowgain, mediumgain, veryhighgain, dynamichg0, fixgain1, fixgain2, forceswitchg1, forceswitchg2]\n\t[Jungfrau][Gotthard] Detector Settings.\n\t[Eiger] Use threshold or thresholdnotb.");      

    /* acquisition parameters */

    INTEGER_COMMAND_NOID(frames, getNumberOfFrames, setNumberOfFrames,
                         std::stol,
                         "[n_frames]\n\tNumber of frames per aquire. In trigger mode, number of frames per trigger.");

    INTEGER_COMMAND_NOID(triggers, getNumberOfTriggers, setNumberOfTriggers,
                         std::stol,
                         "[n_triggers]\n\tNumber of triggers per aquire. Use timing command to set timing mode.");

    TIME_COMMAND(exptime, getExptime, setExptime,
        "[duration] [(optional unit) ns|us|ms|s]\n\tExposure time");

    TIME_COMMAND(period, getPeriod, setPeriod,
                 "[duration] [(optional unit) ns|us|ms|s]\n\tPeriod between frames");

    TIME_COMMAND(delay, getDelayAfterTrigger, setDelayAfterTrigger,
                 "[duration] [(optional unit) ns|us|ms|s]\n\t[Jungfrau][Gotthard][Ctb] Delay after trigger");

    GET_COMMAND(framesl, getNumberOfFramesLeft, 
                "\n\t[Gotthard][Jungfrau][CTB] Number of frames left in acquisition.");       

    GET_COMMAND(triggersl, getNumberOfTriggersLeft, 
                "\n\t[Gotthard][Jungfrau][CTB] Number of triggers left in acquisition.");       

    TIME_GET_COMMAND(delayl, getDelayAfterTriggerLeft, 
                "[(optional unit) ns|us|ms|s]\n\t[Gotthard][Jungfrau][CTB] DelayLeft Delay Left in Acquisition.");    
                
    GET_COMMAND(maxadcphaseshift, getMaxADCPhaseShift, 
                "\n\t[Jungfrau][CTB] Absolute maximum Phase shift of ADC clock.");  

    INTEGER_COMMAND(vhighvoltage, getHighVoltage, setHighVoltage, std::stoi,
                    "[n_value]\n\tHigh voltage to the sensor in Voltage.\n\t[Gotthard] [0|90|110|120|150|180|200]\n\t[Eiger] 0-200\n\t[Jungfrau][Ctb] [0|60-200]");      

    GET_IND_COMMAND(temp_adc, getTemperature, slsDetectorDefs::TEMPERATURE_ADC, " °C",
                    "[n_value]\n\t[Jungfrau][Gotthard] ADC Temperature");

    GET_IND_COMMAND(temp_fpga, getTemperature, slsDetectorDefs::TEMPERATURE_FPGA, " °C",
                    "[n_value]\n\t[Eiger][Jungfrau][Gotthard] FPGA Temperature");    

    GET_IND_COMMAND(temp_fpgaext, getTemperature, slsDetectorDefs::TEMPERATURE_FPGAEXT, " °C",
                    "[n_value]\n\t[Eiger]Temperature close to the FPGA");

    GET_IND_COMMAND(temp_10ge, getTemperature, slsDetectorDefs::TEMPERATURE_10GE, " °C",
                    "[n_value]\n\t[Eiger]Temperature close to the 10GbE");    

    GET_IND_COMMAND(temp_dcdc, getTemperature, slsDetectorDefs::TEMPERATURE_DCDC, " °C",
                    "[n_value]\n\t[Eiger]Temperature close to the dc dc converter");

    GET_IND_COMMAND(temp_sodl, getTemperature, slsDetectorDefs::TEMPERATURE_SODL, " °C",
                    "[n_value]\n\t[Eiger]Temperature close to the left so-dimm memory");    

    GET_IND_COMMAND(temp_sodr, getTemperature, slsDetectorDefs::TEMPERATURE_SODR, " °C",
                    "[n_value]\n\t[Eiger]Temperature close to the right so-dimm memory");

    GET_IND_COMMAND(temp_fpgafl, getTemperature, slsDetectorDefs::TEMPERATURE_FPGA2, " °C",
                    "[n_value]\n\t[Eiger]Temperature of the left front end board fpga");    

    GET_IND_COMMAND(temp_fpgafr, getTemperature, slsDetectorDefs::TEMPERATURE_FPGA3, " °C",
                    "[n_value]\n\t[Eiger]Temperature of the left front end board fpga");  

    //dacs

    INTEGER_COMMAND(timing, getTimingMode, setTimingMode, sls::StringTo<slsDetectorDefs::timingMode>,
                    "[auto|trigger|gating|burst_trigger]\n\tTiming Mode of detector.\n\t[Jungfrau][Gotthard][Ctb] [auto|trigger]\n\t[Eiger] [auto|trigger|gating|burst_trigger]");      

    /* acquisition */

    EXECUTE_SET_COMMAND_NOID(clearbusy, clearAcquiringFlag, 
                "\n\tClears Acquiring Flag for unexpected acquire command terminations.");  

    EXECUTE_SET_COMMAND_NOID(rx_start, startReceiver, 
                "\n\tStarts receiver listener for detector data packets and create a data file (if file write enabled).");  

    EXECUTE_SET_COMMAND_NOID(rx_stop, stopReceiver, 
                "\n\tStops receiver listener for detector data packets and closes current data file (if file write enabled).");      
                                
    EXECUTE_SET_COMMAND_NOID(start, startDetector, 
                "\n\tStarts detector state machine.");  

    EXECUTE_SET_COMMAND_NOID(stop, stopDetector, 
                "\n\tStops detector state machine.");  

    GET_COMMAND(rx_status, getReceiverStatus, 
                "running, idle]\n\tReceiver listener status.");   

    GET_COMMAND(status, getDetectorStatus, 
                "[running, error, transmitting, finished, waiting, idle]\n\tDetector status.");                 

    GET_COMMAND(rx_framescaught, getFramesCaught, 
                    "\n\tNumber of frames caught by receiver."); 

    INTEGER_COMMAND(startingfnum, getStartingFrameNumber, setStartingFrameNumber, std::stoul,
                    "[n_value]\n\t[Eiger[Jungfrau] Starting frame number for next acquisition."); 

    EXECUTE_SET_COMMAND(trigger, sendSoftwareTrigger, 
                "\n\t[Eiger] Sends software trigger signal to detector.");   


    /* Network Configuration (Detector<->Receiver) */

    INTEGER_COMMAND(numinterfaces, getNumberofUDPInterfaces, setNumberofUDPInterfaces, std::stoi,
                    "[1, 2]\n\t[Jungfrau] Number of udp interfaces to stream data from detector. Default: 1.");  

    INTEGER_COMMAND(selinterface, getSelectedUDPInterface, selectUDPInterface, std::stoi,
                    "[0, 1]\n\t[Jungfrau] The udp interface to stream data from detector. Effective only when number of interfaces is 1. Default: 0 (outer)");  

    INTEGER_COMMAND(udp_srcip, getSourceUDPIP, setSourceUDPIP, IpAddr,
                    "[x.x.x.x]\n\tIp address of the detector (source) udp interface. Must be same subnet as destination udp ip.");               
    
    INTEGER_COMMAND(udp_srcip2, getSourceUDPIP2, setSourceUDPIP2, IpAddr,
                    "[x.x.x.x]\n\t[Jungfrau] Ip address of the bottom half of detector (source) udp interface. Must be same subnet as destination udp ip2.");   

    INTEGER_COMMAND(udp_dstip, getDestinationUDPIP, setDestinationUDPIP, IpAddr,
                    "[x.x.x.x]\n\tIp address of the receiver (destination) udp interface.");               
    
    INTEGER_COMMAND(udp_dstip2, getDestinationUDPIP2, setDestinationUDPIP2, IpAddr,
                    "[x.x.x.x]\n\t[Jungfrau] Ip address of the receiver (destination) udp interface where the second half of detector data is sent to.");     
 
    INTEGER_COMMAND(udp_srcmac, getSourceUDPMAC, setSourceUDPMAC, MacAddr,
                    "[x:x:x:x:x:x]\n\tMac address of the detector (source) udp interface. ");

    INTEGER_COMMAND(udp_srcmac2, getSourceUDPMAC2, setSourceUDPMAC2, MacAddr,
                    "[x:x:x:x:x:x]\n\t[Jungfrau] Mac address of the bottom half of detector (source) udp interface. ");     

    INTEGER_COMMAND(udp_dstmac, getDestinationUDPMAC, setDestinationUDPMAC, MacAddr,
                    "[x:x:x:x:x:x]\n\tMac address of the receiver (destination) udp interface. Can be unused as rx_hostname/udp_dstip retrieves it.");                   

    INTEGER_COMMAND(udp_dstmac2, getDestinationUDPMAC2, setDestinationUDPMAC2, MacAddr,
                    "[x:x:x:x:x:x]\n\t[Jungfrau] Mac address of the receiver (destination) udp interface where the second half of detector data is sent to. Can be unused as rx_hostname/udp_dstip2 retrieves it.");
    
    INTEGER_COMMAND(udp_dstport, getDestinationUDPPort, setDestinationUDPPort, std::stoi,
                    "[n]\n\tPort number of the receiver (destination) udp interface. Default is 50001.");               
    
    INTEGER_COMMAND(udp_dstport2, getDestinationUDPPort2, setDestinationUDPPort2, std::stoi,
                    "[n]\n\tDefault is 50002.\n\t[Jungfrau] Port number of the receiver (destination) udp interface where the second half of detector data is sent to. \n[Eiger] Port number of the reciever (desintation) udp interface where the right half of the detector data is sent to."); 

    GET_COMMAND(rx_printconfig, printRxConfiguration, 
                "\n\tPrints the receiver configuration.");   
    
    INTEGER_COMMAND(tengiga, getTenGiga, setTenGiga, std::stoi,
                    "[0, 1]\n\t[Eiger][Ctb] 10GbE Enable.");          

    INTEGER_COMMAND(flowcontrol_10g, getTransmissionDelayFrame, setTenGigaFlowControl, std::stoi,
                    "[0, 1]\n\t[Eiger][Jungfrau] 10GbE Flow Control.");          

    INTEGER_COMMAND(txndelay_frame, getTransmissionDelayFrame, setTransmissionDelayFrame, std::stoi,
                    "[n_delay]\n\t[Eiger][Jungfrau] Transmission delay of each image being streamed out of the module.\n\t[Jungfrau] [0-31] Each value represents 1 ms\n\t[Eiger] Additional delay to txndelay_left and txndelay_right. Each value represents 10ns. Typical value is 50000.");          

    INTEGER_COMMAND(txndelay_left, getTransmissionDelayLeft, setTransmissionDelayLeft, std::stoi,
                    "[n_delay]\n\t[Eiger] Transmission delay of first packet in an image being streamed out of the module's left UDP port. Each value represents 10ns. Typical value is 50000.");          

    INTEGER_COMMAND(txndelay_right, getTransmissionDelayRight, setTransmissionDelayRight, std::stoi,
                    "[n_delay]\n\t[Eiger] Transmission delay of first packet in an image being streamed out of the module's right UDP port. Each value represents 10ns. Typical value is 50000.");          


    /* Receiver Config */

    STRING_COMMAND(rx_hostname, getRxHostname, setRxHostname, 
                "[hostname or ip address]\n\tReceiver hostname or IP. Used for TCP control communication between client and receiver to configure receiver.");

    INTEGER_COMMAND(rx_tcpport, getRxPort, setRxPort, std::stoi,
                    "[port]\n\tTCP port for client-receiver communication. Default is 1954. Must be different if multiple receivers on same pc. Must be first command to set a receiver parameter. Multi command will automatically increment for individual modules.");  
    
    INTEGER_COMMAND(
        rx_fifodepth, getRxFifoDepth, setRxFifoDepth, std::stoi,
        "[n_frames]\n\tSet the number of frames in the receiver fifo (buffer between listener and writer threads).");

    INTEGER_COMMAND(rx_silent, getRxSilentMode, setRxSilentMode, std::stoi,
                    "[0, 1]\n\tSwitch on or off receiver text output during acquisition.");

    INTEGER_COMMAND(rx_discardpolicy, getRxFrameDiscardPolicy, setRxFrameDiscardPolicy, sls::StringTo<slsDetectorDefs::frameDiscardPolicy>,
                    "[nodiscard (default)|discardempty|discardpartial(fastest)]\n\tFrame discard policy of receiver. nodiscard does not discard frames, discardempty discards empty frames, discardpartial discards partial frames.");

    INTEGER_COMMAND(rx_padding, getPartialFramesPadding, setPartialFramesPadding, std::stoi,
                    "[0, 1]\n\tPartial frames padding enable in the "
                    "receiver. 0 does not pad partial frames(fastest), 1 "
                    "(default) pads partial frames");

    INTEGER_COMMAND(rx_udpsocksize, getRxUDPSocketBufferSize, setRxUDPSocketBufferSize, std::stol,
                    "[n_size]\n\tUDP socket buffer size in receiver. Tune rmem_default and rmem_max accordingly. rx_hostname sets it to defaults.");

    GET_COMMAND(rx_realudpsocksize, getRxRealUDPSocketBufferSize, 
                "\n\tActual udp socket buffer size. Double the size of rx_udpsocksize due to kernel bookkeeping."); 

    INTEGER_COMMAND(rx_lock, getRxLock, setRxLock, std::stoi,
                    "[0, 1]\n\tLock receiver to one IP, 1: locks");

    GET_COMMAND(rx_lastclient, getRxLastClientIP, 
                "\n\tClient IP Address that last communicated with the receiver."); 


    /* File */

    INTEGER_COMMAND(fformat, getFileFormat, setFileFormat, sls::StringTo<slsDetectorDefs::fileFormat>,
                    "[binary|hdf5]\n\tFile format of data file. For HDF5, package must be compiled with HDF5 flags. Default is binary.");

    STRING_COMMAND(fpath, getFilePath, setFilePath, 
                "[path]\n\tDirectory where output data files are written in receiver.");

    STRING_COMMAND(fname, getFileNamePrefix, setFileNamePrefix, 
                "[path]\n\tFile name prefix for output data file. Default is run. File name: [file name prefix]_d[detector index]_f[sub file index]_[acquisition/file index].raw.");

    INTEGER_COMMAND(findex, getAcquisitionIndex, setAcquisitionIndex, std::stol,
                    "[0, 1]\n\tFile or Acquisition index.");

    INTEGER_COMMAND(fwrite, getFileWrite, setFileWrite, std::stoi,
                    "[0, 1]\n\tEnable or disable receiver file write. Default is 1.");

    INTEGER_COMMAND_NOID(fmaster, getMasterFileWrite, setMasterFileWrite, std::stoi,
                    "[0, 1]\n\tEnable or disable receiver master file. Default is 1.");

    INTEGER_COMMAND(foverwrite, getFileOverWrite, setFileOverWrite, std::stoi,
                    "[0, 1]\n\tEnable or disable file overwriting. Default is 1.");

    INTEGER_COMMAND(rx_framesperfile, getFramesPerFile, setFramesPerFile, std::stoi, 
                    "[n_frames]\n\tNumber of frames per file in receiver. 0 is infinite or all frames in single file.");


    /* ZMQ Streaming Parameters (Receiver<->Client) */

    INTEGER_COMMAND(rx_datastream, getRxZmqDataStream, setRxZmqDataStream, std::stoi,
                    "[0, 1]\n\tData streaming from receiver enable (eg. to GUI ot another process for further processing). 1 enables zmq data stream (creates zmq streamer threads), 0 disables (destroys streamer threads). Switching to Gui automatically enables data streaming in receiver. Switching back to command line acquire will require disabling data streaming in receiver for fast applications.");

    INTEGER_COMMAND(rx_readfreq, getRxZmqFrequency, setRxZmqFrequency,
                    std::stoi, "[nth frame]\n\tStream out every nth frame. Default is 1. 0 means streaming every 200 ms and discarding frames in this interval.");

    INTEGER_COMMAND(rx_zmqport, getRxZmqPort, setRxZmqPort,
                    std::stoi, "[port]\n\tZmq port for data to be streamed out of the receiver. Also restarts receiver zmq streaming if enabled. Default is 30001. Modified only when using an intermediate process between receiver and client(gui). Must be different for every detector (and udp port). Multi command will automatically increment for individual modules.");

    INTEGER_COMMAND(zmqport, getClientZmqPort, setClientZmqPort,
                    std::stoi, "[port]\n\tZmq port in client(gui) or intermediate process for data to be streamed to from receiver. efault connects to receiver zmq streaming out port (30001). Modified only when using an intermediate process between receiver and client(gui). Also restarts client zmq streaming if enabled. Must be different for every detector (and udp port). Multi command will automatically increment for individual modules.");

    INTEGER_COMMAND(rx_zmqip, getRxZmqIP, setRxZmqIP, IpAddr,
                    "[x.x.x.x]\n\tZmq Ip Address from which data is to be streamed out of the receiver. Also restarts receiver zmq streaming if enabled. Default is from rx_hostname. Modified only when using an intermediate process between receiver and client(gui).");               
    
    INTEGER_COMMAND(zmqip, getClientZmqIp, setClientZmqIp, IpAddr,
                    "[x.x.x.x]\n\tZmq IP Address in client(gui) or intermediate process for data to be streamed to from receiver.  Default connects to receiver zmq Ip Address (from rx_hostname). Modified only when using an intermediate process between receiver and client(gui). Also restarts client zmq streaming if enabled.");               
    

    /* Eiger Specific */

    TIME_COMMAND(subexptime, getSubExptime, setSubExptime,
                 "[duration] [(optional unit) ns|us|ms|s]\n\t[Eiger] Exposure time of EIGER subframes");

    TIME_COMMAND(subdeadtime, getSubDeadTime, setSubDeadTime,
                 "[duration] [(optional unit) ns|us|ms|s]\n\t[Eiger] Dead time of EIGER subframes. Subperiod = subexptime + subdeadtime.");

    STRING_COMMAND(settingspath, getSettingsPath, setSettingsPath, 
                "[path]\n\t[Eiger] Directory where settings files are loaded from/to.");

    EXECUTE_SET_COMMAND_NOID_1ARG(trimbits, loadTrimbits, 
                "[fname]\n\t[Eiger] Loads the trimbit file to detector. If no extension specified, serial number of each module is attached.");

    INTEGER_COMMAND(parallel, getParallelMode, setParallelMode, std::stoi,
                    "[0, 1]\n\t[Eiger] Enable or disable parallel mode.");

    INTEGER_COMMAND(overflow, getOverFlowMode, setOverFlowMode, std::stoi,
                    "[0, 1]\n\t[Eiger] Enable or disable show overflow flag in 32 bit mode.");    

    INTEGER_COMMAND(storeinram, getStoreInRamMode, setStoreInRamMode, std::stoi,
                    "[0, 1]\n\t[Eiger] Enable or disable store in ram mode.");      

    INTEGER_COMMAND(flippeddatax, getBottom, setBottom, std::stoi,
                    "[0, 1]\n\t[Eiger] Top or Bottom Half of Eiger module. 1 is bottom, 0 is top. Used to let Receiver and Gui know to flip the bottom image over the x axis.");      

    INTEGER_COMMAND(trimval, getAllTrimbits, setAllTrimbits, std::stoi,
                    "[n_trimval]\n\t[Eiger] All trimbits set to this value. A get returns -1 if all trimbits are different values.");      

    INTEGER_COMMAND(readnlines, getPartialReadout, setPartialReadout, std::stoi,
                    "[1 - 256]\n\t[Eiger] Number of rows to readout per half module starting from the centre. 256 is default. The permissible values depend on dynamic range and 10Gbe enabled.");      

    INTEGER_COMMAND(interruptsubframe, getInterruptSubframe, setInterruptSubframe, std::stoi,
                    "[0, 1]\n\t[Eiger] 1 interrupts last subframe at required exposure time. 0 will wait for last sub frame to finish exposing. 0 is default.");      

    TIME_GET_COMMAND(measuredperiod, getMeasuredPeriod, 
                "[(optional unit) ns|us|ms|s]\n\t[Eiger] Measured frame period between last frame and previous one. Useful data only for acquisitions with more than 1 frame.");    

    TIME_GET_COMMAND(measuredsubperiod, getMeasuredSubFramePeriod, 
                "[(optional unit) ns|us|ms|s]\n\t[Eiger] Measured sub frame period between last sub frame and previous one.");    

    INTEGER_COMMAND(partialreset, getPartialReset, setPartialReset, std::stoi,
                    "[0, 1]\n\t[Eiger] Sets up detector to do partial or complete reset at start of acquisition. 0 complete reset, 1 partial reset.");  


    /* Jungfrau Specific */

    INTEGER_COMMAND(temp_threshold, getThresholdTemperature, setThresholdTemperature, std::stoi,
                    "[n_temp (in degrees)]\n\t[Jungfrau] Threshold temperature in degrees. If temperature crosses threshold temperature and temperature control is enabled, power to chip will be switched off and temperature event occurs. To power on chip again, temperature has to be less than threshold temperature and temperature event has to be cleared.");  

    INTEGER_COMMAND(temp_control, getTemperatureControl, setTemperatureControl, std::stoi,
                    "[0, 1]\n\t[Jungfrau] Temperature control enable. Default is 0 (disabled). If temperature crosses threshold temperature and temperature control is enabled, power to chip will be switched off and temperature event occurs. To power on chip again, temperature has to be less than threshold temperature and temperature event has to be cleared.");  

    INTEGER_COMMAND(powerchip, getPowerChip, setPowerChip, std::stoi,
                    "[0, 1]\n\t[Jungfrau] Power the chip. Default 0. Get will return power status. Can be off if temperature event occured (temperature over temp_threshold with temp_control enabled.");  

    INTEGER_COMMAND(auto_comp_disable, getAutoCompDisable, setAutoCompDisable, std::stoi,
                    "[0, 1]\n\t[Jungfrau] Auto comparator disable mode. Default 0 or this mode disabled(comparator enabled throughout). 1 enables mode. 0 disables mode. This mode disables the on-chip gain switching comparator automatically after 93.75% of exposure time (only for longer than 100us).");  

    INTEGER_COMMAND_NOID(storagecells, getNumberOfAdditionalStorageCells, setNumberOfAdditionalStorageCells, std::stoi,
                    "[0-15]\n\t[Jungfrau] Number of additional storage cells. Default is 0. For advanced users only. \n\tThe #images = #frames x #triggers x (#storagecells + 1).");

    INTEGER_COMMAND(storagecell_start, getStorageCellStart, setStoragecellStart, std::stoi,
                    "[0-15]\n\t[Jungfrau] Storage cell that stores the first acquisition of the series. Default is 15. For advanced users only.");  

    TIME_COMMAND(storagecell_delay, getStorageCellDelay, setStorageCellDelay,
                 "[duration (0-1638375 ns)] [(optional unit) ns|us|ms|s]\n\t[Jungfrau] Additional time delay between 2 storage cells. For advanced users only. Resolution is 25 ns.");


    /* Gotthard Specific */
    TIME_GET_COMMAND(exptimel, getExptimeLeft, 
                "[(optional unit) ns|us|ms|s]\n\t[Gotthard] Exposure time left for current frame. ");   

    TIME_GET_COMMAND(periodl, getPeriodLeft, 
                "[(optional unit) ns|us|ms|s]\n\t[Gotthard] Period left for current frame.");   

    INTEGER_COMMAND(extsig, getExternalSignalFlags, setExternalSignalFlags, sls::StringTo<slsDetectorDefs::externalSignalFlag>,
                    "[trigger_in_rising_edge|trigger_in_falling_edge]\n\t[Gotthard] External signal mode for trigger timing mode.");

    INTEGER_COMMAND(imagetest, getImageTestMode, setImageTestMode, std::stoi,
                    "[0, 1]\n\t[Gotthard] 1 adds channel intensity with precalculated values when taking an acquisition. Default is 0.");  

    /* Gotthard2 Specific */
    /* CTB Specific */

    INTEGER_COMMAND(asamples, getNumberOfAnalogSamples, setNumberOfAnalogSamples, std::stoi,
                    "[0, 1]\n\t[CTB] Number of analog samples expected.");  

    INTEGER_COMMAND(dsamples, getNumberOfDigitalSamples, setNumberOfDigitalSamples, std::stoi,
                    "[0, 1]\n\t[CTB] Number of digital samples expected.");  

    INTEGER_COMMAND(romode, getReadoutMode, setReadoutMode, sls::StringTo<slsDetectorDefs::readoutMode>,
                    "[analog|digital|analog_digital]\n\t[CTB] Readout mode. Default is analog.");

    GET_COMMAND(maxdbitphaseshift, getMaxDBITPhaseShift, 
                "\n\t[CTB] Absolute maximum Phase shift of of the clock to latch digital bits.");

    INTEGER_COMMAND(adcclk, getADCClock, setADCClock, std::stoi,
                    "[n_clk in MHz]\n\t[Ctb] ADC clock frequency in MHz.");      

    INTEGER_COMMAND(dbitclk, getDBITClock, setDBITClock, std::stoi,
                    "[n_clk in MHz]\n\t[Ctb] Clock for latching the digital bits in MHz.");      

    INTEGER_COMMAND(runclk, getRUNClock, setRUNClock, std::stoi,
                    "[n_clk in MHz]\n\t[Ctb] Run clock in MHz.");      

    GET_COMMAND(syncclk, getSYNCClock,
                    "[n_clk in MHz]\n\t[Ctb] Synch clock in MHz.");      

    INTEGER_COMMAND(adcpipeline, getADCPipeline, setADCPipeline, std::stoi,
                    "[n_value]\n\t[Ctb] Pipeline for ADC clock.");      

    INTEGER_COMMAND(dbitpipeline, getDBITPipeline, setDBITPipeline, std::stoi,
                    "[n_value]\n\t[Ctb] Pipeline of the clock for latching digital bits.");      

    INTEGER_IND_COMMAND(v_limit, getVoltage, setVoltage, std::stoi, defs::V_LIMIT,
                    "[n_value]\n\t[Ctb] Soft limit for power supplies and DACS in mV.");      

    INTEGER_IND_COMMAND(v_a, getVoltage, setVoltage, std::stoi, defs::V_POWER_A,
                    "[n_value]\n\t[Ctb] Voltage supply a in mV.");      

    INTEGER_IND_COMMAND(v_b, getVoltage, setVoltage, std::stoi, defs::V_POWER_B,
                    "[n_value]\n\t[Ctb] Voltage supply b in mV.");      

    INTEGER_IND_COMMAND(v_c, getVoltage, setVoltage, std::stoi, defs::V_POWER_C,
                    "[n_value]\n\t[Ctb] Voltage supply c in mV.");      

    INTEGER_IND_COMMAND(v_d, getVoltage, setVoltage, std::stoi, defs::V_POWER_D,
                    "[n_value]\n\t[Ctb] Voltage supply d in mV.");      

    INTEGER_IND_COMMAND(v_io, getVoltage, setVoltage, std::stoi, defs::V_POWER_IO,
                    "[n_value]\n\t[Ctb] Voltage supply io in mV. Minimum 1200 mV. Must be the first power regulator to be set after fpga reset (on-board detector server start up).");      

    INTEGER_IND_COMMAND(v_chip, getVoltage, setVoltage, std::stoi, defs::V_POWER_CHIP,
                    "[n_value]\n\t[Ctb] Voltage supply chip in mV. Do not use it unless you are completely sure you will not fry the board.");      

    GET_IND_COMMAND(vm_a, getMeasuredVoltage, defs::V_POWER_A, "",
                    "\n\t[Ctb] Measured voltage of power supply a in mV.");  

    GET_IND_COMMAND(vm_b, getMeasuredVoltage, defs::V_POWER_B,  "",
                    "\n\t[Ctb] Measured voltage of power supply b in mV.");  

    GET_IND_COMMAND(vm_c, getMeasuredVoltage, defs::V_POWER_C,  "",
                    "\n\t[Ctb] Measured voltage of power supply c in mV."); 
                    
    GET_IND_COMMAND(vm_d, getMeasuredVoltage, defs::V_POWER_D,  "",
                    "\n\t[Ctb] Measured voltage of power supply d in mV.");                 

    GET_IND_COMMAND(vm_io, getMeasuredVoltage, defs::V_POWER_IO,  "",
                    "\n\t[Ctb] Measured voltage of power supply io in mV."); 

    GET_IND_COMMAND(im_a, getMeasuredVoltage, defs::I_POWER_A, "",
                    "\n\t[Ctb] Measured current of power supply a in mA.");  

    GET_IND_COMMAND(im_b, getMeasuredVoltage, defs::I_POWER_B,  "",
                    "\n\t[Ctb] Measured current of power supply b in mA.");  

    GET_IND_COMMAND(im_c, getMeasuredVoltage, defs::I_POWER_C,  "",
                    "\n\t[Ctb] Measured current of power supply c in mA."); 
                    
    GET_IND_COMMAND(im_d, getMeasuredVoltage, defs::I_POWER_D,  "",
                    "\n\t[Ctb] Measured current of power supply d in mA.");                 

    GET_IND_COMMAND(im_io, getMeasuredVoltage, defs::I_POWER_IO,  "",
                    "\n\t[Ctb] Measured current of power supply io in mA."); 

    INTEGER_COMMAND(adcenable, getADCEnableMask, setADCEnableMask, stoui,
                    "[bitmask]\n\t[Ctb] ADC Enable Mask.");      

    INTEGER_COMMAND(adcinvert, getADCInvert, setADCInvert, stoui,
                    "[bitmask]\n\t[Ctb] ADC Inversion Mask.");      

    INTEGER_COMMAND(extsampling, getExternalSampling, setExternalSampling, std::stoi,
                    "[0, 1]\n\t[Ctb] Enable for external sampling signal to extsamplingsrc signal for digital data. For advanced users only.");

    INTEGER_COMMAND(extsamplingsrc, getExternalSamplingSource, setExternalSamplingSource, std::stoi,
                    "[0-63]\n\t[Ctb] Sampling source signal for digital data. For advanced users only.");

    INTEGER_COMMAND(rx_dbitoffset, getRxDbitOffset, setRxDbitOffset, std::stoi,
                    "[n_bytes]\n\t[Ctb] Offset in bytes in digital data in receiver.");

    INTEGER_COMMAND(led, getLEDEnable, setLEDEnable, std::stoi,
                    "[0, 1]\n\t[Ctb] Switches on/off all LEDs.");


    /* Pattern */

    EXECUTE_SET_COMMAND_NOID_1ARG(savepattern, savePattern, 
                "[fname]\n\t[Ctb] Saves pattern to file (ascii). Also executes pattern."); 

    INTEGER_COMMAND_HEX(patioctrl, getPatternIOControl, setPatternIOControl, std::stoul,
                    "[64 bit mask]\n\t[Ctb] 64 bit mask defining input (0) and output (1) signals.");

    INTEGER_COMMAND_HEX(patclkctrl, getPatternClockControl, setPatternClockControl, std::stoul,
                    "[64 bit mask]\n\t[Ctb] 64 bit mask defining output clock enable.");




    DAC_COMMAND(adcvpp, getDAC, setDAC, defs::ADC_VPP,
                    "[dac or mv value][(optional unit) mv] \n\t[Ctb] Vpp of ADC.\n\t 0 -> 1V ; 1 -> 1.14V ; 2 -> 1.33V ; 3 -> 1.6V ; 4 -> 2V.");    

    GET_COMMAND(lastclient, getLastClientIP, 
                "\n\tClient IP Address that last communicated with the detector."); 

    INTEGER_COMMAND(lock, getDetectorLock, setDetectorLock, std::stoi,
                    "[0, 1]\n\tLock detector to one IP, 1: locks");



};

} // namespace sls
