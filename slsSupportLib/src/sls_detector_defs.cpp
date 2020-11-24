#include "sls/sls_detector_defs.h"
#include "sls/logger.h"
#include "sls/ToString.h"
#include <fstream>
#include <sstream>
#include <iterator>

using sls::RuntimeError;
using sls::StringTo;
using sls::ToString;

void slsDetectorDefs::patternParameters::load(const std::string& fname){
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
                word[addr] = StringTo<uint64_t>(args[2]);
            } else if (cmd == "patioctrl") {
                if (nargs != 1) {
                    throw RuntimeError("Invalid arguments for " +
                                       ToString(args));
                }
                patioctrl = StringTo<uint64_t>(args[1]);
            } else if (cmd == "patlimits") {
                if (nargs != 2) {
                    throw RuntimeError("Invalid arguments for " +
                                       ToString(args));
                }
                patlimits[0] = StringTo<uint32_t>(args[1]);
                patlimits[1] = StringTo<uint32_t>(args[2]);
            } else if (cmd == "patloop0" || cmd == "patloop1" ||
                       cmd == "patloop2") {
                if (nargs != 2) {
                    throw RuntimeError("Invalid arguments for " +
                                       ToString(args));
                }
                int level = cmd[cmd.find_first_of("012")] - '0';
                int patloop1 = StringTo<uint32_t>(args[1]);
                int patloop2 = StringTo<uint32_t>(args[2]);
                patloop[level * 2 + 0] = patloop1;
                patloop[level * 2 + 1] = patloop2;
            } else if (cmd == "patnloop0" || cmd == "patnloop1" ||
                       cmd == "patnloop2") {
                if (nargs != 1) {
                    throw RuntimeError("Invalid arguments for " +
                                       ToString(args));
                }
                int level = cmd[cmd.find_first_of("012")] - '0';
                patnloop[level] = StringTo<uint32_t>(args[1]);
            } else if (cmd == "patwait0" || cmd == "patwait1" ||
                       cmd == "patwait2") {
                if (nargs != 1) {
                    throw RuntimeError("Invalid arguments for " +
                                       ToString(args));
                }
                int level = cmd[cmd.find_first_of("012")] - '0';
                patwait[level] = StringTo<uint32_t>(args[1]);
            } else if (cmd == "patwaittime0" || cmd == "patwaittime1" ||
                       cmd == "patwaittime2") {
                if (nargs != 1) {
                    throw RuntimeError("Invalid arguments for " +
                                       ToString(args));
                }
                int level = cmd[cmd.find_first_of("012")] - '0';
                patwaittime[level] = StringTo<uint64_t>(args[1]);
            } else {
                throw RuntimeError("Unknown command in pattern file " + cmd);
            }
        }
    }
}