#include "ClientInterface.h"

#include "sls/logger.h"
#include "sls/string_utils.h"

#include <unistd.h>

namespace sls {

ClientInterface::ClientInterface(const uint16_t portNumber)
    : portNumber(portNumber), server(portNumber) {
    validatePortNumber(portNumber);
    // parentThreadId = gettid();
    tcpThread =
        std::make_unique<std::thread>(&ClientInterface::startTCPServer, this);
}

ClientInterface::~ClientInterface() {
    killTcpThread = true;
    LOG(logINFO) << "Shutting down TCP Socket on port " << portNumber;
    server.shutdown();
    LOG(logDEBUG) << "TCP Socket closed on port " << portNumber;

    /*
    if (receiver) {
        receiver->shutDownUDPSockets();
    }
    */

    tcpThread->join();
}

void ClientInterface::startTCPServer() {
    const pid_t tcpThreadId = gettid();
    LOG(logINFOBLUE) << "Created [ TCP server Tid: " << tcpThreadId << "]";
    LOG(logINFO) << "SLS Receiver starting TCP Server on port " << portNumber
                 << '\n';

    int function_id{}; // TODO should it be an enum type
    while (!killTcpThread) {
        LOG(logDEBUG1) << "Start accept loop";
        try {
            auto socket = server.accept();
            try {
                // is this to check if I can process a command? or what is that?
                if (checkifReceiverLocked()) {
                    throw SocketError("Receiver locked\n");
                }
                socket.Receive(function_id);
                processReceivedData(static_cast<detFuncs>(function_id), socket);

            } catch (const RuntimeError &e) {
                // We had an error needs to be sent to client
                char mess[MAX_STR_LENGTH]{};
                strcpy_safe(mess, e.what());
                socket.Send(FAIL);
                socket.Send(mess);
            }
            // TODO handle exiting server if tcp command was to exit server
        } catch (const RuntimeError &e) {
            LOG(logERROR) << "Accept failed";
        }
    }

    LOG(logINFOBLUE) << "Exiting [ TCP server Tid: " << tcpThreadId << "]";
}

ReturnCode ClientInterface::processReceivedData(const detFuncs function_id,
                                                ServerInterface &socket) {
    // TODO: is NUM_DET_FUNCTIONS correct?
    if (function_id < 0 || function_id >= NUM_DET_FUNCTIONS) {
        throw RuntimeError(UNRECOGNIZED_FNUM_ENUM +
                           std::to_string(function_id));
    }
    LOG(logDEBUG1) << "calling function fnum: " << function_id << " ("
                   << getFunctionNameFromEnum((enum detFuncs)function_id)
                   << ")";
    ReturnCode returncode = (functionTable[function_id])(
        socket); // how does it pass input arguments?
    LOG(logDEBUG1) << "Function "
                   << getFunctionNameFromEnum((enum detFuncs)function_id)
                   << " finished";

    return returncode;
}

bool ClientInterface::checkifReceiverLocked() {
    return lockedByClient && server.getThisClient() != server.getLockedBy();
}

} // namespace sls