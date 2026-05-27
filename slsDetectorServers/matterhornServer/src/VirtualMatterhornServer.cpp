#include "VirtualMatterhornServer.h"
#include "sls/ToString.h"

namespace sls {

VirtualMatterhornServer::VirtualMatterhornServer(uint16_t port)
    : BaseMatterhornServer<VirtualMatterhornServer>(port) {

    LOG(logDEBUG) << "Initializing virtual Matterhorn server on port " << port;

    udpDetails[0].srcip = LOCALHOSTIP_INT;

    // map the IP core base addresses to virtual memory
    busCommunication.mapToMemory();

    spiCommunication.mapToMemory();

    // should maybe be part of the constructor?
    tcpInterface->startTCPServer();

    // TODO: no init_server function for now is it neccessary to set the init
    // flag
    this->setupDetector();
}

ReturnCode VirtualMatterhornServer::initial_checks(ServerInterface &socket) {

    // TODO: add more checks here, for now just return true to be able to test
    // the should check firmware -client compatibility
    bool initial_checks_passed = true;
    return static_cast<ReturnCode>(socket.sendResult(initial_checks_passed));
}

ReturnCode
VirtualMatterhornServer::get_run_status(ServerInterface &socket) const {

    slsDetectorDefs::runStatus scanstatus{};
    slsDetectorDefs::runStatus status{};

    scanstatus = shm()->scanStatus;
    status = shm()->status;

    // TODO: why only error and running? what about other states?
    if (scanstatus == slsDetectorDefs::runStatus::ERROR ||
        scanstatus == slsDetectorDefs::runStatus::RUNNING) {
        LOG(logINFO) << fmt::format("Scan status: {}\n", ToString(scanstatus));
        return static_cast<ReturnCode>(socket.sendResult(scanstatus));
    }

    LOG(logINFO) << fmt::format("Status: {}\n", ToString(status));
    return static_cast<ReturnCode>(socket.sendResult(status));
}

} // namespace sls