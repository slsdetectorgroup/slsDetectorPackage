#include "ClientInterface.h"

#include "sls/logger.h"
#include "sls/string_utils.h"

#include <unistd.h>

namespace sls {

ClientInterface::ClientInterface(const uint16_t portNumber)
    : portNumber(portNumber), server(portNumber) {
    validatePortNumber(portNumber);
    // parentThreadId = gettid();
}

ClientInterface::~ClientInterface() {
    killTcpThread = true;
    LOG(logINFORED) << "Shutting down TCP Socket on port " << portNumber;
    server.shutdown();
    LOG(logDEBUG) << "TCP Socket closed on port " << portNumber;

    /*
    if (receiver) {
        receiver->shutDownUDPSockets();
    }
    */
}

void ClientInterface::startTCPServer() {

    LOG(logINFO) << "SLS Server starting TCP Server on port " << portNumber
                 << '\n';

    int function_id{}; // TODO should it be an enum type
    while (true) {
        LOG(logDEBUG1) << "Start accept loop";
        try {
            auto socket = server.accept();
            try {
                // is this to check if I can process a command? or what is that?
                /*
                if (checkifReceiverLocked()) {
                    throw SocketError("Receiver locked\n");
                }
                */
                socket.Receive(function_id);
                if (function_id < 0 || function_id >= NUM_DET_FUNCTIONS) {
                    throw RuntimeError(UNRECOGNIZED_FNUM_ENUM +
                                       std::to_string(function_id));
                }
                auto returncode = processReceivedData(
                    static_cast<detFuncs>(function_id), socket);

                if (returncode == FAIL) {
                    LOG(logERROR) << "Error processing command with fnum: "
                                  << function_id;
                }

            } catch (const RuntimeError &e) {
                // We had an error needs to be sent to client
                char mess[MAX_STR_LENGTH]{};
                LOG(logERROR) << "Error processing command: " << e.what();
                strcpy_safe(mess, e.what());
                socket.Send(slsDetectorDefs::FAIL);
                socket.Send(mess);
            }
            // TODO handle exiting server if tcp command was to exit server
        } catch (const RuntimeError &e) {
            LOG(logERROR) << "Accept failed: " << e.what();
        }
    }

    LOG(logINFOBLUE) << "Exiting TCP Server";
}

ReturnCode ClientInterface::processReceivedData(const detFuncs function_id,
                                                ServerInterface &socket) {
    // TODO: is NUM_DET_FUNCTIONS correct?

    LOG(logDEBUG1) << "calling function fnum: " << function_id << " ("
                   << getFunctionNameFromEnum((enum detFuncs)function_id) << ")"
                   << "from thread: " << gettid();

    LOG(logDEBUG1) << "Available functions in the server:";
    std::for_each(functionTable.begin(), functionTable.end(),
                  [](const auto &pair) {
                      LOG(logDEBUG1)
                          << "Function id: " << pair.first
                          << ", Function name: "
                          << getFunctionNameFromEnum((enum detFuncs)pair.first);
                  });

    auto function = functionTable.find(function_id);
    if (function == functionTable.end()) {
        throw RuntimeError("unrecognized Function id: " +
                           std::to_string(function_id));
    }

    ReturnCode returncode =
        function->second(socket); // how does it pass input arguments?
    LOG(logDEBUG1) << "Function "
                   << getFunctionNameFromEnum((enum detFuncs)function_id)
                   << " finished";

    return returncode;
}

bool ClientInterface::checkifReceiverLocked() {
    return lockedByClient && server.getThisClient() != server.getLockedBy();
}

} // namespace sls