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
                WrongNumberOfParameters(2);                                    \
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

#define TEMP_COMMAND(CMDNAME, GETFCN, VAL, HLPSTR)                             \
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
            os << OutString(t) << " °C\n";                                     \
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

    using FunctionMap = std::map<std::string, std::string (CmdProxy::*)(int)>;
    using StringMap = std::map<std::string, std::string>;

    StringMap depreciated_functions{{"r_readfreq", "rx_readfreq"},
                                    {"r_padding", "rx_padding"},
                                    {"r_silent", "rx_silent"},
                                    {"r_lastclient", "rx_lastclient"},
                                    {"r_lock", "rx_lock"},
                                    {"r_online", "rx_online"},
                                    {"r_checkonline", "rx_checkonline"},
                                    {"r_framesperfile", "rx_framesperfile"},
                                    {"r_discardpolicy", "rx_discardpolicy"},
                                    {"receiver", "rx_status"},
                                    {"index", "findex"},
                                    {"exitreceiver", "rx_exit"},
                                    {"enablefwrite", "fwrite"},
                                    {"checkrecversion", "rx_checkversion"},
                                    {"masterfile", "fmaster"},
                                    {"outdir", "fpath"},
                                    {"fileformat", "fformat"},
                                    {"overwrite", "foverwrite"},
                                    {"flags", "romode"},
                                    
                                    {"busy", "clearbusy"},
                                    {"detectorversion", "firmwareversion"},
                                    {"softwareversion", "detectorserverversion"},
                                    {"receiverversion", "rx_version"},
                                    {"thisversion", "clientversion"},
                                    {"detsizechan", "detsize"},
                                    {"cycles", "triggers"},
                                    {"cyclesl", "triggersl"},
                                    {"clkdivider", "speed"} // or runclk for ctb (ignore as speed doesnt exist?)


                                    
                                    };

    // Initialize maps for translating name and function
    FunctionMap functions{{"list", &CmdProxy::ListCommands},

                          {"parallel", &CmdProxy::parallel},
                          {"overflow", &CmdProxy::overflow},
                          {"storeinram", &CmdProxy::storeinram},
                          {"fwrite", &CmdProxy::fwrite},
                          {"fmaster", &CmdProxy::fmaster},
                          {"foverwrite", &CmdProxy::foverwrite},
                          {"findex", &CmdProxy::findex},
                          {"rx_fifodepth", &CmdProxy::rx_fifodepth},
                          {"rx_silent", &CmdProxy::rx_silent},
                          {"rx_lock", &CmdProxy::rx_lock},
                          {"lock", &CmdProxy::lock},
                          {"rx_readfreq", &CmdProxy::rx_readfreq},
                          {"rx_padding", &CmdProxy::rx_padding},
                          {"rx_framesperfile", &CmdProxy::rx_framesperfile},
                          {"detectormac", &CmdProxy::detectormac},
                          {"detectormac2", &CmdProxy::detectormac2},
                          {"rx_udpmac", &CmdProxy::rx_udpmac},
                          {"rx_udpmac2", &CmdProxy::rx_udpmac2},
                          {"detectorip", &CmdProxy::detectorip},
                          {"detectorip2", &CmdProxy::detectorip2},
                          {"rx_udpip", &CmdProxy::rx_udpip},
                          {"rx_udpip2", &CmdProxy::rx_udpip2},
                          {"rx_udpport", &CmdProxy::rx_udpport},
                          {"rx_udpport2", &CmdProxy::rx_udpport2},
                          {"numinterfaces", &CmdProxy::numinterfaces},
                          {"selinterface", &CmdProxy::selinterface},
                          
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
                          {"frames", &CmdProxy::frames},                          
                          {"triggers", &CmdProxy::triggers},
                          {"exptime", &CmdProxy::exptime},
                          {"period", &CmdProxy::period},
                          {"delay", &CmdProxy::delay},
                          {"delay", &CmdProxy::delay},
                          {"framesl", &CmdProxy::framesl},
                          {"triggersl", &CmdProxy::triggersl},
                          {"delayl", &CmdProxy::DelayLeft},
                          {"speed", &CmdProxy::Speed},
                          {"adcphase", &CmdProxy::Adcphase},
                          {"maxadcphaseshift", &CmdProxy::maxadcphaseshift},
                          {"vhighvoltage", &CmdProxy::vhighvoltage},
                          {"temp_fpga", &CmdProxy::temp_fpga},




                          {"start", &CmdProxy::start},
                          {"stop", &CmdProxy::stop},
                          {"trigger", &CmdProxy::trigger},
                          {"status", &CmdProxy::status},
                          {"rx_start", &CmdProxy::rx_start},
                          {"rx_stop", &CmdProxy::rx_stop},
                          {"rx_status", &CmdProxy::rx_status}, 
                          {"clearbusy", &CmdProxy::clearbusy}, 
                          
                          
                          {"rx_hostname", &CmdProxy::rx_hostname},   
                          {"rx_tcpport", &CmdProxy::rx_tcpport},  

                          {"subexptime", &CmdProxy::subexptime},  


                          {"threshold", &CmdProxy::Threshold},
                          {"thresholdnotb", &CmdProxy::ThresholdNoTb},  

                          {"runclk", &CmdProxy::runclk},  


                          {"savepattern", &CmdProxy::savepattern}                         
                          };


    void WrongNumberOfParameters(size_t expected);

    /* Commands */
    std::string ListCommands(int action);
    std::string Hostname(int action); 
    std::string FirmwareVersion(int action);     
    std::string Versions(int action); 
    std::string PackageVersion(int action);     
    std::string ClientVersion(int action);
    std::string DetectorSize(int action);
    std::string DelayLeft(int action);
    std::string Speed(int action);
    std::string Adcphase(int action);



    std::string Threshold(int action);
    std::string ThresholdNoTb(int action);       

    INTEGER_COMMAND(
        rx_fifodepth, getRxFifoDepth, setRxFifoDepth, std::stoi,
        "[n_frames]\n\tSet the number of frames in the receiver fifo");

    INTEGER_COMMAND(rx_silent, getRxSilentMode, setRxSilentMode, std::stoi,
                    "[0, 1]\n\tSwitch on or off receiver text output");

    INTEGER_COMMAND(rx_lock, getRxLock, setRxLock, std::stoi,
                    "[0, 1]\n\tLock receiver to one IP, 1: locks");

    INTEGER_COMMAND(lock, getDetectorLock, setDetectorLock, std::stoi,
                    "[0, 1]\n\tLock detector to one IP, 1: locks");

    INTEGER_COMMAND(rx_readfreq, getRxZmqFrequency, setRxZmqFrequency,
                    std::stoi, "[nth frame]\n\tStream out every nth frame");

    INTEGER_COMMAND(rx_padding, getPartialFramesPadding,
                    setPartialFramesPadding, std::stoi,
                    "[0, 1]\n\tPartial frames padding enable in the "
                    "receiver. 0 does not pad partial frames(fastest), 1 "
                    "(default) pads partial frames");

    INTEGER_COMMAND(rx_framesperfile, getFramesPerFile, setFramesPerFile,
                    std::stoi, "[n_frames]\n\tNumber of frames per file");

    INTEGER_COMMAND(fwrite, getFileWrite, setFileWrite, std::stoi,
                    "[0, 1]\n\tEnable or disable receiver file write");

    INTEGER_COMMAND_NOID(fmaster, getMasterFileWrite, setMasterFileWrite, std::stoi,
                    "[0, 1]\n\tEnable or disable receiver master file");

    INTEGER_COMMAND(foverwrite, getFileOverWrite, setFileOverWrite, std::stoi,
                    "[0, 1]\n\tEnable or disable file overwriting");

    INTEGER_COMMAND(findex, getAcquisitionIndex, setAcquisitionIndex, std::stoi,
                    "[0, 1]\n\tFile index");

    INTEGER_COMMAND(detectormac, getSourceUDPMAC, setSourceUDPMAC, MacAddr,
                    "[x:x:x:x:x:x]\n\tMac address of the detector (source) udp interface. ");

    INTEGER_COMMAND(detectormac2, getSourceUDPMAC2, setSourceUDPMAC2, MacAddr,
                    "[x:x:x:x:x:x]\n\t[Jungfrau] Mac address of the bottom half of detector (source) udp interface. ");     

    INTEGER_COMMAND(rx_udpmac, getDestinationUDPMAC, setDestinationUDPMAC, MacAddr,
                    "[x:x:x:x:x:x]\n\tMac address of the receiver (destination) udp interface. Can be unused as rx_hostname/rx_udpip retrieves it.");                   

    INTEGER_COMMAND(rx_udpmac2, getDestinationUDPMAC2, setDestinationUDPMAC2, MacAddr,
                    "[x:x:x:x:x:x]\n\t[Jungfrau] Mac address of the receiver (destination) udp interface where the second half of detector data is sent to. Can be unused as rx_hostname/rx_udpip2 retrieves it.")

    INTEGER_COMMAND(detectorip, getSourceUDPIP, setSourceUDPIP, IpAddr,
                    "[x.x.x.x]\n\tIp address of the detector (source) udp interface. Must be same subnet as destination udp ip.");               
    
    INTEGER_COMMAND(detectorip2, getSourceUDPIP2, setSourceUDPIP2, IpAddr,
                    "[x.x.x.x]\n\t[Jungfrau] Ip address of the bottom half of detector (source) udp interface. Must be same subnet as destination udp ip2.");     
    
    INTEGER_COMMAND(rx_udpip, getDestinationUDPIP, setDestinationUDPIP, IpAddr,
                    "[x.x.x.x]\n\tIp address of the receiver (destination) udp interface.");               
    
    INTEGER_COMMAND(rx_udpip2, getDestinationUDPIP2, setDestinationUDPIP2, IpAddr,
                    "[x.x.x.x]\n\t[Jungfrau] Ip address of the receiver (destination) udp interface where the second half of detector data is sent to.");     
 
    INTEGER_COMMAND(rx_udpport, getDestinationUDPPort, setDestinationUDPPort, std::stoi,
                    "[n]\n\tPort number of the receiver (destination) udp interface.");               
    
    INTEGER_COMMAND(rx_udpport2, getDestinationUDPPort2, setDestinationUDPPort2, std::stoi,
                    "[n]\n\t[Jungfrau] Port number of the receiver (destination) udp interface where the second half of detector data is sent to.\n[Eiger] Port number of the reciever (desintation) udp interface where the right half of the detector data is sent to.");     

    INTEGER_COMMAND(numinterfaces, getNumberofUDPInterfaces, setNumberofUDPInterfaces, std::stoi,
                    "[1, 2]\n\t[Jungfrau] Number of udp interfaces to stream data from detector. Default: 1.");     

    INTEGER_COMMAND(selinterface, getSelectedUDPInterface, selectUDPInterface, std::stoi,
                    "[0, 1]\n\t[Jungfrau] The udp interface to stream data from detector. Effective only when number of interfaces is 1. Default: 0 (outer)");       
    
    INTEGER_COMMAND(parallel, getParallelMode, setParallelMode, std::stoi,
                    "[0, 1]\n\t[Eiger] Enable or disable parallel mode.");

    INTEGER_COMMAND(overflow, getOverFlowMode, setOverFlowMode, std::stoi,
                    "[0, 1]\n\t[Eiger] Enable or disable show overflow flag in 32 bit mode.");    

    INTEGER_COMMAND(storeinram, getStoreInRamMode, setStoreInRamMode, std::stoi,
                    "[0, 1]\n\t[Eiger] Enable or disable store in ram mode.");      




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

    GET_COMMAND(maxadcphaseshift, getMaxADCPhaseShift, 
                "\n\t[Jungfrau][CTB] Absolute maximum Phase shift of ADC clock.");  

    INTEGER_COMMAND(vhighvoltage, getHighVoltage, setHighVoltage, std::stoi,
                    "[n_value]\n\tHigh voltage to the sensor in Voltage.\n\t[Gotthard] [0|90|110|120|150|180|200]\n\t[Eiger] 0-200\n\t[Jungfrau][Ctb] [0|60-200]");      

    TEMP_COMMAND(temp_fpga, getTemperature, slsDetectorDefs::TEMPERATURE_FPGA,
                    "[n_value]\n\t[Eiger][Jungfrau][Gotthard] FPGA Temperature");    





    EXECUTE_SET_COMMAND_NOID(start, startDetector, 
                "\n\tStarts detector state machine.");  

    EXECUTE_SET_COMMAND_NOID(stop, stopDetector, 
                "\n\tStops detector state machine.");  

    EXECUTE_SET_COMMAND(trigger, sendSoftwareTrigger, 
                "\n\t[Eiger] Sends software trigger signal to detector.");   

    GET_COMMAND(status, getDetectorStatus, 
                "[running, error, transmitting, finished, waiting, idle]\n\tDetector status.");                 

    EXECUTE_SET_COMMAND_NOID(rx_start, startReceiver, 
                "\n\tStarts receiver listener for detector data packets and create a data file (if file write enabled).");  

    EXECUTE_SET_COMMAND_NOID(rx_stop, stopReceiver, 
                "\n\tStops receiver listener for detector data packets and closes current data file (if file write enabled).");      
                                
    GET_COMMAND(rx_status, getReceiverStatus, 
                "running, idle]\n\tReceiver listener status.");   

    EXECUTE_SET_COMMAND_NOID(clearbusy, clearAcquiringFlag, 
                "\n\tClears Acquiring Flag for unexpected acquire command terminations.");  

    


    STRING_COMMAND(rx_hostname, getRxHostname, setRxHostname, 
                "[hostname or ip address]\n\tReceiver hostname or IP. Used for TCP control communication between client and receiver to configure receiver.");

    INTEGER_COMMAND(rx_tcpport, getRxPort, setRxPort, std::stoi,
                    "[port]\n\tTCP port for client-receiver communication. Must be different if multiple receivers on same pc. Must be first command to set a receiver parameter.");  
    

    TIME_COMMAND(subexptime, getSubExptime, setSubExptime,
                 "[duration] [(optional unit) ns|us|ms|s]\n\tExposure time of EIGER subframes");


    INTEGER_COMMAND(runclk, getRUNClock, setRUNClock, std::stoi,
                    "[n_clk in MHz]\n\t[Ctb] Run clk in MHz.");      


    EXECUTE_SET_COMMAND_NOID_1ARG(savepattern, savePattern, 
                "[fname]\n\t[Ctb] Saves pattern to file (ascii). Also executes pattern."); 

};

} // namespace sls
