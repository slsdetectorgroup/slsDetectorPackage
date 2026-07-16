#pragma once
#include "BaseMatterhornServerImpl.hpp"

namespace sls {

template <bool isStopServer = false>
class MatterhornServerImpl
    : public BaseMatterhornServerImpl<MatterhornServerImpl<isStopServer>> {

  public:
    MatterhornServerImpl() = default;
    ~MatterhornServerImpl() = default;

    slsDetectorDefs::runStatus get_run_status() const; // TODO: impement

    void set_module_position_and_update_srcudpmac(
        const std::array<int, 2> &position_info);

    void set_source_udp_mac([[maybe_unused]] const uint64_t src_mac);
};

template <bool isStopServer>
slsDetectorDefs::runStatus
MatterhornServerImpl<isStopServer>::get_run_status() const {

    // TODO: will also have a scanStatus - scanStatus should be in base
    // implementation and shared between virtual and actual detector - split
    // this function into two.
    return slsDetectorDefs::runStatus::IDLE; // TODO: implement
}

template <bool isStopServer>
void MatterhornServerImpl<isStopServer>::
    set_module_position_and_update_srcudpmac(
        const std::array<int, 2> &position_info) {

    // position_info = [num_modules_in_y, module_index]

    const size_t module_row = position_info[1] % position_info[0];
    if (position_info[0] <= 0) {
        throw RuntimeError("Number of modules in y direction cannot be 0.");
    }
    const size_t module_col = position_info[1] / position_info[0];

    try {
        this->set_module_position(module_row, module_col, position_info[1]);
    } catch (const std::exception &e) {
        throw RuntimeError("Failed to set module position: " +
                           std::string(e.what()));
    }

    // TODO: update
    if (this->udpDetails[0].srcmac ==
        0) { // only configure if source mac address is not set already
        uint64_t newSrcMac =
            0x000000000000; // TODO: vendor address will be on SOM memory/
                            // different for 10G/100G
        this->updateSrcMacAddress(newSrcMac);
    }
}

template <bool isStopServer>
void MatterhornServerImpl<isStopServer>::set_source_udp_mac(
    const uint64_t src_mac) {

    throw RuntimeError(
        "Cannot overwrite vendor specific source UDP MAC address.");
}

} // end namespace sls