#include "VirtualMatterhornServerImpl.hpp"
#include "helpers/Defs.hpp"
#include "helpers/Helpers.hpp"
#include "sls/ToString.h"
#include <fmt/format.h>

namespace sls {

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
            generate_mac_address_from_module_position(module_row, module_col);

        this->updateSrcMacAddress(newSrcMac);
    }
}

void VirtualMatterhornServerImpl::set_source_udp_mac(
    const uint64_t newsrcudpMac) {

    if ((newsrcudpMac << INDIVIDUAL_GROUP_BIT_OFFSET) == 0 &&
        (newsrcudpMac << UNIVERSAL_LOCAL_BIT_OFFSET) == 1) {
        throw RuntimeError("Invalid source MAC address: unicast bit or local "
                           "administration bit is not set");
    }

    this->updateSrcMacAddress(newsrcudpMac);
}

} // namespace sls