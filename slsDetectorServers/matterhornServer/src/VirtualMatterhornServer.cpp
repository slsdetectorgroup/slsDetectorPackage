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

ProcessedResult
VirtualMatterhornServer::initial_checks(ServerInterface &socket) {

    // TODO: add more checks here, for now just return true to be able to test
    // the should check firmware -client compatibility
    bool initial_checks_passed = true;
    return ProcessedResult{
        static_cast<ReturnCode>(socket.sendResult(initial_checks_passed))};
}

ProcessedResult
VirtualMatterhornServer::get_run_status(ServerInterface &socket) const {

    slsDetectorDefs::runStatus scanstatus{};
    slsDetectorDefs::runStatus status{};

    scanstatus = shm()->scanStatus;
    status = shm()->status;

    // TODO: why only error and running? what about other states?
    if (scanstatus == slsDetectorDefs::runStatus::ERROR ||
        scanstatus == slsDetectorDefs::runStatus::RUNNING) {
        LOG(logINFO) << fmt::format("Scan status: {}\n", ToString(scanstatus));
        return ProcessedResult{
            static_cast<ReturnCode>(socket.sendResult(scanstatus))};
    }

    LOG(logINFO) << fmt::format("Status: {}\n", ToString(status));
    return ProcessedResult{static_cast<ReturnCode>(socket.sendResult(status))};
}

ProcessedResult
VirtualMatterhornServer::set_module_position_and_update_srcudpmac(
    ServerInterface &socket) {

    std::array<int, 2> position_info{}; // [num_modules_in_y, module_index]
    try {
        int ret = socket.Receive(position_info.data(),
                                 position_info.size() * sizeof(int));
    } catch (const SocketError &e) {
        LOG(logERROR)
            << "Failed to receive num modules in y dimension and module index: "
            << e.what();
        return ProcessedResult{
            ReturnCode::FAIL,
            "Failed to receive num modules in y dimension and module index : " +
                std::string(e.what())};
    }

    const size_t module_row = position_info[1] % position_info[0];
    const size_t module_col = position_info[1] / position_info[0];

    try {
        this->set_module_position(module_row, module_col, position_info[1]);
    } catch (const std::exception &e) {
        LOG(logERROR) << "Failed to set module position: " << e.what();
        return ProcessedResult{ReturnCode::FAIL,
                               "Failed to set module position: " +
                                   std::string(e.what())};
    }

    // configure mac address based on module position
    if (this->udpDetails[0].srcmac ==
        0) { // only configure if source mac address is not set already
        uint64_t newSrcMac = generaterandomMacAddress();
        newSrcMac =
            (newSrcMac & 0xffffffffffff0000) | (module_row << 16) | module_col;

        this->updateSrcMacAddress(newSrcMac);
    }

    return ProcessedResult{
        static_cast<ReturnCode>(socket.Send(ReturnCode::OK))};
}

ProcessedResult
VirtualMatterhornServer::set_source_udp_mac(ServerInterface &socket) {
    uint64_t newsrcudpMac;

    try {
        int ret = socket.Receive<uint64_t>(newsrcudpMac);
    } catch (const SocketError &e) {
        LOG(logERROR) << "Failed to receive new source UDP MAC address: "
                      << e.what();
        return ProcessedResult{
            ReturnCode::FAIL, "Failed to receive new source UDP MAC address: " +
                                  std::string(e.what())};
    }

    if ((newsrcudpMac & 0x020000000000) == 0) {
        LOG(logERROR) << "Invalid source MAC address: unicast bit or local "
                         "administration bit is not set";
        return ProcessedResult{
            ReturnCode::FAIL,
            "Invalid source MAC address: unicast bit or local "
            "administration bit is not set"};
    }

    this->updateSrcMacAddress(newsrcudpMac);

    return ProcessedResult{
        static_cast<ReturnCode>(socket.Send(ReturnCode::OK))};
}

} // namespace sls