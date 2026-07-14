#pragma once
#include "BaseMatterhornServerImpl.hpp"
#include "sls/ToString.h"

namespace sls {

template <bool isStopServer = false>
class VirtualMatterhornServerImpl
    : public BaseMatterhornServerImpl<
          VirtualMatterhornServerImpl<isStopServer>> {

  public:
    VirtualMatterhornServerImpl();
    ~VirtualMatterhornServerImpl() = default;

    slsDetectorDefs::runStatus get_run_status() const;

    void set_module_position_and_update_srcudpmac(
        const std::array<int, 2> &position_info);

    void set_source_udp_mac(const uint64_t newsrcudpMac);
};

template <bool isStopServer>
VirtualMatterhornServerImpl<isStopServer>::VirtualMatterhornServerImpl() {
    this->set_source_udp_ip(LOCALHOSTIP_INT);
}

template <bool isStopServer>
slsDetectorDefs::runStatus
VirtualMatterhornServerImpl<isStopServer>::get_run_status() const {

    slsDetectorDefs::runStatus scanstatus{};
    slsDetectorDefs::runStatus status{};

    scanstatus = this->shm()->scanStatus;
    status = this->shm()->status;

    // TODO: why only error and running? what about other states?
    if (scanstatus == slsDetectorDefs::runStatus::ERROR ||
        scanstatus == slsDetectorDefs::runStatus::RUNNING) {
        LOG(logINFO) << fmt::format("Scan status: {}\n", ToString(scanstatus));
        return scanstatus;
    }

    LOG(logINFO) << fmt::format("Status: {}\n", ToString(status));
    return status;
}

template <bool isStopServer>
void VirtualMatterhornServerImpl<isStopServer>::
    set_module_position_and_update_srcudpmac(
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

template <bool isStopServer>
void VirtualMatterhornServerImpl<isStopServer>::set_source_udp_mac(
    const uint64_t newsrcudpMac) {

    if (!isValidMac(newsrcudpMac)) {
        throw RuntimeError("Invalid source MAC address: unicast bit or local "
                           "administration bit is not set");
    }

    this->updateSrcMacAddress(newsrcudpMac);
}

} // namespace sls