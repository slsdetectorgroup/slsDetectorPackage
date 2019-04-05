#include "ClientSocket.h"
#include "logger.h"
#include "sls_detector_defs.h"
#include "sls_detector_exceptions.h"
#include <arpa/inet.h>
#include <cassert>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
namespace sls {

ClientSocket::ClientSocket(const bool isRx, const std::string &host, uint16_t port)
    : DataSocket(socket(AF_INET, SOCK_STREAM, 0)), isReceiver(isRx) {

    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;

    if (getaddrinfo(host.c_str(), NULL, &hints, &result) != 0) {
        std::string msg =
            "ClientSocket cannot decode host:" + host + " on port " + std::to_string(port) + "\n";
        throw SocketError(msg);
    }

    // TODO! Erik, results could have multiple entries do we need to loop through them?
    // struct sockaddr_in serverAddr {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    memcpy((char *)&serverAddr.sin_addr.s_addr, &((struct sockaddr_in *)result->ai_addr)->sin_addr,
           sizeof(in_addr_t));

    if (::connect(getSocketId(), (struct sockaddr *)&serverAddr, sizeof(serverAddr)) != 0) {
        freeaddrinfo(result);
        const std::string name{(isReceiver ? "Receiver" : "Detector")};
        std::string msg = "ClientSocket: Cannot connect to " + name + ":" + host + " on port " +
                          std::to_string(port) + "\n";
        throw SocketError(msg);
    }
    freeaddrinfo(result);
}

ClientSocket::ClientSocket(const bool isRx, struct sockaddr_in addr)
    : DataSocket(socket(AF_INET, SOCK_STREAM, 0)), isReceiver(isRx) {

    if (::connect(getSocketId(), (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        char address[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr, address, INET_ADDRSTRLEN);
        const std::string name{(isReceiver ? "Receiver" : "Detector")};
        std::string msg = "ClientSocket: Cannot connect to " + name + ":" + address + " on port " +
                          std::to_string(addr.sin_port) + "\n";
        throw SocketError(msg);
    }
}

int ClientSocket::sendCommandThenRead(int fnum, void *args, size_t args_size, void *retval,
                                      size_t retval_size) {
    int ret = slsDetectorDefs::FAIL;
    sendData(&fnum, sizeof(fnum));
    sendData(args, args_size);
    readReply(ret, retval, retval_size);
    return ret;
}

void ClientSocket::readReply(int &ret, void *retval, size_t retval_size) {

    receiveData(&ret, sizeof(ret));
    bool unrecognizedFunction = false;
    if (ret == slsDetectorDefs::FAIL) {
        char mess[MAX_STR_LENGTH]{};
        // get error message
        receiveData(mess, sizeof(mess));
        // cprintf(RED, "%s %d returned error: %s", type.c_str(), index, mess);
        cprintf(RED, "%s returned error: %s", (isReceiver ? "Receiver" : "Detector"), mess);
        std::cout << "\n"; // needed to reset the color.

        // unrecognized function, do not ask for retval
        if (strstr(mess, "Unrecognized Function") != nullptr)
            unrecognizedFunction = true;

        // Do we need to know hostname here?
        // In that case save it???
        if (isReceiver) {
            throw ReceiverError(mess);
        } else {
            throw DetectorError(mess);
        }
    }
    // get retval
    if (!unrecognizedFunction)
        receiveData(retval, retval_size);
}

}; // namespace sls
