#pragma once
#include "BaseMatterhornServerImpl.hpp"

namespace sls {

class MatterhornServerImpl
    : public BaseMatterhornServerImpl<MatterhornServerImpl> {

  public:
    MatterhornServerImpl() = default;
    ~MatterhornServerImpl() = default;

    slsDetectorDefs::runStatus get_run_status() const; // TODO: impement

    /// @brief return true if initial checks pass, false otherwise
    bool
    initial_checks() const; // TODO: Is it the same as for virtual server?
                            // If yes, can be moved to base class implementation

    void set_module_position_and_update_srcudpmac(
        const std::array<int, 2> &position_info);

    void
    set_source_udp_mac([[maybe_unused]] const uint64_t
                           src_mac); // TODO: doesnt it need to receive src_mac?
};

} // end namespace sls