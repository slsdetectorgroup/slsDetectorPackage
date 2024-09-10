// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/Pattern.h"
#include "sls/ToString.h"
#include "sls/logger.h"
#include <fstream>
#include <iterator>
#include <sstream>

namespace sls {

Pattern::Pattern() {
    // initialize pattern addresses
    for (int i = 0; i != MAX_PATTERN_LEVELS; ++i) {
        pat->startloop[i] = MAX_PATTERN_LENGTH - 1;
        pat->stoploop[i] = MAX_PATTERN_LENGTH - 1;
        pat->wait[i] = MAX_PATTERN_LENGTH - 1;
    }
}

Pattern::~Pattern() { delete pat; }

Pattern::Pattern(const Pattern &other) {
    memcpy(pat, other.pat, sizeof(patternParameters));
}

bool Pattern::operator==(const Pattern &other) const {
    for (size_t i = 0; i < (sizeof(pat->word) / sizeof(pat->word[0])); ++i) {
        if (pat->word[i] != other.pat->word[i])
            return false;
    }
    if (pat->ioctrl != other.pat->ioctrl)
        return false;

    for (size_t i = 0; i < (sizeof(pat->limits) / sizeof(pat->limits[0]));
         ++i) {
        if (pat->limits[i] != other.pat->limits[i])
            return false;
    }
    for (size_t i = 0; i < (sizeof(pat->startloop) / sizeof(pat->startloop[0]));
         ++i) {
        if (pat->startloop[i] != other.pat->startloop[i])
            return false;
    }
    for (size_t i = 0; i < (sizeof(pat->stoploop) / sizeof(pat->stoploop[0]));
         ++i) {
        if (pat->stoploop[i] != other.pat->stoploop[i])
            return false;
    }
    for (size_t i = 0; i < (sizeof(pat->nloop) / sizeof(pat->nloop[0])); ++i) {
        if (pat->nloop[i] != other.pat->nloop[i])
            return false;
    }
    for (size_t i = 0; i < (sizeof(pat->wait) / sizeof(pat->wait[0])); ++i) {
        if (pat->wait[i] != other.pat->wait[i])
            return false;
    }
    for (size_t i = 0; i < (sizeof(pat->waittime) / sizeof(pat->waittime[0]));
         ++i) {
        if (pat->waittime[i] != other.pat->waittime[i])
            return false;
    }
    return true;
}

bool Pattern::operator!=(const Pattern &other) const {
    return !(*this == other);
}

patternParameters *Pattern::data() { return pat; }
patternParameters *Pattern::data() const { return pat; }

void Pattern::validate() const {
    if (pat->limits[0] >= MAX_PATTERN_LENGTH ||
        pat->limits[1] >= MAX_PATTERN_LENGTH) {
        throw RuntimeError("Invalid Pattern limits address [" +
                           ToString(pat->limits[0]) + std::string(", ") +
                           ToString(pat->limits[1]) + std::string("]"));
    }
    for (int i = 0; i != MAX_PATTERN_LEVELS; ++i) {
        if (pat->startloop[i] >= MAX_PATTERN_LENGTH ||
            pat->stoploop[i] >= MAX_PATTERN_LENGTH) {
            throw RuntimeError("Invalid Pattern loop address for level " +
                               ToString(i) + std::string(" [") +
                               ToString(pat->startloop[i]) + std::string(", ") +
                               ToString(pat->stoploop[i]) + std::string("]"));
        }
        if (pat->wait[i] >= MAX_PATTERN_LENGTH) {
            throw RuntimeError("Invalid Pattern wait address for level " +
                               ToString(i) + std::string(" ") +
                               ToString(pat->wait[i]));
        }
    }
}

void Pattern::load(const std::string &fname) {
    std::ifstream input_file(fname);
    if (!input_file) {
        throw RuntimeError("Could not open pattern file " + fname +
                           " for reading");
    }
    for (std::string line; std::getline(input_file, line);) {
        if (line.find('#') != std::string::npos) {
            line.erase(line.find('#'));
        }
        LOG(logDEBUG1) << "line after removing comments:\n\t" << line;
        if (line.length() > 1) {

            // convert command and string to a vector
            std::istringstream iss(line);
            auto it = std::istream_iterator<std::string>(iss);
            std::vector<std::string> args = std::vector<std::string>(
                it, std::istream_iterator<std::string>());

            std::string cmd = args[0];
            int nargs = args.size() - 1;

            if (cmd == "patword") {
                if (nargs != 2) {
                    throw RuntimeError("Invalid arguments for " +
                                       ToString(args));
                }
                uint32_t addr = StringTo<uint32_t>(args[1]);
                if (addr >= MAX_PATTERN_LENGTH) {
                    throw RuntimeError("Invalid address for " + ToString(args));
                }
                pat->word[addr] = StringTo<uint64_t>(args[2]);
            } else if (cmd == "patioctrl") {
                if (nargs != 1) {
                    throw RuntimeError("Invalid arguments for " +
                                       ToString(args));
                }
                pat->ioctrl = StringTo<uint64_t>(args[1]);
            } else if (cmd == "patlimits") {
                if (nargs != 2) {
                    throw RuntimeError("Invalid arguments for " +
                                       ToString(args));
                }
                pat->limits[0] = StringTo<uint32_t>(args[1]);
                pat->limits[1] = StringTo<uint32_t>(args[2]);
            } else if (cmd == "patloop0" || cmd == "patloop1" ||
                       cmd == "patloop2" || cmd == "patloop") {
                int level = -1, iArg = 1;
                if (cmd == "patloop") {
                    if (nargs != 3) {
                        throw RuntimeError("Invalid arguments for " +
                                           ToString(args));
                    }
                    level = StringTo<int>(args[iArg++]);
                } else {
                    LOG(logWARNING)
                        << "Deprecated command. Please use patloop next time.";
                    if (nargs != 2) {
                        throw RuntimeError("Invalid arguments for " +
                                           ToString(args));
                    }
                    level = cmd[cmd.find_first_of("012")] - '0';
                }
                if (level < 0 || level >= MAX_PATTERN_LEVELS) {
                    throw RuntimeError("Invalid Pattern level. Options 0-" +
                                       std::to_string(MAX_PATTERN_LEVELS - 1));
                }
                int loop1 = StringTo<uint32_t>(args[iArg++]);
                int loop2 = StringTo<uint32_t>(args[iArg++]);
                pat->startloop[level] = loop1;
                pat->stoploop[level] = loop2;
            } else if (cmd == "patnloop0" || cmd == "patnloop1" ||
                       cmd == "patnloop2" || cmd == "patnloop") {
                int level = -1, iArg = 1;
                if (cmd == "patnloop") {
                    if (nargs != 2) {
                        throw RuntimeError("Invalid arguments for " +
                                           ToString(args));
                    }
                    level = StringTo<int>(args[iArg++]);
                } else {
                    LOG(logWARNING) << "Deprecated command. Please use "
                                       "patnloop next time.";
                    if (nargs != 1) {
                        throw RuntimeError("Invalid arguments for " +
                                           ToString(args));
                    }
                    level = cmd[cmd.find_first_of("012")] - '0';
                }
                if (level < 0 || level >= MAX_PATTERN_LEVELS) {
                    throw RuntimeError("Invalid Pattern level. Options 0-" +
                                       std::to_string(MAX_PATTERN_LEVELS - 1));
                }
                pat->nloop[level] = StringTo<uint32_t>(args[iArg++]);
            } else if (cmd == "patwait0" || cmd == "patwait1" ||
                       cmd == "patwait2" || cmd == "patwait") {
                int level = -1, iArg = 1;
                if (cmd == "patwait") {
                    if (nargs != 2) {
                        throw RuntimeError("Invalid arguments for " +
                                           ToString(args));
                    }
                    level = StringTo<int>(args[iArg++]);
                } else {
                    LOG(logWARNING)
                        << "Deprecated command. Please use patwait next time.";
                    if (nargs != 1) {
                        throw RuntimeError("Invalid arguments for " +
                                           ToString(args));
                    }
                    level = cmd[cmd.find_first_of("012")] - '0';
                }
                if (level < 0 || level >= MAX_PATTERN_LEVELS) {
                    throw RuntimeError("Invalid Pattern level. Options 0-" +
                                       std::to_string(MAX_PATTERN_LEVELS - 1));
                }
                pat->wait[level] = StringTo<uint32_t>(args[iArg++]);
            } else if (cmd == "patwaittime0" || cmd == "patwaittime1" ||
                       cmd == "patwaittime2" || cmd == "patwaittime") {
                int level = -1, iArg = 1;
                if (cmd == "patwaittime") {
                    if (nargs != 2) {
                        throw RuntimeError("Invalid arguments for " +
                                           ToString(args));
                    }
                    level = StringTo<int>(args[iArg++]);
                } else {
                    LOG(logWARNING) << "Deprecated command. Please use "
                                       "patwaittime next time.";
                    if (nargs != 1) {
                        throw RuntimeError("Invalid arguments for " +
                                           ToString(args));
                    }
                    level = cmd[cmd.find_first_of("012")] - '0';
                }
                if (level < 0 || level >= MAX_PATTERN_LEVELS) {
                    throw RuntimeError("Invalid Pattern level. Options 0-" +
                                       std::to_string(MAX_PATTERN_LEVELS - 1));
                }
                pat->waittime[level] = StringTo<uint64_t>(args[iArg++]);
            } else {
                throw RuntimeError("Unknown command in pattern file " + cmd);
            }
        }
    }
}

void Pattern::save(const std::string &fname) {
    std::ofstream output_file(fname);
    if (!output_file) {
        throw RuntimeError("Could not open pattern file " + fname +
                           " for writing");
    }
    std::ostringstream os;
    // pattern word
    for (uint32_t i = pat->limits[0]; i <= pat->limits[1]; ++i) {
        output_file << "patword " << ToStringHex(i, 4) << " "
                    << ToStringHex(pat->word[i], 16) << std::endl;
    }

    // patioctrl
    output_file << "patioctrl " << ToStringHex(pat->ioctrl, 16) << std::endl;

    // patlimits
    output_file << "patlimits " << ToStringHex(pat->limits[0], 4) << " "
                << ToStringHex(pat->limits[1], 4) << std::endl;

    for (size_t i = 0; i < MAX_PATTERN_LEVELS; ++i) {
        // patloop
        output_file << "patloop " << i << " "
                    << ToStringHex(pat->startloop[i], 4) << " "
                    << ToStringHex(pat->stoploop[i], 4) << std::endl;
        // patnloop
        output_file << "patnloop " << i << " " << pat->nloop[i] << std::endl;
    }

    for (size_t i = 0; i < MAX_PATTERN_LEVELS; ++i) {
        // patwait
        output_file << "patwait " << i << " " << ToStringHex(pat->wait[i], 4)
                    << std::endl;
        // patwaittime
        output_file << "patwaittime " << i << " " << pat->waittime[i]
                    << std::endl;
    }
}

std::string Pattern::str() const {
    std::ostringstream oss;
    oss << '[' << std::setfill('0') << std::endl;
    int addr_width = 4;
    int word_width = 16;
    for (int i = 0; i < MAX_PATTERN_LENGTH; ++i) {
        if (pat->word[i] != 0) {
            oss << "patword " << ToStringHex(i, addr_width) << " "
                << ToStringHex(pat->word[i], word_width) << std::endl;
        }
    }
    oss << "patioctrl " << ToStringHex(pat->ioctrl, word_width) << std::endl
        << "patlimits " << ToStringHex(pat->limits[0], addr_width) << " "
        << ToStringHex(pat->limits[1], addr_width) << std::endl;

    for (int i = 0; i != MAX_PATTERN_LEVELS; ++i) {
        oss << "patloop " << i << ' '
            << ToStringHex(pat->startloop[i], addr_width) << " "
            << ToStringHex(pat->stoploop[i], addr_width) << std::endl
            << "patnloop " << pat->nloop[i] << std::endl
            << "patwait " << i << ' ' << ToStringHex(pat->wait[i], addr_width)
            << std::endl
            << "patwaittime " << i << ' ' << pat->waittime[i] << std::endl;
    }

    oss << ']';
    return oss.str();
}

} // namespace sls