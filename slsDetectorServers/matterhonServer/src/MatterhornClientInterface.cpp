#include "MatterhornClientInterface.h"
#include "sls/logger.h"
#include "sls/sls_detector_defs.h"
#include "sls/sls_detector_funcs.h"
#include "sls/versionAPI.h"

namespace sls {

MatterhornClientInterface::MatterhornClientInterface(const uint16_t portNumber)
    : ClientInterface(portNumber) {

    // TODO: maybe define the function list in another .hpp file as inline
    // unorderer map also this F_GET_SERVER_VERSION should be a global enum
    // shared by client and server

    functionTable = {
        {detFuncs::F_GET_SERVER_VERSION,
         [this](ServerInterface &si) { return this->get_version(si); }},
        {detFuncs::F_GET_DETECTOR_TYPE,
         [this](ServerInterface &si) { return this->get_detector_type(si); }}};
}

ReturnCode MatterhornClientInterface::get_version(ServerInterface &socket) {

    auto version = getMatterhornServerVersion();
    version.resize(MAX_STR_LENGTH);
    LOG(TLogLevel::logINFO) << "Matterhorn Server Version: " << version;
    return static_cast<ReturnCode>(socket.sendResult(
        version)); // TODO: check what would be possible return codes!!!
}

ReturnCode
MatterhornClientInterface::get_detector_type(ServerInterface &socket) {
    return static_cast<ReturnCode>(
        socket.sendResult(slsDetectorDefs::detectorType::MATTERHORN));
}

std::string MatterhornClientInterface::getMatterhornServerVersion() {
    return APIMATTERHORN;
}

} // namespace sls