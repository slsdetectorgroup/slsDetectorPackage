#include "sls/Pattern.h"
#include "sls/ToString.h"
#include "sls/logger.h"
#include <fstream>
#include <iterator>
#include <sstream>

namespace sls {

Pattern::Pattern() = default;
Pattern::~Pattern() { delete pat; }

Pattern::Pattern(const Pattern &other) {
    memcpy(pat, other.pat, sizeof(patternParameters));
}

bool Pattern::operator==(const Pattern &other) const {
    for (size_t i = 0; i<(sizeof(pat->word)/sizeof(pat->word[0])); ++i){
        if (pat->word[i] != other.pat->word[i])
            return false;
    }
    if (pat->ioctrl != other.pat->ioctrl)
        return false;
    
    for (size_t i = 0; i<(sizeof(pat->limits)/sizeof(pat->limits[0])); ++i){
        if (pat->limits[i] != other.pat->limits[i])
            return false;
    }
    for (size_t i = 0; i<(sizeof(pat->loop)/sizeof(pat->loop[0])); ++i){
        if (pat->loop[i] != other.pat->loop[i])
            return false;
    }
    for (size_t i = 0; i<(sizeof(pat->nloop)/sizeof(pat->nloop[0])); ++i){
        if (pat->nloop[i] != other.pat->nloop[i])
            return false;
    }
    for (size_t i = 0; i<(sizeof(pat->wait)/sizeof(pat->wait[0])); ++i){
        if (pat->wait[i] != other.pat->wait[i])
            return false;
    }
    for (size_t i = 0; i<(sizeof(pat->waittime)/sizeof(pat->waittime[0])); ++i){
        if (pat->waittime[i] != other.pat->waittime[i])
            return false;
    }
    return true;
}

bool Pattern::operator!=(const Pattern& other) const{
    return !(*this==other);
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
    for (int i = 0; i != 3; ++i) {
        if (pat->loop[i * 2 + 0] >= MAX_PATTERN_LENGTH ||
            pat->loop[i * 2 + 1] >= MAX_PATTERN_LENGTH) {
            throw RuntimeError(
                "Invalid Pattern loop address for level " + ToString(i) +
                std::string(" [") + ToString(pat->loop[i * 2 + 0]) +
                std::string(", ") + ToString(pat->loop[i * 2 + 1]) +
                std::string("]"));
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
                       cmd == "patloop2") {
                if (nargs != 2) {
                    throw RuntimeError("Invalid arguments for " +
                                       ToString(args));
                }
                int level = cmd[cmd.find_first_of("012")] - '0';
                int loop1 = StringTo<uint32_t>(args[1]);
                int loop2 = StringTo<uint32_t>(args[2]);
                pat->loop[level * 2 + 0] = loop1;
                pat->loop[level * 2 + 1] = loop2;
            } else if (cmd == "patnloop0" || cmd == "patnloop1" ||
                       cmd == "patnloop2") {
                if (nargs != 1) {
                    throw RuntimeError("Invalid arguments for " +
                                       ToString(args));
                }
                int level = cmd[cmd.find_first_of("012")] - '0';
                pat->nloop[level] = StringTo<uint32_t>(args[1]);
            } else if (cmd == "patwait0" || cmd == "patwait1" ||
                       cmd == "patwait2") {
                if (nargs != 1) {
                    throw RuntimeError("Invalid arguments for " +
                                       ToString(args));
                }
                int level = cmd[cmd.find_first_of("012")] - '0';
                pat->wait[level] = StringTo<uint32_t>(args[1]);
            } else if (cmd == "patwaittime0" || cmd == "patwaittime1" ||
                       cmd == "patwaittime2") {
                if (nargs != 1) {
                    throw RuntimeError("Invalid arguments for " +
                                       ToString(args));
                }
                int level = cmd[cmd.find_first_of("012")] - '0';
                pat->waittime[level] = StringTo<uint64_t>(args[1]);
            } else {
                throw RuntimeError("Unknown command in pattern file " + cmd);
            }
        }
    }
}

} // namespace sls