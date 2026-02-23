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
         [this](ServerInterface &si) { return this->get_detector_type(si); }},
        {detFuncs::F_INITIAL_CHECKS,
         [this](ServerInterface &si) { return this->initial_checks(si); }}};

    startTCPServer();
}

ReturnCode MatterhornClientInterface::get_version(ServerInterface &socket) {

    auto version = getMatterhornServerVersion();
    char version_cstr[MAX_STR_LENGTH]{};
    strncpy(version_cstr, version.c_str(), version.size());
    LOG(TLogLevel::logDEBUG) << "Matterhorn Server Version: " << version;
    return static_cast<ReturnCode>(socket.sendResult(
        version_cstr)); // TODO: check what would be possible return codes!!!
}

ReturnCode
MatterhornClientInterface::get_detector_type(ServerInterface &socket) {
    int detectortype = slsDetectorDefs::detectorType::MATTERHORN;
    return static_cast<ReturnCode>(socket.sendResult(detectortype));
}

std::string MatterhornClientInterface::getMatterhornServerVersion() {
    return APIMATTERHORN;
}

ReturnCode MatterhornClientInterface::initial_checks(ServerInterface &socket) {

    // TODO: add more checks here, for now just return true to be able to test
    // the should check firmware -client compatibility
    bool initial_checks_passed = true;
    return static_cast<ReturnCode>(socket.sendResult(initial_checks_passed));
}

} // namespace sls