#include "MatterhornServer.h"

VirtualMatterhornServer::VirtualMatterhornServer(uint16_t port)
    : BaseMatterhornServer(port) {

    udpDetails[0].srcip = LOCALHOSTIP_INT;
    udpDetails[0].srcport = DEFAULT_UDP_SRC_PORTNO;
    udpDetails[0].dstport = DEFAULT_UDP_DST_PORTNO;

    // TODO: when do i set the udp mac and ip ?

    BaseMatterhornServer(port);

    tcpInterface = std::make_unique<TCPInterface>(
        function_table, port); // TODO: need a tcp and udp interface

    // should maybe be part of the constructor?
    tcpInterface->startTCPServer();

    // need a function to setup detector - e.g. set all registers etc.
}