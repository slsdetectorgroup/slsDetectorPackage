#include "MatterhornServer.h"

namespace sls {

MatterhornServer::MatterhornServer(uint16_t port)
    : BaseMatterhornServer<MatterhornServer>(port) {
    // map the IP core base addresses to memory
    busCommunication.mapToMemory(); // TODO: should this happen in constructor?

    spiCommunication.mapToMemory(); // TODO: should this happen in constructor?

    // should maybe be part of the constructor?
    tcpInterface->startTCPServer();

    // TODO: no init_server function for now is it neccessary to set the init
    // flag
    this->setupDetector();
}

ReturnCode MatterhornServer::initial_checks(ServerInterface &socket) {

    // TODO: add more checks here, for now just return true to be able to test
    // the should check firmware -client compatibility
    bool initial_checks_passed = true;
    return static_cast<ReturnCode>(socket.sendResult(initial_checks_passed));
}

ReturnCode MatterhornServer::set_module_position_and_update_srcudpmac(
    ServerInterface &socket) {

    std::array<int, 2> position_info{}; // [num_modules_in_y, module_index]
    try {
        int ret = socket.Receive(position_info.data(),
                                 position_info.size() * sizeof(int));
    } catch (const SocketError &e) {
        LOG(logERROR)
            << "Failed to receive num modules in y dimension and module index: "
            << e.what();
        return ReturnCode::FAIL;
    }

    const size_t module_row = position_info[1] % position_info[0];
    const size_t module_col = position_info[1] / position_info[0];

    try {
        this->set_module_position(module_row, module_col, position_info[1]);
    } catch (const std::exception &e) {
        LOG(logERROR) << "Failed to set module position: " << e.what();
        return ReturnCode::FAIL;
    }

    // configure mac address based on module position

    // TODO: update
    if (this->udpDetails[0].srcmac ==
        0) { // only configure if source mac address is not set already
        this->udpDetails[0].srcmac = generaterandomMacAddress();
        uint64_t newSrcMac = (this->udpDetails[0].srcmac & 0xffffffffffff0000) |
                             (module_row << 16) | module_col;
        try {
            this->updateSrcMacAddress(newSrcMac);
        } catch (const std::invalid_argument &e) {
            LOG(logERROR) << "Failed to update source MAC address: "
                          << e.what();
            return ReturnCode::FAIL;
        }
    }

    return static_cast<ReturnCode>(socket.Send(ReturnCode::OK));
}

ReturnCode MatterhornServer::set_source_udp_mac(ServerInterface &socket) {

    LOG(logERROR) << "Cannot overwrite vendor specific source UDP MAC address.";

    return static_cast<ReturnCode>(socket.Send(ReturnCode::FAIL));
}

} // namespace sls