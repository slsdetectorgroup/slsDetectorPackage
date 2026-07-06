#pragma once
#include "BaseMatterhornServerImpl.hpp"

namespace sls {

class VirtualMatterhornServerImpl
    : public BaseMatterhornServerImpl<VirtualMatterhornServerImpl> {

  public:
    VirtualMatterhornServerImpl() = default;
    ~VirtualMatterhornServerImpl() = default;

    slsDetectorDefs::runStatus get_run_status() const;

    /// @brief return true if initial checks pass, false otherwise
    bool initial_checks() const;

    void set_module_position_and_update_srcudpmac(
        const std::array<int, 2> &position_info);

    void set_source_udp_mac(const uint64_t newsrcudpMac);
};

} // namespace sls