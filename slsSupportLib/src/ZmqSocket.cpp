#include "ZmqSocket.h"
#include <zmq.h>
#include <vector>
#include <arpa/inet.h> //inet_ntoa
#include <errno.h>
#include <iostream>
#include <netdb.h>              //gethostbyname()
#include <string.h>
#include <unistd.h> //usleep in some machines

using namespace rapidjson;
ZmqSocket::ZmqSocket(const char *const hostname_or_ip,
                     const uint32_t portnumber)
    : portno(portnumber)
// headerMessage(0)
{
    char ip[MAX_STR_LENGTH] = "";
    memset(ip, 0, MAX_STR_LENGTH);

    // convert hostname to ip (not required, but a test that returns if failed)
    struct addrinfo *result;
    if ((ConvertHostnameToInternetAddress(hostname_or_ip, &result)) ||
        (ConvertInternetAddresstoIpString(result, ip, MAX_STR_LENGTH)))
        throw sls::ZmqSocketError("Could convert IP to string");

    // construct address
    sprintf(sockfd.serverAddress, "tcp://%s:%d", ip, portno);
#ifdef VERBOSE
    cprintf(BLUE, "address:%s\n", sockfd.serverAddress);
#endif

    // create context
    sockfd.contextDescriptor = zmq_ctx_new();
    if (sockfd.contextDescriptor == 0)
        throw sls::ZmqSocketError("Could not create contextDescriptor");

    // create publisher
    sockfd.socketDescriptor = zmq_socket(sockfd.contextDescriptor, ZMQ_SUB);
    if (sockfd.socketDescriptor == 0) {
        PrintError();
        Close();
        throw sls::ZmqSocketError("Could not create socket");
    }

    // Socket Options provided above
    // an empty string implies receiving any messages
    if (zmq_setsockopt(sockfd.socketDescriptor, ZMQ_SUBSCRIBE, "", 0)) {
        PrintError();
        Close();
        throw sls::ZmqSocketError("Could set socket opt");
    }
    // ZMQ_LINGER default is already -1 means no messages discarded. use this
    // options if optimizing required ZMQ_SNDHWM default is 0 means no limit.
    // use this to optimize if optimizing required eg. int value = -1;
    int value = 0;
    if (zmq_setsockopt(sockfd.socketDescriptor, ZMQ_LINGER, &value,
                       sizeof(value))) {
        PrintError();
        Close();
        throw sls::ZmqSocketError("Could not set ZMQ_LINGER");
    }
}

ZmqSocket::ZmqSocket(const uint32_t portnumber, const char *ethip)
    :

      portno(portnumber)
// headerMessage(0)
{
    sockfd.server = true;

    // create context
    sockfd.contextDescriptor = zmq_ctx_new();
    if (sockfd.contextDescriptor == 0)
        throw sls::ZmqSocketError("Could not create contextDescriptor");
    // create publisher
    sockfd.socketDescriptor = zmq_socket(sockfd.contextDescriptor, ZMQ_PUB);
    if (sockfd.socketDescriptor == 0) {
        PrintError();
        Close();
        throw sls::ZmqSocketError("Could not create socket");
    }

    // Socket Options provided above

    // construct addresss
    sprintf(sockfd.serverAddress, "tcp://%s:%d", ethip, portno);
#ifdef VERBOSE
    cprintf(BLUE, "address:%s\n", sockfd.serverAddress);
#endif
    // bind address
    if (zmq_bind(sockfd.socketDescriptor, sockfd.serverAddress) < 0) {
        PrintError();
        Close();
        throw sls::ZmqSocketError("Could not bind socket");
    }

    // sleep for a few milliseconds to allow a slow-joiner
    usleep(200 * 1000);
};

int ZmqSocket::Connect() {
    if (zmq_connect(sockfd.socketDescriptor, sockfd.serverAddress) < 0) {
        PrintError();
        return 1;
    }
    return 0;
}

