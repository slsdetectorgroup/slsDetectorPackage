#include "MatterhornServerImpl.hpp"

namespace sls {

slsDetectorDefs::runStatus MatterhornServerImpl::get_run_status() const {

    return slsDetectorDefs::runStatus::IDLE; // TODO: implement
}

void MatterhornServerImpl::set_module_position_and_update_srcudpmac(
    const std::array<int, 2> &position_info) {

    // position_info = [num_modules_in_y, module_index]

    const size_t module_row = position_info[1] % position_info[0];
    const size_t module_col = position_info[1] / position_info[0];

    try {
        this->set_module_position(module_row, module_col, position_info[1]);
    } catch (const std::exception &e) {
        throw RuntimeError("Failed to set module position: " +
                           std::string(e.what()));
    }

    // TODO: update
    if (udpDetails[0].srcmac ==
        0) { // only configure if source mac address is not set already
        uint64_t newSrcMac =
            0x000000000000; // TODO: vendor address will be on SOM memory/
                            // different for 10G/100G
        updateSrcMacAddress(newSrcMac);
    }
}

void MatterhornServerImpl::set_source_udp_mac(const uint64_t src_mac) {

    throw RuntimeError(
        "Cannot overwrite vendor specific source UDP MAC address.");
}

} // namespace sls