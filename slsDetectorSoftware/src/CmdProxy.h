// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "sls/Detector.h"
#include "sls/Result.h"
#include "sls/network_utils.h"
#include "sls/sls_detector_exceptions.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace sls {

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
            auto t = det->GETFCN(std::vector<int>{det_id});                    \
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
                det->SETFCN(t, std::vector<int>{det_id});                      \
            } else if (args.size() == 2) {                                     \
                auto t = StringTo<time::ns>(args[0], args[1]);                 \
                det->SETFCN(t, std::vector<int>{det_id});                      \
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
            throw RuntimeError("Unknown action");                              \
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
            auto t = det->GETFCN(std::vector<int>{det_id});                    \
            if (args.empty()) {                                                \
                os << OutString(t) << '\n';                                    \
            } else if (args.size() == 1) {                                     \
                os << OutString(t, args[0]) << '\n';                           \
            } else {                                                           \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            throw RuntimeError("cannot put");                                  \
        } else {                                                               \
            throw RuntimeError("Unknown action");                              \
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
            auto t = det->GETFCN(std::vector<int>{det_id});                    \
            os << OutString(t) << '\n';                                        \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (args.size() != 1) {                                            \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
            det->SETFCN(args[0], std::vector<int>{det_id});                    \
            os << args.front() << '\n';                                        \
        } else {                                                               \
            throw RuntimeError("Unknown action");                              \
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
            auto t = det->GETFCN(std::vector<int>{det_id});                    \
            os << OutStringHex(t, 16) << '\n';                                 \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (args.size() != 1) {                                            \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
            auto val = CONV(args[0]);                                          \
            det->SETFCN(val, std::vector<int>{det_id});                        \
            os << ToStringHex(val, 16) << '\n';                                \
        } else {                                                               \
            throw RuntimeError("Unknown action");                              \
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
            auto t = det->GETFCN(std::vector<int>{det_id});                    \
            os << OutStringHex(t) << '\n';                                     \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (args.size() != 1) {                                            \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
            auto val = CONV(args[0]);                                          \
            det->SETFCN(val, std::vector<int>{det_id});                        \
            os << args.front() << '\n';                                        \
        } else {                                                               \
            throw RuntimeError("Unknown action");                              \
        }                                                                      \
        return os.str();                                                       \
    }

/** int or enum */
#define INTEGER_COMMAND_VEC_ID(CMDNAME, GETFCN, SETFCN, CONV, HLPSTR)          \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            if (!args.empty()) {                                               \
                WrongNumberOfParameters(0);                                    \
            }                                                                  \
            auto t = det->GETFCN(std::vector<int>{det_id});                    \
            os << OutString(t) << '\n';                                        \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (args.size() != 1) {                                            \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
            auto val = CONV(args[0]);                                          \
            det->SETFCN(val, std::vector<int>{det_id});                        \
            os << args.front() << '\n';                                        \
        } else {                                                               \
            throw RuntimeError("Unknown action");                              \
        }                                                                      \
        return os.str();                                                       \
    }

#define INTEGER_COMMAND_VEC_ID_GET(CMDNAME, GETFCN, SETFCN, CONV, HLPSTR)      \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            if (!args.empty()) {                                               \
                WrongNumberOfParameters(0);                                    \
            }                                                                  \
            auto t = det->GETFCN(std::vector<int>{det_id});                    \
            os << OutString(t) << '\n';                                        \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (args.size() != 1) {                                            \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
            auto val = CONV(args[0]);                                          \
            det->SETFCN(val, det_id);                                          \
            os << args.front() << '\n';                                        \
        } else {                                                               \
            throw RuntimeError("Unknown action");                              \
        }                                                                      \
        return os.str();                                                       \
    }

/** int or enum */
#define INTEGER_COMMAND_SINGLE_ID(CMDNAME, GETFCN, SETFCN, CONV, HLPSTR)       \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            if (!args.empty()) {                                               \
                WrongNumberOfParameters(0);                                    \
            }                                                                  \
            auto t = det->GETFCN(det_id);                                      \
            os << OutString(t) << '\n';                                        \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (args.size() != 1) {                                            \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
            auto val = CONV(args[0]);                                          \
            det->SETFCN(val, det_id);                                          \
            os << args.front() << '\n';                                        \
        } else {                                                               \
            throw RuntimeError("Unknown action");                              \
        }                                                                      \
        return os.str();                                                       \
    }

/** int, set no id, get id */
#define INTEGER_COMMAND_SET_NOID_GET_ID(CMDNAME, GETFCN, SETFCN, CONV, HLPSTR) \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            if (!args.empty()) {                                               \
                WrongNumberOfParameters(0);                                    \
            }                                                                  \
            auto t = det->GETFCN(std::vector<int>{det_id});                    \
            os << OutString(t) << '\n';                                        \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (det_id != -1) {                                                \
                throw RuntimeError("Cannot execute this at module level");     \
            }                                                                  \
            if (args.size() != 1) {                                            \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
            auto val = CONV(args[0]);                                          \
            det->SETFCN(val);                                                  \
            os << args.front() << '\n';                                        \
        } else {                                                               \
            throw RuntimeError("Unknown action");                              \
        }                                                                      \
        return os.str();                                                       \
    }

/** int, no id */
#define INTEGER_COMMAND_NOID(CMDNAME, GETFCN, SETFCN, CONV, HLPSTR)            \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (det_id != -1) {                                                    \
            throw RuntimeError("Cannot execute this at module level");         \
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
            throw RuntimeError("Unknown action");                              \
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
            auto t = det->GETFCN(INDEX, std::vector<int>{det_id});             \
            os << OutString(t) << '\n';                                        \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (args.size() != 1) {                                            \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
            auto val = CONV(args[0]);                                          \
            det->SETFCN(INDEX, val, std::vector<int>{det_id});                 \
            os << args.front() << '\n';                                        \
        } else {                                                               \
            throw RuntimeError("Unknown action");                              \
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
            auto t = det->GETFCN(INDEX, StringTo<int>(args[0]),                \
                                 std::vector<int>{det_id});                    \
            os << args[0] << ' ' << OutStringHex(t) << '\n';                   \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (args.size() != 2) {                                            \
                WrongNumberOfParameters(2);                                    \
            }                                                                  \
            auto val = CONV(args[1]);                                          \
            det->SETFCN(INDEX, StringTo<int>(args[0]), val,                    \
                        std::vector<int>{det_id});                             \
            os << args[0] << ' ' << args[1] << '\n';                           \
        } else {                                                               \
            throw RuntimeError("Unknown action");                              \
        }                                                                      \
        return os.str();                                                       \
    }

/** set only, no arguments, no id */
#define EXECUTE_SET_COMMAND_NOID(CMDNAME, SETFCN, HLPSTR)                      \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (det_id != -1) {                                                    \
            throw RuntimeError("Cannot execute this at module level");         \
        }                                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            throw RuntimeError("Cannot get");                                  \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (!args.empty()) {                                               \
                WrongNumberOfParameters(0);                                    \
            }                                                                  \
            det->SETFCN();                                                     \
            os << "successful\n";                                              \
        } else {                                                               \
            throw RuntimeError("Unknown action");                              \
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
            throw RuntimeError("Cannot get");                                  \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (!args.empty()) {                                               \
                WrongNumberOfParameters(0);                                    \
            }                                                                  \
            det->SETFCN(std::vector<int>{det_id});                             \
            os << "successful\n";                                              \
        } else {                                                               \
            throw RuntimeError("Unknown action");                              \
        }                                                                      \
        return os.str();                                                       \
    }

/** set only, 1 argument, no id */
#define EXECUTE_SET_COMMAND_NOID_1ARG(CMDNAME, SETFCN, HLPSTR)                 \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (det_id != -1) {                                                    \
            throw RuntimeError("Cannot execute this at module level");         \
        }                                                                      \
        if (action == slsDetectorDefs::HELP_ACTION)                            \
            os << HLPSTR << '\n';                                              \
        else if (action == slsDetectorDefs::GET_ACTION) {                      \
            throw RuntimeError("Cannot get");                                  \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (args.size() != 1) {                                            \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
            det->SETFCN(args[0]);                                              \
            os << args.front() << '\n';                                        \
        } else {                                                               \
            throw RuntimeError("Unknown action");                              \
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
            throw RuntimeError("Cannot get");                                  \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (args.size() != 1) {                                            \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
            det->SETFCN(args[0], std::vector<int>{det_id});                    \
            os << args.front() << '\n';                                        \
        } else {                                                               \
            throw RuntimeError("Unknown action");                              \
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
            auto t = det->GETFCN(std::vector<int>{det_id});                    \
            os << OutString(t) << '\n';                                        \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            throw RuntimeError("Cannot put");                                  \
        } else {                                                               \
            throw RuntimeError("Unknown action");                              \
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
            os << ToString(t) << '\n';                                         \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            throw RuntimeError("Cannot put");                                  \
        } else {                                                               \
            throw RuntimeError("Unknown action");                              \
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
            auto t = det->GETFCN(std::vector<int>{det_id});                    \
            os << OutStringHex(t) << '\n';                                     \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            throw RuntimeError("Cannot put");                                  \
        } else {                                                               \
            throw RuntimeError("Unknown action");                              \
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
            auto t = det->GETFCN(VAL, std::vector<int>{det_id});               \
            os << OutString(t) << APPEND << '\n';                              \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            throw RuntimeError("Cannot put");                                  \
        } else {                                                               \
            throw RuntimeError("Unknown action");                              \
        }                                                                      \
        return os.str();                                                       \
    }

#define CTB_NAMED_LIST(CMDNAME, GETFCN, SETFCN, HLPSTR)                        \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION) {                          \
            os << HLPSTR << '\n';                                              \
            return os.str();                                                   \
        }                                                                      \
        if (cmd != "daclist" &&                                                \
            det->getDetectorType().squash() != defs::CHIPTESTBOARD) {          \
            throw RuntimeError(cmd + " only allowed for CTB.");                \
        }                                                                      \
        if (det_id != -1) {                                                    \
            throw RuntimeError("Cannot configure " + cmd +                     \
                               " at module level");                            \
        }                                                                      \
        if (action == slsDetectorDefs::GET_ACTION) {                           \
            if (!args.empty()) {                                               \
                WrongNumberOfParameters(0);                                    \
            }                                                                  \
            auto t = det->GETFCN();                                            \
            os << ToString(t) << '\n';                                         \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (cmd == "daclist" &&                                            \
                det->getDetectorType().squash() != defs::CHIPTESTBOARD) {      \
                throw RuntimeError("This detector already has fixed dac "      \
                                   "names. Cannot change them.");              \
            }                                                                  \
            det->SETFCN(args);                                                 \
            os << ToString(args) << '\n';                                      \
        } else {                                                               \
            throw RuntimeError("Unknown action");                              \
        }                                                                      \
        return os.str();                                                       \
    }

#define CTB_VALUES(CMDNAME, GETFCN, GETFCNLIST, GETFCNNAME, HLPSTR)            \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION) {                          \
            os << HLPSTR << '\n';                                              \
            return os.str();                                                   \
        }                                                                      \
        if (action == slsDetectorDefs::GET_ACTION) {                           \
            if (!args.empty()) {                                               \
                WrongNumberOfParameters(0);                                    \
            }                                                                  \
            std::string suffix = " mV";                                        \
            auto t = det->GETFCNLIST();                                        \
            auto names = det->GETFCNNAME();                                    \
            auto name_it = names.begin();                                      \
            os << '[';                                                         \
            auto it = t.cbegin();                                              \
            os << ToString(*name_it++) << ' ';                                 \
            os << OutString(det->GETFCN(*it++, std::vector<int>{det_id}))      \
               << suffix;                                                      \
            while (it != t.cend()) {                                           \
                os << ", " << ToString(*name_it++) << ' ';                     \
                os << OutString(det->GETFCN(*it++, std::vector<int>{det_id}))  \
                   << suffix;                                                  \
            }                                                                  \
            os << "]\n";                                                       \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            throw RuntimeError("Cannot put");                                  \
        } else {                                                               \
            throw RuntimeError("Unknown action");                              \
        }                                                                      \
        return os.str();                                                       \
    }