int ZmqSocket::ConvertHostnameToInternetAddress(const char *const hostname,
                                                struct addrinfo **res) {
    // criteria in selecting socket address structures returned by res
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    // get host info into res
    int errcode = getaddrinfo(hostname, NULL, &hints, res);
    if (errcode != 0) {
        LOG(logERROR) << "Error: Could not convert hostname " << hostname
                      << " to internet address (zmq):" << gai_strerror(errcode);
    } else {
        if (*res == NULL) {
            LOG(logERROR) << "Could not convert hostname " << hostname
                          << " to internet address (zmq): "
                             "gettaddrinfo returned null";
        } else {
            return 0;
        }
    }
    LOG(logERROR) << "Could not convert hostname to internet address";
    return 1;
};

int ZmqSocket::ConvertInternetAddresstoIpString(struct addrinfo *res, char *ip,
                                                const int ipsize) {
    if (inet_ntop(res->ai_family,
                  &((struct sockaddr_in *)res->ai_addr)->sin_addr, ip,
                  ipsize) != NULL) {
        freeaddrinfo(res);
        return 0;
    }
    LOG(logERROR) << "Could not convert internet address to ip string";
    return 1;
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
    default:
        LOG(logERROR) << "Unknown socket error (zmq)";
        break;
    }
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
                      << " for socket " << index;
        memset(buf, 0xFF, size);
    }

    zmq_msg_close(&message);
    return length;
}

int ZmqSocket::ParseHeader(const int index, int length, char *buff,
                           Document &document, bool &dummy, uint32_t version) {
    if (document.Parse(buff, length).HasParseError()) {
        LOG(logERROR) << index << " Could not parse. len:" << length
                      << ": Message:" << buff;
        fflush(stdout);
        // char* buf =  (char*) zmq_msg_data (&message);
        for (int i = 0; i < length; ++i) {
            cprintf(RED, "%02x ", buff[i]);
        }
        printf("\n");
        fflush(stdout);
        return 0;
    }

    if (document["jsonversion"].GetUint() != version) {
        LOG(logERROR) << "version mismatch. required " << version << ", got "
                      << document["jsonversion"].GetUint();
        return 0;
    }

    dummy = false;
    int temp = document["data"].GetUint();
    dummy = temp ? false : true;

    return 1;
}

int ZmqSocket::ReceiveHeader(const int index, Document &document,
                             uint32_t version) {
    std::vector<char> buffer(MAX_STR_LENGTH);
    int len =
        zmq_recv(sockfd.socketDescriptor, buffer.data(), buffer.size(), 0);
    if (len > 0) {
        bool dummy = false;
#ifdef ZMQ_DETAIL
        cprintf(BLUE, "Header %d [%d] Length: %d Header:%s \n", index, portno,
                len, buffer.data());
#endif
        if (ParseHeader(index, len, buffer.data(), document, dummy, version)) {
#ifdef ZMQ_DETAIL
            cprintf(RED, "Parsed Header %d [%d] Length: %d Header:%s \n", index,
                    portno, len, buffer.data());
#endif
            if (dummy) {
#ifdef ZMQ_DETAIL
                cprintf(RED, "%d [%d] Received end of acquisition\n", index,
                        portno);
#endif
                return 0;
            }
#ifdef ZMQ_DETAIL
            cprintf(GREEN, "%d [%d] data\n", index, portno);
#endif
            return 1;
        }
    }
    return 0;
};

int ZmqSocket::ReceiveMessage(const int index, zmq_msg_t &message) {
    int length = zmq_msg_recv(&message, sockfd.socketDescriptor, 0);
    if (length == -1) {
        PrintError();
        LOG(logERROR) << "Could not read header for socket " << index;
    }
    return length;
}

int ZmqSocket::SendData(char *buf, int length) {
    if (zmq_send(sockfd.socketDescriptor, buf, length, 0) < 0) {
        PrintError();
        return 0;
    }
    return 1;
}

