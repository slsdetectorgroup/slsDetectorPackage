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

    std::string sip(ip);
    // construct address
    sprintf(sockfd.serverAddress, "tcp://%s:%d", sip.c_str(), portno);
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

int ZmqSocket::SendHeader(
    int index, zmqHeader header) {

    /** Json Header Format */
    const char jsonHeaderFormat[] = "{"
                                    "\"jsonversion\":%u, "
                                    "\"bitmode\":%u, "
                                    "\"fileIndex\":%lu, "
                                    "\"detshape\":[%u, %u], "
                                    "\"shape\":[%u, %u], "
                                    "\"size\":%u, "
                                    "\"acqIndex\":%lu, "
                                    "\"frameIndex\":%lu, "
                                    "\"fname\":\"%s\", "
                                    "\"data\": %d, "
                                    "\"completeImage\": %d, "

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
                                    "\"flippedDataX\":%u, "
                                    "\"quad\":%u"

        ; //"}\n";
    char buf[MAX_STR_LENGTH] = "";
    sprintf(buf, jsonHeaderFormat, 
            header.jsonversion, 
            header.dynamicRange, 
            header.fileIndex, 
            header.ndetx,
            header.ndety, 
            header.npixelsx, 
            header.npixelsy, 
            header.imageSize, 
            header.acqIndex, 
            header.frameIndex,
            header.fname.c_str(), 
            header.data ? 1 : 0, 
            header.completeImage ? 1 : 0,

            header.frameNumber, 
            header.expLength, 
            header.packetNumber, 
            header.bunchId, 
            header.timestamp, 
            header.modId,
            header.row, 
            header.column, 
            header.reserved, 
            header.debug, 
            header.roundRNumber, 
            header.detType, 
            header.version,

            // additional stuff
            header.flippedDataX, 
            header.quad);

    if (header.addJsonHeader.size() > 0) {
        strcat(buf, ", ");
        strcat(buf, "\"addJsonHeader\": {");
        for (size_t i = 0; i < header.addJsonHeader.size(); ++i) {
            strcat(buf, "\"");
            strcat(buf, header.addJsonHeader[i][0].c_str());
            strcat(buf, "\":\"");
            strcat(buf, header.addJsonHeader[i][1].c_str());
            strcat(buf, "\"");
            if (i < header.addJsonHeader.size() -1) {
                strcat(buf, ", ");
            }
        }
        strcat(buf, " } ");
    }

    strcat(buf, "}\n");
    int length = strlen(buf);

//#ifdef VERBOSE
    // if(!index)
    cprintf(BLUE, "%d : Streamer: buf: %s\n", index, buf);
//#endif

    if (zmq_send(sockfd.socketDescriptor, buf, length,
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

int ZmqSocket::ReceiveHeader(const int index, zmqHeader& zHeader,
                             uint32_t version) {
    Document document;
    std::vector<char> buffer(MAX_STR_LENGTH);
    int len =
        zmq_recv(sockfd.socketDescriptor, buffer.data(), buffer.size(), 0);
    if (len > 0) {
#ifdef ZMQ_DETAIL
        cprintf(BLUE, "Header %d [%d] Length: %d Header:%s \n", index, portno,
                len, buffer.data());
#endif
        if (ParseHeader(index, len, buffer.data(), zHeader, document, version)) {
#ifdef ZMQ_DETAIL
            cprintf(RED, "Parsed Header %d [%d] Length: %d Header:%s \n", index,
                    portno, len, buffer.data());
#endif
            if (!zHeader.data) {
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

int ZmqSocket::ParseHeader(const int index, int length, char *buff,
                           zmqHeader& zHeader, Document &document,
                           uint32_t version) {
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
    zHeader.fname = document["fname"].GetString();

    zHeader.frameNumber = document["frameNumber"].GetUint64();
    zHeader.expLength = document["expLength"].GetUint();
    zHeader.packetNumber = document["packetNumber"].GetUint();
    zHeader.bunchId = document["bunchId"].GetUint64();
    zHeader.timestamp = document["timestamp"].GetUint64();
    zHeader.modId = document["modId"].GetUint();
    zHeader.row = document["row"].GetUint();
    zHeader.column = document["column"].GetUint();
    zHeader.reserved = document["reserved"].GetUint();
    zHeader.debug = document["debug"].GetUint();
    zHeader.roundRNumber = document["roundRNumber"].GetUint();
    zHeader.detType = document["detType"].GetUint();
    zHeader.version = document["version"].GetUint();

    zHeader.flippedDataX = document["flippedDataX"].GetUint();
    zHeader.quad = document["quad"].GetUint();
    zHeader.completeImage = document["completeImage"].GetUint();

    if (document.HasMember("addJsonHeader")) {
        const Value& V = document["addJsonHeader"];
        zHeader.addJsonHeader.clear();
        for (Value::ConstMemberIterator iter = V.MemberBegin(); iter != V.MemberEnd(); ++iter){
            zHeader.addJsonHeader.resize(zHeader.addJsonHeader.size() + 1);  
            int i  = iter - V.MemberBegin();
            zHeader.addJsonHeader[i].resize(2);
            zHeader.addJsonHeader[i][0] = iter->name.GetString();
            zHeader.addJsonHeader[i][1] = iter->value.GetString();
        }
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
                      << " for socket " << index;
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
    default:
        LOG(logERROR) << "Unknown socket error (zmq)";
        break;
    }
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