#define CTB_SINGLE_DACNAME(CMDNAME, GETFCN, SETFCN, STARTINDEX, HLPSTR)        \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION) {                          \
            os << HLPSTR << '\n';                                              \
            return os.str();                                                   \
        }                                                                      \
        if (det->getDetectorType().squash() != defs::CHIPTESTBOARD) {          \
            throw RuntimeError(cmd + " only allowed for CTB.");                \
        }                                                                      \
        if (det_id != -1) {                                                    \
            throw RuntimeError("Cannot configure " + cmd +                     \
                               " at module level");                            \
        }                                                                      \
        defs::dacIndex index = defs::DAC_0;                                    \
        if (args.size() > 0) {                                                 \
            index = static_cast<defs::dacIndex>(StringTo<int>(args[0]) +       \
                                                STARTINDEX);                   \
        }                                                                      \
        if (action == slsDetectorDefs::GET_ACTION) {                           \
            if (args.size() != 1) {                                            \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
            auto t = det->GETFCN(index);                                       \
            os << args[0] << ' ' << t << '\n';                                 \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (args.size() != 2) {                                            \
                WrongNumberOfParameters(2);                                    \
            }                                                                  \
            det->SETFCN(index, args[1]);                                       \
            os << ToString(args) << '\n';                                      \
        } else {                                                               \
            throw RuntimeError("Unknown action");                              \
        }                                                                      \
        return os.str();                                                       \
    }

#define CTB_GET_DACINDEX(CMDNAME, GETFCN, STARTINDEX, HLPSTR)                  \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION) {                          \
            os << HLPSTR << '\n';                                              \
            return os.str();                                                   \
        }                                                                      \
        if (det->getDetectorType().squash() != defs::CHIPTESTBOARD) {          \
            throw RuntimeError(cmd + " only allowed for CTB.");                \
        }                                                                      \
        if (det_id != -1) {                                                    \
            throw RuntimeError("Cannot configure " + cmd +                     \
                               " at module level");                            \
        }                                                                      \
        if (action == slsDetectorDefs::GET_ACTION) {                           \
            if (args.size() != 1) {                                            \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
            auto t = det->GETFCN(args[0]);                                     \
            os << ToString(static_cast<int>(t) - STARTINDEX) << '\n';          \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            throw RuntimeError("Cannot put");                                  \
        } else {                                                               \
            throw RuntimeError("Unknown action");                              \
        }                                                                      \
        return os.str();                                                       \
    }

#define CTB_SINGLE_NAME(CMDNAME, GETFCN, SETFCN, HLPSTR)                       \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION) {                          \
            os << HLPSTR << '\n';                                              \
            return os.str();                                                   \
        }                                                                      \
        if (det->getDetectorType().squash() != defs::CHIPTESTBOARD) {          \
            throw RuntimeError(cmd + " only allowed for CTB.");                \
        }                                                                      \
        if (det_id != -1) {                                                    \
            throw RuntimeError("Cannot configure " + cmd +                     \
                               " at module level");                            \
        }                                                                      \
        if (action == slsDetectorDefs::GET_ACTION) {                           \
            if (args.size() != 1) {                                            \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
            auto t = det->GETFCN(StringTo<int>(args[0]));                      \
            os << args[0] << ' ' << t << '\n';                                 \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            if (args.size() != 2) {                                            \
                WrongNumberOfParameters(2);                                    \
            }                                                                  \
            det->SETFCN(StringTo<int>(args[0]), args[1]);                      \
            os << ToString(args) << '\n';                                      \
        } else {                                                               \
            throw RuntimeError("Unknown action");                              \
        }                                                                      \
        return os.str();                                                       \
    }

#define CTB_GET_INDEX(CMDNAME, GETFCN, HLPSTR)                                 \
    std::string CMDNAME(const int action) {                                    \
        std::ostringstream os;                                                 \
        os << cmd << ' ';                                                      \
        if (action == slsDetectorDefs::HELP_ACTION) {                          \
            os << HLPSTR << '\n';                                              \
            return os.str();                                                   \
        }                                                                      \
        if (det->getDetectorType().squash() != defs::CHIPTESTBOARD) {          \
            throw RuntimeError(cmd + " only allowed for CTB.");                \
        }                                                                      \
        if (det_id != -1) {                                                    \
            throw RuntimeError("Cannot configure " + cmd +                     \
                               " at module level");                            \
        }                                                                      \
        if (action == slsDetectorDefs::GET_ACTION) {                           \
            if (args.size() != 1) {                                            \
                WrongNumberOfParameters(1);                                    \
            }                                                                  \
            auto t = det->GETFCN(args[0]);                                     \
            os << ToString(static_cast<int>(t)) << '\n';                       \
        } else if (action == slsDetectorDefs::PUT_ACTION) {                    \
            throw RuntimeError("Cannot put");                                  \
        } else {                                                               \
            throw RuntimeError("Unknown action");                              \
        }                                                                      \
        return os.str();                                                       \
    }

class CmdProxy {
  public:
    explicit CmdProxy(Detector *ptr) : det(ptr) {}

    void Call(const std::string &command,
              const std::vector<std::string> &arguments, int detector_id = -1,
              int action = -1, std::ostream &os = std::cout,
              int receiver_id = -1);

    bool ReplaceIfDepreciated(std::string &command);
    size_t GetFunctionMapSize() const noexcept { return functions.size(); };
    std::vector<std::string> GetProxyCommands();
    std::map<std::string, std::string> GetDepreciatedCommands();

  private:
    Detector *det;
    std::string cmd;
    std::vector<std::string> args;
    int det_id{-1};
    int rx_id{-1};

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

    template <typename V> std::string OutString(const Result<V> &value) {
        if (value.equal())
            return ToString(value.front());
        return ToString(value);
    }

