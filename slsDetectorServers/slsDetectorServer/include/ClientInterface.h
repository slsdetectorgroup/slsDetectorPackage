#pragma once
#include "sls/ServerSocket.h"
#include "sls/sls_detector_defs.h"
#include "sls/sls_detector_funcs.h"

#include <atomic>
#include <future>
#include <unordered_map>

namespace sls {

/**
 * @brief ClientInterface class handles communication and processing of commands
 * from Client to Server.
 */
class ClientInterface : private virtual slsDetectorDefs {

  protected:
    // TODO probably requires std::variant
    /// @brief map of function IDs and corresponding functions
    std::unordered_map<detFuncs, std::function<ReturnCode(ServerInterface &)>>
        functionTable{}; // set in constructor of child process

  private:
    /// @brief listener thread for TCP/IP communication with the client
    std::unique_ptr<std::thread> tcpThread;

    /// @brief flag to signal the TCP/IP listener thread to stop
    std::atomic<bool> killTcpThread{false};

    /// @brief flag to indicate if the receiver is currently locked by a client
    bool lockedByClient{false}; // TODO should it be atomic?

    uint16_t portNumber{};

    /// @brief socket for TCP/IP communication with the client
    ServerSocket server;

  public:
    ~ClientInterface();

    explicit ClientInterface(
        const uint16_t portNumber = DEFAULT_TCP_CNTRL_PORTNO);

    // std::string getReceiverVersion();

  private:
    /// @brief starts the TCP/IP server to listen for client commands
    void startTCPServer();

    /**
     *  @brief decodes the received command and calls the corresponding function
     *  @param function_id The ID of the function recived by the server and to
     * be executed
     */
    ReturnCode processReceivedData(const detFuncs function_id,
                                   ServerInterface &socket);

    bool checkifReceiverLocked();
};

} // namespace sls