#pragma once
#include "sls/ServerSocket.h"
#include "sls/sls_detector_defs.h"
#include "sls/sls_detector_funcs.h"

#include <atomic>
#include <functional>
#include <thread>
#include <unordered_map>

namespace sls {

struct ProcessedResult {
    /// @brief return code of the processed command
    slsDetectorDefs::ReturnCode returnCode{};
    /// @brief error message to be sent to client in case of failure
    std::string error_message{};
};

/**
 * @brief TCPInterface class handles communication and processing of commands
 * from Client to Server.
 */
class TCPInterface {

  public:
    ~TCPInterface();

    TCPInterface(
        std::function<ProcessedResult(const detFuncs &, ServerInterface &)>
            &processFunction_,
        const uint16_t portNumber = DEFAULT_TCP_CNTRL_PORTNO);

    /// @brief creates tcp thread
    void startTCPServer();

    std::atomic<bool> killTcpThread{false};

  private:
    /**
     * @brief starts the TCP/IP server to listen for client commands and process
     * them
     */
    void startTCPServerClientConnection();

    /**
     *  @brief decodes the received command and calls the corresponding function
     *  @param function_id The ID of the function recived by the server and to
     * be executed
     */
    ProcessedResult processReceivedData(const detFuncs function_id,
                                        ServerInterface &socket);

    /// @brief map of function IDs and corresponding functions
    std::function<ProcessedResult(const detFuncs &, ServerInterface &)>
        processFunction;

    /// @brief TCP/IP port number for the detector server
    uint16_t portNumber{};

    /// @brief socket for TCP/IP communication with the client
    ServerSocket server;

    /// @brief thread for running the TCP/IP server
    std::unique_ptr<std::thread> tcpThread;
};

} // namespace sls