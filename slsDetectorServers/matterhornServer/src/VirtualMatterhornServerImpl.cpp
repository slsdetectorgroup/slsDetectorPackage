#include "VirtualMatterhornServerImpl.hpp"
#include "helpers/Defs.hpp"
#include "helpers/Helpers.hpp"
#include "sls/ToString.h"
#include <fmt/format.h>

namespace sls {

VirtualMatterhornServerImpl::VirtualMatterhornServerImpl() {
    this->set_source_udp_ip(LOCALHOSTIP_INT);
}

slsDetectorDefs::runStatus VirtualMatterhornServerImpl::get_run_status() const {

    slsDetectorDefs::runStatus scanstatus{};
    slsDetectorDefs::runStatus status{};

    scanstatus = shm()->scanStatus;
    status = shm()->status;

    // TODO: why only error and running? what about other states?
    if (scanstatus == slsDetectorDefs::runStatus::ERROR ||
        scanstatus == slsDetectorDefs::runStatus::RUNNING) {
        LOG(logINFO) << fmt::format("Scan status: {}\n", ToString(scanstatus));
        return scanstatus;
    }

    LOG(logINFO) << fmt::format("Status: {}\n", ToString(status));
    return status;
}

void VirtualMatterhornServerImpl::set_module_position_and_update_srcudpmac(
    const std::array<int, 2> &position_info) {

    const size_t module_row = position_info[1] % position_info[0];
    const size_t module_col = position_info[1] / position_info[0];

    try {
        this->set_module_position(module_row, module_col, position_info[1]);
    } catch (const std::exception &e) {
        throw RuntimeError("Failed to set module position: " +
                           std::string(e.what()));
    }

    // configure mac address based on module position
    if (this->udpDetails[0].srcmac ==
        0) { // only configure if source mac address is not set already
        uint64_t newSrcMac =
            generateMacAddressfromModulePosition(module_row, module_col);

        this->updateSrcMacAddress(newSrcMac);
    }
}

void VirtualMatterhornServerImpl::set_source_udp_mac(
    const uint64_t newsrcudpMac) {

    if (!isValidMac(newsrcudpMac)) {
        throw RuntimeError("Invalid source MAC address: unicast bit or local "
                           "administration bit is not set");
    }

    this->updateSrcMacAddress(newsrcudpMac);
}

} // namespace sls