#pragma once
#include "DetectorServer.h"
#include "TCPInterface.h"
#include "fmt/format.h"
#include "helpers/Helpers.hpp"
#include "sls/logger.h"
#include "sls/network_utils.h"
#include "sls/sls_detector_defs.h"
#include "utils/type_traits.hpp"
#include <array>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace sls {

/// @brief Base class for Matterhorn Server, can be used to implement a virtual
/// server for testing and actual server
template <typename DerivedServer>
class BaseMatterhornServer
    : public DetectorServer<BaseMatterhornServer<DerivedServer>> {

  public:
    /**
     * Constructor
     * Starts up a Matterhorn server.
     * Assembles a Matterhorn server using TCP and UDP detector interfaces
     * throws an exception in case of failure
     * @param port TCP/IP port number
     */
    explicit BaseMatterhornServer(std::unique_ptr<DetectorServerImpl> impl,
                                  uint16_t port = DEFAULT_TCP_CNTRL_PORTNO)
        : DetectorServer<BaseMatterhornServer<DerivedServer>>(std::move(impl),
                                                              port) {}

    ~BaseMatterhornServer() = default;

    ProcessedResult set_counter_mask(ServerInterface &socket);

    ProcessedResult get_counter_mask(ServerInterface &socket) const;

    /**
     * @brief call function corresponding to the function ID received from the
     * client and send back the result
     * @param function_id the function ID received from the client
     * @param socket the socket to send the result back to the client
     */
    ProcessedResult processFunction(const detFuncs function_id,
                                    ServerInterface &socket);

    using ImplType = typename implementation_typetrait<DerivedServer>::ImplType;

  protected:
    auto *const getImpl() const { return this->getDerivedImpl(); }

  private:
    const DerivedServer *getDerived() const {
        return static_cast<const DerivedServer *>(this);
    }
};

template <typename DerivedServer>
ProcessedResult
BaseMatterhornServer<DerivedServer>::processFunction(const detFuncs function_id,
                                                     ServerInterface &socket) {

    switch (function_id) {
    case detFuncs::F_SET_COUNTER_MASK:
        return set_counter_mask(socket);
    case detFuncs::F_GET_COUNTER_MASK:
        return get_counter_mask(socket);
    default:
        throw RuntimeError(
            fmt::format("Function {} not implemented",
                        getFunctionNameFromEnum((enum detFuncs)function_id)));
    }
}

template <typename DerivedServer>
ProcessedResult
BaseMatterhornServer<DerivedServer>::set_counter_mask(ServerInterface &socket) {

    uint32_t counter_mask{};
    try {
        (void)socket.Receive(counter_mask);
    } catch (const SocketError &e) {
        LOG(logERROR) << "Failed to receive counter mask: " << e.what();
        return_fail("Failed to receive counter mask: " + std::string(e.what()));
    }

    try {
        getImpl()->set_counter_mask(counter_mask);
    } catch (const std::exception &e) {
        return_fail("Failed to set counter mask: " + std::string(e.what()));
    }

    return send_ok(socket);
}

template <typename DerivedServer>
ProcessedResult BaseMatterhornServer<DerivedServer>::get_counter_mask(
    ServerInterface &socket) const {

    uint32_t counter_mask{};

    try {
        counter_mask = getImpl()->get_counter_mask();
    } catch (const std::exception &e) {
        return_fail("Failed to get counter mask: " + std::string(e.what()));
    }

    return ProcessedResult{
        static_cast<ReturnCode>(socket.sendResult(counter_mask))};
}

} // namespace sls