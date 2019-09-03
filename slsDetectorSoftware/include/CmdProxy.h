#pragma once

#include "Detector.h"
#include "Result.h"
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

#define INTEGER_COMMAND(CMDNAME, GETFCN, SETFCN, CONV, HLPSTR)                 \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            auto t = det->GETFCN({det_id});                                    \
            if (args.size() == 0) {                                            \
                os << OutString(t) << '\n';                                    \
            } else {                                                           \
                WrongNumberOfParameters(2);                                    \
            }                                                                  \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (args.size() == 1) {                                            \
                auto val = CONV(args[0]);                                      \
                det->SETFCN(val, {det_id});                                    \
                os << args.front() << '\n';                                    \
            } else {                                                           \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
                                                                               \
        } else {                                                               \
            throw sls::RuntimeError("Unknown action");                         \
        }                                                                      \
        return os.str();                                                       \
    }

#define INTEGER_COMMAND_NOID(CMDNAME, GETFCN, SETFCN, CONV, HLPSTR)            \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            auto t = det->GETFCN();                                            \
            if (args.size() == 0) {                                            \
                os << OutString(t) << '\n';                                    \
            } else {                                                           \
                WrongNumberOfParameters(2);                                    \
            }                                                                  \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (args.size() == 1) {                                            \
                auto val = CONV(args[0]);                                      \
                det->SETFCN(val);                                              \
                os << args.front() << '\n';                                    \
            } else {                                                           \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
                                                                               \
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

    // Initialize maps for translating name and function
    FunctionMap functions{{"list", &CmdProxy::ListCommands},
                          {"exptime", &CmdProxy::Exptime},
                          {"period", &CmdProxy::Period},
                          {"subexptime", &CmdProxy::SubExptime},
                          {"frames", &CmdProxy::frames},
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
                          {"rx_framesperfile", &CmdProxy::rx_framesperfile}};

    StringMap depreciated_functions{{"r_readfreq", "rx_readfreq"},
                                    {"r_padding", "rx_padding"},
                                    {"r_silent", "rx_silent"},
                                    {"r_lastclient", "rx_lastclient"},
                                    {"r_lock", "rx_lock"},
                                    {"r_online", "rx_online"},
                                    {"r_checkonline", "rx_checkonline"},
                                    {"r_framesperfile", "rx_framesperfile"},
                                    {"r_discardpolicy", "rx_discardpolicy"},
                                    {"receiverversion", "rx_version"},
                                    {"receiver", "rx_status"},
                                    {"index", "findex"},
                                    {"exitreceiver", "rx_exit"},
                                    {"enablefwrite", "fwrite"},
                                    {"checkrecversion", "rx_checkversion"},
                                    {"masterfile", "fmaster"},
                                    {"outdir", "fpath"},
                                    {"fileformat", "fformat"},
                                    {"overwrite", "foverwrite"},
                                    {"flags", "readout"}};

    void WrongNumberOfParameters(size_t expected);

    /* Commands */
    std::string ListCommands(int action);
    std::string Period(int action);
    std::string Exptime(int action);
    std::string SubExptime(int action);

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
                    "[0, 1]\n\tgets partial frames padding enable in the "
                    "receiver. 0 does not pad partial frames(fastest), 1 "
                    "(default) pads partial frames");
    INTEGER_COMMAND(rx_framesperfile, getFramesPerFile, setFramesPerFile,
                    std::stoi, "[n_frames]\n\tNumber of frames per file");

    INTEGER_COMMAND_NOID(frames, getNumberOfFrames, setNumberOfFrames,
                         std::stol,
                         "[n_frames]\n\tNumber of frames per aquire");

    INTEGER_COMMAND(fwrite, getFileWrite, setFileWrite, std::stoi,
                    "[0, 1]\n\tEnable or disable receiver file write");

    INTEGER_COMMAND(fmaster, getMasterFileWrite, setMasterFileWrite, std::stoi,
                    "[0, 1]\n\tEnable or disable receiver master file");

    INTEGER_COMMAND(foverwrite, getFileOverWrite, setFileOverWrite, std::stoi,
                    "[0, 1]\n\tEnable or disable file overwriting");

    INTEGER_COMMAND(findex, getAcquisitionIndex, setAcquisitionIndex, std::stoi,
                    "[0, 1]\n\tFile index");
    
    INTEGER_COMMAND(parallel, getParallelMode, setParallelMode, std::stoi,
                    "[0, 1]\n\tEnable or disable parallel mode. [Eiger]");

    INTEGER_COMMAND(overflow, getOverFlowMode, setOverFlowMode, std::stoi,
                    "[0, 1]\n\tEnable or disable show overflow flag in 32 bit mode. [Eiger]");    

    INTEGER_COMMAND(storeinram, getStoreInRamMode, setStoreInRamMode, std::stoi,
                    "[0, 1]\n\tEnable or disable store in ram mode. [Eiger]");              
};

} // namespace sls
