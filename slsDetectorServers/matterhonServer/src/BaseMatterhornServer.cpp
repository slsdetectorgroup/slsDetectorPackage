#include "MatterhornServer.h"
#include "communication_funcs.h"
#include "sls/logger.h"
#include "sls/network_utils.h"
#include "sls/sls_detector_defs.h"
#include "sls/versionAPI.h"

namespace sls {

BaseMatterhornServer::BaseMatterhornServer(uint16_t port) {

    validatePortNumber(port);

// mmh do I want a virtual server inheriting from parent Server class? and
// parent Matterhorn class - probably better
#ifdef VIRTUAL
    udpDetails[0].srcip = LOCALHOSTIP_INT;
#endif
    udpDetails[0].srcport = DEFAULT_UDP_SRC_PORTNO;
    udpDetails[0].dstport = DEFAULT_UDP_DST_PORTNO;

    // TODO: when do i set the udp mac and ip ?

    tcpInterface = std::make_unique<TCPInterface>(
        function_table, port); // TODO: need a tcp and udp interface

    // should maybe be part of the constructor?
    tcpInterface->startTCPServer();

    // need a function to setup detector - e.g. set all registers etc.
}

ReturnCode BaseMatterhornServer::get_version(ServerInterface &socket) {

    auto version = getMatterhornServerVersion();
    char version_cstr[MAX_STR_LENGTH]{};
    strncpy(version_cstr, version.c_str(), version.size());
    LOG(TLogLevel::logDEBUG) << "Matterhorn Server Version: " << version;
    return static_cast<ReturnCode>(socket.sendResult(
        version_cstr)); // TODO: check what would be possible return codes!!!
}

ReturnCode BaseMatterhornServer::get_detector_type(ServerInterface &socket) {
    int detectortype = slsDetectorDefs::detectorType::MATTERHORN;
    return static_cast<ReturnCode>(socket.sendResult(detectortype));
}

std::string BaseMatterhornServer::getMatterhornServerVersion() {
    return APIMATTERHORN;
}

size_t BaseMatterhornServer::num_udp_interfaces() const {
    return udpDetails.size();
}

ReturnCode MatterhornServer::initial_checks(ServerInterface &socket) {

    // TODO: add more checks here, for now just return true to be able to test
    // the should check firmware -client compatibility
    bool initial_checks_passed = true;
    return static_cast<ReturnCode>(socket.sendResult(initial_checks_passed));
}

ReturnCode
BaseMatterhornServer::get_num_udp_interfaces(ServerInterface &socket) {
    int numUDPInterfaces = static_cast<int>(num_udp_interfaces());
    return static_cast<ReturnCode>(socket.sendResult(numUDPInterfaces));
}

ReturnCode MatterhornServer::get_update_mode(ServerInterface &socket) {
    return static_cast<ReturnCode>(
        socket.sendResult(static_cast<int>(updateMode)));
}

ReturnCode BaseMatterhornServer::get_source_udp_mac(ServerInterface &socket) {
    uint64_t srcMac = udpDetails[0].srcmac;
    return static_cast<ReturnCode>(socket.sendResult(srcMac));
}

} // namespace sls