    template <typename V> std::string OutString(const V &value) {
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

    StringMap depreciated_functions{
        /* configuration */
        {"detectorversion", "firmwareversion"},
        {"softwareversion", "detectorserverversion"},
        {"receiverversion", "rx_version"},
        {"detectornumber", "serialnumber"},
        {"thisversion", "clientversion"},
        {"detsizechan", "detsize"},
        {"trimdir", "settingspath"},
        {"settingsdir", "settingspath"},
        {"flippeddatax", "fliprows"},

        /* acquisition parameters */
        {"cycles", "triggers"},
        {"cyclesl", "triggersl"},
        {"clkdivider", "readoutspeed"},
        {"speed", "readoutspeed"},
        {"vhighvoltage", "highvoltage"},
        {"digitest", "imagetest"},
        {"filter", "filterresistor"},
        {"readnlines", "readnrows"},

        /** temperature */

        /** super old dacs */
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
        /** dacs */
        {"vthreshold", "dac"},
        {"vsvp", "dac"},
        {"vsvn", "dac"},
        {"vtrim", "dac"},
        {"vrpreamp", "dac"},
        {"vrshaper", "dac"},
        {"vtgstv", "dac"},
        {"vcmp_ll", "dac"},
        {"vcmp_lr", "dac"},
        {"vcal", "dac"},
        {"vcmp_rl", "dac"},
        {"vcmp_rr", "dac"},
        {"rxb_rb", "dac"},
        {"rxb_lb", "dac"},
        {"vcp", "dac"},
        {"vcn", "dac"},
        {"vishaper", "dac"},
        {"iodelay", "dac"},
        {"vref_ds", "dac"},
        {"vcascn_pb", "dac"},
        {"vcascp_pb", "dac"},
        {"vout_cm", "dac"},
        {"vcasc_out", "dac"},
        {"vin_cm", "dac"},
        {"vref_comp", "dac"},
        {"ib_test_c", "dac"},
        {"vrshaper_n", "dac"},
        {"vipre", "dac"},
        {"vdcsh", "dac"},
        {"vth1", "dac"},
        {"vth2", "dac"},
        {"vth3", "dac"},
        {"vcal_n", "dac"},
        {"vcal_p", "dac"},
        {"vcassh", "dac"},
        {"vcas", "dac"},
        {"vicin", "dac"},
        {"vipre_out", "dac"},
        {"vref_h_adc", "dac"},
        {"vb_comp_fe", "dac"},
        {"vb_comp_adc", "dac"},
        {"vcom_cds", "dac"},
        {"vref_rstore", "dac"},
        {"vb_opa_1st", "dac"},
        {"vref_comp_fe", "dac"},
        {"vcom_adc1", "dac"},
        {"vref_prech", "dac"},
        {"vref_l_adc", "dac"},
        {"vref_cds", "dac"},
        {"vb_cs", "dac"},
        {"vb_opa_fd", "dac"},
        {"vcom_adc2", "dac"},
        {"vb_ds", "dac"},
        {"vb_comp", "dac"},
        {"vb_pixbuf", "dac"},
        {"vin_com", "dac"},
        {"vdd_prot", "dac"},
        {"vbp_colbuf", "dac"},
        {"vb_sda", "dac"},
        {"vcasc_sfp", "dac"},
        {"vipre_cds", "dac"},
        {"ibias_sfp", "dac"},

        {"defaultdacs", "resetdacs"},

        /* acquisition */
        {"busy", "clearbusy"},
        {"receiver", "rx_status"},
        {"framescaught", "rx_framescaught"},
        {"startingfnum", "nextframenumber"},

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
        {"txndelay_frame", "txdelay_frame"},
        {"txndelay_left", "txdelay_left"},
        {"txndelay_right", "txdelay_right"},

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
        {"resmat", "partialreset"},

        /* Jungfrau Specific */
        {"storagecells", "extrastoragecells"},
        {"auto_comp_disable", "autocompdisable"},
        {"comp_disable_time", "compdisabletime"},

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
        {"copydetectorserver", "updatedetectorserver"},

        /* Insignificant */
        {"nframes", "framecounter"},
        {"now", "runtime"},
        {"timestamp", "frametime"},
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
        {"hardwareversion", &CmdProxy::hardwareversion},
        {"detectorserverversion", &CmdProxy::detectorserverversion},
        {"kernelversion", &CmdProxy::kernelversion},
        {"rx_version", &CmdProxy::rx_version},
        {"serialnumber", &CmdProxy::serialnumber},
        {"moduleid", &CmdProxy::moduleid},
        {"type", &CmdProxy::type},
        {"nmod", &CmdProxy::nmod},
        {"detsize", &CmdProxy::DetectorSize},
        {"settingslist", &CmdProxy::settingslist},
        {"settings", &CmdProxy::settings},
        {"threshold", &CmdProxy::Threshold},
        {"thresholdnotb", &CmdProxy::Threshold},
        {"settingspath", &CmdProxy::settingspath},
        {"trimbits", &CmdProxy::Trimbits},
        {"trimval", &CmdProxy::trimval},
        {"trimen", &CmdProxy::TrimEnergies},
        {"gappixels", &CmdProxy::GapPixels},
        {"fliprows", &CmdProxy::fliprows},
        {"master", &CmdProxy::master},
        {"sync", &CmdProxy::sync},
        {"badchannels", &CmdProxy::BadChannels},
        {"row", &CmdProxy::row},
        {"column", &CmdProxy::column},

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
        {"dr", &CmdProxy::dr},
        {"drlist", &CmdProxy::drlist},
        {"timing", &CmdProxy::timing},
        {"timinglist", &CmdProxy::timinglist},
        {"readoutspeed", &CmdProxy::ReadoutSpeed},
        {"readoutspeedlist", &CmdProxy::readoutspeedlist},
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
        {"filterresistor", &CmdProxy::filterresistor},
        {"currentsource", &CmdProxy::CurrentSource},
        {"dbitpipeline", &CmdProxy::dbitpipeline},
        {"readnrows", &CmdProxy::readnrows},

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

        /* lists */
        {"daclist", &CmdProxy::daclist},
        {"dacname", &CmdProxy::dacname},
        {"dacindex", &CmdProxy::dacindex},
        {"adclist", &CmdProxy::adclist},
        {"adcname", &CmdProxy::adcname},
        {"adcindex", &CmdProxy::adcindex},
        {"signallist", &CmdProxy::signallist},
        {"signalname", &CmdProxy::signalname},
        {"signalindex", &CmdProxy::signalindex},
        {"powerlist", &CmdProxy::powerlist},
        {"powername", &CmdProxy::powername},
        {"powerindex", &CmdProxy::powerindex},
        {"powervalues", &CmdProxy::powervalues},
        {"slowadclist", &CmdProxy::slowadclist},
        {"slowadcname", &CmdProxy::slowadcname},
        {"slowadcindex", &CmdProxy::slowadcindex},
        {"slowadcvalues", &CmdProxy::slowadcvalues},

        /* dacs */
        {"dac", &CmdProxy::Dac},
        {"dacvalues", &CmdProxy::DacValues},
        {"resetdacs", &CmdProxy::ResetDacs},
        {"defaultdac", &CmdProxy::DefaultDac},

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
        {"readout", &CmdProxy::readout},
        {"stop", &CmdProxy::stop},
        {"rx_status", &CmdProxy::ReceiverStatus},
        {"status", &CmdProxy::DetectorStatus},
        {"rx_framescaught", &CmdProxy::rx_framescaught},
        {"rx_missingpackets", &CmdProxy::rx_missingpackets},
        {"rx_frameindex", &CmdProxy::rx_frameindex},
        {"nextframenumber", &CmdProxy::nextframenumber},
        {"trigger", &CmdProxy::Trigger},
        {"scan", &CmdProxy::Scan},
        {"scanerrmsg", &CmdProxy::scanerrmsg},

        /* Network Configuration (Detector<->Receiver) */
        {"numinterfaces", &CmdProxy::numinterfaces},
        {"selinterface", &CmdProxy::selinterface},
        {"udp_dstlist", &CmdProxy::UDPDestinationList},
        {"udp_numdst", &CmdProxy::udp_numdst},
        {"udp_cleardst", &CmdProxy::udp_cleardst},
        {"udp_firstdst", &CmdProxy::udp_firstdst},
        {"udp_srcip", &CmdProxy::UDPSourceIP},
        {"udp_srcip2", &CmdProxy::UDPSourceIP2},
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
        {"txdelay_frame", &CmdProxy::txdelay_frame},
        {"txdelay_left", &CmdProxy::txdelay_left},
        {"txdelay_right", &CmdProxy::txdelay_right},
        {"txdelay", &CmdProxy::TransmissionDelay},

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
        {"rx_arping", &CmdProxy::rx_arping},
        {"rx_roi", &CmdProxy::Rx_ROI},
        {"rx_clearroi", &CmdProxy::rx_clearroi},

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
        {"zmqhwm", &CmdProxy::ZMQHWM},
        {"rx_zmqhwm", &CmdProxy::rx_zmqhwm},

        /* Eiger Specific */
        {"blockingtrigger", &CmdProxy::Trigger},
        {"subexptime", &CmdProxy::subexptime},
        {"subdeadtime", &CmdProxy::subdeadtime},
        {"overflow", &CmdProxy::overflow},
        {"ratecorr", &CmdProxy::RateCorrection},
        {"interruptsubframe", &CmdProxy::interruptsubframe},
        {"measuredperiod", &CmdProxy::measuredperiod},
        {"measuredsubperiod", &CmdProxy::measuredsubperiod},
        {"activate", &CmdProxy::activate},
        {"partialreset", &CmdProxy::partialreset},
        {"pulse", &CmdProxy::PulsePixel},
        {"pulsenmove", &CmdProxy::PulsePixelAndMove},
        {"pulsechip", &CmdProxy::PulseChip},
        {"quad", &CmdProxy::Quad},
        {"datastream", &CmdProxy::DataStream},
        {"top", &CmdProxy::top},

        /* Jungfrau Specific */
        {"chipversion", &CmdProxy::chipversion},
        {"temp_threshold", &CmdProxy::temp_threshold},
        {"temp_control", &CmdProxy::temp_control},
        {"temp_event", &CmdProxy::TemperatureEvent},
        {"autocompdisable", &CmdProxy::autocompdisable},
        {"compdisabletime", &CmdProxy::compdisabletime},
        {"extrastoragecells", &CmdProxy::extrastoragecells},
        {"storagecell_start", &CmdProxy::storagecell_start},
        {"storagecell_delay", &CmdProxy::storagecell_delay},
        {"gainmode", &CmdProxy::gainmode},
        {"filtercells", &CmdProxy::filtercells},
        {"pedestalmode", &CmdProxy::PedestalMode},

        /* Gotthard Specific */
        {"roi", &CmdProxy::ROI},
        {"clearroi", &CmdProxy::clearroi},
        {"exptimel", &CmdProxy::exptimel},

        /* Gotthard2 Specific */
        {"bursts", &CmdProxy::bursts},
        {"burstperiod", &CmdProxy::burstperiod},
        {"burstsl", &CmdProxy::burstsl},
        {"inj_ch", &CmdProxy::InjectChannel},
        {"vetophoton", &CmdProxy::VetoPhoton},
        {"vetoref", &CmdProxy::VetoReference},
        {"vetofile", &CmdProxy::VetoFile},
        {"burstmode", &CmdProxy::BurstMode},
        {"cdsgain", &CmdProxy::cdsgain},
        {"timingsource", &CmdProxy::timingsource},
        {"veto", &CmdProxy::veto},
        {"vetostream", &CmdProxy::VetoStreaming},
        {"vetoalg", &CmdProxy::VetoAlgorithm},
        {"confadc", &CmdProxy::ConfigureADC},

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
        {"gaincaps", &CmdProxy::GainCaps},
        {"polarity", &CmdProxy::polarity},
        {"interpolation", &CmdProxy::interpolation},
        {"pumpprobe", &CmdProxy::pumpprobe},
        {"apulse", &CmdProxy::apulse},
        {"dpulse", &CmdProxy::dpulse},

        /* CTB Specific */
        {"samples", &CmdProxy::Samples},
        {"asamples", &CmdProxy::asamples},
        {"adcclk", &CmdProxy::adcclk},
        {"runclk", &CmdProxy::runclk},
        {"syncclk", &CmdProxy::syncclk},
        {"v_limit", &CmdProxy::v_limit},
        {"adcenable", &CmdProxy::adcenable},
        {"adcenable10g", &CmdProxy::adcenable10g},
        {"transceiverenable", &CmdProxy::transceiverenable},
        {"dsamples", &CmdProxy::dsamples},
        {"tsamples", &CmdProxy::tsamples},
        {"romode", &CmdProxy::romode},
        {"dbitclk", &CmdProxy::dbitclk},
        {"adcvpp", &CmdProxy::AdcVpp},
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
        {"slowadc", &CmdProxy::SlowADC},
        {"extsampling", &CmdProxy::extsampling},
        {"extsamplingsrc", &CmdProxy::extsamplingsrc},
        {"rx_dbitlist", &CmdProxy::ReceiverDbitList},
        {"rx_dbitoffset", &CmdProxy::rx_dbitoffset},
        {"diodelay", &CmdProxy::DigitalIODelay},
        {"led", &CmdProxy::led},

        /* Pattern */
        {"pattern", &CmdProxy::Pattern},
        {"patfname", &CmdProxy::patfname},
        {"savepattern", &CmdProxy::savepattern},
        {"defaultpattern", &CmdProxy::defaultpattern},
        {"patioctrl", &CmdProxy::patioctrl},
        {"patword", &CmdProxy::PatternWord},
        {"patlimits", &CmdProxy::PatternLoopAddresses},
        {"patloop", &CmdProxy::PatternLoopAddresses},
        {"patloop0", &CmdProxy::PatternLoopAddresses},
        {"patloop1", &CmdProxy::PatternLoopAddresses},
        {"patloop2", &CmdProxy::PatternLoopAddresses},
        {"patnloop", &CmdProxy::PatternLoopCycles},
        {"patnloop0", &CmdProxy::PatternLoopCycles},
        {"patnloop1", &CmdProxy::PatternLoopCycles},
        {"patnloop2", &CmdProxy::PatternLoopCycles},
        {"patwait", &CmdProxy::PatternWaitAddress},
        {"patwait0", &CmdProxy::PatternWaitAddress},
        {"patwait1", &CmdProxy::PatternWaitAddress},
        {"patwait2", &CmdProxy::PatternWaitAddress},
        {"patwaittime", &CmdProxy::PatternWaitTime},
        {"patwaittime0", &CmdProxy::PatternWaitTime},
        {"patwaittime1", &CmdProxy::PatternWaitTime},
        {"patwaittime2", &CmdProxy::PatternWaitTime},
        {"patmask", &CmdProxy::patmask},
        {"patsetbit", &CmdProxy::patsetbit},
        {"patternstart", &CmdProxy::patternstart},

        /* Moench */

        /* Advanced */
        {"adcpipeline", &CmdProxy::adcpipeline},
        {"rx_jsonaddheader", &CmdProxy::AdditionalJsonHeader},
        {"rx_jsonpara", &CmdProxy::JsonParameter},
        {"programfpga", &CmdProxy::ProgramFpga},
        {"resetfpga", &CmdProxy::resetfpga},
        {"updatedetectorserver", &CmdProxy::UpdateDetectorServer},
        {"updatekernel", &CmdProxy::UpdateKernel},
        {"rebootcontroller", &CmdProxy::rebootcontroller},
        {"update", &CmdProxy::UpdateFirmwareAndDetectorServer},
        {"updatemode", &CmdProxy::updatemode},
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
        {"framecounter", &CmdProxy::framecounter},
        {"runtime", &CmdProxy::runtime},
        {"frametime", &CmdProxy::frametime},
        {"user", &CmdProxy::UserDetails}

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
    std::string Threshold(int action);
    std::string Trimbits(int action);
    std::string TrimEnergies(int action);
    std::string GapPixels(int action);
    std::string BadChannels(int action);
    /* acquisition parameters */
    std::string Acquire(int action);
    std::string Exptime(int action);
    std::string ReadoutSpeed(int action);
    std::string Adcphase(int action);
    std::string Dbitphase(int action);
    std::string ClockFrequency(int action);
    std::string ClockPhase(int action);
    std::string MaxClockPhaseShift(int action);
    std::string ClockDivider(int action);
    std::string ExternalSignal(int action);
    std::string CurrentSource(int action);
    /** temperature */
    std::string TemperatureValues(int action);
    /* list */
    /* dacs */
    std::string Dac(int action);
    std::string DacValues(int action);
    std::string ResetDacs(int action);
    std::string DefaultDac(int action);
    /* acquisition */
    std::string ReceiverStatus(int action);
    std::string DetectorStatus(int action);
    std::string RxMissingPackets(int action);
    std::string Scan(int action);
    std::string Trigger(int action);
    /* Network Configuration (Detector<->Receiver) */
    IpAddr getDstIpFromAuto();
    IpAddr getSrcIpFromAuto();
    UdpDestination getUdpEntry();
    std::string UDPDestinationList(int action);
    std::string UDPSourceIP(int action);
    std::string UDPSourceIP2(int action);
    std::string UDPDestinationIP(int action);
    std::string UDPDestinationIP2(int action);
    std::string TransmissionDelay(int action);
    /* Receiver Config */
    std::string ReceiverHostname(int action);
    std::string Rx_ROI(int action);
    /* File */
    /* ZMQ Streaming Parameters (Receiver<->Client) */
    std::string ZMQHWM(int action);
    /* Eiger Specific */
    std::string RateCorrection(int action);
    std::string PulsePixel(int action);
    std::string PulsePixelAndMove(int action);
    std::string PulseChip(int action);
    std::string Quad(int action);
    std::string DataStream(int action);
    /* Jungfrau Specific */
    std::string TemperatureEvent(int action);
    std::string PedestalMode(int action);
    /* Gotthard Specific */
    std::string ROI(int action);
    /* Gotthard2 Specific */
    std::string InjectChannel(int action);
    std::string VetoPhoton(int action);
    std::string VetoReference(int action);
    std::string VetoFile(int action);
    std::string BurstMode(int action);
    std::string VetoStreaming(int action);
    std::string VetoAlgorithm(int action);
    std::string ConfigureADC(int action);
    /* Mythen3 Specific */
    std::string Counters(int action);
    std::string GateDelay(int action);
    std::string GainCaps(int action);
    /* CTB/ Moench Specific */
    std::string Samples(int action);
    /* CTB Specific */
    std::string AdcVpp(int action);
    std::string SlowADC(int action);
    std::string ReceiverDbitList(int action);
    std::string DigitalIODelay(int action);
    /* Pattern */
    std::string Pattern(int action);
    std::string PatternWord(int action);
    void GetLevelAndUpdateArgIndex(int action,
                                   std::string levelSeparatedCommand,
                                   int &level, int &iArg, size_t nGetArgs,
                                   size_t nPutArgs);
    std::string PatternLoopAddresses(int action);
    std::string PatternLoopCycles(int action);
    std::string PatternWaitAddress(int action);
    std::string PatternWaitTime(int action);
    /* Moench */
    std::string AdditionalJsonHeader(int action);
    std::string JsonParameter(int action);
    /* Advanced */
    std::string ProgramFpga(int action);
    std::string UpdateDetectorServer(int action);
    std::string UpdateKernel(int action);
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

    EXECUTE_SET_COMMAND_NOID_1ARG(parameters, loadParameters,
                                  "[fname]\n\tSets detector measurement "
                                  "parameters to those contained in "
                                  "fname. Set up per measurement.");

    GET_COMMAND(detectorserverversion, getDetectorServerVersion,
                "\n\tOn-board detector server software version");

    GET_COMMAND(hardwareversion, getHardwareVersion,
                "\n\t[Jungfrau][Gotthard2][Myhten3][Gotthard][Ctb][Moench] "
                "Hardware version of detector. \n\t[Eiger] Hardware version of "
                "front FPGA on detector.");

    GET_COMMAND(
        kernelversion, getKernelVersion,
        "\n\tGet kernel version on the detector including time and date.");

    GET_COMMAND(rx_version, getReceiverVersion, "\n\tReceiver version");

    GET_COMMAND_HEX(serialnumber, getSerialNumber,
                    "\n\t[Jungfrau][Moench][Gotthard][Mythen3][Gotthard2][CTB]"
                    "Serial number of detector.");

    GET_COMMAND(
        moduleid, getModuleId,
        "\n\t[Gotthard2][Eiger][Mythen3][Jungfrau][Moench] 16 bit value "
        "(ideally unique) that is streamed out in the UDP header of "
        "the detector. Picked up from a file on the module.");

    GET_COMMAND(type, getDetectorType,
                "\n\tReturns detector type. Can be Eiger, Jungfrau, Gotthard, "
                "Moench, Mythen3, Gotthard2, ChipTestBoard");

    GET_COMMAND_NOID(nmod, size, "\n\tNumber of modules in shared memory.");

    GET_COMMAND_NOID(settingslist, getSettingsList,
                     "\n\tList of settings implemented for this detector.");

    INTEGER_COMMAND_VEC_ID(
        settings, getSettings, setSettings,
        StringTo<slsDetectorDefs::detectorSettings>,
        "[standard, fast, highgain, dynamicgain, lowgain, "
        "mediumgain, veryhighgain, highgain0, "
        "fixgain1, fixgain2, forceswitchg1, forceswitchg2, "
        "verylowgain, g1_hg, g1_lg, g2_hc_hg, g2_hc_lg, "
        "g2_lc_hg, g2_lc_lg, g4_hg, g4_lg, gain0]"
        "\n\t Detector Settings"
        "\n\t[Jungfrau] - [ gain0 | highgain0]"
        "\n\t[Gotthard] - [dynamicgain | highgain | lowgain | "
        "mediumgain | veryhighgain]"
        "\n\t[Gotthard] Also loads default dacs on to the detector."
        "\n\t[Gotthard2] - [dynamicgain | fixgain1 | fixgain2]"
        "\n\t[Mythen3] - [standard | fast | highgain] Also changes vrshaper "
        "and vrpreamp. \n\t[Eiger] Use threshold or thresholdnotb. \n\t[Eiger] "
        "threshold and settings loaded from file found in settingspath. "
        "\n\t[Moench] - [g1_hg | g1_lg | g2_hc_hg | g2_hc_lg | "
        "g2_lc_hg | g2_lc_lg | g4_hg | g4_lg]");

    STRING_COMMAND(settingspath, getSettingsPath, setSettingsPath,
                   "[path]\n\t[Eiger][Mythen3] Directory where settings files "
                   "are loaded from/to.");

    INTEGER_COMMAND_VEC_ID(
        trimval, getAllTrimbits, setAllTrimbits, StringTo<int>,
        "[n_trimval]\n\t[Eiger][Mythen3] All trimbits set to this "
        "value. Returns -1 if all trimbits are different values.");

    INTEGER_COMMAND_VEC_ID(
        fliprows, getFlipRows, setFlipRows, StringTo<int>,
        "[0, 1]\n\t[Eiger] flips rows paramater sent to slsreceiver "
        "to stream as json parameter to flip rows in gui "
        "\n\t[Jungfrau][Moench] flips "
        "rows in the detector itself. For bottom module and number of "
        "interfaces must be set to 2. slsReceiver and slsDetectorGui "
        "does not handle.");

    INTEGER_COMMAND_VEC_ID_GET(master, getMaster, setMaster, StringTo<int>,
                               "[0, 1]\n\t[Eiger][Gotthard2][Jungfrau][Moench] "
                               "Sets (half) module to master "
                               "and other(s) to "
                               "slaves.\n\t[Gotthard][Gotthard2][Mythen3]["
                               "Eiger][Jungfrau][Moench] Gets if "
                               "the current (half) module is master.");

    INTEGER_COMMAND_SET_NOID_GET_ID(
        sync, getSynchronization, setSynchronization, StringTo<int>,
        "[0, 1]\n\t[Jungfrau][Moench] Enables or disables "
        "synchronization between modules.");

    INTEGER_COMMAND_VEC_ID(row, getRow, setRow, StringTo<int>,
                           "[value]\n\tSet Detector row (udp header) to value. "
                           "\n\tGui uses it to rearrange for complete image");

    INTEGER_COMMAND_VEC_ID(
        column, getColumn, setColumn, StringTo<int>,
        "[value]\n\tSet Detector column (udp header) to value. \n\tGui uses it "
        "to rearrange for complete image");

    /* acquisition parameters */

    INTEGER_COMMAND_SET_NOID_GET_ID(
        frames, getNumberOfFrames, setNumberOfFrames, StringTo<int64_t>,
        "[n_frames]\n\tNumber of frames per acquisition. In "
        "trigger mode, number of frames per trigger. \n\tCannot be set in "
        "modular level. \n\tIn scan mode, number of frames is set to "
        "number of steps.\n\t[Gotthard2] Burst mode has a maximum of 2720 "
        "frames.");

    INTEGER_COMMAND_SET_NOID_GET_ID(
        triggers, getNumberOfTriggers, setNumberOfTriggers, StringTo<int64_t>,
        "[n_triggers]\n\tNumber of triggers per aquire. Set "
        "timing mode to use triggers.");

    TIME_COMMAND(
        period, getPeriod, setPeriod,
        "[duration] [(optional unit) ns|us|ms|s]\n\tPeriod between frames");

    TIME_COMMAND(delay, getDelayAfterTrigger, setDelayAfterTrigger,
                 "[duration] [(optional unit) "
                 "ns|us|ms|s]\n\t[Jungfrau][Moench][Gotthard][Mythen3]["
                 "Gotthard2][Ctb][Moench] Delay after trigger");

    GET_COMMAND(framesl, getNumberOfFramesLeft,
                "\n\t[Gotthard][Jungfrau][Moench][Mythen3][Gotthard2][CTB] "
                "Number of frames left in acquisition."
                "\n\t[Gotthard2] only in continuous auto mode.");

    GET_COMMAND(triggersl, getNumberOfTriggersLeft,
                "\n\t[Gotthard][Jungfrau][Moench][Mythen3][Gotthard2][CTB] "
                "Number of triggers left in acquisition. Only when external "
                "trigger used.");

    TIME_GET_COMMAND(delayl, getDelayAfterTriggerLeft,
                     "\n\t[Gotthard][Jungfrau][Moench][Mythen3][Gotthard2][CTB]"
                     " DelayLeft Delay Left in Acquisition."
                     "\n\t[Gotthard2] only in continuous mode.");

    TIME_GET_COMMAND(periodl, getPeriodLeft,
                     "\n\t[Gotthard][Jungfrau][Moench][CTB][Mythen3][Gotthard2]"
                     " Period left for current frame."
                     "\n\t[Gotthard2] only in continuous mode.");

    INTEGER_COMMAND_SET_NOID_GET_ID(
        dr, getDynamicRange, setDynamicRange, StringTo<int>,
        "[value]\n\tDynamic Range or number of bits per "
        "pixel in detector.\n\t"
        "[Eiger] Options: 4, 8, 12, 16, 32. If set to 32, also sets "
        "clkdivider to 2, else to 0.\n\t"
        "[Mythen3] Options: 8, 16, 32\n\t"
        "[Jungfrau][Moench][Gotthard][Ctb][Mythen3][Gotthard2] 16");

    GET_COMMAND_NOID(drlist, getDynamicRangeList,
                     "\n\tGets the list of dynamic ranges for this detector.");

    INTEGER_COMMAND_VEC_ID(
        timing, getTimingMode, setTimingMode,
        StringTo<slsDetectorDefs::timingMode>,
        "[auto|trigger|gating|burst_trigger]\n\tTiming Mode of "
        "detector.\n\t[Jungfrau][Moench][Gotthard][Ctb][Gotthard2] "
        "[auto|trigger]\n\t[Mythen3] "
        "[auto|trigger|gating|trigger_gating]\n\t[Eiger] "
        "[auto|trigger|gating|burst_trigger]");

    GET_COMMAND_NOID(timinglist, getTimingModeList,
                     "\n\tGets the list of timing modes for this detector.");

    GET_COMMAND_NOID(
        readoutspeedlist, getReadoutSpeedList,
        "\n\tList of readout speed levels implemented for this detector.");

    GET_COMMAND(maxadcphaseshift, getMaxADCPhaseShift,
                "\n\t[Jungfrau][Moench][CTB] Absolute maximum Phase shift of "
                "ADC clock.");

    GET_COMMAND(maxdbitphaseshift, getMaxDBITPhaseShift,
                "\n\t[CTB][Jungfrau] Absolute maximum Phase shift of of the "
                "clock to latch digital bits.");

    INTEGER_COMMAND_VEC_ID(highvoltage, getHighVoltage, setHighVoltage,
                           StringTo<int>,
                           "[n_value]\n\tHigh voltage to the sensor in Voltage."
                           "\n\t[Gotthard] [0|90|110|120|150|180|200]"
                           "\n\t[Eiger][Mythen3][Gotthard2] 0-200"
                           "\n\t[Jungfrau][Moench][Ctb] [0|60-200]");

    INTEGER_COMMAND_VEC_ID(
        powerchip, getPowerChip, setPowerChip, StringTo<int>,
        "[0, 1]\n\t[Jungfrau][Moench][Mythen3][Gotthard2] Power "
        "the chip. \n\t[Jungfrau][Moench] Default is 0. Get "
        "will return power status. Can be off if temperature event occured "
        "(temperature over temp_threshold with temp_control "
        "enabled. Will configure chip (only chip v1.1)\n\t[Mythen3][Gotthard2] "
        "Default is 1. If module not connected or wrong module, powerchip will "
        "fail.");

    INTEGER_COMMAND_VEC_ID(
        imagetest, getImageTestMode, setImageTestMode, StringTo<int>,
        "[0, 1]\n\t[Gotthard] 1 adds channel intensity with precalculated "
        "values when taking an acquisition. Default is 0."
        "\n\t[Eiger][Jungfrau][Moench] Only for Virtual servers. If 0, each "
        "pixel intensity incremented by 1. If 1, all pixels almost saturated.");

    INTEGER_COMMAND_VEC_ID(
        parallel, getParallelMode, setParallelMode, StringTo<int>,
        "[0, 1]\n\t[Eiger][Mythen3][Gotthard2][Moench] Enable or disable "
        "parallel "
        "mode.\n\t[Mythen3] If exptime is too short, the "
        "acquisition will return ERROR status and take fewer "
        "frames than expected.\n\t[Mythen3][Eiger][Moench] Default: Non "
        "parallel.\n\t[Gotthard2] Default: Parallel. Non parallel mode works "
        "only in continuous mode.");

    INTEGER_COMMAND_VEC_ID(
        filterresistor, getFilterResistor, setFilterResistor, StringTo<int>,
        "[value] [Gotthard2][Jungfrau] Set filter resistor. Increasing "
        "values for increasing resistance.\n\t[Gotthard2] Options: "
        "[0|1|2|3]. Default is 0.\n\t[Jungfrau] Options: [0|1]. Default is 1.");

    INTEGER_COMMAND_VEC_ID(dbitpipeline, getDBITPipeline, setDBITPipeline,
                           StringTo<int>,
                           "[n_value]\n\t[Ctb][Gotthard2] Pipeline of the "
                           "clock for latching digital bits.\n\t[Gotthard2] "
                           "Options: 0-7\n\t[CTB] Options: 0-255");

    INTEGER_COMMAND_VEC_ID(
        readnrows, getReadNRows, setReadNRows, StringTo<int>,
        "\n\t[1-256]\n\t\t[Eiger] Number of rows to readout per half "
        "module starting from the centre. Options: 0 - 256. 256 is default. "
        "The permissible values depend on dynamic range and 10Gbe "
        "enabled.\n\t[8-512 (multiple of 8)]\n\t\t[Jungfrau] Number of rows "
        "per module starting from the centre. Options: 8 - 512, must be "
        "multiples of 8. Default is 512.\n\t\t[Moench] Number of rows per "
        "module starting from the centre. Options:16 - 400, must be multiples "
        "of 16. Default is 400.");

    /** temperature */
    GET_COMMAND_NOID(
        templist, getTemperatureList,
        "\n\tList of temperature commands implemented for this detector.");

    GET_IND_COMMAND(
        temp_adc, getTemperature, slsDetectorDefs::TEMPERATURE_ADC, " °C",
        "[n_value]\n\t[Jungfrau][Moench][Gotthard] ADC Temperature");

    GET_IND_COMMAND(temp_fpga, getTemperature,
                    slsDetectorDefs::TEMPERATURE_FPGA, " °C",
                    "[n_value]\n\t[Eiger][Jungfrau][Moench][Gotthard][Mythen3]["
                    "Gotthard2] FPGA Temperature");

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

    GET_IND_COMMAND(temp_sodr, getTemperature,
                    slsDetectorDefs::TEMPERATURE_SODR, " °C",
                    "[n_value]\n\t[Eiger]Temperature close to the right "
                    "so-dimm memory");

    GET_IND_COMMAND(temp_fpgafl, getTemperature,
                    slsDetectorDefs::TEMPERATURE_FPGA2, " °C",
                    "[n_value]\n\t[Eiger]Temperature of the left front end "
                    "board fpga.");

    GET_IND_COMMAND(temp_fpgafr, getTemperature,
                    slsDetectorDefs::TEMPERATURE_FPGA3, " °C",
                    "[n_value]\n\t[Eiger]Temperature of the left front end "
                    "board fpga.");

    GET_IND_COMMAND(temp_slowadc, getTemperature,
                    slsDetectorDefs::SLOW_ADC_TEMP, " °C",
                    "[n_value]\n\t[Ctb]Temperature of the slow adc");

    /* lists */
    CTB_NAMED_LIST(daclist, getDacNames, setDacNames,
                   "[dacname1 dacname2 .. dacname18] \n\t\t[ChipTestBoard] Set "
                   "the list of dac names for this detector.\n\t\t[All] Gets "
                   "the list of dac names for every dac for this detector.");

    CTB_SINGLE_DACNAME(dacname, getDacName, setDacName, defs::DAC_0,
                       "\n\t[0-17][name] \n\t\t[ChipTestBoard] Set "
                       "the dac at the given position to the given name.");

    CTB_GET_DACINDEX(dacindex, getDacIndex, defs::DAC_0,
                     "\n\t[name] \n\t\t[ChipTestBoard] Get "
                     "the dac index for the given name.");

    CTB_NAMED_LIST(adclist, getAdcNames, setAdcNames,
                   "[adcname1 adcname2 .. adcname32] \n\t\t[ChipTestBoard] Set "
                   "the list of adc names for this board.");

    CTB_SINGLE_NAME(adcname, getAdcName, setAdcName,
                    "[0-31][name] \n\t\t[ChipTestBoard] Set "
                    "the adc at the given position to the given name.");

    CTB_GET_INDEX(adcindex, getAdcIndex,
                  "[name] \n\t\t[ChipTestBoard] Get "
                  "the adc index for the given name.");

    CTB_NAMED_LIST(signallist, getSignalNames, setSignalNames,
                   "[signalname1 signalname2 .. signalname63] "
                   "\n\t\t[ChipTestBoard] Set "
                   "the list of signal names for this board.");

    CTB_SINGLE_NAME(signalname, getSignalName, setSignalName,
                    "[0-63][name] \n\t\t[ChipTestBoard] Set "
                    "the signal at the given position to the given name.");

    CTB_GET_INDEX(signalindex, getSignalIndex,
                  "[name] \n\t\t[ChipTestBoard] Get "
                  "the signal index for the given name.");

    CTB_NAMED_LIST(powerlist, getPowerNames, setPowerNames,
                   "[powername1 powername2 .. powername4] "
                   "\n\t\t[ChipTestBoard] Set "
                   "the list of power names for this board.");

    CTB_SINGLE_DACNAME(powername, getPowerName, setPowerName, defs::V_POWER_A,
                       "[0-4][name] \n\t\t[ChipTestBoard] Set "
                       "the power at the given position to the given name.");

    CTB_GET_DACINDEX(powerindex, getPowerIndex, defs::V_POWER_A,
                     "[name] \n\t\t[ChipTestBoard] Get "
                     "the power index for the given name.");

    CTB_VALUES(powervalues, getPower, getPowerList, getPowerNames,
               "[name] \n\t\t[ChipTestBoard] Get values of all powers.");

    CTB_VALUES(slowadcvalues, getSlowADC, getSlowADCList, getSlowADCNames,
               "[name] \n\t\t[ChipTestBoard] Get values of all slow adcs.");

    CTB_NAMED_LIST(
        slowadclist, getSlowADCNames, setSlowADCNames,
        "[slowadcname1 slowadcname2 .. slowadcname7] "
        "\n\t\t[ChipTestBoard] Set the list of slowadc names for this board.");

    CTB_SINGLE_DACNAME(slowadcname, getSlowADCName, setSlowADCName,
                       defs::SLOW_ADC0,
                       "[0-7][name] \n\t\t[ChipTestBoard] Set "
                       "the slowadc at the given position to the given name.");

    CTB_GET_DACINDEX(slowadcindex, getSlowADCIndex, defs::SLOW_ADC0,
                     "[name] \n\t\t[ChipTestBoard] Get "
                     "the slowadc index for the given name.");

    /* dacs */

    /* on chip dacs */
    INTEGER_USER_IND_COMMAND(
        vchip_comp_fe, getOnChipDAC, setOnChipDAC, StringTo<int>,
        defs::VB_COMP_FE,
        "[chip index 0-10, -1 for all][10 bit hex value] \n\t[Gotthard2] "
        "On chip Dac for comparator current of analogue front end.");

    INTEGER_USER_IND_COMMAND(
        vchip_opa_1st, getOnChipDAC, setOnChipDAC, StringTo<int>,
        defs::VB_OPA_1ST,
        "[chip index 0-10, -1 for all][10 bit hex value] \n\t[Gotthard2] "
        "On "
        "chip Dac for opa current for driving the other DACs in chip.");

    INTEGER_USER_IND_COMMAND(vchip_opa_fd, getOnChipDAC, setOnChipDAC,
                             StringTo<int>, defs::VB_OPA_FD,
                             "[chip index 0-10, -1 for all][10 bit hex "
                             "value] \n\t[Gotthard2] On "
                             "chip Dac current for CDS opa stage.");

    INTEGER_USER_IND_COMMAND(vchip_comp_adc, getOnChipDAC, setOnChipDAC,
                             StringTo<int>, defs::VB_COMP_ADC,
                             "[chip index 0-10, -1 for all][10 bit hex "
                             "value] \n\t[Gotthard2] On "
                             "chip Dac for comparator current of ADC.");

    INTEGER_USER_IND_COMMAND(vchip_ref_comp_fe, getOnChipDAC, setOnChipDAC,
                             StringTo<int>, defs::VREF_COMP_FE,
                             "[chip index 0-10, -1 for all][10 bit hex "
                             "value] \n\t[Gotthard2] On "
                             "chip Dac for reference voltage of the "
                             "comparator of analogue front "
                             "end.");

    INTEGER_USER_IND_COMMAND(
        vchip_cs, getOnChipDAC, setOnChipDAC, StringTo<int>, defs::VB_CS,
        "[chip index 0-10, -1 for all][10 bit hex value] \n\t[Gotthard2] "
        "On chip Dac for current injection into preamplifier.");

    /* acquisition */

    EXECUTE_SET_COMMAND_NOID(
        clearbusy, clearAcquiringFlag,
        "\n\tIf acquisition aborted during acquire command, use this to "
        "clear acquiring flag in shared memory before starting next "
        "acquisition");

    EXECUTE_SET_COMMAND_NOID(rx_start, startReceiver,
                             "\n\tStarts receiver listener for detector "
                             "data packets and create a "
                             "data file (if file write enabled).");

    EXECUTE_SET_COMMAND_NOID(
        rx_stop, stopReceiver,
        "\n\tStops receiver listener for detector data packets and closes "
        "current data file (if file write enabled).");

    EXECUTE_SET_COMMAND(
        start, startDetector,
        "\n\tStarts detector acquisition. Status changes to RUNNING or "
        "WAITING and automatically returns to idle at the end of acquisition. "
        "If the acquisition was abruptly stopped, some detectors come back to "
        "STOPPED.");

    EXECUTE_SET_COMMAND_NOID(
        readout, startDetectorReadout,
        "\n\t[Mythen3] Starts detector readout. Status changes to "
        "TRANSMITTING and automatically returns to idle at the end of "
        "readout.");

    EXECUTE_SET_COMMAND(stop, stopDetector,
                        "\n\tAbort detector acquisition. Status changes "
                        "to IDLE or STOPPED. Goes to stop server.");

    GET_COMMAND(rx_framescaught, getFramesCaught,
                "\n\tNumber of frames caught by each port in receiver.");

    GET_COMMAND(rx_missingpackets, getNumMissingPackets,
                "\n\tNumber of missing packets for each port in receiver. If "
                "negative, they are packets in excess. ");

    GET_COMMAND(rx_frameindex, getRxCurrentFrameIndex,
                "\n\tCurrent frame index received for each port in receiver "
                "during acquisition.");

    INTEGER_COMMAND_VEC_ID(
        nextframenumber, getNextFrameNumber, setNextFrameNumber,
        StringTo<uint64_t>,
        "[n_value]\n\t[Eiger][Jungfrau][Moench][CTB] Next frame number. "
        "Stopping acquisition might result in different frame numbers for "
        "different modules.");

    GET_COMMAND(scanerrmsg, getScanErrorMessage,
                "\n\tGets Scan error message if scan ended in error for non "
                "blocking acquisitions.");

    /* Network Configuration (Detector<->Receiver) */

    INTEGER_COMMAND_VEC_ID(
        numinterfaces, getNumberofUDPInterfaces, setNumberofUDPInterfaces,
        StringTo<int>,
        "[1, 2]\n\t[Jungfrau][Moench] Number of udp interfaces to stream "
        "data from detector. Default: 1.\n\tAlso enables second interface "
        "in receiver for listening (Writes a file per interface if writing "
        "enabled).\n\tAlso restarts client and receiver zmq sockets if zmq "
        "streaming enabled.\n\t[Eiger] Only gets with result 2.");

    INTEGER_COMMAND_VEC_ID(selinterface, getSelectedUDPInterface,
                           selectUDPInterface, StringTo<int>,
                           "[0, 1]\n\t[Jungfrau][Moench] The udp interface "
                           "to stream data from detector. Effective only when "
                           "number of interfaces is 1. Default: 0 (outer)");

    GET_COMMAND(
        udp_numdst, getNumberofUDPDestinations,
        "\n\t[Jungfrau][Moench][Eiger][Mythen3][Gotthard2] One can enter "
        "upto 32 (64 for Mythen3) destinations that the detector will stream "
        "images out in a round robin fashion. This is get only command. "
        "Default: 1");

    EXECUTE_SET_COMMAND(udp_cleardst, clearUDPDestinations,
                        "\n\tClears udp destination details on the detector.");

    INTEGER_COMMAND_VEC_ID(
        udp_firstdst, getFirstUDPDestination, setFirstUDPDestination,
        StringTo<int>,
        "\n[0 - 31 (or number of udp "
        "destinations)]\n\t[Jungfrau][Moench][Gotthard2]\n[0-63]\n\t["
        "Mythen3]\n\n\t One can set which is the first destination that the "
        "detector will stream images out from in a round robin fashion. The "
        "entry must not have been empty. Default: 0");

    INTEGER_COMMAND_VEC_ID(
        udp_srcmac, getSourceUDPMAC, setSourceUDPMAC, MacAddr,
        "[x:x:x:x:x:x]\n\tMac address of the detector (source) udp "
        "interface. \n\t[Eiger] Do not set as detector will replace with "
        "its own DHCP Mac (1G) or DHCP Mac + 1 (10G).");

    INTEGER_COMMAND_VEC_ID(
        udp_srcmac2, getSourceUDPMAC2, setSourceUDPMAC2, MacAddr,
        "[x:x:x:x:x:x]\n\t[Jungfrau][Moench] Mac address of the top "
        "half or inner (source) udp interface. ");

    INTEGER_COMMAND_VEC_ID(
        udp_dstmac, getDestinationUDPMAC, setDestinationUDPMAC, MacAddr,
        "[x:x:x:x:x:x]\n\tMac address of the receiver (destination) udp "
        "interface. Not mandatory to set as udp_dstip retrieves it from "
        "slsReceiver process, but must be set if you use a custom receiver "
        "(not slsReceiver). Use router mac if router between detector and "
        "receiver.");

    INTEGER_COMMAND_VEC_ID(
        udp_dstmac2, getDestinationUDPMAC2, setDestinationUDPMAC2, MacAddr,
        "[x:x:x:x:x:x]\n\t[Jungfrau][Moench] Mac address of the receiver "
        "(destination) udp interface 2. Not mandatory to set as udp_dstip2 "
        "retrieves it from slsReceiver process but must be set if you use a "
        "custom receiver (not slsReceiver). \n\t [Jungfrau][Moench] top half "
        "or inner interface \n\t [Gotthard2] veto debugging. Use router mac if "
        "router between detector and receiver.");

    INTEGER_COMMAND_VEC_ID_GET(
        udp_dstport, getDestinationUDPPort, setDestinationUDPPort,
        StringTo<uint16_t>,
        "[n]\n\tPort number of the receiver (destination) udp "
        "interface. Default is 50001. \n\tIf multi command, ports for each "
        "module is calculated (incremented by 1 if no 2nd interface)");

    INTEGER_COMMAND_VEC_ID_GET(
        udp_dstport2, getDestinationUDPPort2, setDestinationUDPPort2,
        StringTo<uint16_t>,
        "[n]\n\t[Jungfrau][Moench][Eiger][Gotthard2] Port number of the "
        "receiver (destination) udp interface 2. Default is 50002. "
        "\n\tIf multi command, ports for each module is calculated "
        "(incremented by 2) \n\t[Jungfrau][Moench] top half or inner "
        "interface \n\t[Eiger] right half \n\t[Gotthard2] veto debugging");

    EXECUTE_SET_COMMAND(
        udp_reconfigure, reconfigureUDPDestination,
        "\n\tReconfigures Detector with UDP destination. More for "
        "debugging as the configuration is done automatically when the "
        "detector has sufficient UDP details.");

    EXECUTE_SET_COMMAND(
        udp_validate, validateUDPConfiguration,
        "\n\tValidates that UDP configuration in the detector is "
        "valid. If not configured, it will throw with error message "
        "requesting missing udp information.");

    GET_COMMAND(rx_printconfig, printRxConfiguration,
                "\n\tPrints the receiver configuration.");

    INTEGER_COMMAND_VEC_ID(tengiga, getTenGiga, setTenGiga, StringTo<int>,
                           "[0, 1]\n\t[Eiger][Ctb][Mythen3] 10GbE Enable.");

    INTEGER_COMMAND_VEC_ID(
        flowcontrol10g, getTenGigaFlowControl, setTenGigaFlowControl,
        StringTo<int>,
        "[0, 1]\n\t[Eiger][Jungfrau][Moench] 10GbE Flow Control.");

    INTEGER_COMMAND_VEC_ID(
        txdelay_frame, getTransmissionDelayFrame, setTransmissionDelayFrame,
        StringTo<int>,
        "[n_delay]\n\t[Eiger][Jungfrau][Moench][Mythen3] Transmission "
        "delay of first udp packet being streamed out of the "
        "module.\n\t[Jungfrau][Moench] [0-31] Each value represents 1 "
        "ms\n\t[Eiger] Additional delay to txdelay_left and txdelay_right. "
        "Each value represents 10ns. Typical value is 50000.\n\t[Mythen3] "
        "[0-16777215] Each value represents 8 ns (125 MHz clock), max is 134 "
        "ms.");

    INTEGER_COMMAND_VEC_ID(
        txdelay_left, getTransmissionDelayLeft, setTransmissionDelayLeft,
        StringTo<int>,
        "[n_delay]\n\t[Eiger] Transmission delay of first packet in an "
        "image being streamed out of the module's left UDP port. Each value "
        "represents 10ns. Typical value is 50000.");

    INTEGER_COMMAND_VEC_ID(
        txdelay_right, getTransmissionDelayRight, setTransmissionDelayRight,
        StringTo<int>,
        "[n_delay]\n\t[Eiger] Transmission delay of first packet in an "
        "image being streamed out of the module's right UDP port. Each value "
        "represents 10ns. Typical value is 50000.");

    /* Receiver Config */

    INTEGER_COMMAND_VEC_ID_GET(
        rx_tcpport, getRxPort, setRxPort, StringTo<uint16_t>,
        "[port]\n\tTCP port for client-receiver communication. Default is "
        "1954. Must be different if multiple receivers on same pc. Must be "
        "first command to set a receiver parameter. Multi command will "
        "automatically increment for individual modules.");

    INTEGER_COMMAND_VEC_ID(
        rx_fifodepth, getRxFifoDepth, setRxFifoDepth, StringTo<int>,
        "[n_frames]\n\tSet the number of frames in the receiver "
        "fifo depth (buffer between listener and writer threads).");

    INTEGER_COMMAND_VEC_ID(rx_silent, getRxSilentMode, setRxSilentMode,
                           StringTo<int>,
                           "[0, 1]\n\tSwitch on or off receiver text "
                           "output during acquisition.");

    INTEGER_COMMAND_VEC_ID(
        rx_discardpolicy, getRxFrameDiscardPolicy, setRxFrameDiscardPolicy,
        StringTo<slsDetectorDefs::frameDiscardPolicy>,
        "[nodiscard (default)|discardempty|discardpartial(fastest)]\n\tFrame "
        "discard policy of receiver. nodiscard does not discard frames, "
        "discardempty discards empty frames, discardpartial discards partial "
        "frames.");

    INTEGER_COMMAND_VEC_ID(rx_padding, getPartialFramesPadding,
                           setPartialFramesPadding, StringTo<int>,
                           "[0, 1]\n\tPartial frames padding enable in the "
                           "receiver. Default: enabled. Disabling is fastest.");

    INTEGER_COMMAND_VEC_ID(rx_udpsocksize, getRxUDPSocketBufferSize,
                           setRxUDPSocketBufferSize, StringTo<int>,
                           "[n_size]\n\tUDP socket buffer size in "
                           "receiver. Tune rmem_default and rmem_max "
                           "accordingly. Max value is INT_MAX/2.");

    GET_COMMAND(rx_realudpsocksize, getRxRealUDPSocketBufferSize,
                "\n\tActual udp socket buffer size. Double the size of "
                "rx_udpsocksize due to kernel bookkeeping.");

    INTEGER_COMMAND_VEC_ID(rx_lock, getRxLock, setRxLock, StringTo<int>,
                           "[0, 1]\n\tLock receiver to one client IP, 1 locks, "
                           "0 unlocks. Default is unlocked.");

    GET_COMMAND(
        rx_lastclient, getRxLastClientIP,
        "\n\tClient IP Address that last communicated with the receiver.");

    GET_COMMAND(
        rx_threads, getRxThreadIds,
        "\n\tGet kernel thread ids from the receiver in order of [parent, "
        "tcp, listener 0, processor 0, streamer 0, listener 1, processor 1, "
        "streamer 1, arping]. If no streamer yet or there is no second "
        "interface, it gives 0 in its place.");

    INTEGER_COMMAND_VEC_ID(
        rx_arping, getRxArping, setRxArping, StringTo<int>,
        "[0, 1]\n\tStarts a thread in slsReceiver to arping "
        "the interface it is listening to every minute. Useful in 10G mode.");

    EXECUTE_SET_COMMAND_NOID(rx_clearroi, clearRxROI,
                             "Resets Region of interest in receiver. Default "
                             "is all channels/pixels enabled.");

    /* File */

    INTEGER_COMMAND_VEC_ID(
        fformat, getFileFormat, setFileFormat,
        StringTo<slsDetectorDefs::fileFormat>,
        "[binary|hdf5]\n\tFile format of data file. For "
        "HDF5, package must be compiled with HDF5 flags. Default is binary.");

    STRING_COMMAND(fpath, getFilePath, setFilePath,
                   "[path]\n\tDirectory where output data files are written in "
                   "receiver. Default is '/'. \n\tIf path does not exist, it "
                   "will try to create it.");

    STRING_COMMAND(fname, getFileNamePrefix, setFileNamePrefix,
                   "[name]\n\tFile name prefix for output data file. Default "
                   "is run. File name: [file name prefix]_d[detector "
                   "index]_f[sub file index]_[acquisition/file index].raw.");

    INTEGER_COMMAND_VEC_ID(findex, getAcquisitionIndex, setAcquisitionIndex,
                           StringTo<int64_t>,
                           "[n_value]\n\tFile or Acquisition index.");

    INTEGER_COMMAND_VEC_ID(
        fwrite, getFileWrite, setFileWrite, StringTo<int>,
        "[0, 1]\n\tEnable or disable receiver file write. Default is 1.");

    INTEGER_COMMAND_NOID(
        fmaster, getMasterFileWrite, setMasterFileWrite, StringTo<int>,
        "[0, 1]\n\tEnable or disable receiver master file. Default is 1.");

    INTEGER_COMMAND_VEC_ID(
        foverwrite, getFileOverWrite, setFileOverWrite, StringTo<int>,
        "[0, 1]\n\tEnable or disable file overwriting. Default is 1.");

    INTEGER_COMMAND_VEC_ID(
        rx_framesperfile, getFramesPerFile, setFramesPerFile, StringTo<int>,
        "[n_frames]\n\tNumber of frames per file in receiver in an "
        "acquisition. Default depends on detector type. 0 is infinite or "
        "all "
        "frames in single file.");

    /* ZMQ Streaming Parameters (Receiver<->Client) */

    INTEGER_COMMAND_VEC_ID(
        rx_zmqstream, getRxZmqDataStream, setRxZmqDataStream, StringTo<int>,
        "[0, 1]\n\tEnable/ disable data streaming from receiver via zmq "
        "(eg. to GUI or to another process for further processing). This "
        "creates/ destroys zmq streamer threads in receiver. \n\tSwitching to "
        "Gui automatically enables data streaming in receiver. \n\tSwitching "
        "back to command line acquire will require disabling data streaming in "
        "receiver for fast applications. ");

    INTEGER_COMMAND_VEC_ID(
        rx_zmqfreq, getRxZmqFrequency, setRxZmqFrequency, StringTo<int>,
        "[nth frame]\n\tFrequency of frames streamed out from receiver via "
        "zmq\n\tDefault: 1, Means every frame is streamed out. \n\tIf 2, "
        "every second frame is streamed out. \n\tIf 0, streaming timer is the "
        "timeout, after which current frame is sent out. (default timeout is "
        "500 ms). Usually used for gui purposes.");

    INTEGER_COMMAND_VEC_ID(
        rx_zmqstartfnum, getRxZmqStartingFrame, setRxZmqStartingFrame,
        StringTo<int>,
        "[fnum]\n\tThe starting frame index to stream out. 0 by "
        "default, which streams the first frame in an acquisition, "
        "and then depending on the rx zmq frequency/ timer");

    INTEGER_COMMAND_VEC_ID_GET(
        rx_zmqport, getRxZmqPort, setRxZmqPort, StringTo<uint16_t>,
        "[port]\n\tZmq port for data to be streamed out of the receiver. "
        "Also restarts receiver zmq streaming if enabled. Default is 30001. "
        "Modified only when using an intermediate process between receiver and "
        "client(gui). Must be different for every detector (and udp port). "
        "Multi command will automatically increment for individual modules.");

    INTEGER_COMMAND_VEC_ID_GET(
        zmqport, getClientZmqPort, setClientZmqPort, StringTo<uint16_t>,
        "[port]\n\tZmq port in client(gui) or intermediate process for "
        "data to be streamed to from receiver. Default connects to receiver "
        "zmq streaming out port (30001). Modified only when using an "
        "intermediate process between receiver and client(gui). Also restarts "
        "client zmq streaming if enabled. Must be different for every detector "
        "(and udp port). Multi command will automatically increment for "
        "individual modules.");

    INTEGER_COMMAND_VEC_ID(
        rx_zmqip, getRxZmqIP, setRxZmqIP, IpAddr,
        "[x.x.x.x]\n\tZmq Ip Address from which data is to be streamed out "
        "of the receiver. Also restarts receiver zmq streaming if enabled. "
        "Default is from rx_hostname. Modified only when using an intermediate "
        "process between receiver.");

    INTEGER_COMMAND_VEC_ID(
        zmqip, getClientZmqIp, setClientZmqIp, IpAddr,
        "[x.x.x.x]\n\tIp Address to listen to zmq data streamed out from "
        "receiver or intermediate process. Default connects to receiver zmq Ip "
        "Address (from rx_hostname). Modified only when using an intermediate "
        "process between receiver and client(gui). Also restarts client zmq "
        "streaming if enabled.");

    INTEGER_COMMAND_SET_NOID_GET_ID(
        rx_zmqhwm, getRxZmqHwm, setRxZmqHwm, StringTo<int>,
        "[n_value]\n\tReceiver's zmq send high water mark. Default is the "
        "zmq library's default (1000). This is a high number and can be set to "
        "2 for gui purposes. One must also set the client's receive high water "
        "mark to similar value. Final effect is sum of them. Also restarts "
        "receiver zmq streaming if enabled. Can set to -1 to set default "
        "value.");

    /* Eiger Specific */

    TIME_COMMAND(subexptime, getSubExptime, setSubExptime,
                 "[duration] [(optional unit) ns|us|ms|s]\n\t[Eiger] Exposure "
                 "time of EIGER subframes in 32 bit mode.");

    TIME_COMMAND(subdeadtime, getSubDeadTime, setSubDeadTime,
                 "[duration] [(optional unit) ns|us|ms|s]\n\t[Eiger] Dead time "
                 "of EIGER subframes in 32 bit mode. Subperiod = subexptime + "
                 "subdeadtime.");

    INTEGER_COMMAND_VEC_ID(
        overflow, getOverFlowMode, setOverFlowMode, StringTo<int>,
        "[0, 1]\n\t[Eiger] Enable or disable show overflow flag in "
        "32 bit mode. Default is disabled.");

    INTEGER_COMMAND_VEC_ID(
        interruptsubframe, getInterruptSubframe, setInterruptSubframe,
        StringTo<int>,
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

    INTEGER_COMMAND_VEC_ID(activate, getActive, setActive, StringTo<int>,
                           "[0, 1] \n\t[Eiger] 1 is default. 0 deactivates "
                           "readout and does not send data.");

    INTEGER_COMMAND_VEC_ID(
        partialreset, getPartialReset, setPartialReset, StringTo<int>,
        "[0, 1]\n\t[Eiger] Sets up detector to do "
        "partial or complete reset at start of acquisition. 0 complete reset, "
        "1 partial reset. Default is complete reset. Advanced function!");

    INTEGER_COMMAND_VEC_ID(
        top, getTop, setTop, StringTo<int>,
        "[0, 1]\n\t[Eiger] Sets half module to top (1), else bottom.");

    /* Jungfrau Specific */

    GET_COMMAND(chipversion, getChipVersion,
                "\n\t[Jungfrau] Returns chip version. Can be 1.0 or 1.1");

    INTEGER_COMMAND_VEC_ID(
        temp_threshold, getThresholdTemperature, setThresholdTemperature,
        StringTo<int>,
        "[n_temp (in degrees)]\n\t[Jungfrau][Moench] Threshold temperature "
        "in degrees. If temperature crosses threshold temperature and "
        "temperature control is enabled, power to chip will be switched off "
        "and temperature event occurs. To power on chip again, temperature has "
        "to be less than threshold temperature and temperature event has to be "
        "cleared.");

    INTEGER_COMMAND_VEC_ID(
        temp_control, getTemperatureControl, setTemperatureControl,
        StringTo<int>,
        "[0, 1]\n\t[Jungfrau][Moench] Temperature control enable. Default "
        "is 0 (disabled). If temperature crosses threshold temperature and "
        "temperature control is enabled, power to chip will be switched off "
        "and temperature event occurs. To power on chip again, temperature has "
        "to be less than threshold temperature and temperature event has to be "
        "cleared.");

    INTEGER_COMMAND_VEC_ID(
        autocompdisable, getAutoComparatorDisable, setAutoComparatorDisable,
        StringTo<int>,
        "[0, 1]\n\t[Jungfrau] Auto comparator disable mode. By "
        "default, the on-chip gain switching is active during the entire "
        "exposure.This mode disables the on - chip gain switching "
        "comparator automatically after 93.75% (only for chipv1.0) of exposure "
        "time (only for longer than 100us). It is possible to set the duration "
        "for chipv1.1 using compdisabletime command.\n\tDefault is 0 or this "
        "mode disabled(comparator enabled throughout). 1 enables mode. 0 "
        "disables mode. ");

    TIME_COMMAND(compdisabletime, getComparatorDisableTime,
                 setComparatorDisableTime,
                 "[duration] [(optional unit) ns|us|ms|s]\n\t[Jungfrau] Time "
                 "before end of exposure when comparator is disabled. It is "
                 "only possible for chipv1.1.");

    INTEGER_COMMAND_SET_NOID_GET_ID(
        extrastoragecells, getNumberOfAdditionalStorageCells,
        setNumberOfAdditionalStorageCells, StringTo<int>,
        "[0-15]\n\t[Jungfrau] Only for chipv1.0. Number of additional "
        "storage cells. Default is 0. For advanced users only. \n\tThe #images "
        "= #frames x #triggers x (#extrastoragecells + 1).");

    INTEGER_COMMAND_VEC_ID(
        storagecell_start, getStorageCellStart, setStorageCellStart,
        StringTo<int>,
        "[0-max]\n\t[Jungfrau] Storage cell that stores "
        "the first acquisition of the series. max is 15 (default) for chipv1.0 "
        "and 3 (default) for chipv1.1. For advanced users only.");

    TIME_COMMAND(storagecell_delay, getStorageCellDelay, setStorageCellDelay,
                 "[duration (0-1638375 ns)] [(optional unit) "
                 "ns|us|ms|s]\n\t[Jungfrau] Additional time delay between 2 "
                 "consecutive exposures in burst mode (resolution of 25ns). "
                 "Only applicable for chipv1.0. For advanced users only.");

    INTEGER_COMMAND_VEC_ID(
        gainmode, getGainMode, setGainMode, StringTo<slsDetectorDefs::gainMode>,
        "[dynamicgain|forceswitchg1|forceswitchg2|fixg1|fixg2|fixg0]\n\t["
        "Jungfrau] Gain mode.\n\tCAUTION: Do not use fixg0 without "
        "caution, you can damage the detector!!!");

    INTEGER_COMMAND_VEC_ID(filtercells, getNumberOfFilterCells,
                           setNumberOfFilterCells, StringTo<int>,
                           "[0-12]\n\t[Jungfrau] Set Filter Cell. Only for "
                           "chipv1.1. Advanced user Command");

    /* Gotthard Specific */
    TIME_GET_COMMAND(exptimel, getExptimeLeft,
                     "[(optional unit) ns|us|ms|s]\n\t[Gotthard] Exposure time "
                     "left for current frame. ");

    EXECUTE_SET_COMMAND(clearroi, clearROI,
                        "[Gotthard] Resets Region of interest in detector. All "
                        "channels enabled. Default is all channels enabled.");

    /* Gotthard2 Specific */
    INTEGER_COMMAND_SET_NOID_GET_ID(
        bursts, getNumberOfBursts, setNumberOfBursts, StringTo<int64_t>,
        "[n_bursts]\n\t[Gotthard2] Number of bursts per aquire. Only in auto "
        "timing mode and burst mode. Use timing command to set timing mode and "
        "burstmode command to set burst mode.");

    TIME_COMMAND(burstperiod, getBurstPeriod, setBurstPeriod,
                 "[duration] [(optional unit) ns|us|ms|s]\n\t[Gotthard2] "
                 "Period between 2 bursts. Only in burst mode and auto "
                 "timing mode.");

    GET_COMMAND(burstsl, getNumberOfBurstsLeft,
                "\n\t[Gotthard2] Number of bursts left in acquisition. Only in "
                "burst auto mode.");

    INTEGER_COMMAND_VEC_ID(
        cdsgain, getCDSGain, setCDSGain, StringTo<bool>,
        "[0, 1]\n\t[Gotthard2] Enable or disable CDS gain. Default "
        "is disabled.");

    INTEGER_COMMAND_VEC_ID(
        timingsource, getTimingSource, setTimingSource,
        StringTo<slsDetectorDefs::timingSourceType>,
        "[internal|external]\n\t[Gotthard2] Timing source. Internal is "
        "crystal and external is system timing. Default is internal.");

    INTEGER_COMMAND_VEC_ID(veto, getVeto, setVeto, StringTo<int>,
                           "[0, 1]\n\t[Gotthard2] Enable or disable veto data "
                           "data from chip. Default is 0.");

    /* Mythen3 Specific */

    INTEGER_COMMAND_VEC_ID(
        gates, getNumberOfGates, setNumberOfGates, StringTo<int>,
        "[n_gates]\n\t[Mythen3] Number of external gates in gating "
        "or trigger_gating mode (external gating).");

    INTEGER_COMMAND_VEC_ID(polarity, getPolarity, setPolarity,
                           StringTo<defs::polarity>,
                           "[pos|neg]\n\t[Mythen3] Sets negative or positive "
                           "polarity. Default is positive");

    INTEGER_COMMAND_VEC_ID(interpolation, getInterpolation, setInterpolation,
                           StringTo<int>,
                           "[0, 1]\n\t[Mythen3] Enables or disables "
                           "interpolation. Default is disabled.  Interpolation "
                           "mode enables all counters and disables vth3. "
                           "Disabling sets back counter mask and vth3.");

    INTEGER_COMMAND_VEC_ID(
        pumpprobe, getPumpProbe, setPumpProbe, StringTo<int>,
        "[0, 1]\n\t[Mythen3] Enables or disables pump probe "
        "mode. Default is disabled. Pump probe mode only enables vth2. "
        "Disabling sets back to previous value.");

    INTEGER_COMMAND_VEC_ID(apulse, getAnalogPulsing, setAnalogPulsing,
                           StringTo<int>,
                           "[0, 1]\n\t[Mythen3] Enables or disables analog "
                           "pulsing. Default is disabled");

    INTEGER_COMMAND_VEC_ID(dpulse, getDigitalPulsing, setDigitalPulsing,
                           StringTo<int>,
                           "[0, 1]\n\t[Mythen3] Enables or disables digital "
                           "pulsing. Default is disabled");

    /* CTB/ Moench Specific */

    INTEGER_COMMAND_VEC_ID(
        asamples, getNumberOfAnalogSamples, setNumberOfAnalogSamples,
        StringTo<int>,
        "[n_samples]\n\t[CTB] Number of analog samples expected.");

    INTEGER_COMMAND_VEC_ID(
        adcclk, getADCClock, setADCClock, StringTo<int>,
        "[n_clk in MHz]\n\t[Ctb] ADC clock frequency in MHz.");

    INTEGER_COMMAND_VEC_ID(runclk, getRUNClock, setRUNClock, StringTo<int>,
                           "[n_clk in MHz]\n\t[Ctb] Run clock in MHz.");

    GET_COMMAND(syncclk, getSYNCClock,
                "[n_clk in MHz]\n\t[Ctb] Sync clock in MHz.");

    INTEGER_IND_COMMAND(v_limit, getPower, setPower, StringTo<int>,
                        defs::V_LIMIT,
                        "[n_value]\n\t[Ctb] Soft limit for power "
                        "supplies (ctb only) and DACS in mV.");

    INTEGER_COMMAND_HEX(adcenable, getADCEnableMask, setADCEnableMask,
                        StringTo<uint32_t>,
                        "[bitmask]\n\t[Ctb] ADC Enable Mask for 1Gb "
                        "Enable for each 32 ADC channel.");

    INTEGER_COMMAND_HEX(
        adcenable10g, getTenGigaADCEnableMask, setTenGigaADCEnableMask,
        StringTo<uint32_t>,
        "[bitmask]\n\t[Ctb] ADC Enable Mask for 10Gb mode for each 32 "
        "ADC channel. However, if any of a consecutive 4 bits are enabled, "
        "the complete 4 bits are enabled.");

    INTEGER_COMMAND_HEX(transceiverenable, getTransceiverEnableMask,
                        setTransceiverEnableMask, StringTo<uint32_t>,
                        "[bitmask]\n\t[Ctb] Transceiver Enable Mask. Enable "
                        "for each 4 Transceiver channel.");

    INTEGER_COMMAND_VEC_ID(
        dsamples, getNumberOfDigitalSamples, setNumberOfDigitalSamples,
        StringTo<int>,
        "[n_value]\n\t[CTB] Number of digital samples expected.");

    INTEGER_COMMAND_VEC_ID(
        tsamples, getNumberOfTransceiverSamples, setNumberOfTransceiverSamples,
        StringTo<int>,
        "[n_value]\n\t[CTB] Number of transceiver samples expected.");

    INTEGER_COMMAND_VEC_ID(
        romode, getReadoutMode, setReadoutMode,
        StringTo<slsDetectorDefs::readoutMode>,
        "[analog|digital|analog_digital|transceiver|digital_transceiver]\n\t["
        "CTB] Readout mode. Default is analog.");

    INTEGER_COMMAND_VEC_ID(dbitclk, getDBITClock, setDBITClock, StringTo<int>,
                           "[n_clk in MHz]\n\t[Ctb] Clock for latching the "
                           "digital bits in MHz.");

    INTEGER_IND_COMMAND(v_a, getPower, setPower, StringTo<int>, defs::V_POWER_A,
                        "[n_value]\n\t[Ctb] Power supply a in mV.");

    INTEGER_IND_COMMAND(v_b, getPower, setPower, StringTo<int>, defs::V_POWER_B,
                        "[n_value]\n\t[Ctb] Power supply b in mV.");

    INTEGER_IND_COMMAND(v_c, getPower, setPower, StringTo<int>, defs::V_POWER_C,
                        "[n_value]\n\t[Ctb] Power supply c in mV.");

    INTEGER_IND_COMMAND(v_d, getPower, setPower, StringTo<int>, defs::V_POWER_D,
                        "[n_value]\n\t[Ctb] Power supply d in mV.");

    INTEGER_IND_COMMAND(
        v_io, getPower, setPower, StringTo<int>, defs::V_POWER_IO,
        "[n_value]\n\t[Ctb] Power supply io in mV. Minimum 1200 mV. Must "
        "be the first power regulator to be set after fpga reset (on-board "
        "detector server start up).");

    INTEGER_IND_COMMAND(
        v_chip, getPower, setPower, StringTo<int>, defs::V_POWER_CHIP,
        "[n_value]\n\t[Ctb] Power supply chip in mV. Do not use it "
        "unless "
        "you are completely sure you will not fry the board.");

    GET_IND_COMMAND(vm_a, getMeasuredPower, defs::V_POWER_A, "",
                    "\n\t[Ctb] Measured voltage of power supply a in mV.");

    GET_IND_COMMAND(vm_b, getMeasuredPower, defs::V_POWER_B, "",
                    "\n\t[Ctb] Measured voltage of power supply b in mV.");

    GET_IND_COMMAND(vm_c, getMeasuredPower, defs::V_POWER_C, "",
                    "\n\t[Ctb] Measured voltage of power supply c in mV.");

    GET_IND_COMMAND(vm_d, getMeasuredPower, defs::V_POWER_D, "",
                    "\n\t[Ctb] Measured voltage of power supply d in mV.");

    GET_IND_COMMAND(vm_io, getMeasuredPower, defs::V_POWER_IO, "",
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

    INTEGER_COMMAND_VEC_ID(
        extsampling, getExternalSampling, setExternalSampling, StringTo<int>,
        "[0, 1]\n\t[Ctb] Enable for external sampling signal for digital "
        "data to signal by extsampling src command. For advanced users only.");

    INTEGER_COMMAND_VEC_ID(
        extsamplingsrc, getExternalSamplingSource, setExternalSamplingSource,
        StringTo<int>,
        "[0-63]\n\t[Ctb] Sampling source signal for digital data. "
        "For advanced users only.");

    INTEGER_COMMAND_VEC_ID(
        rx_dbitoffset, getRxDbitOffset, setRxDbitOffset, StringTo<int>,
        "[n_bytes]\n\t[Ctb] Offset in bytes in digital data to "
        "skip in receiver.");

    INTEGER_COMMAND_VEC_ID(led, getLEDEnable, setLEDEnable, StringTo<int>,
                           "[0, 1]\n\t[Ctb] Switches on/off all LEDs.");

    /* Pattern */

    GET_COMMAND(patfname, getPatterFileName,
                "\n\t[Ctb][Mythen3] Gets the pattern file name including "
                "path of the last pattern uploaded. Returns an empty if "
                "nothing was uploaded or via a server default file");

    EXECUTE_SET_COMMAND_NOID_1ARG(
        savepattern, savePattern,
        "[fname]\n\t[Ctb][Mythen3] Saves pattern to file (ascii). "
        "\n\t[Ctb] Also executes pattern.");

    EXECUTE_SET_COMMAND(
        defaultpattern, loadDefaultPattern,
        "\n\t[Mythen3] Loads and runs default pattern in pattern "
        "generator. It is to go back to initial settings.");

    INTEGER_COMMAND_HEX_WIDTH16(patioctrl, getPatternIOControl,
                                setPatternIOControl, StringTo<uint64_t>,
                                "[64 bit mask]\n\t[Ctb] 64 bit mask "
                                "defining input (0) and output (1) signals.");

    INTEGER_COMMAND_HEX_WIDTH16(
        patmask, getPatternMask, setPatternMask, StringTo<uint64_t>,
        "[64 bit mask]\n\t[Ctb][Mythen3] Selects the bits that will "
        "have a pattern mask applied to the selected patmask for every "
        "pattern.");

    INTEGER_COMMAND_HEX_WIDTH16(
        patsetbit, getPatternBitMask, setPatternBitMask, StringTo<uint64_t>,
        "[64 bit mask]\n\t[Ctb][Mythen3] Sets the mask applied to "
        "every pattern to the selected bits.");

    EXECUTE_SET_COMMAND(patternstart, startPattern,
                        "\n\t[Mythen3] Starts Pattern");

    /* Moench */
    /* Advanced */

    INTEGER_COMMAND_VEC_ID(
        adcpipeline, getADCPipeline, setADCPipeline, StringTo<int>,
        "[n_value]\n\t[Ctb][Moench] Pipeline for ADC clock.");

    EXECUTE_SET_COMMAND(resetfpga, resetFPGA,
                        "\n\t[Jungfrau][Moench][Ctb] Reset FPGA.");

    EXECUTE_SET_COMMAND(rebootcontroller, rebootController,
                        "\n\t[Jungfrau][Moench][Ctb][Gotthard][Mythen3]["
                        "Gotthard2] Reboot controller of detector.");

    INTEGER_COMMAND_VEC_ID(
        updatemode, getUpdateMode, setUpdateMode, StringTo<int>,
        "[0|1]\n\tRestart the detector server in update "
        "mode or not. This is useful when server-firmware compatibility is at "
        "its worst and server cannot start up normally");

    EXECUTE_SET_COMMAND(
        firmwaretest, executeFirmwareTest,
        "\n\t[Jungfrau][Moench][Gotthard][Mythen3][Gotthard2][Ctb] "
        "Firmware test, ie. reads a read fixed pattern from a register.");

    EXECUTE_SET_COMMAND(
        bustest, executeBusTest,
        "\n\t[Jungfrau][Moench][Gotthard][Mythen3][Gotthard2][Ctb] Bus "
        "test, ie. Writes different values in a R/W register and confirms the "
        "writes to check bus.\n\tAdvanced User function!");

    INTEGER_COMMAND_HEX(
        adcinvert, getADCInvert, setADCInvert, StringTo<uint32_t>,
        "[bitmask]\n\t[Ctb][Jungfrau][Moench] ADC Inversion "
        "Mask.\n\t[Jungfrau][Moench] Inversions on top of the default mask.");

    /* Insignificant */

    INTEGER_COMMAND_VEC_ID(
        port, getControlPort, setControlPort, StringTo<uint16_t>,
        "[n]\n\tPort number of the control server on detector for "
        "detector-client tcp interface. Default is 1952. Normally unchanged. "
        "Set different ports for virtual servers on same pc.");

    INTEGER_COMMAND_VEC_ID(
        stopport, getStopPort, setStopPort, StringTo<uint16_t>,
        "[n]\n\tPort number of the stop server on detector for detector-client "
        "tcp interface. Default is 1953. Normally unchanged.");

    INTEGER_COMMAND_VEC_ID(
        lock, getDetectorLock, setDetectorLock, StringTo<int>,
        "[0, 1]\n\tLock detector to one IP, 1: locks. Default is unlocked");

    GET_COMMAND(
        lastclient, getLastClientIP,
        "\n\tClient IP Address that last communicated with the detector.");

    GET_COMMAND(
        framecounter, getNumberOfFramesFromStart,
        "\n\t[Jungfrau][Moench][Mythen3][Gotthard2][CTB] Number of frames from "
        "start run control.\n\t[Gotthard2] only in continuous mode.");

    TIME_GET_COMMAND(runtime, getActualTime,
                     "[(optional unit) "
                     "ns|us|ms|s]\n\t[Jungfrau][Moench][Mythen3][Gotthard2]["
                     "CTB] Time from detector start up.\n\t[Gotthard2] not in "
                     "burst and auto mode.");

    TIME_GET_COMMAND(frametime, getMeasurementTime,
                     "[(optional unit) "
                     "ns|us|ms|s]\n\t[Jungfrau][Moench][Mythen3][Gotthard2]["
                     "CTB] Timestamp at a frame start.\n\t[Gotthard2] not in "
                     "burst and auto mode.");
};

} // namespace sls
