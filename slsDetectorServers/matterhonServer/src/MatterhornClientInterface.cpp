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

    LOG(logDEBUG1)
        << "Function table from child class MatterhornClientInterface: ";
    std::for_each(functionTable.begin(), functionTable.end(),
                  [](const auto &pair) {
                      LOG(logDEBUG1)
                          << "Function id: " << pair.first
                          << ", Function name: "
                          << getFunctionNameFromEnum((enum detFuncs)pair.first);
                  });

    startTCPServer();
}

ReturnCode MatterhornClientInterface::get_version(ServerInterface &socket) {

    auto version = getMatterhornServerVersion();
    char version_cstr[MAX_STR_LENGTH]{};
    strncpy(version_cstr, version.c_str(), version.size());
    // version.resize(MAX_STR_LENGTH);
    LOG(TLogLevel::logINFO) << "Matterhorn Server Version: " << version;
    LOG(TLogLevel::logDEBUG1)
        << "size of version: " << sizeof(version) << " bytes";
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

} // namespace sls