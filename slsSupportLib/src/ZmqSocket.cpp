// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "sls/ZmqSocket.h"
#include "sls/logger.h"
#include "sls/network_utils.h" //ip
#include <chrono>
#include <errno.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include <thread>
#include <vector>
#include <zmq.h>
namespace sls {

using namespace rapidjson;
ZmqSocket::ZmqSocket(const char *const hostname_or_ip,
                     const uint16_t portnumber)
    : portno(portnumber), sockfd(false) {
    // Extra check that throws if conversion fails, could be removed
    auto ipstr = HostnameToIp(hostname_or_ip).str();
    std::ostringstream oss;
    oss << "tcp://" << ipstr << ":" << portno;
    sockfd.serverAddress = oss.str();
    LOG(logDEBUG) << "zmq address: " << sockfd.serverAddress;

    // create context
    sockfd.contextDescriptor = zmq_ctx_new();
    if (sockfd.contextDescriptor == nullptr)
        throw ZmqSocketError("Could not create contextDescriptor");

    // create subscriber
    sockfd.socketDescriptor = zmq_socket(sockfd.contextDescriptor, ZMQ_SUB);
    if (sockfd.socketDescriptor == nullptr) {
        PrintError();
        throw ZmqSocketError("Could not create socket");
    }

    // Socket Options provided above
    // an empty string implies receiving any messages
    if (zmq_setsockopt(sockfd.socketDescriptor, ZMQ_SUBSCRIBE, "", 0)) {
        PrintError();
        throw ZmqSocketError("Could set socket opt");
    }
    // ZMQ_LINGER default is already -1 means no messages discarded. use this
    // options if optimizing required ZMQ_SNDHWM default is 0 means no limit.
    // use this to optimize if optimizing required eg. int value = -1;
    const int value = 0;
    if (zmq_setsockopt(sockfd.socketDescriptor, ZMQ_LINGER, &value,
                       sizeof(value))) {
        PrintError();
        throw ZmqSocketError("Could not set ZMQ_LINGER");
    }
    LOG(logDEBUG) << "Default receive high water mark:"
                  << GetReceiveHighWaterMark();

    // enable IPv6 addresses
    int ipv6 = 1;
    if (zmq_setsockopt(sockfd.socketDescriptor, ZMQ_IPV6, &ipv6,
                       sizeof(ipv6))) {
        PrintError();
        throw ZmqSocketError("Could not set ZMQ_IPV6");
    }
}

ZmqSocket::ZmqSocket(const uint16_t portnumber)
    : portno(portnumber), sockfd(true) {

    // create context
    sockfd.contextDescriptor = zmq_ctx_new();
    if (sockfd.contextDescriptor == nullptr)
        throw ZmqSocketError("Could not create contextDescriptor");

    // create publisher
    sockfd.socketDescriptor = zmq_socket(sockfd.contextDescriptor, ZMQ_PUB);
    if (sockfd.socketDescriptor == nullptr) {
        PrintError();
        throw ZmqSocketError("Could not create socket");
    }
    LOG(logDEBUG) << "Default send high water mark:" << GetSendHighWaterMark();

    // construct address, can be refactored with libfmt
    std::ostringstream oss;
    oss << "tcp://" << ZMQ_PUBLISHER_IP << ":" << portno;
    sockfd.serverAddress = oss.str();
    LOG(logDEBUG) << "zmq address: " << sockfd.serverAddress;

    // enable IPv6 addresses
    int ipv6 = 1;
    if (zmq_setsockopt(sockfd.socketDescriptor, ZMQ_IPV6, &ipv6,
                       sizeof(ipv6))) {
        PrintError();
        throw ZmqSocketError("Could not set ZMQ_IPV6");
    }

    // Socket Options for keepalive
    // enable TCP keepalive
    int keepalive = 1;
    if (zmq_setsockopt(sockfd.socketDescriptor, ZMQ_TCP_KEEPALIVE, &keepalive,
                       sizeof(keepalive))) {
        PrintError();
        throw ZmqSocketError("Could set socket opt ZMQ_TCP_KEEPALIVE");
    }
    // set the number of keepalives before death
    keepalive = 10;
    if (zmq_setsockopt(sockfd.socketDescriptor, ZMQ_TCP_KEEPALIVE_CNT,
                       &keepalive, sizeof(keepalive))) {
        PrintError();
        throw ZmqSocketError("Could set socket opt ZMQ_TCP_KEEPALIVE_CNT");
    }
    // set the time before the first keepalive
    keepalive = 60;
    if (zmq_setsockopt(sockfd.socketDescriptor, ZMQ_TCP_KEEPALIVE_IDLE,
                       &keepalive, sizeof(keepalive))) {
        PrintError();
        throw ZmqSocketError("Could set socket opt ZMQ_TCP_KEEPALIVE_IDLE");
    }
    // set the interval between keepalives
    keepalive = 1;
    if (zmq_setsockopt(sockfd.socketDescriptor, ZMQ_TCP_KEEPALIVE_INTVL,
                       &keepalive, sizeof(keepalive))) {
        PrintError();
        throw ZmqSocketError("Could set socket opt ZMQ_TCP_KEEPALIVE_INTVL");
    }

    // bind address
    if (zmq_bind(sockfd.socketDescriptor, sockfd.serverAddress.c_str())) {
        PrintError();
        throw ZmqSocketError("Could not bind socket");
    }
    // sleep to allow a slow-joiner
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

int ZmqSocket::GetSendHighWaterMark() {
    int value = 0;
    size_t value_size = sizeof(value);
    if (zmq_getsockopt(sockfd.socketDescriptor, ZMQ_SNDHWM, &value,
                       &value_size)) {
        PrintError();
        throw ZmqSocketError("Could not get ZMQ_SNDHWM");
    }
    return value;
}

void ZmqSocket::SetSendHighWaterMark(int limit) {
    if (zmq_setsockopt(sockfd.socketDescriptor, ZMQ_SNDHWM, &limit,
                       sizeof(limit))) {
        PrintError();
        throw ZmqSocketError("Could not set ZMQ_SNDHWM");
    }
    if (GetSendHighWaterMark() != limit) {
        throw ZmqSocketError("Could not set ZMQ_SNDHWM to " +
                             std::to_string(limit));
    }

    int bufsize = DEFAULT_ZMQ_BUFFERSIZE;
    if (limit < DEFFAULT_LOW_ZMQ_HWM) {
        bufsize = DEFAULT_LOW_ZMQ_HWM_BUFFERSIZE;
    }
    SetSendBuffer(bufsize);
}

int ZmqSocket::GetReceiveHighWaterMark() {
    int value = 0;
    size_t value_size = sizeof(value);
    if (zmq_getsockopt(sockfd.socketDescriptor, ZMQ_RCVHWM, &value,
                       &value_size)) {
        PrintError();
        throw ZmqSocketError("Could not get ZMQ_RCVHWM");
    }
    return value;
}

void ZmqSocket::SetReceiveHighWaterMark(int limit) {
    if (zmq_setsockopt(sockfd.socketDescriptor, ZMQ_RCVHWM, &limit,
                       sizeof(limit))) {
        PrintError();
        throw ZmqSocketError("Could not set ZMQ_RCVHWM");
    }
    if (GetReceiveHighWaterMark() != limit) {
        throw ZmqSocketError("Could not set ZMQ_RCVHWM to " +
                             std::to_string(limit));
    }
    int bufsize = DEFAULT_ZMQ_BUFFERSIZE;
    if (limit < DEFFAULT_LOW_ZMQ_HWM) {
        bufsize = DEFAULT_LOW_ZMQ_HWM_BUFFERSIZE;
    }
    SetReceiveBuffer(bufsize);
}

int ZmqSocket::GetSendBuffer() {
    int value = 0;
    size_t value_size = sizeof(value);
    if (zmq_getsockopt(sockfd.socketDescriptor, ZMQ_SNDBUF, &value,
                       &value_size)) {
        PrintError();
        throw ZmqSocketError("Could not get ZMQ_SNDBUF");
    }
    return value;
}

void ZmqSocket::SetSendBuffer(int limit) {
    if (zmq_setsockopt(sockfd.socketDescriptor, ZMQ_SNDBUF, &limit,
                       sizeof(limit))) {
        PrintError();
        throw ZmqSocketError("Could not set ZMQ_SNDBUF");
    }
    if (GetSendBuffer() != limit) {
        throw ZmqSocketError("Could not set ZMQ_SNDBUF to " +
                             std::to_string(limit));
    }
}

int ZmqSocket::GetReceiveBuffer() {
    int value = 0;
    size_t value_size = sizeof(value);
    if (zmq_getsockopt(sockfd.socketDescriptor, ZMQ_RCVBUF, &value,
                       &value_size)) {
        PrintError();
        throw ZmqSocketError("Could not get ZMQ_RCVBUF");
    }
    return value;
}

void ZmqSocket::SetReceiveBuffer(int limit) {
    if (zmq_setsockopt(sockfd.socketDescriptor, ZMQ_RCVBUF, &limit,
                       sizeof(limit))) {
        PrintError();
        throw ZmqSocketError("Could not set ZMQ_RCVBUF");
    }
    if (GetReceiveBuffer() != limit) {
        throw ZmqSocketError("Could not set ZMQ_RCVBUF to " +
                             std::to_string(limit));
    }
}

void ZmqSocket::Rebind() {
    // the purpose is to apply HWL changes, which are
    // frozen at bind, which is in the constructor.

    //    unbbind
    if (zmq_unbind(sockfd.socketDescriptor, sockfd.serverAddress.c_str())) {
        PrintError();
        throw ZmqSocketError("Could not unbind socket");
    }
    // bind address
    if (zmq_bind(sockfd.socketDescriptor, sockfd.serverAddress.c_str())) {
        PrintError();
        throw ZmqSocketError("Could not bind socket");
    }
}

int ZmqSocket::Connect() {
    if (zmq_connect(sockfd.socketDescriptor, sockfd.serverAddress.c_str())) {
        PrintError();
        return 1;
    }
    return 0;
}

int ZmqSocket::SendHeader(int index, zmqHeader header) {
    std::ostringstream oss;
    oss << "{\"jsonversion\":" << header.jsonversion
        << ", \"bitmode\":" << header.dynamicRange
        << ", \"fileIndex\":" << header.fileIndex << ", \"detshape\":["
        << header.ndetx << ", " << header.ndety << ']' << ", \"shape\":["
        << header.npixelsx << ", " << header.npixelsy << ']'
        << ", \"size\":" << header.imageSize
        << ", \"acqIndex\":" << header.acqIndex
        << ", \"frameIndex\":" << header.frameIndex
        << ", \"progress\":" << header.progress << ", \"fname\":\""
        << header.fname << '\"' << ", \"data\":" << (header.data ? 1 : 0)
        << ", \"completeImage\":" << (header.completeImage ? 1 : 0)

        << ", \"frameNumber\":" << header.frameNumber
        << ", \"expLength\":" << header.expLength
        << ", \"packetNumber\":" << header.packetNumber
        << ", \"detSpec1\":" << header.detSpec1
        << ", \"timestamp\":" << header.timestamp
        << ", \"modId\":" << header.modId << ", \"row\":" << header.row
        << ", \"column\":" << header.column
        << ", \"detSpec2\":" << header.detSpec2
        << ", \"detSpec3\":" << header.detSpec3
        << ", \"detSpec4\":" << header.detSpec4
        << ", \"detType\":" << static_cast<int>(header.detType)
        << ", \"version\":"
        << static_cast<int>(header.version)

        // additional stuff
        << ", \"flipRows\":" << header.flipRows << ", \"quad\":" << header.quad;

    if (!header.addJsonHeader.empty()) {
        oss << ", \"addJsonHeader\": {";
        for (auto it = header.addJsonHeader.begin();
             it != header.addJsonHeader.end(); ++it) {
            if (it != header.addJsonHeader.begin()) {
                oss << ", ";
            }
            oss << "\"" << it->first.c_str() << "\":\"" << it->second.c_str()
                << "\"";
        }
        oss << " } ";
    }
    oss << ", \"rx_roi\":[" << header.rx_roi[0] << ", " << header.rx_roi[1]
        << ", " << header.rx_roi[2] << ", " << header.rx_roi[3] << "]";
    oss << "}\n";
    std::string message = oss.str();
    int length = message.length();
#ifdef ZMQ_DETAIL
    // if(!index)
    LOG(logINFOBLUE) << index << " : Streamer: buf: " << message;
#endif

    if (zmq_send(sockfd.socketDescriptor, message.c_str(), length,
                 header.data ? ZMQ_SNDMORE : 0) < 0) {
        PrintError();
        return 0;
    }
#ifdef VERBOSE
    cprintf(GREEN, "[%u] send header data\n", portno);
#endif
    return 1;
}

int ZmqSocket::SendData(char *buf, int length) {
    if (zmq_send(sockfd.socketDescriptor, buf, length, 0) < 0) {
        PrintError();
        return 0;
    }
    return 1;
}

int ZmqSocket::ReceiveHeader(const int index, zmqHeader &zHeader,
                             uint32_t version) {
    const int bytes_received = zmq_recv(sockfd.socketDescriptor,
                                        header_buffer.get(), MAX_STR_LENGTH, 0);
    if (bytes_received > 0) {
#ifdef ZMQ_DETAIL
        cprintf(BLUE, "Header %d [%hu] Length: %d Header:%s \n", index, portno,
                bytes_received, header_buffer.get());
#endif
        if (ParseHeader(index, bytes_received, header_buffer.get(), zHeader,
                        version)) {
#ifdef ZMQ_DETAIL
            cprintf(RED, "Parsed Header %d [%hu] Length: %d Header:%s \n",
                    index, portno, bytes_received, header_buffer.get());
#endif
            if (!zHeader.data) {
#ifdef ZMQ_DETAIL
                cprintf(RED, "%d [%hu] Received end of acquisition\n", index,
                        portno);
#endif
                return 0;
            }
#ifdef ZMQ_DETAIL
            cprintf(GREEN, "%d [%hu] data\n", index, portno);
#endif
            return 1;
        }
    }
    return 0;
};

int ZmqSocket::ParseHeader(const int index, int length, char *buff,
                           zmqHeader &zHeader, uint32_t version) {
    Document document;
    if (document.Parse(buff, length).HasParseError()) {
        LOG(logERROR) << index << " Could not parse. len:" << length
                      << ": Message:" << buff;
        for (int i = 0; i < length; ++i) {
            cprintf(RED, "%02x ", buff[i]);
        }
        std::cout << std::endl;
        return 0;
    }

    // version check
    zHeader.jsonversion = document["jsonversion"].GetUint();
    if (zHeader.jsonversion != version) {
        LOG(logERROR) << "version mismatch. required " << version << ", got "
                      << zHeader.jsonversion;
        return 0;
    }

    // parse
    zHeader.data = ((document["data"].GetUint()) == 0) ? false : true;
    zHeader.dynamicRange = document["bitmode"].GetUint();
    zHeader.fileIndex = document["fileIndex"].GetUint64();
    zHeader.ndetx = document["detshape"][0].GetUint();
    zHeader.ndety = document["detshape"][1].GetUint();
    zHeader.npixelsx = document["shape"][0].GetUint();
    zHeader.npixelsy = document["shape"][1].GetUint();
    zHeader.imageSize = document["size"].GetUint();
    zHeader.acqIndex = document["acqIndex"].GetUint64();
    zHeader.frameIndex = document["frameIndex"].GetUint64();
    zHeader.progress = document["progress"].GetDouble();
    zHeader.fname = document["fname"].GetString();

    zHeader.frameNumber = document["frameNumber"].GetUint64();
    zHeader.expLength = document["expLength"].GetUint();
    zHeader.packetNumber = document["packetNumber"].GetUint();
    zHeader.detSpec1 = document["detSpec1"].GetUint64();
    zHeader.timestamp = document["timestamp"].GetUint64();
    zHeader.modId = document["modId"].GetUint();
    zHeader.row = document["row"].GetUint();
    zHeader.column = document["column"].GetUint();
    zHeader.detSpec2 = document["detSpec2"].GetUint();
    zHeader.detSpec3 = document["detSpec3"].GetUint();
    zHeader.detSpec4 = document["detSpec4"].GetUint();
    zHeader.detType = document["detType"].GetUint();
    zHeader.version = document["version"].GetUint();

    zHeader.flipRows = document["flipRows"].GetUint();
    zHeader.quad = document["quad"].GetUint();
    zHeader.completeImage = document["completeImage"].GetUint();

    if (document.HasMember("addJsonHeader")) {
        const Value &V = document["addJsonHeader"];
        zHeader.addJsonHeader.clear();
        for (Value::ConstMemberIterator iter = V.MemberBegin();
             iter != V.MemberEnd(); ++iter) {
            zHeader.addJsonHeader[iter->name.GetString()] =
                iter->value.GetString();
        }
    }

    const Value &a = document["rx_roi"].GetArray();
    for (SizeType i = 0; i != a.Size(); ++i) {
        zHeader.rx_roi[i] = a[i].GetInt();
    }

    return 1;
}

int ZmqSocket::ReceiveData(const int index, char *buf, const int size) {
    zmq_msg_t message;
    zmq_msg_init(&message);
    int length = ReceiveMessage(index, message);
    if (length == size) {
        memcpy(buf, (char *)zmq_msg_data(&message), size);
    } else if (length < size) {
        memcpy(buf, (char *)zmq_msg_data(&message), length);
        memset(buf + length, 0xFF, size - length);
    } else {
        LOG(logERROR) << "Received weird packet size " << length
                      << " (expected " << size << ") for socket " << index;
        memset(buf, 0xFF, size);
    }

    zmq_msg_close(&message);
    return length;
}

int ZmqSocket::ReceiveMessage(const int index, zmq_msg_t &message) {
    int length = zmq_msg_recv(&message, sockfd.socketDescriptor, 0);
    if (length == -1) {
        PrintError();
        LOG(logERROR) << "Could not read header for socket " << index;
    }
    return length;
}

void ZmqSocket::PrintError() {
    switch (errno) {
    case EINVAL:
        LOG(logERROR) << "The socket type/option or value/endpoint supplied is "
                         "invalid (zmq)";
        break;
    case EAGAIN:
        LOG(logERROR) << "Non-blocking mode was requested and the message "
                         "cannot be sent/available at the moment (zmq)";
        break;
    case ENOTSUP:
        LOG(logERROR) << "The zmq_send()/zmq_msg_recv() operation is not "
                         "supported by this socket type (zmq)";
        break;
    case EFSM:
        LOG(logERROR) << "The zmq_send()/zmq_msg_recv() unavailable now as "
                         "socket in inappropriate state (eg. ZMQ_REP). Look up "
                         "messaging patterns (zmq)";
        break;
    case EFAULT:
        LOG(logERROR) << "The provided context/message is invalid (zmq)";
        break;
    case EMFILE:
        LOG(logERROR) << "The limit on the total number of open ØMQ sockets "
                         "has been reached (zmq)";
        break;
    case EPROTONOSUPPORT:
        LOG(logERROR)
            << "The requested transport protocol is not supported (zmq)";
        break;
    case ENOCOMPATPROTO:
        LOG(logERROR) << "The requested transport protocol is not compatible "
                         "with the socket type (zmq)";
        break;
    case EADDRINUSE:
        LOG(logERROR) << "The requested address is already in use (zmq)";
        break;
    case EADDRNOTAVAIL:
        LOG(logERROR) << "The requested address was not local (zmq)";
        break;
    case ENODEV:
        LOG(logERROR)
            << "The requested address specifies a nonexistent interface (zmq)";
        break;
    case ETERM:
        LOG(logERROR) << "The ØMQ context associated with the specified socket "
                         "was terminated (zmq)";
        break;
    case ENOTSOCK:
        LOG(logERROR) << "The provided socket was invalid (zmq)";
        break;
    case EINTR:
        LOG(logERROR)
            << "The operation was interrupted by delivery of a signal (zmq)";
        break;
    case EMTHREAD:
        LOG(logERROR)
            << "No I/O thread is available to accomplish the task (zmq)";
        break;
    case ENOENT:
        LOG(logERROR) << "The requested endpoint does not exist (zmq)";
        break;
    default:
        LOG(logERROR) << "Unknown socket error (zmq). Error code: " << errno;
        break;
    }
}

// Nested class to do RAII handling of socket descriptors
ZmqSocket::mySocketDescriptors::mySocketDescriptors(bool server)
    : server(server), contextDescriptor(nullptr), socketDescriptor(nullptr){};
ZmqSocket::mySocketDescriptors::~mySocketDescriptors() {
    Disconnect();
    Close();
}
void ZmqSocket::mySocketDescriptors::Disconnect() {
    if (server)
        zmq_unbind(socketDescriptor, serverAddress.c_str());
    else
        zmq_disconnect(socketDescriptor, serverAddress.c_str());
};
void ZmqSocket::mySocketDescriptors::Close() {
    if (socketDescriptor != nullptr) {
        zmq_close(socketDescriptor);
        socketDescriptor = nullptr;
    }

    if (contextDescriptor != nullptr) {
        zmq_ctx_destroy(contextDescriptor);
        contextDescriptor = nullptr;
    }
};

} // namespace sls
