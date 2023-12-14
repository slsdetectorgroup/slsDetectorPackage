#include "inferAction.h"
#include "sls/sls_detector_defs.h"
namespace sls {

int InferAction::infer(sls::CmdParser &parser, std::ostream &os) {
    args = parser.arguments();
    cmd = parser.command();
    auto it = functions.find(parser.command());
    if (it != functions.end()) {
        return ((*this).*(it->second))();
    } else {
        throw RuntimeError(
            "sls_detector not implemented for command: " + parser.command() +
            ". Use sls_detector_get or sls_detector_put.");
    }
}

// THIS COMMENT TO BE REPLACED BY THE ACTUAL CODE (1) - DO NOT REMOVE

} // namespace sls