#pragma once
#include "sls/ServerSocket.h"
#include "sls/sls_detector_defs.h"
#include "sls/sls_detector_funcs.h"

#include <atomic>
#include <future>
#include <unordered_map>

namespace sls {

/**
 * @brief TCPInterface class handles communication and processing of commands
 * from Client to Server.
 */
class TCPInterface {

  public:
    ~TCPInterface();

    TCPInterface(std::unordered_map<
                     detFuncs, std::function<ReturnCode(ServerInterface &)>>
                     &functionTable_,
                 const uint16_t portNumber = DEFAULT_TCP_CNTRL_PORTNO);

    /// @brief starts the TCP/IP server to listen for client commands
    void startTCPServer();

  private:
    /**
     *  @brief decodes the received command and calls the corresponding function
     *  @param function_id The ID of the function recived by the server and to
     * be executed
     */
    ReturnCode processReceivedData(const detFuncs function_id,
                                   ServerInterface &socket);

    /// @brief map of function IDs and corresponding functions
    std::unordered_map<detFuncs, std::function<ReturnCode(ServerInterface &)>>
        functionTable{};

    uint16_t portNumber{};

    /// @brief socket for TCP/IP communication with the client
    ServerSocket server;
};

} // namespace sls