int ZmqSocket::SendHeaderData(
    int index, bool dummy, uint32_t jsonversion, uint32_t dynamicrange,
    uint64_t fileIndex, uint32_t ndetx, uint32_t ndety, uint32_t npixelsx,
    uint32_t npixelsy, uint32_t imageSize, uint64_t acqIndex, uint64_t fIndex,
    std::string fname, uint64_t frameNumber, uint32_t expLength,
    uint32_t packetNumber, uint64_t bunchId, uint64_t timestamp, uint16_t modId,
    uint16_t row, uint16_t column, uint16_t reserved, uint32_t debug,
    uint16_t roundRNumber, uint8_t detType, uint8_t version,
    int gapPixelsEnable, int flippedDataX, uint32_t quadEnable,
    std::string *additionalJsonHeader) {

    /** Json Header Format */
    const char jsonHeaderFormat[] = "{"
                                    "\"jsonversion\":%u, "
                                    "\"bitmode\":%u, "
                                    "\"fileIndex\":%lu, "
                                    "\"detshape\":[%u, %u], "
                                    "\"shape\":[%u, %u], "
                                    "\"size\":%u, "
                                    "\"acqIndex\":%lu, "
                                    "\"fIndex\":%lu, "
                                    "\"fname\":\"%s\", "
                                    "\"data\": %d, "

                                    "\"frameNumber\":%lu, "
                                    "\"expLength\":%u, "
                                    "\"packetNumber\":%u, "
                                    "\"bunchId\":%lu, "
                                    "\"timestamp\":%lu, "
                                    "\"modId\":%u, "
                                    "\"row\":%u, "
                                    "\"column\":%u, "
                                    "\"reserved\":%u, "
                                    "\"debug\":%u, "
                                    "\"roundRNumber\":%u, "
                                    "\"detType\":%u, "
                                    "\"version\":%u, "

                                    // additional stuff
                                    "\"gappixels\":%u, "
                                    "\"flippedDataX\":%u, "
                                    "\"quad\":%u"

        ; //"}\n";
    char buf[MAX_STR_LENGTH] = "";
    sprintf(buf, jsonHeaderFormat, jsonversion, dynamicrange, fileIndex, ndetx,
            ndety, npixelsx, npixelsy, imageSize, acqIndex, fIndex,
            fname.c_str(), dummy ? 0 : 1,

            frameNumber, expLength, packetNumber, bunchId, timestamp, modId,
            row, column, reserved, debug, roundRNumber, detType, version,

            // additional stuff
            gapPixelsEnable, flippedDataX, quadEnable);

    if (additionalJsonHeader && !((*additionalJsonHeader).empty())) {
        strcat(buf, ", ");
        strcat(buf, (*additionalJsonHeader).c_str());
    }
    strcat(buf, "}\n");
    int length = strlen(buf);

#ifdef VERBOSE
    // if(!index)
    cprintf(BLUE, "%d : Streamer: buf: %s\n", index, buf);
#endif

    if (zmq_send(sockfd.socketDescriptor, buf, length,
                 dummy ? 0 : ZMQ_SNDMORE) < 0) {
        PrintError();
        return 0;
    }
#ifdef VERBOSE
    cprintf(GREEN, "[%u] send header data\n", portno);
#endif
    return 1;
}


//Nested class to do RAII handling of socket descriptors
ZmqSocket::mySocketDescriptors::mySocketDescriptors()
    : server(false), contextDescriptor(0), socketDescriptor(0){};
ZmqSocket::mySocketDescriptors::~mySocketDescriptors() {
    Disconnect();
    Close();
}
void ZmqSocket::mySocketDescriptors::Disconnect() {
    if (server)
        zmq_unbind(socketDescriptor, serverAddress);
    else
        zmq_disconnect(socketDescriptor, serverAddress);
};
void ZmqSocket::mySocketDescriptors::Close() {
    if (socketDescriptor != NULL) {
        zmq_close(socketDescriptor);
        socketDescriptor = NULL;
    }

    if (contextDescriptor != NULL) {
        zmq_ctx_destroy(contextDescriptor);
        contextDescriptor = NULL;
    }
};
