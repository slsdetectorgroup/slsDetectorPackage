// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/ClientSocket.h"
#include "sls/logger.h"
#include "sls/sls_detector_defs.h"
#include "sls/sls_detector_exceptions.h"
#include "sls/sls_detector_funcs.h"
#include "sls/string_utils.h"
#include <arpa/inet.h>
#include <cassert>
#include <cstring>
#include <fmt/format.h>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
namespace sls {

ClientSocket::ClientSocket(std::string stype, const std::string &host,
                           uint16_t port)
    : DataSocket(socket(AF_INET, SOCK_STREAM, 0)), socketType(stype) {

    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;

    if (getaddrinfo(host.c_str(), nullptr, &hints, &result) != 0) {

        auto msg = fmt::format("Cannot resolve {} hostname: '{}'",
                               to_lower(socketType), host);
        throwError(msg);
    }

    // TODO! Erik, results could have multiple entries do we need to loop
    // through them? struct sockaddr_in serverAddr {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    memcpy((char *)&serverAddr.sin_addr.s_addr,
           &((struct sockaddr_in *)result->ai_addr)->sin_addr,
           sizeof(in_addr_t));

    if (::connect(getSocketId(), (struct sockaddr *)&serverAddr,
                  sizeof(serverAddr)) != 0) {
        freeaddrinfo(result);
        auto msg = fmt::format("Cannot connect to {} on {}:{}\n",
                               to_lower(socketType), host, port);

        throwError(msg);
    }
    freeaddrinfo(result);
}

ClientSocket::ClientSocket(std::string sType, struct sockaddr_in addr)
    : DataSocket(socket(AF_INET, SOCK_STREAM, 0)), socketType(sType) {

    if (::connect(getSocketId(), (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        char address[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr, address, INET_ADDRSTRLEN);
        auto msg = fmt::format("Cannot connect to {} on {}:{}",
                               to_lower(socketType), address, addr.sin_port);
        throwError(msg);
    }
}

void ClientSocket::throwError(const std::string &msg) const {
    if (socketType == "Receiver") {
        throw ReceiverError(msg);
    } else if (socketType == "Detector") {
        throw DetectorError(msg);
    } else {
        throw GuiError(msg);
    }
}

void ClientSocket::sendCommandThenRead(int fnum, const void *args,
                                       size_t args_size, void *retval,
                                       size_t retval_size) {
    sendCommand(fnum, args, args_size);
    readReply(retval, retval_size);
}

void ClientSocket::sendCommand(int fnum, const void *args, size_t args_size) {
    Send(fnum);
    setFnum(fnum);
    Send(args, args_size);
}

void ClientSocket::readReply(void *retval, size_t retval_size) {

    try {
        int ret = slsDetectorDefs::FAIL;
        Receive(&ret, sizeof(ret));
        if (ret == slsDetectorDefs::FAIL) {
            std::string mess = readErrorMessage();
            // Do we need to know hostname here?
            // In that case save it???
            throwError(socketType + " returned: " + mess);
        }
        // get retval
        Receive(retval, retval_size);
    }
    // debugging
    catch (SocketError &e) {
        auto msg = fmt::format("While reading reply from {} {}",
                               to_lower(socketType), e.what());
        throwError(msg);
    }
}

std::string ClientSocket::readErrorMessage() {
    std::string error_msg(MAX_STR_LENGTH, '\0');
    Receive(&error_msg[0], error_msg.size());
    if (error_msg.find(UNRECOGNIZED_FNUM_ENUM) != std::string::npos) {
        error_msg.insert(0, "Software version mismatch. ");
    }
    return error_msg;
}

}; // namespace sls
