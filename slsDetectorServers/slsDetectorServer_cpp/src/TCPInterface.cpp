#include "TCPInterface.h"

#include "fmt/format.h"
#include "sls/logger.h"
#include "sls/string_utils.h"
#include <unistd.h>

namespace sls {

TCPInterface::TCPInterface(
    std::function<ProcessedResult(const detFuncs &, ServerInterface &)>
        &processFunction_,
    const uint16_t portNumber_)
    : processFunction(processFunction_), portNumber(portNumber_),
      server(portNumber_) {
    // validatePortNumber(portNumber); TODO: where to validate?
}

TCPInterface::~TCPInterface() {
    killTcpThread = true; // mmh but if the receiver is stuck in a function,
                          // this will be of no help
    LOG(logINFO) << "Shutting down TCP Socket on port " << portNumber;
    server.shutdown();
    LOG(logDEBUG) << "TCP Socket closed on port " << portNumber;
    if (tcpThread && tcpThread->joinable()) {
        tcpThread->join();
    }
}

void TCPInterface::startTCPServer() {

    tcpThread = std::make_unique<std::thread>(
        &TCPInterface::startTCPServerClientConnection, this);
}

void TCPInterface::startTCPServerClientConnection() {

    LOG(logINFO) << "SLS Server starting TCP Server on port " << portNumber
                 << '\n';

    int function_id{}; // TODO should it be an enum type
    while (!killTcpThread) {
        try {
            auto socket = server.accept();
            try {

                socket.Receive(function_id);
                if (function_id < 0 || function_id >= NUM_DET_FUNCTIONS) {
                    throw RuntimeError(fmt::format(
                        "{}:{}", UNRECOGNIZED_FNUM_ENUM,
                        getFunctionNameFromEnum((enum detFuncs)function_id)));
                }
                auto processedResult = processReceivedData(
                    static_cast<detFuncs>(function_id), socket);

                // TODO: should technically fail before
                if (processedResult.returnCode ==
                    slsDetectorDefs::ReturnCode::FAIL) {
                    throw RuntimeError(fmt::format(
                        "Error while processing command with fnum: {}, Error: "
                        "{}",
                        getFunctionNameFromEnum((enum detFuncs)function_id),
                        processedResult.error_message));
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

ProcessedResult TCPInterface::processReceivedData(const detFuncs function_id,
                                                  ServerInterface &socket) {

    LOG(logDEBUG1) << "calling function fnum: " << function_id << " ("
                   << getFunctionNameFromEnum((enum detFuncs)function_id)
                   << ")";

    ProcessedResult processedResult = processFunction(function_id, socket);

    LOG(logDEBUG1) << "Function "
                   << getFunctionNameFromEnum((enum detFuncs)function_id)
                   << " finished";

    return processedResult;
}

} // namespace sls